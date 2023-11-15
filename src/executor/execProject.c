/*
 *	toadb project node proc
 * Copyright (C) 2023-2023, senllang
*/

#include "execProject.h"
#include "scan.h"
#include "planNode.h"
#include "execNode.h"
#include "queryNode.h"

#include <stdio.h>

#define log printf
#define error printf


static PTableRowData FetchTargetColumns(PScanTableRowData scanTblRowInfo, PList targetList, PList rangTbl);

static PTableRowData GetColRowData(PTableRowDataPosition tblRowPosition, PColumnRef colDef);

PTableRowData ExecTableProject(PExecState eState)
{
    
    return NULL;
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
    PScanTableRowData scanTblRowInfo = NULL;

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

        /* transform column data matching targetlist. */
        rowData = FetchTargetColumns((PScanTableRowData)rowData, plan->targetList, plan->rtable);
        if(NULL == rowData)
        {
            /* erorr ocurr, query ending. */
            break;
        }

        /* send to portal */
        SendToPort(planState->stateNode, rowData);

        rowNum++;
    }
    
    /* client show */
    FinishSend(planState->stateNode->portal); 
    eState->retCode = rowNum;

    return NULL;
}

/* 
 * fetch row data, which column matching targetlist. 
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

        rte = (PRangTblEntry)GetCellValueByIndex(rangTbl, targetEntry->rindex);
        if(NULL == rte)
        {
            error("Rang table not founded.");
            break;
        }

        tblRowPosition = GetTblRowDataPosition(scanTblRowInfo, rte->tblInfo);
        if(NULL == tblRowPosition)
        {
            error("rowdata position not founded.");
            break;
        }

        rawcolrow[colrowIndex] = GetColRowData(tblRowPosition, colDef);
        if(NULL == rawcolrow[colrowIndex])
        {
            error("column %d rowdata not founded.", colrowIndex);
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
        error("column %d rowdata, and target request %d column, not equality.", colrowIndex, targetList->length);
    }

    return resultRowData;
}

/*
 * getting rowdata, specified colmn infomation.
 */
static PTableRowData GetColRowData(PTableRowDataPosition tblRowPosition, PColumnRef colDef)
{
    PTableRowData colRowData = NULL;
    
    int *colIndexArr = NULL;
    int attrIndex = -1;
    
    int pageno = -1, rowno = -1;
    int i = 0;

    if(tblRowPosition->rowNum <= 0)
    {
        return NULL;
    }

    /* column index of table metadata. */
    attrIndex = GetAttrIndex(tblRowPosition->tblInfo, colDef->field);
    if(attrIndex < 0)
    {
        return NULL;
    }

    /* coldata is already readed? */
    colIndexArr = tblRowPosition->rowDataPosition->scanPostionInfo->colindexList;
    for(i = 0; i < tblRowPosition->rowDataPosition->scanPostionInfo->pageListNum; i++)
    {
        if(attrIndex == colIndexArr[i])
        {
            /* TODO: alread exist */
            colRowData = FormColData2RowData(tblRowPosition->rowDataPosition->rowData->columnData[i]);
            return colRowData;
        }
    }
    
    /* coldata read from page. */
    pageno = GetPageNoFromGroupInfo(&(tblRowPosition->rowDataPosition->scanPostionInfo->groupPageInfo), attrIndex);
    colRowData = GetRowDataFromPageByIndex(tblRowPosition->tblInfo, pageno, rowno);

    return colRowData;
}