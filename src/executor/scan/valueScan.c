/*
 *	toadb valueScan 
 * Copyright (C) 2023-2023, senllang
*/

#include <stdio.h>
#include "valueScan.h"
#include "execNode.h"
#include "queryNode.h"
#include "tuples.h"
#include "scan.h"

#define hat_log printf


/* values relation  scan */
static PTableRowData ValueScanNext(PExecState eState);
static PTableRowData ValueScanEnd(PExecState eState);

/* 
 * excutor value node scan 
*/
PTableRowData ExecScanValuesNode(PExecState eState)
{
    return ValueScanNext(eState);
}

/* 
 * value node scan excutor end
*/
PTableRowData ExecScanValuesNodeEnd(PExecState eState)
{
    return ValueScanEnd(eState);
}


/* 
 * values list transform to tuples. 
 * every function call will be returning one row data, 
 * until all completed.
 */
static PTableRowData ValueScanNext(PExecState eState)
{
    int transformNum = 0;
    PValueScanState planState = NULL;
    PTableRowData rowData = NULL;
    PDLCell cell = NULL;
    PScanState scanState = NULL;

    planState = (PValueScanState)eState->subPlanStateNode;
    scanState = planState->scanState;
    if(NULL == scanState)
    {
        transformNum = TransformValues(eState);
        if(transformNum <= 0)
            return NULL;

        /* update pointer */    
        scanState = planState->scanState;
    }

    cell = (PDLCell)PopDListHeadNode(&(scanState->rows));
    if(NULL == cell)
    {
        eState->retCode = ExecRetCode_SUC;
        return NULL;
    }

    rowData = (PTableRowData)DList_Node_Value(cell);
    EreaseDListNode((PDList)cell);
    return rowData;
}

int TransformValues(PExecState eState)
{
    PTableList tblInfo = NULL;
    PRangTblEntry rte = NULL;
    PValueScanState planState = NULL;
    PValueScan plan = NULL;
    PScanState scanState = NULL;

    PTableRowData rowData = NULL;
    PList targetList = NULL;
    PList valueList = NULL;
    PNode values = NULL;

    PListCell tmpCell = NULL;
    unsigned int rowNum = 0;

    plan = (PValueScan)eState->subPlanNode;
    planState = (PValueScanState)eState->subPlanStateNode;

    rte = (PRangTblEntry)plan->rangTbl;

    /* 
     * value rang table, which tblInfo is NULL, 
     * maybe use target table metadata define. 
     */
    tblInfo = rte->tblInfo;
    if((NULL == tblInfo) || (NULL == rte))
    {
        eState->retCode = ExecRetCode_ERR;
        return -1;
    }

    scanState = InitScanState(NULL, NULL);
    targetList = rte->targetList;
    valueList = (PList)rte->ValueList;
    for(tmpCell = valueList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        values = GetCellNodeValue(tmpCell);
        rowData = ValuesFormRowData(tblInfo->tableDef, (PNode)targetList, values);

        if(NULL == rowData)
            break;
        
        /* add to tblScan */
        AddCellToListTail(&(scanState->rows), rowData);
        rowNum++;
    }    
    
    scanState->rowNum = rowNum;
    planState->scanState = scanState;

    return rowNum;
}

/* 
 * release scan resource 
 */
static PTableRowData ValueScanEnd(PExecState eState)
{
    return NULL;
}