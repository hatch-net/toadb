/*
 *	toadb executor node proc
 * Copyright (C) 2023-2023, senllang
*/

#include <stddef.h>
#include "queryNode.h"
#include "execNodeProc.h"
#include "executor.h"
#include "seqscan.h"
#include "valueScan.h"
#include "execModifyTable.h"
#include "exectable.h"
#include "execNestLoop.h"
#include "execProject.h"

PTableRowData ExecProcPlan(PExecState eState)
{
    PPlanState planState = NULL;
    PPlan plan = NULL;
    PTableRowData rowData = NULL;
    
    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */
    plan = (PPlan)eState->subPlanNode;
    planState = (PPlanState)eState->subPlanStateNode;

    /* process left and right node */
    if(NULL == plan->leftplan)
    {
        eState->retCode = ExecRetCode_ERR;
        return NULL;
    }

    eState->subPlanNode = (PNode)plan->leftplan;
    eState->subPlanStateNode = (PNode)planState->left;
    
    /* 
     * retCode is 0 when success ending, or < 0 is error. 
     * rowData is NULL, when returning is not set.
    */
    rowData = ExecNodeProc(eState);

    return rowData;
}

PTableRowData ExecProcNestLoop(PExecState eState)
{
    /* insert stmt */
    return ExecNestLoopNode(eState);
}


PTableRowData ExecProcSeqScan(PExecState eState)
{
    return ExecSeqscanNode(eState);
}

PTableRowData ExecProcValueScan(PExecState eState)
{
    return ExecScanValuesNode(eState);;
}

/* 
 * operator modify table data( insert/update/delete)
 * operator all rows which scanned by scan node, when rows is NULL, and retCode <= 0;
 */
PTableRowData ExecProcModifyTbl(PExecState eState)
{
    return ExecTableModifyTbl(eState);
}

PTableRowData ExecProcProjectTbl(PExecState eState)
{
    return ExecTableProject(eState);
}

PTableRowData ExecTableQueryNode(PExecState eState)
{
    return ExecTableQuery(eState);
}