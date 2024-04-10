/*
 *	toadb queryNode 
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

#ifndef HAT_QUERY_NODE_H_H
#define HAT_QUERY_NODE_H_H

#include "node.h"
#include "buffer.h"

typedef struct Query 
{
    NodeType    type;
    CmdType     commandType;
    int         queryId;
    List        *targetList;    /* top target list */
    List        *rtable;        /* rang table list */
    List        *joinTree;      /* fromlist, qual */
    List        *qualTargetList;    /* qual target list */
    List        *rtjoinTree;      /* fromlist, qual */
    PNode       parserTree;     /* raw parse tree */
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
    int relid;
    int relkind;
    int rindex;             /* 索引计数 */
    PNode presTarget;       /* parser 解析的结果 */
    PRangeVar rangVar;      /* parser 解析 */
    PList targetList;       /* 基本表的targetlist */
    PTableList tblInfo;
    PNode ValueList;
    int isScaned;       /* 是否已经被检查过有表达式中需要 */
    int isNeeded;       /* 是否在顶层结果中需要 */
    int isProExpr;      /* 是否加到了逻辑表达式列表中 */
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
    int         rtNum;
    int         rindex;              /* 对应的RTE rindex , 当rtNum = 1 */
    int         rrindex;             /* 对应的RTE rindex , 当rtNum = 2 */
    int         op;
    PNode       lefttree;
    PNode       righttree;
    int         isScan;
}ExprEntry, *PExprEntry;


typedef struct TargetEntry
{
    NodeType	type;
    PNode       colRef;     /* resTarget Node */
    int         rindex;
    int         attrIndex;
}TargetEntry, *PTargetEntry;


#endif