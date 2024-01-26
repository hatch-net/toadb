/*
 *	toadb executor node initialize
 * Copyright (C) 2023-2023, senllang
*/

#include <stddef.h>
#include <stdio.h>

#include "execInitNode.h"
#include "node.h"
#include "execNode.h"
#include "execNodeProc.h"
#include "parserNode.h"
#include "queryNode.h"
#include "valueScan.h"
#include "execNestLoop.h"
#include "execProject.h"


#define hat_log printf 
#define debug 

typedef PNode (*InitExecFun)(PExecState eState);


static PNode InitExecNodePlan(PExecState eState)
{
    PPlanState planState = NULL;
    PPlanStateNode psn = NULL;
    PPlan plan = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PPlan)eState->subPlanNode;
    if(NULL == plan)
    {
        hat_log("[InitExecNodePlan] subPlanNode is NULL\n");
        return NULL;
    }

    planState = NewNode(PlanState);
    psn = (PPlanStateNode)planState;

    psn->commandType = plan->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;

    /* process left and right node */
    if(NULL != plan->leftplan)
    {
        debug("[InitExecNodePlan] leftplan \n");
        eState->subPlanNode = (PNode)plan->leftplan;
        planState->left = InitExecNode(eState);       
    }

    if(NULL != plan->rightplan)
    {
        debug("[InitExecNodePlan] rightplan \n");
        eState->subPlanNode = (PNode)plan->rightplan;
        planState->right = InitExecNode(eState);
    }

    psn->execProcNode = ExecProcPlan;
    
    return (PNode)planState;
}


static PNode InitExecNodeNestLoop(PExecState eState)
{
    PNestLoopState planState = NULL;
    PPlanStateNode psn = NULL;
    PNestLoop plan = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PNestLoop)eState->subPlanNode;

    planState = NewNode(NestLoopState);
    psn = (PPlanStateNode)planState;

    psn->commandType = eState->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;
    psn->execProcNode = ExecProcNestLoop;
    psn->execReScanNode = ReScanNestLoopNode;

    eState->subPlanStateNode = (PNode)planState;
    ExecInitNestLoopNode(eState);

    /* process left and right node */
    if(NULL != plan->leftplan)
    {
        eState->subPlanNode = (PNode)plan->leftplan;
        planState->left = InitExecNode(eState);       
    }

    if(NULL != plan->rightplan)
    {
        eState->subPlanNode = (PNode)plan->rightplan;
        planState->right = InitExecNode(eState);
    }

    return (PNode)planState;
}

static PNode InitExecNodeSeqScan(PExecState eState)
{
    PSeqScanState planState = NULL;
    PPlanStateNode psn = NULL;
    PSeqScan plan = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PSeqScan)eState->subPlanNode;

    planState = NewNode(SeqScanState);
    psn = (PPlanStateNode)planState;

    psn->commandType = eState->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;
    psn->execProcNode = ExecProcSeqScan;

    planState->scanState = InitScanState(((PRangTblEntry)(plan->rangTbl))->tblInfo, NULL);

    /* initialize search postion */
    eState->subPlanStateNode = (PNode)planState;
    planState->scanState->scanPostionInfo = InitScanPositionInfo(eState);

    return (PNode)planState;
}


static PNode InitExecNodeValueScan(PExecState eState)
{
    PValueScanState planState = NULL;
    PPlanStateNode psn = NULL;
    PSeqScan plan = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PSeqScan)eState->subPlanNode;

    planState = NewNode(ValueScanState);
    psn = (PPlanStateNode)planState;

    psn->commandType = eState->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;

    psn->execProcNode = ExecProcValueScan;
    return (PNode)planState;
}

static PNode InitExecNodeModifyTbl(PExecState eState)
{
    PModifyTblState planState = NULL;
    PPlanStateNode psn = NULL;
    PModifyTbl plan = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PModifyTbl)eState->subPlanNode;

    planState = NewNode(ModifyTblState);
    psn = (PPlanStateNode)planState;

    psn->commandType = eState->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;

    /* process left and right node */
    if(NULL != plan->leftplan)
    {
        eState->subPlanNode = (PNode)plan->leftplan;
        planState->left = InitExecNode(eState);       
    }

    if(NULL != plan->rightplan)
    {
        eState->subPlanNode = (PNode)plan->rightplan;
        planState->right = InitExecNode(eState);
    }

    psn->execProcNode = ExecProcModifyTbl;
    return (PNode)planState;
}

static PNode InitExecNodeProjectTbl(PExecState eState)
{
    PProjectTblState planState = NULL;
    PPlanStateNode psn = NULL;
    PProjectTbl plan = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PProjectTbl)eState->subPlanNode;

    planState = NewNode(ProjectTblState);
    psn = (PPlanStateNode)planState;

    psn->commandType = eState->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;
    psn->execProcNode = ExecProcProjectTbl;

    /* process subplan node */
    if(NULL != plan->subplan)
    {
        eState->subPlanNode = (PNode)plan->subplan;
        planState->subplanState = InitExecNode(eState);       
    }

    return (PNode)planState;
}

static PNode InitExecNodeQuerybl(PExecState eState)
{
    PQueryTblState planState = NULL;
    PPlanStateNode psn = NULL;
    PQueryTbl plan = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PQueryTbl)eState->subPlanNode;

    planState = NewNode(QueryTblState);
    psn = (PPlanStateNode)planState;

    psn->commandType = eState->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;
    psn->execProcNode = ExecTableQueryNode;

    /* generator partal client title. */
    InitSelectPortal(plan->targetList, psn->portal);

    /* process subplan node */
    if(NULL != plan->subplan)
    {
        eState->subPlanNode = (PNode)plan->subplan;
        planState->subplanState = InitExecNode(eState);       
    }

    return (PNode)planState;
}

static PNode InitExecSelect(PExecState eState)
{
    PSelectState planState = NULL;
    PPlanStateNode psn = NULL;
    PSelectResult plan = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PSelectResult)eState->subPlanNode;

    planState = NewNode(SelectState);
    psn = (PPlanStateNode)planState;

    psn->commandType = eState->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;
    psn->execProcNode = ExecSelectResultNode;

    /* process subplan node */
    if(NULL != plan->subplan)
    {
        eState->subPlanNode = (PNode)plan->subplan;
        planState->subplanState = InitExecNode(eState);   
        planState->rtable = plan->rtable;    
    }

    return (PNode)planState;
}

PNode InitExecNode(PExecState eState)
{
    PNode node = NULL;
    PNode plan = NULL;

    if((NULL == eState) || (NULL == eState->subPlanNode))
        return NULL;

    plan = eState->subPlanNode;
    
    debug("[InitExecNode] node type %d\n", plan->type);
    switch(plan->type)
    {
        case T_Plan:
            node = InitExecNodePlan(eState);
        break;
        case T_NestLoop:
            node = InitExecNodeNestLoop(eState);
        break;
        case T_SeqScan:
            node = InitExecNodeSeqScan(eState);
        break;
        case T_ValueScan:
            node = InitExecNodeValueScan(eState);
        break;
        case T_ModifyTbl:
            node = InitExecNodeModifyTbl(eState);
        break;
        case T_ProjectTbl:
            node = InitExecNodeProjectTbl(eState);
        break;
        case T_QueryTbl:
            node = InitExecNodeQuerybl(eState);
        break;
        case T_SelectResult:
            node = InitExecSelect(eState);
        break;
        default:
            node = NULL;
        break;
    }

    return node;
}