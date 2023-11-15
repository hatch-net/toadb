/*
 *	toadb queryNode 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_QUERY_NODE_H_H
#define HAT_QUERY_NODE_H_H

#include "node.h"
#include "buffer.h"

typedef struct Query 
{
    NodeType    type;
    CmdType     commandType;
    int         queryId;
    List        *targetList;
    List        *rtable;        /* rang table list */
    List        *joinTree;      /* fromlist, qual */
    PNode       parserTree;
}Query, *PQuery;

typedef enum RelationKind
{
    RELKIND_COMMON,
    RELKIND_VALUES,
    RELKIND_MAX
}RelationKind;

/* table infomation */
typedef struct RangTblEntry 
{
    NodeType	type;
    PNode presTarget;       /* parser 解析的结果 */
    int relid;
    int relkind;
    PNode targetList;
    PTableList tblInfo;
    PNode ValueList;
    int rindex;
}RangTblEntry, *PRangTblEntry;

typedef struct MergerEntry
{
    NodeType	type;
    PList       targetList;
    int         rindex;             /* 对应的RTE rindex */
    int         isJoin;
    int         mergeType;
    PNode       lefttree;
    PNode       righttree;
}MergerEntry, *PMergerEntry;

typedef struct JoinEntry
{
    NodeType	type;
    PList       targetList;
    int         rindex;             /* 对应的RTE rindex */
    int         isJoin;
    int         joinOp;
    PNode       lefttree;
    PNode       righttree;
}JoinEntry, *PJoinEntry;


typedef struct ExprEntry
{
    NodeType	type;
    PList       targetList;
    int         rindex;             /* 对应的RTE rindex */
    int         op;
    PNode       lefttree;
    PNode       righttree;
}ExprEntry, *PExprEntry;


typedef struct TargetEntry
{
    NodeType	type;
    PNode       colRef;
    int         rindex;
}TargetEntry, *PTargetEntry;


#endif