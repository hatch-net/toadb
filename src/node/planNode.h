/*
 *	toadb planNode 
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
#ifndef HAT_PLAN_NODE_H_H
#define HAT_PLAN_NODE_H_H

#include "node.h"

typedef struct Plan
{
    NodeType    type;
    CmdType     commandType;
    PNode       QueryTree;          /* 当前计划对应的逻辑执行计划 */
    int         planLevel;
    PNode       leftplan;
    PNode       rightplan;
    PNode       subQueryPlan;
}Plan, *PPlan;

typedef struct NestLoop
{
    NodeType    type;
    PNode       leftplan;
    PNode       rightplan;
    PNode       expr;         /* join expr */
    int         isJoin;
    int         mergeType;
    int         rtNum;
    PList       targetList;   /* result columns */
}NestLoop, *PNestLoop;

typedef struct SeqScan
{
    NodeType    type;
    PNode       rangTbl;
    PNode       expr;         /* fliter expr */
    PList       targetList;   /* result columns */
}SeqScan, *PSeqScan;

typedef struct ValueScan
{
    NodeType    type;
    PNode       rangTbl;
    PNode       expr;         /* fliter expr */
    PList       targetList;   /* result columns */
}ValueScan, *PValueScan;

typedef struct ModifyTbl
{
    NodeType    type;
    PNode       leftplan;
    PNode       rightplan;
    PNode       rangTbl;
}ModifyTbl, *PModifyTbl;

typedef struct ProjectTbl
{
    NodeType    type;
    PNode       subplan;
    PList       rtable;
    PList       targetList;   /* result columns */
}ProjectTbl, *PProjectTbl;

typedef struct QueryTbl
{
    NodeType    type;
    PNode       subplan;
    PList       rtable;
    PList       targetList;   /* result columns */
}QueryTbl, *PQueryTbl;

typedef struct MergeResult
{
    NodeType    type;
    PList       subplan;     /* list subnode */
    PList       rtable;
    PList       targetList;   /* result columns */
}MergeResult, *PMergeResult;

typedef struct SelectResult
{
    NodeType    type;
    PList       subplan;     /* list subnode */
    PList       qual;        /* qual list */
    PList       rtable;
    PList       targetList;   /* qual target list */
}SelectResult, *PSelectResult;

typedef struct SelectNewValue
{
    NodeType    type;
    PList       subplan;     /* list subnode */
    PList       qual;        /* qual list */
    PList       rtable;
    PList       targetList;   /* qual target list */
}SelectNewValue, *PSelectNewValue;

#endif