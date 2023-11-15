/*
 *	toadb planNode 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_PLAN_NODE_H_H
#define HAT_PLAN_NODE_H_H

#include "node.h"

typedef struct Plan
{
    NodeType    type;
    CmdType     commandType;
    PNode       QueryTree;
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

#endif