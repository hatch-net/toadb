/*
 *	toadb executor
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "executor.h"
#include "exectable.h"
#include "execUtility.h"
#include "buffer.h"
#include "execInitNode.h"
#include "execNodeProc.h"
#include "public.h"
#include "memStack.h"
#include "servprocess.h"
#include "transactionControl.h"

void ExecutorMain(PList list)
{
    PPlan subPlan = NULL;
    PListCell tmpCell = NULL;
    PExecState eState = NULL;
    PMemContextNode oldContext = NULL;

    oldContext = MemMangerNewContext("ExecutorMain");

    /* excutor subplan one by one */
    for(tmpCell = list->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        subPlan = (PPlan)GetCellNodeValue(tmpCell);

        ExecutorStart((PNode)subPlan, &eState);
        
        ExecutorPlan(eState);
        
        ExecutorEnd(eState);
    }

    MemMangerSwitchContext(oldContext);

    return ;
}

void ExecutorPlan(PExecState eState)
{
    PPlan  subPlan = (PPlan)eState->plan;
    PPortal portal = eState->portal;
    PPlanState  pState = (PPlanState)eState->planState;
    char    *pbuf = NULL;
    int     ret = 0;

    hat_debugExcutorPathSelect("excutor start.");

    /* Utility process first. */
    if(subPlan->commandType == CMD_UTILITY)
    {
        ExecutorUtility(subPlan, portal);
        return ;
    }

    eState->subPlanNode = (PNode)subPlan;
    eState->subPlanStateNode = (PNode)pState;
    ExecNodeProc(eState);

    pbuf = portal->buffer;
    if(eState->retCode >= ExecRetCode_SUC)
        snprintf(pbuf, PORT_BUFFER_SIZE, "total %d rows sucess", eState->retCode);
    else
        snprintf(pbuf, PORT_BUFFER_SIZE, "process falure(%d)", eState->retCode);
        
    FlushPortal(portal);
    hat_debugExcutorPathSelect("excutor end.");
    return ;
}


/* 
 * pre excutor, we initialize some thing.
 */
void ExecutorStart(PNode subPlan, PExecState *eState)
{
    int portalSize = 0;
    char *pMem = NULL;

    if(NULL == subPlan)
    {
        hat_log("[ExecutorStart]NULL subPlan");
        return;
    }

    hat_debugExcutorPathSelect("excutor ready start.");

    pMem = AllocMem(sizeof(ExecState));

    *eState = (PExecState)pMem;
    (*eState)->portal = GetServPortal();
    (*eState)->plan = subPlan;
    (*eState)->snapshot = GetServSnapShot();

    InitPortal((*eState)->portal);

    /* get snapshot */
    GenerateSnapshot();

    InitExecState(*eState);
    hat_debugExcutorPathSelect("excutor ready finish.");
}

/* 
 * after excutor, we clean some thing.
 */
void ExecutorEnd(PExecState eState)
{
    if(NULL == eState)
        return ;
    
    EndExecState(eState);
    hat_debugExcutorPathSelect("excutor clean finish.");
}

void InitExecState(PExecState eState)
{
    PPlan plan = NULL;
    PNode subPlanStateNode = NULL;

    if(NULL == eState)
    {
        return ;
    }

    plan = (PPlan)eState->plan;
    eState->commandType = plan->commandType;

    /* 
     * We will generator excutor state tree, 
     * that has node types same as  plan tree. 
     */
    eState->subPlanNode = (PNode)plan;
    eState->planState = (PNode)InitExecNode(eState);

    return;
}

void EndExecState(PExecState eState)
{
    /*  release plan State */
    EndExecPlan(eState);

    FreeMem(eState);
}

void EndExecPlan(PExecState eState)
{
    /* TODO: release */
    return ;
}

/*
 * common function, that call node proc. 
 */
PTableRowData ExecNodeProc(PExecState eState)
{
    PTableRowData rowData = NULL;

    PPlanStateNode node = (PPlanStateNode)eState->subPlanStateNode;

    if((NULL != node) && (NULL != node->execProcNode))
    {        
        hat_debugExcutorPathSelect("excutor node type:%d.", node->type);
        rowData = node->execProcNode(eState);
    }

    return rowData;
}

/*
 * rescan function, return NULL. 
 * reinitialize scanstate.
 */
PTableRowData ExecNodeReScan(PExecState eState)
{
    PTableRowData rowData = NULL;

    PPlanStateNode node = (PPlanStateNode)eState->subPlanStateNode;

    if((NULL != node) && (NULL != node->execReScanNode))
        rowData = node->execReScanNode(eState);

    return rowData;
}

