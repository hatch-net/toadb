/*
 *	toadb query 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_QUERY_H_H
#define HAT_QUERY_H_H

#include "node.h"
#include "queryNode.h"

typedef struct QueryState
{
    int queryId;
    int querylevel;
    int isJoin;
    int rindex;
    PList rtable;
    PList parentTargetList;
    PList joinTree;
    PNode parserTree;
    PTableList tblInfo;     
}QueryState, *PQueryState;

PList QueryAnalyzeAndRewrite(PList parserTree);

PQuery SubQueryAnalyzeAndReWrite(PNode subParserTree);

PQuery transformStmt(PNode parserTree);

PQuery transformSelectStmt(PSelectStmt selectStmt);

/* DDL process */
PQuery transformUtilityStmt(PNode parser);

PList RangTblRewriteFromClause(PList fromList);

PList QueryJoinTransform(PList qual, PQueryState queryState);

PNode MergerNodeProcess(PNode node, PQueryState queryState);

PNode QualNodeProcess(PNode node, PQueryState queryState);

PNode ExprNodeProcess(PNode node, PQueryState queryState);

PNode JoinQualNodeProcess(PNode node, PQueryState queryState);

PNode CommExprNodeProcess(PNode node, PQueryState queryState);

PNode TargetNodeProcess(PNode node, PQueryState queryState);

PList ProcessCheckTargetList(PList targetList, PQueryState queryState);

PQuery transformInsertStmt(PInsertStmt insertStmt);

PList RangTblRewriteValuseClause(PList valuesList, PQueryState queryState);

PRangTblEntry GetRangTblNode(char *tableName);

PList ProcessAttrList(PList attrList, PQueryState queryState);
#endif
