/*
 *	toadb executor node proc
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
#include "execSelect.h"

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

PTableRowData ExecProcReSeqScan(PExecState eState)
{
    ExecReSeqScan(eState);
    return NULL;
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
    PTableRowData rowData = NULL;

    rowData = ExecTableProject(eState);


    return rowData;
}

PTableRowData ExecTableQueryNode(PExecState eState)
{
    return ExecTableQuery(eState);
}

PTableRowData ExecSelectResultNode(PExecState eState)
{
    PTableRowData rowData = NULL;

    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */

    rowData = ExecSelect(eState);

    return rowData;
}


PTableRowData ExecSelectNewValue(PExecState eState)
{
    PTableRowData rowData = NULL;

    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */

    rowData = ExecUpdateSelect(eState);

    return rowData;
}
            