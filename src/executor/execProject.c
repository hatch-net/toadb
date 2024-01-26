/*
 *	toadb project node proc
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

#include "execProject.h"
#include "scan.h"
#include "planNode.h"
#include "execNode.h"
#include "queryNode.h"
#include "public.h"

#include <stdio.h>



static PTableRowData FetchTargetColumns(PScanTableRowData scanTblRowInfo, PList targetList, PList rangTbl);


PTableRowData ExecTableProject(PExecState eState)
{
    PProjectTbl plan = NULL;
    PProjectTblState planState = NULL;    
    PTableRowData rowData = NULL;

    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */
    plan = (PProjectTbl)eState->subPlanNode;
    planState = (PProjectTblState)eState->subPlanStateNode;

    eState->subPlanNode = (PNode)plan->subplan;
    eState->subPlanStateNode = (PNode)planState->subplanState;
    rowData = ExecNodeProc(eState);
    if(NULL == rowData)
    {
        /* erorr ocurr, query ending. */
        return NULL;
    }

    /* transform column data matching targetlist. */
    rowData = FetchTargetColumns((PScanTableRowData)rowData, plan->targetList, plan->rtable);
    return rowData;
}

/*
 * There will be query all rows that we want.
 * all rows send to portal when found one.
 */
PTableRowData ExecTableQuery(PExecState eState)
{
    PQueryTbl plan = NULL;
    PQueryTblState planState = NULL;
    
    PTableRowData rowData = NULL;
    int rowNum = 0;

    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */
    plan = (PQueryTbl)eState->subPlanNode;
    planState = (PQueryTblState)eState->subPlanStateNode;

    /* process left and right node */
    if(NULL == plan->subplan)
    {
        eState->retCode = ExecRetCode_ERR;
        return NULL;
    }

    /* query all rows until return NULL. */
    for(; ;)
    {
        eState->subPlanNode = (PNode)plan->subplan;
        eState->subPlanStateNode = (PNode)planState->subplanState;
        rowData = ExecNodeProc(eState);
        if(NULL == rowData)
        {
            /* query ending. */
            break;
        }

        /* send to portal */
        SendToPort(&(planState->stateNode), rowData);

        rowNum++;
    }
    
    /* client show */
    FinishSend(planState->stateNode.portal); 
    eState->retCode = rowNum;

    return NULL;
}

/* 
 * fetch row data, which column matching targetlist. 
 * 两个表的查询结果行，要根据target中的列信息，将行数据投影成一个新的结果行。
 */
static PTableRowData FetchTargetColumns(PScanTableRowData scanTblRowInfo, PList targetList, PList rangTbl)
{
    PTableRowData resultRowData = NULL;
    PTableRowData *rawcolrow = NULL;

    PTableList tblInfo = NULL;
    PTableRowDataPosition tblRowPosition = NULL;
    int colrowIndex = 0;

    PListCell tmpCell = NULL;
    PRangTblEntry rte = NULL;

    if((NULL == targetList) || (NULL == scanTblRowInfo) || (NULL == rangTbl))
    {
        return NULL;
    }

    rawcolrow = (PTableRowData *)AllocMem(sizeof(PTableRowData) * targetList->length);

    for(tmpCell = targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PTargetEntry targetEntry = (PTargetEntry)GetCellNodeValue(tmpCell);
        PResTarget restarget = (PResTarget)targetEntry->colRef;
        PColumnRef colDef = (PColumnRef)restarget->val;

        /* 根据target中列对应的表index，找到表的信息记录 */
        rte = (PRangTblEntry)GetCellValueByIndex(rangTbl, targetEntry->rindex);
        if(NULL == rte)
        {
            hat_error("Rang table not founded.\n");
            break;
        }

        /* 根据表元数据定义，找到对应的表的查询行 */
        tblRowPosition = GetTblRowDataPosition(scanTblRowInfo, rte->tblInfo);
        if(NULL == tblRowPosition)
        {
            hat_error("rowdata position not founded.\n");
            break;
        }

        /* 根据target中列的定义，找到对应列的信息进行投影，得到该表列的投影字段数组 */
        rawcolrow[colrowIndex] = GetColRowData(tblRowPosition, colDef);
        if(NULL == rawcolrow[colrowIndex])
        {
            hat_error("column %d rowdata not founded.\n", colrowIndex);
            break;
        }

        colrowIndex++;
    }

    if(colrowIndex == targetList->length)
    {
        resultRowData = FormCol2RowData(rawcolrow, colrowIndex);
    }
    else
    {
        hat_error("column %d rowdata, and target request %d column, not equality.\n", colrowIndex, targetList->length);
    }

    return resultRowData;
}
