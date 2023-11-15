/*
 *	toadb query 
 * Copyright (C) 2023-2023, senllang
*/

#include <stddef.h>
#include <stdio.h>
#include "query.h"
#include "buffer.h"
#include "tables.h"

#define log printf
#define error printf

int g_queryId = 0;

static int GetQueryId();

/* 
 * analyze and rewrite, transform parser tree to query tree.
 */
PList QueryAnalyzeAndRewrite(PList parserTree)
{
    PList Query = NULL;
    PQuery subQuery = NULL;
    PListCell tmpCell = NULL;

    if(NULL == parserTree)
    {
        log("[QueryRewrite]invalid parser tree\n");
        return NULL;
    }

    /* traverse all subParserTree, generator query tree. */
    for(tmpCell = parserTree->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)(tmpCell->value.pValue);
        
        subQuery = SubQueryAnalyzeAndReWrite(node);

        if(NULL != subQuery)
            Query = AppendNode(Query, (PNode)subQuery);
    }

    return Query;
}

PQuery SubQueryAnalyzeAndReWrite(PNode subParserTree)
{
    PQuery subQueryTree = NULL;

    if(NULL == subParserTree)
    {
        log("[SubQueryAnalyzeAndReWrite]invalid parser tree\n");
        return NULL;
    }

    subQueryTree = transformStmt(subParserTree);
    return subQueryTree;
}

PQuery transformStmt(PNode parserTree)
{
    PQuery query = NULL;

    switch(parserTree->type)
    {
        case T_SelectStmt:
            query = transformSelectStmt((PSelectStmt)parserTree);
            break;
        case T_InsertStmt:
            query = transformInsertStmt((PInsertStmt)parserTree);
            break;
        case T_CreateStmt:
        case T_DropStmt:
            query = transformUtilityStmt(parserTree);
            break;
        default:
            break;
    }

    return query;
}

PQuery transformSelectStmt(PSelectStmt selectStmt)
{
    PQuery selectQuery = NULL;
    PQueryState queryState = NULL;    

    /* create query */
    selectQuery = NewNode(Query);
    selectQuery->commandType = CMD_SELECT;
    selectQuery->queryId = GetQueryId();
    
    queryState = (PQueryState)AllocMem(sizeof(QueryState));
    queryState->queryId = selectQuery->queryId;
    queryState->querylevel = 1;
    queryState->isJoin = 0;

    /* collect rang table infomation */
    selectQuery->rtable = NULL;
    if(NULL != selectStmt->fromList)
        selectQuery->rtable = RangTblRewriteFromClause(selectStmt->fromList);

    queryState->rtable = selectQuery->rtable;

    /* targetlist of parent node */
    queryState->parentTargetList = ProcessCheckTargetList(selectStmt->targetlist, queryState);
    selectQuery->targetList = queryState->parentTargetList;    

    /* generator jointree, which has target list per node. */
    selectQuery->joinTree = QueryJoinTransform(selectStmt->whereList, queryState);

    if(NULL != queryState)
    {
        FreeMem(queryState);
    }

    return selectQuery;
}

PList RangTblRewriteFromClause(PList fromList)
{
    PListCell tmpCell = NULL;
    PTableList tblInfo = NULL;
    PList rangTblList = NULL;
    PRangTblEntry rangEntry = NULL;
    int rindex = 1;

    rangTblList = NewNode(List);

    /* search table list */
    for(tmpCell = fromList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PRangeVar rangVar = (PRangeVar)(tmpCell->value.pValue);
        
        rangEntry = GetRangTblNode(rangVar->relname);
        if(NULL == rangEntry)
        {
            // TODO: error(); 
            error("range table %s is failure.\n", rangVar->relname);
            return NULL;
        }

        rangEntry->rindex = rindex++;
        rangTblList = AppendNode(rangTblList, (PNode)rangEntry);
    }

    return rangTblList;
}

static int GetQueryId()
{
    return g_queryId++;
}

/*
 * transform qualifications to query nodes. 
 * per node has a target list, and operator id which is mapping process function of types.
 */
PList QueryJoinTransform(PList qual, PQueryState queryState)
{
    PList joinTree = NULL;
    PListCell tmpCell = NULL;
    PNode node = NULL;

    if(NULL == qual)
    {
        return NULL;
    }

    /* search qual list */
    for(tmpCell = qual->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode valueNode = (PNode)(tmpCell->value.pValue);

        node = QualNodeProcess(valueNode, queryState);

        if(NULL != node)
            joinTree = AppendNode(joinTree, node);
    }

    return joinTree;
}

PNode MergerNodeProcess(PNode node, PQueryState queryState)
{
    PBoolExpr boolExprNode = (PBoolExpr)node;
    PMergerEntry mergerNode = NULL;
    PList parentTagetList = queryState->parentTargetList;

    mergerNode = NewNode(MergerEntry);
    mergerNode->mergeType = boolExprNode->boolop;
    mergerNode->rindex = -1;
    mergerNode->targetList = NULL;
   
    /* next level has not target list. */
    queryState->parentTargetList = NULL;

    /* left and right subqual */
    mergerNode->lefttree = QualNodeProcess(GetFirstCellNode(boolExprNode->args), queryState);
    mergerNode->righttree = QualNodeProcess(GetSecondCellNode(boolExprNode->args), queryState);
    mergerNode->isJoin = queryState->isJoin;

    /* targetlist is same as parent. */
    mergerNode->targetList = parentTagetList;

    /* restore parent targetlist */
    queryState->parentTargetList = parentTagetList;

    return (PNode)mergerNode;
}

PNode QualNodeProcess(PNode node, PQueryState queryState)
{
    PNode valueNode = NULL;

    if(NULL == node)
        return NULL;
    
    switch(node->type)
    {
        case T_BoolExpr:
                /* replace to merger node */
                valueNode = MergerNodeProcess(node, queryState);                
            break;
        case T_A_Expr:
                /* replace to JoinEntry node or ExprEntry node */
                valueNode = ExprNodeProcess(node, queryState);
            break;
        default:
            valueNode = NULL;
            break;        
    }

    return valueNode;
}

PNode ExprNodeProcess(PNode node, PQueryState queryState)
{
    PA_Expr exprNode = (PA_Expr)node;
    PJoinEntry joinNode = NULL;
    PExprEntry commExprNode = NULL;
    PNode targetNode = NULL;

    /* judge join expr or common */
    if((NULL != exprNode->lexpr) && (T_ColumnRef == ((PNode)(exprNode->lexpr))->type)
        && (NULL != exprNode->rexpr) && (T_ColumnRef == ((PNode)(exprNode->rexpr))->type))
    {
        /* join qualication */
        joinNode = NewNode(JoinEntry);

        joinNode->isJoin = 1;
        queryState->isJoin = joinNode->isJoin;

        joinNode->joinOp = exprNode->exprOpType;
        joinNode->lefttree = exprNode->lexpr;
        joinNode->righttree = exprNode->rexpr;                  

        /* two columns, maybe two or one tables */
        targetNode = TargetNodeProcess(exprNode->lexpr, queryState);
        if(NULL != targetNode)
            joinNode->targetList = AppendNode(joinNode->targetList, targetNode);

        targetNode = TargetNodeProcess(exprNode->rexpr, queryState);
        if(NULL != targetNode)
            joinNode->targetList = AppendNode(joinNode->targetList, targetNode);

        joinNode->rindex = -1;

        return (PNode)joinNode;
    }

    /* common expr */
    commExprNode = NewNode(ExprEntry);
    commExprNode->op = exprNode->exprOpType;

    commExprNode->lefttree = exprNode->lexpr;
    commExprNode->righttree = exprNode->rexpr;

    /* target list */
    targetNode = TargetNodeProcess(exprNode->lexpr, queryState);
    if(NULL != targetNode)
        commExprNode->targetList = AppendNode(commExprNode->targetList, targetNode);

    commExprNode->rindex = queryState->rindex;

    return (PNode)commExprNode;
}

/* 
 * generate targetEntry for columnRef  
 */
PNode TargetNodeProcess(PNode node, PQueryState queryState)
{
    PColumnRef columnRefNode = (PColumnRef)node;
    PRangTblEntry rte = NULL;
    PTargetEntry target = NULL;
    PListCell tmpCell = NULL;
    PResTarget resTarget = NULL;
    PColumnDefInfo colDef = NULL;

    /* column which is owner by table. */
    if(NULL == node)
    {
        return NULL;
    }

    /* search tables */
    for(tmpCell = queryState->rtable->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        rte = (PRangTblEntry)(tmpCell->value.pValue);
        colDef = GetAttrDef(rte->tblInfo, columnRefNode->field);

        if(NULL == colDef)
            break; 
    }

    if(NULL == colDef)
        return NULL;

    /* used to client show */
    columnRefNode->vt = colDef->type;
    
    resTarget = NewNode(ResTarget);
    resTarget->val = (PNode)columnRefNode;
    resTarget->name = NULL;
    resTarget->indirection = NULL;
    resTarget->all = 0;

    target = NewNode(TargetEntry);
    target->colRef = (PNode)resTarget;
    target->rindex = rte->rindex;
    queryState->rindex = target->rindex;

    return (PNode)target;
}

PQuery transformUtilityStmt(PNode parser)
{
    PQuery queryUtility = NULL;

    /* create query */
    queryUtility = NewNode(Query);
    queryUtility->commandType = CMD_UTILITY;
    queryUtility->queryId = GetQueryId();
    queryUtility->parserTree = parser;

    return queryUtility;
}

/* 
 * transform TargetList to TargetEntry
 */
PList ProcessCheckTargetList(PList targetList, PQueryState queryState)
{
    PList targetEntryList = NULL;
    PResTarget resTarget = NULL;
    PListCell tmpCell = NULL;
    PNode targetNode = NULL;

    if(NULL == targetList)
    {
        return NULL;
    }

    for(tmpCell = targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        resTarget = (PResTarget)(tmpCell->value.pValue);
        if(NULL == resTarget)
            break;
       
        targetNode = TargetNodeProcess(resTarget->val, queryState);
        if(NULL == targetNode)
            break;

        targetEntryList = AppendNode(targetEntryList, targetNode);
    }

    return targetEntryList;
}


PQuery transformInsertStmt(PInsertStmt insertStmt)
{
    PQuery query = NULL;
    PQueryState queryState = NULL;    
    PRangTblEntry rtableNode = NULL;

    /* create query */
    query = NewNode(Query);
    query->commandType = CMD_INSERT;
    query->queryId = GetQueryId();
    
    queryState = (PQueryState)AllocMem(sizeof(QueryState));
    queryState->queryId = query->queryId;
    queryState->querylevel = 1;
    queryState->isJoin = 0;
    queryState->rtable = NULL;
    queryState->rindex = 0;
    queryState->parserTree = (PNode)insertStmt;

    /* range table list, which generated by target table of insert stmt. */
    rtableNode = GetRangTblNode(insertStmt->tableName);
    if(NULL == rtableNode)
    {
        return NULL;
    }
    rtableNode->rindex = ++queryState->rindex;    
    queryState->rtable = AppendNode(queryState->rtable, (PNode)rtableNode);

    /* target list, queryState->parentTargetList is same as targetlist. */
    queryState->tblInfo = rtableNode->tblInfo;
    query->targetList = ProcessAttrList(insertStmt->attrNameList, queryState);

    /* main table targetlist */
    rtableNode->targetList = (PNode)queryState->parentTargetList;

    /* value list 形成rangtable */
    query->rtable = RangTblRewriteValuseClause(insertStmt->valuesList, queryState);

    return query;
}

/* 
 *  rang table check metadata infomation with table name.
 */
PRangTblEntry GetRangTblNode(char *tableName)
{
    PTableList tblInfo = NULL;
    PRangTblEntry rangEntry = NULL;

    if(NULL == tableName)
        return NULL;

    /* get table information */
    tblInfo = GetTableInfo(tableName);
    if (NULL == tblInfo)
    {
        // TODO: error(); 
        error("table %s is not found.\n", tableName);
        return NULL;
    }

    rangEntry = NewNode(RangTblEntry);
    rangEntry->tblInfo = tblInfo;
    rangEntry->relkind = RELKIND_COMMON;
    rangEntry->ValueList = NULL;

    return rangEntry;
}

PList RangTblRewriteValuseClause(PList valuesList, PQueryState queryState)
{
    PListCell tmpCell = NULL;
    PList rangTblList = NULL;
    PRangTblEntry rangEntry = NULL;

    rangTblList = queryState->rtable;
    rangEntry = NewNode(RangTblEntry);

    rangEntry->ValueList = (PNode)valuesList;
    rangEntry->targetList = (PNode)queryState->parentTargetList;
    rangEntry->rindex = ++ queryState->rindex ;
    rangEntry->relkind = RELKIND_VALUES;

    queryState->rtable = AppendNode(rangTblList, (PNode)rangEntry);    

    return queryState->rtable;
}

PList ProcessAttrList(PList attrList, PQueryState queryState)
{
    PListCell tmpCell = NULL;
    PAttrName attrNameNode = NULL;
    PTableList tblInfo = NULL;
    PTargetEntry target = NULL;
    PResTarget resTarget = NULL;
    PColumnRef columnRefNode = NULL;
    PColumnDefInfo colDef = NULL;
    int index = -1;
    
    tblInfo = queryState->tblInfo;
    if (NULL == tblInfo)
    {
        // TODO: error(); 
        error("table is not found.\n");
        return NULL;
    }

    /* not specify column name, then all values must be supplied. */
    if(NULL == attrList)
    {
        colDef = tblInfo->tableDef->column;
        for(index = 0; index < tblInfo->tableDef->colNum; index ++)
        {
            /* target node */
            columnRefNode = NewNode(ColumnRef);
            columnRefNode->field = colDef[index].colName;

            resTarget = NewNode(ResTarget);
            resTarget->val = (PNode)columnRefNode;
            resTarget->name = NULL;
            resTarget->indirection = NULL;
            resTarget->all = 0;

            target = NewNode(TargetEntry);
            target->colRef = (PNode)resTarget;
            target->rindex = 1;

            queryState->parentTargetList = AppendNode(queryState->parentTargetList, (PNode)target);
        }

        return queryState->parentTargetList;
    }

    for(tmpCell = attrList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        attrNameNode = (PAttrName)(tmpCell->value.pValue);
        index = GetAttrIndex(tblInfo, attrNameNode->attrName);
        if(index < 0)
        {
            /* TODO: ERROR */
            log("attribute Name %s is not found\n", attrNameNode->attrName);
            break;
        }

        /* target node */
        columnRefNode = NewNode(ColumnRef);
        columnRefNode->field = attrNameNode->attrName;

        resTarget = NewNode(ResTarget);
        resTarget->val = (PNode)columnRefNode;
        resTarget->name = NULL;
        resTarget->indirection = NULL;
        resTarget->all = 0;

        target = NewNode(TargetEntry);
        target->colRef = (PNode)resTarget;
        target->rindex = 1;

        queryState->parentTargetList = AppendNode(queryState->parentTargetList, (PNode)target);
    }

    return queryState->parentTargetList;
}
