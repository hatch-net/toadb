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

#include <stddef.h>
#include <stdio.h>
#include "query.h"
#include "buffer.h"
#include "tables.h"
#include "hatstring.h"
#include "public.h"


int g_queryId = 0;

static int GetQueryId();
static PNode CommonQueryTransform(PQueryState queryState);
static PNode AddNormalTblNode(PRangTblEntry rangTbl, PList targetList);
static PNode AddJoinTblNode(PList left, PRangTblEntry rangTbl, PList targetList);
static int GetNodeRtNum(PNode node);
static PList GetNodeTargetList(PNode node);
static PList MergeNodeTargetList(PNode node1, PNode node2);

static PList TargetRewriteAllColumns(PList targetList, PQueryState queryState);
static PList TargetRewriteAllColumnsOneRte(PList targetList, PRangTblEntry rte);
static PNode SearchColumDef(PNode colRef, PList rangTbl, PColumnDefInfo *colDef);

static PNode SimpleExprNodeProcess(PNode node, PQueryState queryState);
static PList SubExprNodeTargetProcess(PNode node, PQueryState queryState);
static void AddRangTblTargetEntry(PTargetEntry target, PRangTblEntry rte);
static PTargetEntry QueryRangTblTarget(PList targetList, char *colName);
PList MergetTargetList(PList target1, PList target2);

/* 
 * analyze and rewrite, transform parser tree to query tree.
 */
PList QueryAnalyzeAndRewrite(PList parserTree)
{
    PList Query = NULL;
    PQuery subQuery = NULL;
    PListCell tmpCell = NULL;
    PMemContextNode oldContext = NULL;

    oldContext = MemMangerNewContext("queryTree");

    if(NULL == parserTree)
    {
        hat_log("[QueryRewrite]invalid parser tree\n");
        return NULL;
    }

    /* traverse all subParserTree, generator query tree. */
    for(tmpCell = parserTree->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)(tmpCell->value.pValue);
        
        subQuery = SubQueryAnalyzeAndReWrite(node);

        if(NULL != subQuery)
            Query = AppendNode(Query, (PNode)subQuery);
        else
            return NULL;
    }

    MemMangerSwitchContext(oldContext);

    return Query;
}

/*
 * rewrite and logical plan 
*/
PQuery SubQueryAnalyzeAndReWrite(PNode subParserTree)
{
    PQuery subQueryTree = NULL;

    if(NULL == subParserTree)
    {
        hat_log("[SubQueryAnalyzeAndReWrite]invalid parser tree\n");
        return NULL;
    }

    QueryRewrite(subParserTree);

    subQueryTree = transformStmt(subParserTree);
    return subQueryTree;
}

/*
 * TODO:
*/
int QueryRewrite(PNode subParserTree)
{
    return 0;
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
        case T_UpdateStmt:
            query = transformUpdateStmt((PUpdateStmt)parserTree);
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

/*
 * 创建查询逻辑查询计划树 
 * 主要做以下几件事情：
 * 1. 生成基本表列表，来源有from子句，并产生序号供引用；
 * 2. 生成目标结果列，分为两类：顶层targetlist,顶层投影节点使用；每个基本表会有各自的（
 *  来源于targetlist,表达式等,在where处理时会加到基本表中）；
 * 3. 根据基本表生成查询树底层节点；
 * 4. 根据表达式生成中间控制节点；
 * 5. 根据叶子节点的表达式生成下层列节点及表达式；(TODO：)
 */
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
    queryState->processState = 0;
    queryState->rindex = FIRST_RANGTABLE_INDEX;

    /* collect rang table infomation */
    selectQuery->rtable = NULL;
    queryState->qualTargetList = NULL;  /* 条件表达式中需要的属性列 */

    if(NULL != selectStmt->fromList)
        selectQuery->rtable = RangTblRewriteFromClause(queryState, selectStmt->fromList);

    if(NULL == selectQuery->rtable)
    {
        /* TODO: 暂时不支持 “select 表达式; ” 类似语法 */
        hat_error("not support that rtable is null.\n");
        return NULL;
    }

    queryState->rtable = selectQuery->rtable;

    /* 
     * targetlist of parent node, 
     * 将targetEntry与rtable用rindex关联起来, 生成顶层targetlist 
     */
    queryState->parentTargetList = ProcessCheckTargetList(selectStmt->targetlist, queryState);
    selectQuery->targetList = queryState->parentTargetList;    

    /* children node generator. */
    queryState->querylevel = 2;

    /* 
     * generator jointree, which has target list per node, 
     * meanwhile target add to rangtable. 
     */
    queryState->qualTargetList = NULL;
    selectQuery->joinTree = NULL;
    if(NULL != selectStmt->whereList)
    {
        queryState->joinTree = QueryJoinTransform(selectStmt->whereList, queryState);
     
        selectQuery->qualTargetList = queryState->qualTargetList;
        selectQuery->joinTree = queryState->joinTree;
    }  
    
    /* 并将剩余表添加到表达式中 */
    selectQuery->rtjoinTree = (PList)CommonQueryTransform(queryState);

    /* error ocur */
    if(queryState->processState)
    {
        selectQuery = NULL;
    }

    if(NULL != queryState)
    {
        FreeMem(queryState);
    }

    return selectQuery;
}

/*
 * 生成基本表列表
 * 检查基本表信息，并进行编号，以便后面引用 
 */
PList RangTblRewriteFromClause(PQueryState queryState, PList fromList)
{
    PListCell tmpCell = NULL;
    PTableList tblInfo = NULL;
    PList rangTblList = NULL;
    PRangTblEntry rangEntry = NULL;
    int rindex = queryState->rindex;

    rangTblList = NewNode(List);

    /* search table list */
    for(tmpCell = fromList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PRangeVar rangVar = (PRangeVar)(tmpCell->value.pValue);
        
        /* 这里不能使用别名 */
        rangEntry = GetRangTblNode(rangVar->relname);
        if(NULL == rangEntry)
        {
            // TODO: hat_error(); 
            hat_error("range table %s is failure.\n", rangVar->relname);
            return NULL;
        }

        /* 这里主要保留表的别名 */
        rangEntry->rangVar = rangVar; 
        if(rangVar->alias != NULL)
        {
            /* memory free here up to memory context. */
            rangVar->relname = ((PAlias)(rangVar->alias))->aliasname;
        }

        rangEntry->rindex = rindex++;
        rangTblList = AppendNode(rangTblList, (PNode)rangEntry);
    }

    queryState->rindex = rindex;
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
    PList qualTargetList = queryState->qualTargetList;

    queryState->qualTargetList = NULL;

    mergerNode = NewNode(MergerEntry);
    mergerNode->mergeType = boolExprNode->boolop;
    mergerNode->rindex = -1;
    mergerNode->targetList = NULL;
   
    /* left and right subqual, generate target list for each oher. */
    mergerNode->lefttree = QualNodeProcess(GetFirstCellNode(boolExprNode->args), queryState);
    mergerNode->righttree = QualNodeProcess(GetSecondCellNode(boolExprNode->args), queryState);
    mergerNode->isJoin = queryState->isJoin;

    /* targetlist is subnode merger. */
    mergerNode->targetList = queryState->qualTargetList;

    /* restore parent targetlist */
    queryState->qualTargetList = MergeList(qualTargetList, queryState->qualTargetList);

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
            /* as is */
            valueNode = node;
            break;        
    }

    return valueNode;
}

/*
 * 表达式处理，对于嵌套表达式递归调用
 */
PNode ExprNodeProcess(PNode node, PQueryState queryState)
{
    PA_Expr exprNode = (PA_Expr)node;
    PNode leftExpr = NULL, rightExpr = NULL;
    PExprEntry commExprNode = NULL;

    /* left or right is NULL, entire expression is null. */
    if((NULL == exprNode->lexpr) || (NULL == exprNode->rexpr))
    {
        return NULL;
    }

    /* left expre process */
    if((T_ColumnRef != exprNode->lexpr->type) && (T_ConstValue != exprNode->lexpr->type))
    {
        leftExpr = QualNodeProcess(exprNode->lexpr, queryState);
    }
    else
    {
        leftExpr = exprNode->lexpr;
    }

    /* right expre process */
    if((T_ColumnRef != exprNode->rexpr->type) && (T_ConstValue != exprNode->rexpr->type))
    {
        rightExpr = QualNodeProcess(exprNode->rexpr, queryState);
    }
    else
    {
        rightExpr = exprNode->rexpr;
    }

    /* common expr */
    commExprNode = NewNode(ExprEntry);
    commExprNode->op = exprNode->exprOpType;
    commExprNode->lefttree = leftExpr;
    commExprNode->righttree = rightExpr;

    commExprNode = (PExprEntry)SimpleExprNodeProcess((PNode)commExprNode, queryState);
    return (PNode)commExprNode;;
}


/* 
 * 1.检查表达式中列对应的基本表是否存在，这里会用别名查找；
 * 2.生成表达式所需要qualtarget; 
 * 3.转换表达式为代数表达式；
 * 这里需要区分表达式中是两表的连接条件，还是只涉及到一张表；
 * 如果涉及两张表时，需要创建joinExpr节点，如果是一张表时，创建Expr节点；
 */
static PNode SimpleExprNodeProcess(PNode node, PQueryState queryState)
{
    PList targetList = NULL;
    PList qualTargetList = NULL;
    PExprEntry commExprNode = NULL;
    PNode tempExpr = NULL;
    PList oriTargetList = NULL;
    int lrindex = -1, rrindex = -1;

    commExprNode = (PExprEntry)node;
    oriTargetList = queryState->qualTargetList;
    

    /* process left and right expr */
    queryState->qualTargetList = NULL;
    qualTargetList = SubExprNodeTargetProcess(commExprNode->lefttree, queryState);
    if(NULL != qualTargetList)
    {
        lrindex = queryState->rindex;
        commExprNode->rtNum++;
    }

    queryState->qualTargetList = NULL;
    targetList = SubExprNodeTargetProcess(commExprNode->righttree, queryState);
    if(NULL != targetList)
    {
        qualTargetList = MergetTargetList(qualTargetList, targetList);
        rrindex = queryState->rindex;
        commExprNode->rtNum++;
    }  

    /* 生成节点 */
    if((lrindex < 0) && (rrindex < 0))
    {
        /* TODO: double const expr or NULL expr, example :1==2 
         * compute result directly
        */
        ;
    }
    else if(lrindex < 0)
    {
        /* right is column expr */
        tempExpr = commExprNode->lefttree;
        commExprNode->lefttree = commExprNode->righttree;
        commExprNode->righttree = tempExpr;
        commExprNode->rindex = rrindex;
        commExprNode->rtNum = 1;
    }
    else if((rrindex < 0) || (lrindex == rrindex))
    {
        /* left is column expr */
        commExprNode->rindex = lrindex;
        commExprNode->rtNum = 1;
    }
    else 
    {
        /* join node */
        commExprNode->rindex = lrindex;
        commExprNode->rrindex = rrindex;
        commExprNode->rtNum = 2;
    }

    commExprNode->targetList = qualTargetList;

    /* 合并各表达式的targetlist , 这里合并到原始记录的 oritargetlist. */
    queryState->qualTargetList = MergetTargetList(oriTargetList, qualTargetList);

    return (PNode)commExprNode;
}

/*
 * 条件表达式中产生的目标列
 */
static PList SubExprNodeTargetProcess(PNode node, PQueryState queryState)
{
    PNode targetNode = NULL;
    PList targetList = NULL;
    
    if(NULL == node)
        return NULL;

    switch(node->type)
    {
        case T_ColumnRef:
        targetNode = TargetNodeProcess(node, queryState);
        if(NULL != targetNode)
        {
            targetList = AppendNode(targetList, targetNode);
        }
        break;

        case T_ExprEntry:
        /* target list is added to queryState->qualTargetList */
        break;

        case T_ConstValue:
        break;

        default:
        break;
    }
    return targetList;
}

/* 
 * generate targetEntry for columnRef  
 * Checking it valid, which is referenced by rte;
 * node type may be column or restarget.
 */
PNode TargetNodeProcess(PNode node, PQueryState queryState)
{
    PRangTblEntry rte = NULL;
    PTargetEntry target = NULL;
    PListCell tmpCell = NULL;
    PResTarget resTarget = (PResTarget)node;
    PColumnRef columnRefNode = (PColumnRef)resTarget->val;
    PColumnDefInfo colDef = NULL;

    /* There are two type here, T_ColumnRef and  T_ResTarget */
    if(T_ColumnRef == node->type)
    {
        /* create ResTarget structure. */
        columnRefNode = (PColumnRef)node;

        resTarget = NewNode(ResTarget);
        resTarget->val = node;
        resTarget->indirection = NULL;
        resTarget->all = 0;
        resTarget->name = columnRefNode->field;
    }

    /* column which is owner by table. */
    if(T_ColumnRef != columnRefNode->type)
    {
        /* TODO: May be expre, not support. */
        hat_log("olny support column target.\n");
        return NULL;
    }

    /* 根据表名找到对应的表定义, 这里列的表名可能是别名 */
    rte = (PRangTblEntry)SearchColumDef((PNode)columnRefNode, queryState->rtable, &colDef);
    if(NULL == rte)
    {
        hat_error("column %s is not found, maybe check first.\n", columnRefNode->field);
        queryState->processState = -1;
        return NULL;
    }

    /* used to client show */
    columnRefNode->vt = colDef->type;
    columnRefNode->attrIndex = colDef->attrIndex;

    /* 最后查询结果的列名，如果没有别名，就以表列名显示 */
    if(resTarget->name == NULL)
        resTarget->name = columnRefNode->field;
    
    target = NewNode(TargetEntry);
    target->colRef = (PNode)resTarget;
    target->rindex = rte->rindex;
    target->attrIndex = colDef->attrIndex;    
    
    queryState->rindex = target->rindex;

    /* 增加target 到基本表上 */
    AddRangTblTargetEntry(target, rte);

    if(queryState->querylevel > 1)
        rte->isScaned = 1;
    else
        rte->isNeeded = 1;

    return (PNode)target;
}

/* 
 * Add one targetEntry to rang table targetList.
 * maybe already in the list, when this target is ignored.
 */
static void AddRangTblTargetEntry(PTargetEntry target, PRangTblEntry rte)
{
    PTargetEntry tempTarget = NULL;
    PResTarget resTarget = NULL;

    if((NULL == target) || (NULL == rte))
        return ;

    if(NULL != rte->targetList)
    {
        /* check repeat */
        resTarget = (PResTarget)target->colRef;
        tempTarget = QueryRangTblTarget(rte->targetList, resTarget->name);

        if((NULL != tempTarget) && (tempTarget->rindex == rte->rindex))
            return ;
    }

    /* 增加target 到基本表上 */
    rte->targetList = AppendNode(rte->targetList, (PNode)target);
}

/* 
 * query targetEntry by colName.
 * colName maybe is unique identified, which is colName defined, or it can be alias colName.
 */
static PTargetEntry QueryRangTblTarget(PList targetList, char *colName)
{
    PListCell tmpCell = NULL;
    PTargetEntry te = NULL;
    PResTarget resTarget = NULL;

    if((NULL == targetList) || (NULL == colName))
        return NULL;

    for(tmpCell = targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        te = (PTargetEntry)GetCellNodeValue(tmpCell);
        resTarget = (PResTarget)te->colRef;
        
        if(hat_strcmp(colName, resTarget->name, NAME_MAX_LEN) == 0)
            return te;
    }

    return NULL;
}

/* 
 * 在基本表中找到引用信息，然后查找表定义；
 * 如果该列在多个表中有定义，返回 -2，有歧义产生；
 * 如果查找成功，返回rindex, colDef为列定义；
 * 如果找不到，返回-1；
 */
static PNode SearchColumDef(PNode colRef, PList rangTbl, PColumnDefInfo *colDef)
{
    PColumnDefInfo col = NULL;
    PColumnDefInfo firstCol = NULL;
    
    PRangTblEntry rte = NULL;
    PRangTblEntry firstRte = NULL;

    PRangeVar rangVar = NULL;
    PListCell tmpCell = NULL;
    PColumnRef columnRefNode = (PColumnRef)colRef;
    int findAll = 0;
    int findCnt = 0;

    if((rangTbl == NULL) || (colRef == NULL))
    {
        return NULL;
    }

    /* 是否指定了表名，如果没有指定表名，需要在所有基本表中查找 */
    if(columnRefNode->tableName == NULL)
    {
        findAll = 1;
    }

    for(tmpCell = rangTbl->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        rte = (PRangTblEntry)(tmpCell->value.pValue);
        
        if(!findAll)
        {
            /* 需要使用别名查找到rte,然后再找对应的表元数据 */
            rangVar = rte->rangVar;
            if(hat_strcmp(rangVar->relname, columnRefNode->tableName, NAME_MAX_LEN) != 0)
            {
                /* not found */
                continue;
            }
        }

        col = GetAttrDef(rte->tblInfo, columnRefNode->field);

        /* found */
        if(NULL != col)
        {
            findCnt++; 
            firstCol = col;
            firstRte = rte;
        }

        if(findCnt > 1)
        {
            hat_log("column %s ambiguous.\n", columnRefNode->field);
            firstCol = NULL;
            break;
        }
    }

    if(firstCol == NULL)
        return NULL;
    
    *colDef = firstCol;
    return (PNode)firstRte;
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
 * TODO: * transform to all column
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
        
        if(NULL != resTarget->val)
        {
            targetNode = TargetNodeProcess((PNode)resTarget, queryState);
            if(NULL == targetNode)
                break;

            targetEntryList = AppendNode(targetEntryList, targetNode);
        }        
        else if(resTarget->all)
        {
            /* It is all column, We rewrited is. */
            targetEntryList = TargetRewriteAllColumns(targetEntryList, queryState);
        }
        else 
        {
            targetEntryList = NULL;
            break;
        }

        /* case for update set right of =, which will be id = id+1 */
        if(NULL != resTarget->setValue)
        {
            resTarget->setValue = QualNodeProcess(resTarget->setValue, queryState);
        }
    }

    return targetEntryList;
}

static PList TargetRewriteAllColumns(PList targetList, PQueryState queryState)
{
    PColumnRef columnRefNode = NULL;
    PRangTblEntry rte = NULL;
    PTargetEntry target = NULL;
    PListCell tmpCell = NULL;
    PResTarget resTarget = NULL;
    
    int colNum = 0;

    /* search tables */
    for(tmpCell = queryState->rtable->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        rte = (PRangTblEntry)(tmpCell->value.pValue);
        
        targetList = TargetRewriteAllColumnsOneRte(targetList, rte);

        /* querylevel = 1时，在处理顶层target */
        if(queryState->querylevel > 1)
            rte->isScaned = 1;
        else
            rte->isNeeded = 1;  
    }
  
    return targetList;
}

static PList TargetRewriteAllColumnsOneRte(PList targetList, PRangTblEntry rte)
{
    PColumnRef columnRefNode = NULL;
    PTargetEntry target = NULL;
    PResTarget resTarget = NULL;    
    int colNum = 0;

    if((NULL == rte) || (NULL == rte->tblInfo))
        return targetList;

    for(colNum = 0; colNum < rte->tblInfo->tableDef->colNum; colNum++)
    {
        columnRefNode = NewNode(ColumnRef);
        columnRefNode->field = rte->tblInfo->tableDef->column[colNum].colName;
        columnRefNode->vt = rte->tblInfo->tableDef->column[colNum].type;

        resTarget = NewNode(ResTarget);
        resTarget->val = (PNode)columnRefNode;
        resTarget->name = columnRefNode->field;
        resTarget->indirection = NULL;
        resTarget->all = 0;

        target = NewNode(TargetEntry);
        target->colRef = (PNode)resTarget;
        target->rindex = rte->rindex;
        target->attrIndex = rte->tblInfo->tableDef->column[colNum].attrIndex;

        targetList = AppendNode(targetList, (PNode)target);
    }         
  
    return targetList;
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
    queryState->rindex = FIRST_RANGTABLE_INDEX;
    queryState->parserTree = (PNode)insertStmt;

    /* range table list, which generated by target table of insert stmt. */
    rtableNode = GetRangTblNode(insertStmt->tableName);
    if(NULL == rtableNode)
    {
        return NULL;
    }
    rtableNode->rindex = queryState->rindex++;    
    queryState->rtable = AppendNode(queryState->rtable, (PNode)rtableNode);

    /* target list, queryState->parentTargetList is same as targetlist. */
    queryState->tblInfo = rtableNode->tblInfo;
    query->targetList = ProcessAttrList(insertStmt->attrNameList, queryState);

    /* main table targetlist */
    rtableNode->targetList = queryState->parentTargetList;

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
        // TODO: hat_error(); 
        hat_error("table %s is not found.\n", tableName);
        return NULL;
    }

    rangEntry = NewNode(RangTblEntry);
    rangEntry->tblInfo = tblInfo;
    rangEntry->relkind = RELKIND_COMMON;
    rangEntry->ValueList = NULL;
    rangEntry->isProExpr = 0;
    rangEntry->isNeeded = 0;
    rangEntry->isScaned = 0;

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
    rangEntry->targetList = queryState->parentTargetList;
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
        // TODO: hat_error(); 
        hat_error("table is not found.\n");
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
            hat_log("attribute Name %s is not found\n", attrNameNode->attrName);
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

/*
 * range table check, which is not referenced by qual, but references by target list..
 * 分配target到每个表中，并将剩余表添加到表达式中。
 */
static PNode CommonQueryTransform(PQueryState queryState)
{
    PListCell tmpCell = NULL;
    PRangTblEntry rangTbl = NULL;

    PList rangTblList = queryState->rtable;
    PList joinTree = NULL;
    PList targetList = queryState->parentTargetList;
    PList qualtargetList = queryState->qualTargetList;
    PNode node = NULL;

    if((NULL == rangTblList) || (NULL == targetList))
        return (PNode)joinTree;

    /* No scanned table search from rang table. */
    for(tmpCell = rangTblList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        rangTbl = (PRangTblEntry)GetCellNodeValue(tmpCell);
        
        /* 
         * 将target 分配给每张表，优先分配qualtarget，如果没有将parentTarge分配; 
         */
        if(rangTbl->isScaned)
        {
            rangTbl->targetList = GetTblTargetList(qualtargetList, rangTbl->rindex);
        }
        else if(rangTbl->isNeeded)
        {
            rangTbl->targetList = GetTblTargetList(targetList, rangTbl->rindex);
        }   
        else
        {
            /* 两者都不需要，需要补充此表的所有列 */
            rangTbl->targetList = TargetRewriteAllColumnsOneRte(rangTbl->targetList, rangTbl);
        }       

        /* TODO: table order, qual first */
        joinTree = (PList)AddJoinTblNode(joinTree, rangTbl, rangTbl->targetList);
    }

    /* TODO: joinTree nothing todo? */
    return (PNode)joinTree;
}


static PNode AddNormalTblNode(PRangTblEntry rangTbl, PList targetList)
{
    PExprEntry expr = NewNode(ExprEntry);

    expr->rindex = rangTbl->rindex;
    expr->targetList = targetList;
    expr->isScan = 0;
    expr->lefttree = NULL;
    expr->righttree = NULL;
    expr->rtNum = 1;
    return (PNode)expr;
}

/* 
 * 生成Scan 树，左二叉树，也就是左侧是树，每层的只有一个右子节点，顶层没有或只有一个右子节点；
 * 原来的Scan树，会变成后加入节点的子树，也就是后加节点为root；或者当前树的根的右子节点
 */
static PNode AddJoinTblNode(PList left, PRangTblEntry rangTbl, PList targetList)
{
    PNode node = NULL;
    PJoinEntry joinNode = NULL;
    PList joinList = NULL;
    int isSpace = 0;
   
    if(NULL != left)
    {
        node = GetFirstCellNode(left);
        joinNode = (PJoinEntry)node;
        if((T_JoinEntry == node->type) && (NULL == joinNode->righttree))
        {
            isSpace = 1;
        }
    }

    /* left and right subqual */
    if(NULL == left)
    {
        /* first node */
        node = AddNormalTblNode(rangTbl, targetList);
        joinList = AppendNode(joinList, node);
    }
    else if(!isSpace)
    {
        joinNode = NewNode(JoinEntry);
    
        joinNode->joinOp = AND_EXPR;
        joinNode->rindex = -1;

        joinNode->lefttree = node;
        joinNode->righttree = AddNormalTblNode(rangTbl, targetList);
        joinNode->isJoin = 1;

        joinNode->rtNum = GetNodeRtNum(joinNode->lefttree) + GetNodeRtNum(joinNode->righttree);
        joinNode->targetList = MergeNodeTargetList(joinNode->lefttree, joinNode->righttree);
        
        joinList = AppendNode(joinList, (PNode)joinNode);
    }
    else
    {
        /* right node is null */
        joinNode = (PJoinEntry)GetFirstCellNode(left);

        joinNode->righttree = AddNormalTblNode(rangTbl, targetList);    
        joinNode->targetList =  MergeList(joinNode->targetList, targetList);

        joinList = left;
    }
    
    return (PNode)joinList;
}


PList GetTblTargetList(PList targetList, int rindex)
{
    PList tblTargetList = NULL;
    PListCell tmpCell = NULL;
    PTargetEntry te = NULL;

    if(NULL == targetList)
        return NULL;

    for(tmpCell = targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        te = (PTargetEntry)GetCellNodeValue(tmpCell);
        
        if(rindex == te->rindex)
        {
            tblTargetList = AppendNode(tblTargetList, (PNode)te);
        }
    }

    return tblTargetList;
}

PQuery transformUpdateStmt(PUpdateStmt updateStmt)
{
    PQuery query = NULL;
    PQueryState queryState = NULL;    
    PRangTblEntry rtableNode = NULL;
    PRangeVar rangVar = NULL;
    PList rangTblList = NULL;

    /* create query */
    query = NewNode(Query);
    query->commandType = CMD_UPDATE;
    query->queryId = GetQueryId();
    
    queryState = (PQueryState)AllocMem(sizeof(QueryState));
    queryState->queryId = query->queryId;
    queryState->querylevel = 1;
    queryState->isJoin = 0;
    queryState->rtable = NULL;
    queryState->rindex = FIRST_RANGTABLE_INDEX;
    queryState->parserTree = (PNode)updateStmt;

    /* range table list, which generated by target table of update stmt. */
    if(NULL != updateStmt->fromList)
        rangTblList = RangTblRewriteFromClause(queryState, updateStmt->fromList);
    
    rangVar = (PRangeVar)updateStmt->relation;
    rtableNode = GetRangTblNode(rangVar->relname);
    if(NULL == rtableNode)
    {
        return NULL;
    }

    /* 这里主要保留表的别名 */
    rtableNode->rangVar = (PRangeVar)updateStmt->relation; 
    if(rangVar->alias != NULL)
    {
        /* memory free here up to memory context. */
        rangVar->relname = ((PAlias)(rangVar->alias))->aliasname;
    }

    rtableNode->rindex = queryState->rindex++; 
    
    /* add result table to the first of rang table list. */
    queryState->rtable = AppendNode(queryState->rtable, (PNode)rtableNode);
    queryState->rtable = MergeList(queryState->rtable, rangTblList);
    query->rtable = queryState->rtable;

    /* 
     * set value list
     * targetlist of parent node, 
     * 将targetEntry与rtable用rindex关联起来, 生成顶层targetlist 
     */
    queryState->parentTargetList = ProcessCheckTargetList(updateStmt->targetlist, queryState);
    query->targetList = queryState->parentTargetList;    

    /* children node generator. */
    queryState->querylevel = 2;

    /* 
     * generator jointree, which has target list per node, 
     * meanwhile target add to rangtable. 
     */
    queryState->qualTargetList = NULL;
    query->joinTree = NULL;
    if(NULL != updateStmt->whereList)
    {
        queryState->joinTree = QueryJoinTransform(updateStmt->whereList, queryState);
     
        query->qualTargetList = queryState->qualTargetList;
        query->joinTree = queryState->joinTree;
    }  
    
    /* 并将剩余表添加到表达式中 */
    query->rtjoinTree = (PList)CommonQueryTransform(queryState);

    /* error ocur */
    if(queryState->processState)
    {
        query = NULL;
    }

    if(NULL != queryState)
    {
        FreeMem(queryState);
    }
    return query;
}

/* 
 *  This merget two targetList into one list,
 * And targetEntry is repeated, save only one in list.
 * targetEntry unique identify is rindex and colName.
 */
PList MergetTargetList(PList target1, PList target2)
{
    foreachWithSize_define_Head;
    PList result = NULL;
    PTargetEntry te = NULL;
    PTargetEntry resultTE = NULL;
    PResTarget resTarget = NULL;

    if((target1 == NULL) && (target2 == NULL))
        return NULL;

    /* new list */
    result = MergeList(target1, NULL);

    if(target2 != NULL)
    {
        /* check repeat, and append target2 to result List. */
        foreachWithSize(target2, tmpCell, listLen)
        {
            te = (PTargetEntry)GetCellNodeValue(tmpCell);
            resTarget = (PResTarget)te->colRef;

            resultTE = QueryRangTblTarget(result, resTarget->name);
            if(NULL != resultTE && (resultTE->rindex == te->rindex))
            {
                continue;
            }

            result = AppendNode(result, (PNode)te);
        }
    }

    return result;
}

static int GetNodeRtNum(PNode node)
{
    int rtNum = 0;

    switch(node->type)
    {
        case T_ExprEntry:
            rtNum = ((PExprEntry)node)->rtNum;
            break;
        case T_JoinEntry:
            rtNum = ((PJoinEntry)node)->rtNum;
            break;
        case T_MergerEntry:
            rtNum = ((PMergerEntry)node)->rtNum;
            break;
        default:
            break;
    }
    return rtNum;
}

static PList GetNodeTargetList(PNode node)
{
    PList targetList = NULL;

    switch(node->type)
    {
        case T_ExprEntry:
            targetList = ((PExprEntry)node)->targetList;
            break;
        case T_JoinEntry:
            targetList = ((PJoinEntry)node)->targetList;
            break;
        case T_MergerEntry:
            targetList = ((PMergerEntry)node)->targetList;
            break;
        default:
            break;
    }
    return targetList;
}

static PList MergeNodeTargetList(PNode node1, PNode node2)
{
    PList mergeTarget = NULL;
    PList targetList1 = NULL, targetList2 = NULL;

    targetList1 = GetNodeTargetList(node1);
    targetList2 = GetNodeTargetList(node2);

    mergeTarget = MergetTargetList(targetList1, targetList2);
    return mergeTarget;
}