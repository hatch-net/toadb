/*
 *	toadb nestloop executor 
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

#include "execNestLoop.h"
#include "execNode.h"
#include "parserNode.h"
#include "public.h"
#include "exectable.h"
#include "memStack.h"

#include <stdio.h>

//#define hat_debug_nestloop(...) log_report(LOG_DEBUG, __VA_ARGS__) 
#define hat_debug_nestloop(...) 


static PTableRowData ExecMergeRowData(PExecState eState, PScanTableRowData scanTblRowInfo);

PNode ExecInitNestLoopNode(PExecState eState)
{
    PNestLoopState planState = NULL;
    PNestLoop plan = NULL;
    int size = 0;

    plan = (PNestLoop)eState->subPlanNode;
    planState = (PNestLoopState)eState->subPlanStateNode;

    planState->outerNeedNew = HAT_YES;
    planState->outerIsEnd = HAT_NO;
    planState->innerNeedNew = HAT_YES;
    planState->innerIsEnd = HAT_NO;

    size = sizeof(NestLoopData) + sizeof(ScanTableRowData) + (plan->rtNum -1) * sizeof(PTableRowDataPosition);
    planState->nestloopData = (PNestLoopData)AllocMem(size);
    planState->nestloopData->scanTableRow = (PScanTableRowData)(((char*)planState->nestloopData) + sizeof(NestLoopData));

    return (PNode)planState;
}

/* 
 * First, it excute left node and right node, then we have two results.
 * Second, it excute express with results.
 * Entirety is a double Cycle, Left is outside and right is innerside.
 */
PTableRowData ExecNestLoopNode(PExecState eState)
{
    PNestLoopState planState = NULL;
    PNestLoop plan = NULL;
    PTableRowData rowData = NULL;

    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */
    plan = (PNestLoop)eState->subPlanNode;
    planState = (PNestLoopState)eState->subPlanStateNode;

    if(NULL == plan->leftplan)
    {
        eState->retCode = ExecRetCode_ERR;
        return NULL;
    }

    rowData = ExecNestLoopScan(eState);
    /* form new rowdata with targetList. */
    
    return rowData;
}

PTableRowData ExecNestLoopScan(PExecState eState)
{
    PNestLoopState planState = NULL;
    PNestLoop plan = NULL;
     
    PTableRowData rowDataLeft = NULL, rowDataRight = NULL;
    PTableRowData rowData = NULL;

    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */
    plan = (PNestLoop)eState->subPlanNode;
    planState = (PNestLoopState)eState->subPlanStateNode;

    hat_debug_nestloop("enter nest loop...");

    /* process left and right node */
    for(; ;)
    {
        if(HAT_YES == planState->outerIsEnd)
        {
            /* node task completed. */
            return NULL;
        }

        rowDataLeft = (PTableRowData)planState->outResultRow;
        if(HAT_YES == planState->outerNeedNew)
        {
            eState->subPlanNode = (PNode)plan->leftplan;
            eState->subPlanStateNode = (PNode)planState->left;
            rowDataLeft = ExecNodeProc(eState);

            /* outer loop is complete */            
            if(NULL == rowDataLeft)
            {
                planState->outerIsEnd = HAT_YES;
                planState->outerNeedNew = HAT_NO;

                return NULL;
            }

            planState->outResultRow = (PNode)rowDataLeft;
            planState->outerNeedNew = HAT_NO;      
            planState->innerNeedNew = HAT_YES;

            /* rescan right node */
            eState->subPlanNode = (PNode)plan->rightplan;
            eState->subPlanStateNode = (PNode)planState->right;
            ExecNodeReScan(eState);
        }

        for( ; HAT_YES == planState->innerNeedNew; )
        {
            eState->subPlanNode = (PNode)plan->rightplan;
            eState->subPlanStateNode = (PNode)planState->right;

            rowDataRight = ExecNodeProc(eState);
            if(NULL == rowDataRight)
            {
                planState->innerNeedNew = HAT_NO;       
                planState->innerIsEnd = HAT_YES;

                /* next outer tuple */
                planState->outerNeedNew = HAT_YES; 

                /* next outer loop */
                break;
            }
                                
            /* founded */
            goto FOUND;
        }
    }

    hat_debug_nestloop("nest loop found one couple rows.");

FOUND:
    /* merge rows, matching the targetlist. */
    eState->scanRowDataLeft = rowDataLeft;
    eState->scanRowDataRight = rowDataRight;

    rowData = ExecMergeRowData(eState, planState->nestloopData->scanTableRow);    
    hat_debug_nestloop("nest loop merge left and right rows.");
    return rowData;
}

/*
 * reset scan state
 */
PTableRowData ReScanNestLoopNode(PExecState eState)
{
    PNestLoopState node = (PNestLoopState)eState->subPlanStateNode;

    /* TODO: nestloop node reset scan */
    return NULL;
}


/* 
 * Merge two PScanTableRowData into one struct PScanTableRowData. 
 */
PTableRowData ExecMergeRowData(PExecState eState, PScanTableRowData scanTblRowInfo)
{
    PScanTableRowData  rightRow = NULL, leftRow = NULL, tmpRow = NULL;
    PTableRowDataPosition *tblRow = NULL, *tmpRowData = NULL;
    int tableNum = 0;
    int index = 0;

    leftRow = (PScanTableRowData)eState->scanRowDataLeft;
    rightRow = (PScanTableRowData)eState->scanRowDataRight;
    
    if(NULL != leftRow)
    {
        tableNum = leftRow->tableNum;
    }

    if(NULL != rightRow)
    {
        tableNum += rightRow->tableNum;
    }

    if(tableNum < 1)
    {
        return NULL;
    }

    tblRow = &(scanTblRowInfo->tableRowData);
    scanTblRowInfo->tableNum = tableNum;

    tmpRow = leftRow;
    while(NULL != tmpRow)
    {
        tmpRowData = &(tmpRow->tableRowData);
        tableNum = tmpRow->tableNum;
        while(tableNum-- > 0)
        {
            tblRow[index++] = *(tmpRowData++);
        }
        
        if(index == scanTblRowInfo->tableNum)
            break;

        tmpRow = rightRow;
    }

    return (PTableRowData)scanTblRowInfo;
}

