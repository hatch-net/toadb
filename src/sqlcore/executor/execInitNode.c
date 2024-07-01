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
#include "execSelect.h"
#include "memStack.h"


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
        hat_error("[InitExecNodePlan] subPlanNode is NULL");
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
        hat_debug("[InitExecNodePlan] leftplan ");
        eState->subPlanNode = (PNode)plan->leftplan;
        planState->left = InitExecNode(eState);       
    }

    if(NULL != plan->rightplan)
    {
        hat_debug("[InitExecNodePlan] rightplan ");
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
    PRangTblEntry rte = NULL;

    if(NULL == eState)
        return NULL;

    plan = (PSeqScan)eState->subPlanNode;

    planState = NewNode(SeqScanState);
    psn = (PPlanStateNode)planState;

    psn->commandType = eState->commandType;
    psn->portal = eState->portal;
    psn->plan = (PNode)plan;
    psn->execProcNode = ExecProcSeqScan;
    psn->execReScanNode = ExecProcReSeqScan;

    planState->scanState = InitScanState(((PRangTblEntry)(plan->rangTbl))->tblInfo, NULL);

    /* initialize search postion */
    eState->subPlanStateNode = (PNode)planState;
    planState->scanState->scanPostionInfo = InitScanPositionInfo(eState);

    rte = (PRangTblEntry)plan->rangTbl;
    planState->scanState->rindex = rte->rindex;
    
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
    PTableList tblInfo = NULL;
    PRangTblEntry rte = NULL;

    int scanMemSize = 0, rowDataSize = 0;
    int colNum = 0;
    char *pMem = NULL;

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

    rte = (PRangTblEntry)(plan->rangTbl);
    tblInfo = rte->tblInfo;
    planState->scanState = InitScanState(tblInfo, NULL);

    if (NULL != rte->targetList)
    {
        colNum = rte->targetList->length;
    }

    if (tblInfo->tableDef->colNum > colNum)
        colNum = tblInfo->tableDef->colNum;

    scanMemSize = sizeof(ScanPageInfo) + sizeof(SearchPageInfo) + sizeof(PPageDataHeader) * colNum + sizeof(GroupItemData) + (sizeof(MemberData) + sizeof(PageOffset)) * tblInfo->tableDef->colNum;

    /* column counts equal colNum */
    rowDataSize = sizeof(RowData) + sizeof(PRowColumnData);

    pMem = (char *)AllocMem(scanMemSize + rowDataSize);

    planState->scanState->scanPostionInfo = (PScanPageInfo)pMem;
    planState->scanState->scanPostionInfo->searchPageList = (PSearchPageInfo)(pMem + sizeof(ScanPageInfo));
    planState->scanState->scanPostionInfo->searchPageList->pageNum = PAGE_HEAD_PAGE_NUM + 1;

    /* find one new group. */
    planState->scanState->scanPostionInfo->isNoSpace = HAT_TRUE;

    scanMemSize = sizeof(ScanPageInfo) + sizeof(SearchPageInfo);
    planState->scanState->scanPostionInfo->pageList = (PPageDataHeader *)(pMem + scanMemSize);
    planState->scanState->scanPostionInfo->pageListNum = colNum;

    scanMemSize += sizeof(PPageDataHeader) * colNum;
    planState->scanState->scanPostionInfo->groupItem = (PGroupItemData)(pMem + scanMemSize);

    /* RowData structure. */
    scanMemSize += sizeof(GroupItemData) + (sizeof(MemberData) + sizeof(PageOffset)) * tblInfo->tableDef->colNum;
    planState->scanState->scanPostionInfo->rowData = (PRowData)(pMem + scanMemSize);

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

    planState->prjData = InitProjectData(plan->targetList->length);
    
    return (PNode)planState;
}

static PNode InitExecNodeQuerybl(PExecState eState)
{
    PQueryTblState planState = NULL;
    PPlanStateNode psn = NULL;
    PQueryTbl plan = NULL;
    int ret = 0;

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
    ret = InitSelectPortal(plan->targetList, psn->portal);
    if(ret < 0)
    {
        hat_error("init select portal failure.");
        return NULL;
    }

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
    
    planState->rtable = plan->rtable;

    /* process subplan node */
    if(NULL != plan->subplan)
    {
        eState->subPlanNode = (PNode)plan->subplan;
        planState->subplanState = InitExecNode(eState);       
    }

    planState->selectExpreData = InitSelectExpreData();
    return (PNode)planState;
}


static PNode InitExecSelectNewValue(PExecState eState)
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
    psn->execProcNode = ExecSelectNewValue;
    
    planState->rtable = plan->rtable;

    /* process subplan node */
    if(NULL != plan->subplan)
    {
        eState->subPlanNode = (PNode)plan->subplan;
        planState->subplanState = InitExecNode(eState);       
    }

    planState->selectExpreData = InitSelectExpreData();

    return (PNode)planState;
}


PNode InitExecNode(PExecState eState)
{
    PNode node = NULL;
    PNode plan = NULL;

    if((NULL == eState) || (NULL == eState->subPlanNode))
        return NULL;

    plan = eState->subPlanNode;
    
    hat_debug("[InitExecNode] node type %d", plan->type);
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
        case T_SelectNewValue:
            node = InitExecSelectNewValue(eState);
        break;
        default:
            node = NULL;
        break;
    }

    return node;
}