/*
 *	toadb plan
 * Copyright (C) 2023-2023, senllang
*/

#include <stddef.h>
#include <stdio.h>
#include "plan.h"
#include "planNode.h"
#include "queryNode.h"

#define log printf

PList QueryPlan(PList queryTree)
{
    PList Plan = NULL;
    PNode subPlan = NULL;
    PListCell tmpCell = NULL;
    PMemContextNode oldContext = NULL;
    
    /* add scan node and target node */
    if(NULL == queryTree)
    {
        log("[QueryPlan] queryTree is NULL\n");
        return NULL;
    }

    oldContext = MemMangerNewContext("planTree");
    
    /* traverse all subParserTree, generator query tree. */
    for(tmpCell = queryTree->head; tmpCell != NULL; tmpCell = tmpCell->next) 
    {
        PQuery node = (PQuery)(tmpCell->value.pValue);
        
        subPlan = SubPlanProcess(node);

        if(NULL != subPlan)
            Plan = AppendNode(Plan, (PNode)subPlan);
    }

    MemMangerSwitchContext(oldContext);
    return Plan;
}

/* 
 * materialize plan 
 */
PNode SubPlanProcess(PQuery subQuery)
{
    PPlan subPlan = NULL;
    PPlanProcessor planProcessor = NULL;

    if(NULL == subQuery)
    {
        log("[SubPlanProcess]invalid subQuery\n");
        return NULL;
    }
    
    planProcessor = AllocMem(sizeof(PlanProcessor));
    subPlan = NewNode(Plan);

    subPlan->commandType = subQuery->commandType;

    planProcessor->plan = subPlan;
    planProcessor->query = subQuery;

    switch(subQuery->commandType)
    {
        case CMD_UTILITY:
            subPlan->commandType = subQuery->commandType;
            subPlan->QueryTree = (PNode)subQuery;            
            break;
        case CMD_SELECT:
            subPlanSelectStmt(planProcessor);
            break;
        case CMD_UPDATE:
            
            break;
        case CMD_INSERT:
            subPlanInsertStmt(subPlan, subQuery);
            break;
        case CMD_DELETE:
            break;            
        default:
            /* empty list */
            break;
    }
    
    FreeMem(planProcessor);    
    return (PNode)subPlan;
}

/* 
 * 将条件查询树转换为物理执行节点；最后返回基础表的结果集。
 */
PNode subPlanSelectStmt(PPlanProcessor planProcessor)
{
    PQueryTbl queryTblNode = NULL;

    if(NULL == planProcessor)
        return NULL;
    
    queryTblNode = NewNode(QueryTbl);
    planProcessor->currentNode = (PNode)planProcessor->query->joinTree;

    /* process jointree, insert scan node and nestloop node */
    queryTblNode->subplan = ProcessQual(planProcessor);

    /* root of select clause is queryTblNode */
    planProcessor->plan->leftplan = (PNode)queryTblNode;

    queryTblNode->targetList = planProcessor->query->targetList;
    queryTblNode->rtable = planProcessor->query->rtable;

    /* nothing need returning. */
    return NULL;
}

/*
 * 将insert into ... values ... 形式转换为 insert table select * from valuesttable
 * 这样就可以使用scan节点进行处理，同样带有子查询时也是类似处理。
 */
PNode subPlanInsertStmt(PPlan plan, PQuery subQuery)
{
    PModifyTbl modifyNode = NULL;
    PValueScan seqscanValuesNode = NULL;

    if((NULL == plan) || (NULL == subQuery))
        return NULL;

    /* 
     * Add modify node as root, which execute inserting values, 
     * has subnode with scan values from range table of values type. 
     */    
    modifyNode = NewNode(ModifyTbl);    
    seqscanValuesNode = NewNode(ValueScan);

    seqscanValuesNode->rangTbl = GetValuesTypeRangTbl(subQuery->rtable);
    seqscanValuesNode->targetList = subQuery->targetList;

    modifyNode->leftplan = (PNode)seqscanValuesNode;
    modifyNode->rangTbl = (PNode)GetFirstCellNode(subQuery->rtable);

    /* values rang table metadata is same as target table. */
    ((PRangTblEntry)(seqscanValuesNode->rangTbl))->tblInfo = ((PRangTblEntry)(modifyNode->rangTbl))->tblInfo; 
    
    plan->leftplan = (PNode)modifyNode;

    return (PNode)plan;
}

PNode GetValuesTypeRangTbl(PList rangTblList)
{
    PListCell tmpCell = NULL;
    PRangTblEntry rangTbl = NULL;

    /* add scan node and target node */
    if(NULL == rangTblList)
    {
        log("[GetValuesTypeRangTbl] rangTblList is NULL\n");
        return NULL;
    }

    /* traverse all subParserTree, generator query tree. */
    for(tmpCell = rangTblList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        rangTbl = (PRangTblEntry)GetCellNodeValue(tmpCell);
        
        if(rangTbl->relkind == RELKIND_VALUES)
            return (PNode)rangTbl;
    }
        
    return NULL;
}

/* 
 * It will do some things as follow,
 * - bool expr node transform to nestloop node
 * - const expr node transform to scan node 
 * - join expr node transform to nestloop node 
 */
PNode ProcessQual(PPlanProcessor planProcess)
{
    PList joinTreeList = (PList)planProcess->currentNode;
    PListCell tmpCell = NULL;
    PList subPlanList = NULL;
    PNode exprNode = NULL;
    
    if(NULL == joinTreeList)
        return NULL;
    
    /* search qual list */
    for(tmpCell = joinTreeList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode valueNode = (PNode)GetCellNodeValue(tmpCell);

        planProcess->currentNode = valueNode;
        exprNode = ProcessJoinTree(planProcess);

        if(NULL != exprNode)
            subPlanList = AppendNode(subPlanList, exprNode);
    }

    /* 
     * if subPlan List length > 1, then promote one as top, 
     * the other is the left and right subnode. 
     */
    if((subPlanList != NULL) && (subPlanList->length > 1))
    {
        /* TODO: */
        exprNode = NULL;
        log("not support.\n");
    }

    return exprNode;
}

PNode ProcessJoinTree(PPlanProcessor planProcess)
{
    PNode valueNode = NULL;

    if(NULL == planProcess->currentNode)
        return NULL;
    
    switch(planProcess->currentNode->type)
    {
        case T_MergerEntry:
            valueNode = ProcessMergerNode(planProcess);                             
            break;

        case T_JoinEntry:
            valueNode = ProcessJoinNode(planProcess); 
            break;

        case T_ExprEntry:
            valueNode = ProcessScanNode(planProcess);
            break;
            
        default:
            valueNode = NULL;
            break;        
    }

    return valueNode;
}

PNode ProcessMergerNode(PPlanProcessor planProcess)
{
    PNestLoop nlNode = NULL;
    PMergerEntry mergerNode = (PMergerEntry)planProcess->currentNode;

    if(NULL == mergerNode)
        return NULL;

    nlNode = NewNode(NestLoop);
    nlNode->mergeType = mergerNode->mergeType;
    nlNode->targetList = mergerNode->targetList;

    /* process left and right subplan */
    planProcess->currentNode = mergerNode->lefttree;
    nlNode->leftplan = ProcessJoinTree(planProcess);

    planProcess->currentNode = mergerNode->righttree;
    nlNode->rightplan = ProcessJoinTree(planProcess);

    return (PNode)nlNode;
}

PNode ProcessScanNode(PPlanProcessor planProcess)
{
    PSeqScan seqScanNode = NULL;
    PExprEntry exprNode = (PExprEntry)planProcess->currentNode;
    PQuery query = planProcess->query;
    PRangTblEntry rte = NULL;

    if(NULL == exprNode)
        return NULL;

    seqScanNode = NewNode(SeqScan);

    rte = (PRangTblEntry)GetCellValueByIndex(query->rtable, exprNode->rindex);
    if(NULL != rte)
        rte->isScaned = 1;

    seqScanNode->expr = (PNode)exprNode;
    seqScanNode->targetList = exprNode->targetList;
    seqScanNode->rangTbl = (PNode)rte;

    return (PNode)seqScanNode;
}

PNode ProcessJoinNode(PPlanProcessor planProcess)
{
    PNestLoop nlNode = NULL;
    PJoinEntry joinNode = (PJoinEntry)planProcess->currentNode;

    if(NULL == joinNode)
        return NULL;

    nlNode = NewNode(NestLoop);
    nlNode->mergeType = joinNode->joinOp;
    nlNode->isJoin = joinNode->isJoin;
    nlNode->targetList = joinNode->targetList;

    /* process left and right subplan */
    planProcess->currentNode = joinNode->lefttree;
    nlNode->leftplan = ProcessJoinTree(planProcess);

    planProcess->currentNode = joinNode->righttree;
    nlNode->rightplan = ProcessJoinTree(planProcess);

    return (PNode)nlNode;
}

