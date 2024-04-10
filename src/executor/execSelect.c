/*
 *	toadb execSelect 
 *
 * Copyright (c) 2023-2024 senllang
 * 
 * toadb is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/

#include "execSelect.h"
#include "public.h"
#include "server_pub.h"
#include "planNode.h"
#include "queryNode.h"
#include "execNode.h"
#include "execExpre.h"
#include "tuples.h"

#include <string.h>

static int FormNewRowsPos(PExecState eState, PList targetList, PList rangTbl);
static PExprDataInfo ProcessSetValueExpr(PExecState eState, PNode setValueExpr);

/*
 * 对扫描结果进行条件表达式选择过滤
 * 条件表达式也是以树形式记录，需要递归处理 
 */
PTableRowData ExecSelect(PExecState eState)
{
    PSelectResult psr = (PSelectResult)eState->subPlanNode;
    PSelectState pst = (PSelectState)eState->subPlanStateNode;
    PList qual = psr->qual;
    PTableRowData rowData = NULL;
    PListCell tmpCell = NULL;
    PNode qualNode = NULL;
    PExprDataInfo resultExpr = 0;
    int bResult = HAT_FALSE;

    for(; ;)
    {
        /* 获取一条记录 */
        eState->subPlanNode = (PNode)psr->subplan;
        eState->subPlanStateNode = (PNode)pst->subplanState;
        rowData = ExecNodeProc(eState);
        if(NULL == rowData)
        {
            /* 如果为空，说明已经扫描结束，退出 */
            break;
        }

        /* 没有选持条件,返回当前行 */
        if(NULL == qual)
            break;

        /* 记录结果，用于表达式计算 */
        pst->resultRow = (PNode)rowData;
        eState->subPlanNode = (PNode)psr;
        eState->subPlanStateNode = (PNode)pst;

        /* 查询结果记录在pst中 */
        bResult = HAT_FALSE;
        for(tmpCell = qual->head; tmpCell != NULL; tmpCell = tmpCell->next)
        {
            PNode qualNode = GetCellNodeValue(tmpCell);
            resultExpr = ExecSelectQual(qualNode, eState);

            /* 
             * 当有多组表达式时，各表达式之间是or的关系，
             * 只要有一个为真时，最终结果为真；
             * 一般只有一组表达式。
            */
            if(getDataBool(resultExpr))
            {
                bResult = HAT_TRUE;
                break;
            }
        }

        /* 
         * 表达式为真，返回当前扫描结果; 
         * 表达式为假，获取下一条数据
         */
        if(HAT_TRUE == bResult)
            break;
        
        if(NULL != rowData)
        {
            ReleaseRowData((PScanTableRowData)rowData);
            rowData = NULL;
        }
    } 
    
    return rowData;
}

/*  
 * returning type is PExprDataInfo really. 
 */
PTableRowData ExecUpdateSelect(PExecState eState)
{
    PSelectResult psr = (PSelectResult)eState->subPlanNode;
    // PSelectState pst = (PSelectState)eState->subPlanStateNode;

    /* find set expression result, and replace rowdata column values. */
    if(FormNewRowsPos(eState, psr->targetList, psr->rtable) < 0)
        return NULL;

    /* new rowData return, type is PTableRowDataWithPos */
    return eState->scanRowDataLeft;
}


/* 
 * travers set value target list
 * execupdateSelectQual for setvalue qual.
 * replace attributes values with qual result.
 */
static int FormNewRowsPos(PExecState eState, PList targetList, PList rangTbl)
{
    foreachWithSize_define_Head;
    PRangTblEntry rte = NULL;
    PAttrDataPosition attrDataPos = NULL;
    PExprDataInfo newColExprValue = NULL;

    /* rowdata is reslut of pre project logical. */
    PScanTableRowData scanRowData = (PScanTableRowData)eState->scanRowDataLeft;
    // PTableRowData targetRowData = (PTableRowData)eState->scanRowDataRight;
    
    PRowColumnData newRowColData = NULL;
    PTableRowDataWithPos resultRowData = NULL;
    PTableRowData newColData = NULL;

    AttrDataPosition tempDataPos = {0};
    PTableRowDataPosition tblRowPosition = NULL;
    PColumnRef colDef = NULL;
    int rowIndex = 0;

    PResTarget restarget = NULL;
    PTargetEntry targetEntry = NULL;
    PColumnRef colNode = NULL;
    int colrowIndex = 0;
    int rowSize = 0;

    if((NULL == targetList) || (NULL == scanRowData) || (NULL == rangTbl))
    {
        return -1;
    }

    resultRowData = (PTableRowDataWithPos)AllocMem(sizeof(TableRowDataWithPos) + sizeof(PAttrDataPosition) * targetList->length);
    resultRowData->size = sizeof(TableRowDataWithPos) + sizeof(PAttrDataPosition) * targetList->length;
    resultRowData->num = 0;

    /* traverse target list */
    foreachWithSize(targetList, tmpCell, listLen)
    {
        targetEntry = (PTargetEntry)GetCellNodeValue(tmpCell);
        restarget = (PResTarget)targetEntry->colRef;
        if(NULL == restarget->setValue)
            continue;
    
        colDef = (PColumnRef)restarget->val;
        colDef->attrIndex = targetEntry->attrIndex;

        /* 根据target中列对应的表index，找到表的信息记录 */
        rte = (PRangTblEntry)GetCellValueByIndex(rangTbl, targetEntry->rindex);
        if(NULL == rte)
        {
            hat_error("Rang table not founded.\n");
            break;
        }

        newColExprValue = ProcessSetValueExpr(eState, restarget->setValue);
        if(NULL == newColExprValue)
        {
            /* error ocur. */
            break;
        }

        /* 根据表元数据定义，找到对应的表的查询行 */
        tblRowPosition = GetTblRowDataPosition(scanRowData, rte->tblInfo, rte->rindex);
        if(NULL == tblRowPosition)
        {
            hat_error("rowdata position not founded.\n");
            break;
        }

        /* 根据target中列的定义，找到对应列的位置信息 */
        newColData = GetColRowData(tblRowPosition, colDef, &tempDataPos, 0);
        if(NULL == newColData)
        {
            hat_error("column %d rowdata not founded.\n", resultRowData->num);
            break;
        } 

        /* replace value in the rowdata. */
        newRowColData = transFormExpr2RowColumnData(newColExprValue, targetEntry->attrIndex);

        /* only one column */
        rowSize = sizeof(AttrDataPosition) + sizeof(PRowColumnData) ;
        attrDataPos = (PAttrDataPosition)AllocMem(rowSize);

        attrDataPos->headItem = tempDataPos.headItem;
        attrDataPos->rowData.columnData[0] = newRowColData;
        attrDataPos->rowData.size = newRowColData->size;
        attrDataPos->rowData.num = 1;

        resultRowData->attrDataPos[resultRowData->num] = attrDataPos;
        resultRowData->num += 1;
    }

    if(resultRowData->num != targetList->length)
    {
        hat_error("FormNewRowsPos column %d rowdata, and target request %d column, not equality.\n", colrowIndex, targetList->length);
        return -1;
    }

    eState->scanRowDataLeft = (PTableRowData)resultRowData;

    return 0;
}

/*
 * update command , set column = expr;
 * we compute express value.
 */
static PExprDataInfo ProcessSetValueExpr(PExecState eState, PNode setValueExpr)
{
    PSelectState pst = (PSelectState)eState->subPlanStateNode;
    PTableRowData scanRowData = eState->scanRowDataLeft;
    PExprDataInfo exprResult = NULL;

    /* setvalue express type */
    switch(setValueExpr->type)
    {
        case T_ConstValue:
            /* replace old value with new value */
            exprResult = TransformConstExprValue((PConstValue)setValueExpr);
        break;

        default:
        {
            pst->resultRow = (PNode)scanRowData;
            exprResult = ExecSelectQual(setValueExpr, eState);
        }
        break;
    }

    return exprResult;
}

static PTableRowData UpdateRowColumnData()
{
    return NULL;
}