/*
 *	toadb executor
 * Copyright (C) 2023-2023, senllang
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
    char    *pbuf = portal->buffer;
    int     ret = 0;

    /* Utility process first. */
    if(subPlan->commandType == CMD_UTILITY)
    {
        ExecutorUtility(subPlan, portal);
        return ;
    }

    eState->subPlanNode = (PNode)subPlan;
    eState->subPlanStateNode = (PNode)pState;
    ExecNodeProc(eState);

    if(eState->retCode >= ExecRetCode_SUC)
        snprintf(pbuf, PORT_BUFFER_SIZE, "total %d rows sucess", eState->retCode);
    else
        snprintf(pbuf, PORT_BUFFER_SIZE, "process falure(%d)", eState->retCode);
        
    FlushPortal(portal);

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
        hat_log("[ExecutorStart]NULL subPlan\n");
        return;
    }

    portalSize = GetPortalSize();

    pMem = AllocMem(sizeof(ExecState) + portalSize);

    *eState = (PExecState)pMem;
    (*eState)->portal = (PPortal)(pMem + sizeof(ExecState));
    (*eState)->plan = subPlan;

    InitExecState(*eState);
}

/* 
 * after excutor, we clean some thing.
 */
void ExecutorEnd(PExecState eState)
{
    if(NULL == eState)
        return ;

    EndPort(eState->portal);

    EndExecState(eState);
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

    if(NULL != node)
        rowData = node->execProcNode(eState);

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

