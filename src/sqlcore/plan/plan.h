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

#ifndef HAT_PLAN_H_H
#define HAT_PLAN_H_H

#include "node.h"
#include "queryNode.h"
#include "planNode.h"

typedef struct PlanProcessor
{
    PPlan plan;            /* 生成的执行计划 */
    PQuery query;          /* 逻辑执行计划 */
    PQuery subQuery;       /* 子逻辑计划树 */
    PNode  currentNode;
    PNode rtableJoinList;
}PlanProcessor, *PPlanProcessor;

PList QueryPlan(PQuery queryTree);

PNode SubPlanProcess(PQuery subQuery);

PNode subPlanSelectStmt(PPlanProcessor planProcessor);

PNode subPlanInsertStmt(PPlan plan, PQuery subQuery);

PNode subPlanUpdateStmt(PPlanProcessor planProcessor);

PNode subPlanDeleteStmt(PPlanProcessor planProcessor);

PNode GetValuesTypeRangTbl(PList rangTblList);

PNode ProcessQual(PPlanProcessor planProcess);

PNode ProcessJoinTree(PPlanProcessor planProcess);

PNode ProcessScanNode(PPlanProcessor planProcess);

PNode ProcessMergerNode(PPlanProcessor planProcess);

PNode ProcessScanNode(PPlanProcessor planProcess);

PNode ProcessJoinNode(PPlanProcessor planProcess);
#endif 