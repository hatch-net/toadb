/*
 *	toadb plan
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
#include <stdio.h>
#include "plan.h"
#include "planNode.h"
#include "queryNode.h"
#include "memStack.h"


static PNode ProcessTableScan(PPlanProcessor planProcess);
static PNode ProcessTopNode(PPlanProcessor planProcessor);
static PNode ProcessSelectQual(PPlanProcessor planProcessor);
static PNode ProcessQualUpdateSetQual(PPlanProcessor planProcessor);

/* 
 * 物理执行计划生成入口
 */
PList QueryPlan(PList queryTree)
{
    PList Plan = NULL;
    PNode subPlan = NULL;
    PListCell tmpCell = NULL;
    PMemContextNode oldContext = NULL;
    
    /* add scan node and target node */
    if(NULL == queryTree)
    {
        hat_log("[QueryPlan] queryTree is NULL");
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
    PlanProcessor planProcessor;
    
    if(NULL == subQuery)
    {
        hat_log("[SubPlanProcess]invalid subQuery");
        return NULL;
    }
    
    subPlan = NewNode(Plan);
    subPlan->commandType = subQuery->commandType;
    subPlan->QueryTree = (PNode)subQuery;  
    
    planProcessor.plan = subPlan;
    planProcessor.query = subQuery;

    switch(subQuery->commandType)
    {
        case CMD_UTILITY:                      
            break;
        case CMD_SELECT:
            subPlanSelectStmt(&planProcessor);
            break;
        case CMD_UPDATE:
            subPlanUpdateStmt(&planProcessor);
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
      
    return (PNode)subPlan;
}

/* 
 * 将条件查询树转换为物理执行节点；最后返回基础表的结果集。
 * It will do some things as follow,
 * - creating scan node as leaf node from rtable list, query phare do it.
 * - push down qual to scan node.
 * - creating control node from qual.
 * - creating column scan node under scan node.
 * - push down qual to column scan node.
 */
PNode subPlanSelectStmt(PPlanProcessor planProcessor)
{
    PQueryTbl queryTblNode = NULL;

    if(NULL == planProcessor)
        return NULL;
    
    queryTblNode = NewNode(QueryTbl);

    /* rangtable scan node, which is from rtJoinTree List. */
    planProcessor->currentNode = (PNode)planProcessor->query->rtjoinTree;
    queryTblNode->subplan = ProcessTableScan(planProcessor);

    /* if qual is not null, add select node beyond scan node. */
    if(NULL != planProcessor->query->joinTree)
    {
        planProcessor->currentNode = (PNode)planProcessor->query->joinTree;
        planProcessor->subQuery = (PQuery)queryTblNode->subplan;
        /* process jointree,   qual node, and push down select  */
        queryTblNode->subplan = ProcessSelectQual(planProcessor);
    }

    /* if insert/udpate/delete create modify node, if select create project node. */
    planProcessor->currentNode = (PNode)queryTblNode->subplan;
    queryTblNode->subplan = ProcessTopNode(planProcessor);

    /* root of select clause is queryTblNode */
    planProcessor->plan->leftplan = (PNode)queryTblNode;

    queryTblNode->targetList = planProcessor->query->targetList;
    queryTblNode->rtable = planProcessor->query->rtable;

    /* nothing need returning. */
    return NULL;
}

/*
 * 增加中间层节点；处理选择条件和逻辑条件；
 * 部分条件可以下推到扫描节点；
 */
static PNode ProcessSelectQual(PPlanProcessor planProcessor)
{
    PNode rootNode = (PNode)planProcessor->subQuery;
    PList JoinTree = (PList)planProcessor->currentNode;

    PSelectResult selectNode = NewNode(SelectResult);
    selectNode->qual = JoinTree;
    selectNode->rtable = planProcessor->query->rtable;

    /* 向上增加selectNode */
    selectNode->subplan = (PList)rootNode;

    /* 这里是qual 对应的targetlist或者是总的. */
    selectNode->targetList = planProcessor->query->qualTargetList;
    
    rootNode = (PNode)selectNode;

    return rootNode;
}

/* 
 * 增加计划树的顶层节点；
 * 当select时，增加投影project节点；
 */
PNode ProcessTopNode(PPlanProcessor planProcessor)
{
    PNode rootNode = planProcessor->currentNode;
    PQuery query = planProcessor->query;

    switch(query->commandType)
    {
        case CMD_SELECT:
        case CMD_UPDATE:
            {
                /* project node */
                PProjectTbl projectNode = NewNode(ProjectTbl);
                projectNode->rtable = query->rtable;
                projectNode->targetList = query->targetList;
                projectNode->subplan = rootNode;
                rootNode = (PNode)projectNode;
            }
            break;
        case CMD_INSERT:
        case CMD_DELETE:
            {
                ;
            }
            break;
        default:
        break;        
    }
    return rootNode;
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
        hat_log("[GetValuesTypeRangTbl] rangTblList is NULL");
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
 * push down qual to table 
 * and create column scan node, then push qual to column scan node.
 */
static PNode ProcessTableScan(PPlanProcessor planProcess)
{
    PList rtbList = planProcess->query->rtable;
    PList rtJoinList = planProcess->query->rtjoinTree;
    PList joinList = planProcess->query->joinTree;
    PNode rtScanTree = NULL;
    PNode rtTreeRoot = NULL;

    /* create the other scan node, and create column scan node */
    planProcess->currentNode = (PNode)planProcess->query->rtjoinTree;
    rtTreeRoot = ProcessQual(planProcess);

    /* search qual */
    if(NULL != joinList)
    {
        /* TODO: create scan node, push down to table scan, and create column node */
        ;
    }    
    return rtTreeRoot;
}


/* 
 * creating control node from qual.
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
        hat_log("not support.");
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
    nlNode->rtNum = mergerNode->rtNum;

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
    nlNode->rtNum = joinNode->rtNum;

    /* process left and right subplan */
    planProcess->currentNode = joinNode->lefttree;
    nlNode->leftplan = ProcessJoinTree(planProcess);

    planProcess->currentNode = joinNode->righttree;
    nlNode->rightplan = ProcessJoinTree(planProcess);

    return (PNode)nlNode;
}

PNode subPlanUpdateStmt(PPlanProcessor planProcessor)
{
    PModifyTbl modifyNode = NULL;
    PPlan plan = NULL;

    if(NULL == planProcessor)
        return NULL;
    
    if(planProcessor->query->rtable->length > 1)
    {
        hat_error("update table ambiguous.");
        return NULL;
    }

    plan = planProcessor->plan;
    
    /* 
     * Add modify node as root, which execute update values, 
     * has subnode with scan values from range table of values type. 
     */    
    modifyNode = NewNode(ModifyTbl);    

     /* rangtable scan node, which is from rtJoinTree List. */
    modifyNode->leftplan = ProcessTableScan(planProcessor);
    modifyNode->rangTbl = GetFirstCellNode(planProcessor->query->rtable);

    /* if qual is not null, add select node beyond scan node. */
    if(NULL != planProcessor->query->joinTree)
    {
        planProcessor->currentNode = (PNode)planProcessor->query->joinTree;
        planProcessor->subQuery = (PQuery)modifyNode->leftplan;

        /* process jointree,   qual node, and push down select  */
        modifyNode->leftplan = ProcessSelectQual(planProcessor);
    }

    /* if insert/udpate/delete, add project node. */
    planProcessor->currentNode = (PNode)modifyNode->leftplan;
    modifyNode->leftplan = ProcessTopNode(planProcessor);

    /* update modify node right is updateSelect node */
    planProcessor->currentNode = (PNode)modifyNode;
    modifyNode->rightplan = ProcessQualUpdateSetQual(planProcessor);

    plan->leftplan = (PNode)modifyNode;
    return (PNode)plan;
}

static PNode ProcessQualUpdateSetQual(PPlanProcessor planProcessor)
{
    PSelectNewValue selectNode = NewNode(SelectNewValue);

    /* It is initialize when excutor this node. */
    selectNode->qual = NULL;
    selectNode->subplan = NULL;
    selectNode->rtable = planProcessor->query->rtable;

    /* 这里的target是 top target list. */
    selectNode->targetList = planProcessor->query->targetList;

    return (PNode)selectNode;
}