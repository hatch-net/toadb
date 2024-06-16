/*
 *	toadb query 
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

#ifndef HAT_QUERY_H_H
#define HAT_QUERY_H_H

#include "node.h"
#include "queryNode.h"

#define FIRST_RANGTABLE_INDEX 1

typedef struct QueryState
{
    int queryId;
    int querylevel;                 /* querylevel = 1时，在处理顶层target；其它值时，处理表达式层次中的target */
    int isJoin;
    int rindex;
    PList rtable;
    PList parentTargetList;         /* 最终顶层结果需要的属性列 */
    PList qualTargetList;           /* 条件表达式中需要的属性列 */
    PList joinTree;
    PList rtjoinTree;
    PNode parserTree;
    PTableList tblInfo;     
    int processState;
}QueryState, *PQueryState;

PList QueryAnalyzeAndRewrite(PList parserTree);

PQuery SubQueryAnalyzeAndReWrite(PNode subParserTree);

int QueryRewrite(PNode subParserTree);

PQuery transformStmt(PNode parserTree);

PQuery transformSelectStmt(PSelectStmt selectStmt);

PQuery transformUpdateStmt(PUpdateStmt updateStmt);

/* DDL process */
PQuery transformUtilityStmt(PNode parser);

PList RangTblRewriteFromClause(PQueryState queryState, PList fromList);

PList QueryJoinTransform(PList qual, PQueryState queryState);

PNode MergerNodeProcess(PNode node, PQueryState queryState);

PNode QualNodeProcess(PNode node, PQueryState queryState);

PNode ExprNodeProcess(PNode node, PQueryState queryState);

PNode TargetNodeProcess(PNode node, PQueryState queryState);

PList ProcessCheckTargetList(PList targetList, PQueryState queryState);

PQuery transformInsertStmt(PInsertStmt insertStmt);

PList RangTblRewriteValuseClause(PList valuesList, PQueryState queryState);

PRangTblEntry GetRangTblNode(char *tableName);

PList GetTblTargetList(PList targetList, int rindex);

PList ProcessAttrList(PList attrList, PQueryState queryState);
#endif
