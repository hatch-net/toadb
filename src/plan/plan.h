/*
 *	toadb plan
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_PLAN_H_H
#define HAT_PLAN_H_H

#include "node.h"
#include "queryNode.h"
#include "planNode.h"

typedef struct PlanProcessor
{
    PPlan plan;
    PQuery query;
    PQuery subQuery;
    PNode  currentNode;
}PlanProcessor, *PPlanProcessor;

PList QueryPlan(PList queryTree);

PNode SubPlanProcess(PQuery subQuery);

PNode subPlanSelectStmt(PPlanProcessor planProcessor);

PNode subPlanInsertStmt(PPlan plan, PQuery subQuery);

PNode GetValuesTypeRangTbl(PList rangTblList);

PNode ProcessQual(PPlanProcessor planProcess);

PNode ProcessJoinTree(PPlanProcessor planProcess);

PNode ProcessScanNode(PPlanProcessor planProcess);

PNode ProcessMergerNode(PPlanProcessor planProcess);

PNode ProcessScanNode(PPlanProcessor planProcess);

PNode ProcessJoinNode(PPlanProcessor planProcess);
#endif 