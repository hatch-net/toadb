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
#include "execExpre.h"
#include "memStack.h"

#include <string.h>


static PTableRowData FetchTargetColumns(PScanTableRowData scanTblRowInfo, PList targetList, PList rangTbl, PProjectStateData pjData);
// static PTableRowDataWithPos FetchTargetColumnsPos(PScanTableRowData scanTblRowInfo, PList targetList, PList rangTbl);

PProjectStateData InitProjectData(int columnNum)
{
    PProjectStateData pjData = NULL;

    char *pMem = NULL;
    int rawColsize = 0;
    int rowDataSize = 0;

    rawColsize = sizeof(PRowColumnData) * columnNum;
    rowDataSize = rawColsize + sizeof(RowData);

    pMem = AllocMem(sizeof(ProjectStateData) + rawColsize + rowDataSize);
    pjData = (PProjectStateData)pMem;

    /* memory of column rows is start from sizeof(ProjectStateData). */
    rowDataSize = sizeof(ProjectStateData);
    pjData->rawcolrow = (PRowColumnData *)(pMem + rowDataSize);

    /* memory of column rows is start from the position of two pre structure. */
    rowDataSize += rawColsize;
    pjData->rowData = (PRowData)(pMem + rowDataSize);

    return pjData;
}

/* 
 * project logical 
*/
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

    /* 
     * transform column data matching qualtargetlist.
     * record scanrowdata to estate->scanRowDataLeft.
     * for update
     */
    eState->scanRowDataLeft = rowData;

    /* transform column data matching targetlist. */
    rowData = FetchTargetColumns((PScanTableRowData)rowData, plan->targetList, plan->rtable, planState->prjData);
    return rowData;
}

#if 0
/* 
 * project logical 
 * tuple position will be saved for update command;
 * 
*/
PTableRowData ExecTableUpdateProject(PExecState eState)
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

    /* this rowData column matching parenttarget list*/
    rowData = (PTableRowData)FetchTargetColumnsPos((PScanTableRowData)rowData, plan->targetList, plan->rtable);

    /* 将设置的新字段值替换到行数据中，生成新的行数据 */
    eState->subPlanNode = (PNode)plan;
    eState->subPlanStateNode = (PNode)planState;
    planState->resultRow = (PNode)rowData;
    
    return rowData;
}

#endif 

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
    int ret = 0;

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

        /* rowData type is PRowData at fact. */
        rowData = ExecNodeProc(eState);
        if(NULL == rowData)
        {
            /* query ending. */
            break;
        }

        /* send to portal */
        ret = SendToPort(&(planState->stateNode), &(((PRowData)rowData)->rowsData));
        if(ret < 0)
        {
            hat_error("client is disconnected, interrupt excutor.");
            return NULL;
        }

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
static PTableRowData FetchTargetColumns(PScanTableRowData scanTblRowInfo, PList targetList, PList rangTbl, PProjectStateData pjData)
{
    PRowColumnData *rawcol = NULL;

    PTableList tblInfo = NULL;
    PTableRowDataPosition tblRowPosition = NULL;
    int colrowIndex = 0;
    int found = 0;

    PListCell tmpCell = NULL;
    PRangTblEntry rte = NULL;

    if((NULL == targetList) || (NULL == scanTblRowInfo) || (NULL == rangTbl) || (NULL == pjData))
    {
        return NULL;
    }

    rawcol = pjData->rawcolrow;

    for(tmpCell = targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PTargetEntry targetEntry = (PTargetEntry)GetCellNodeValue(tmpCell);
        PResTarget restarget = (PResTarget)targetEntry->colRef;
        PColumnRef colDef = (PColumnRef)restarget->val;
        colDef->attrIndex = targetEntry->attrIndex;
        
        /* 根据target中列对应的表index，找到表的信息记录 */
        rte = (PRangTblEntry)GetCellValueByIndex(rangTbl, targetEntry->rindex);
        if(NULL == rte)
        {
            hat_error("Rang table not founded.");
            break;
        }

        /* 根据表元数据定义，找到对应的表的查询行 */
        tblRowPosition = GetTblRowDataPosition(scanTblRowInfo, rte->tblInfo, rte->rindex);
        if(NULL == tblRowPosition)
        {
            hat_error("rowdata position not founded.");
            break;
        }

        /* 根据target中列的定义，找到对应列的信息进行投影，得到该表列的投影字段数组 */
        // rawcol[colrowIndex] = GetColRowDataEx(tblRowPosition, colDef, NULL, 0, &found, HAT_TRUE);
        rawcol[colrowIndex] = GetColRowDataEx(tblRowPosition, colDef, NULL, 0, &found, HAT_FALSE);
        if(NULL == rawcol[colrowIndex])
        {
            hat_error("column %d rowdata not founded.", colrowIndex);
            break;
        }

        colrowIndex++;
    }

    if(colrowIndex == targetList->length)
    {
        FormCol2RowDataEx(rawcol, colrowIndex, pjData->rowData);
    }
    else
    {
        hat_error("column %d rowdata, and target request %d column, not equality.", colrowIndex, targetList->length);
    }

    return (PTableRowData)(pjData->rowData);
}



#if 0
/* 
 * fetch row data, which column matching targetlist. 
 * 两个表的查询结果行，要根据target中的列信息，将行数据投影成一个新的结果行。
 * 并且返回数据和行的位置信息；
 */
static PTableRowDataWithPos FetchTargetColumnsPos(PScanTableRowData scanTblRowInfo, PList targetList, PList rangTbl)
{
    PTableRowDataWithPos resultRowData = NULL;
    PTableRowDataPosition tblRowPosition = NULL;
    int colrowIndex = 0;

    PListCell tmpCell = NULL;
    PRangTblEntry rte = NULL;
    PAttrDataPosition attrDataPos = NULL;
    AttrDataPosition tempDataPos = {0};
    PTableRowData newRowData = NULL;
    int size = 0;

    if((NULL == targetList) || (NULL == scanTblRowInfo) || (NULL == rangTbl))
    {
        return NULL;
    }

    resultRowData = (PTableRowDataWithPos)AllocMem(sizeof(TableRowDataWithPos) + sizeof(PAttrDataPosition) * targetList->length);
    resultRowData->size = sizeof(TableRowDataWithPos) + sizeof(PAttrDataPosition) * targetList->length;

    /* traverse target list */
    for(tmpCell = targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PTargetEntry targetEntry = (PTargetEntry)GetCellNodeValue(tmpCell);
        PResTarget restarget = (PResTarget)targetEntry->colRef;
        PColumnRef colDef = (PColumnRef)restarget->val;

        /* 根据target中列对应的表index，找到表的信息记录 */
        rte = (PRangTblEntry)GetCellValueByIndex(rangTbl, targetEntry->rindex);
        if(NULL == rte)
        {
            hat_error("Rang table not founded.");
            break;
        }

        /* 根据表元数据定义，找到对应的表的查询行 */
        tblRowPosition = GetTblRowDataPosition(scanTblRowInfo, rte->tblInfo, rte->rindex);
        if(NULL == tblRowPosition)
        {
            hat_error("rowdata position not founded.");
            break;
        }

        /* 根据target中列的定义，找到对应列的信息进行投影，得到该表列的投影字段数组 */
        newRowData = GetColRowData(tblRowPosition, colDef, &tempDataPos, 0);
        if(NULL == newRowData)
        {
            hat_error("column %d rowdata not founded.", colrowIndex);
            break;
        } 
        
        size = sizeof(AttrDataPosition) + sizeof(PRowColumnData) * newRowData->num;
        attrDataPos = (PAttrDataPosition)AllocMem(size);
        attrDataPos->headItem = tempDataPos.headItem;

        size = sizeof(TableRowData) + sizeof(PRowColumnData) * newRowData->num;
        memcpy(&(attrDataPos->rowData), newRowData, size);

        resultRowData->attrDataPos[colrowIndex] = attrDataPos;
        resultRowData->num += 1;
        
        colrowIndex++;
    }

    if(colrowIndex != targetList->length)
    {
        hat_error("column %d rowdata, and target request %d column, not equality.", colrowIndex, targetList->length);
        /* TODO: release resultRowData */
        resultRowData = NULL;
    }

    return resultRowData;
}
#endif 