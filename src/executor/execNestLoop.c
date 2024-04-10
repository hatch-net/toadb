/*
 *	toadb nestloop executor 
 * Copyright (C) 2023-2023, senllang
*/

#include "execNestLoop.h"
#include "execNode.h"
#include "parserNode.h"
#include "public.h"
#include "exectable.h"

#include <stdio.h>

#define hat_log printf
#define error printf

PNode ExecInitNestLoopNode(PExecState eState)
{
    PNestLoopState planState = NULL;
    PNestLoop plan = NULL;

    plan = (PNestLoop)eState->subPlanNode;
    planState = (PNestLoopState)eState->subPlanStateNode;

    planState->outerNeedNew = HAT_YES;
    planState->outerIsEnd = HAT_NO;
    planState->innerNeedNew = HAT_YES;
    planState->innerIsEnd = HAT_NO;

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

    int found = 0;

    /*
     * eState->subPlanNode and  eState->subPlanStateNode 
     * is current plan , planState Node;
     */
    plan = (PNestLoop)eState->subPlanNode;
    planState = (PNestLoopState)eState->subPlanStateNode;

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
            found = 1;
            break;
        }

        if(found)
        {
            /* founded */
            break;
        }
    }

    /* merge rows, matching the targetlist. */
    eState->scanRowDataLeft = rowDataLeft;
    eState->scanRowDataRight = rowDataRight;
    rowData = ExecMergeRowData(eState);    

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



