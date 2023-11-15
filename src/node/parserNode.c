/*
 *	toadb parserNode 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parserNode.h"
#include "queryNode.h"
#include "planNode.h"

#define log printf
#define debug 


/* static function declare */
static void ShowNode(PNode n, char *prompt, int level);
static void TravelListCell(PList list, char *prompt, int level);
static void ShowBlank(int level);

/* 
 * 根据表名获取取 attrnode 
 * list , List *attrNameList, cell is AttrName
 * list 的成员类型为 PAttrName
 * 获取指定attrName在list中的位置序号，从1开始
 */
int GetAtrrIndexByName(char *attrName, PList list)
{
    int index = 0;
    PAttrName attr = NULL;
    PListCell cell = NULL;

    if(NULL == list || NULL == attrName)
        return -1;
    
    cell = list->head;
    for(index = 1; cell != NULL; cell = cell->next, index++)
    {
        attr = (PAttrName)(cell->value.pValue);
        if(strcmp(attrName, attr->attrName) == 0)
        {
            break;
        }
    }

    return index;
}

/* 
 * 根据表名获取取 attrnode 
 * list , List *attrNameList, cell is AttrName
 * list 是targetlist, 它的成员类型为PTargetEntry, 内部节点 PResTarget，内部节点类型为 PColumnRef
 * 获取指定attrName在list中的位置序号，从1开始
 */
int GetColumnRefIndexByName(char *attrName, PList list)
{
    int index = 0, found = 0;
    PColumnRef colRef = NULL;
    PResTarget resTarget = NULL;
    PTargetEntry targetEntry = NULL;
    PListCell cell = NULL;

    if(NULL == list || NULL == attrName)
        return -1;
    
    cell = list->head;
    for(index = 1; cell != NULL; cell = cell->next, index++)
    {
        targetEntry = (PTargetEntry)GetCellNodeValue(cell);
        resTarget = (PResTarget)(targetEntry->colRef);
        colRef = (PColumnRef)resTarget->val;
        if(strcmp(attrName, colRef->field) == 0)
        {
            found = 1;
            break;
        }
    }

    /* not found. */
    if(!found)
        index = -1;
    return index;
}

/* 
 * 根据atrrnode index， 获取对应的values 
 * list , cell is ValuesData type.
 */
PValuesData GetDataByIndex(int index, PList list)
{
    PValuesData data = NULL;
    PListCell cell = NULL;
    int count = 0;

    if(NULL == list)
        return data;
    
    for(cell = list->head; cell != NULL; cell = cell->next)
    {
        count++;
        if(count == index)
        {
            data = (PValuesData)(cell->value.pValue);
            break;
        }        
    }

    return data;
}

static void ShowBlank(int level)
{
    int i = 0;
    for(i = level; i > 0; i--)
    {
        log("  ");
    }
}

static void ShowNodCreateStmt(PNode n, char *prompt, int level)
{
    PCreateStmt createstmt = (PCreateStmt)n;

    ShowBlank(level);
    log("T_CreateStmt Node: tablename:%s \n", createstmt->tableName);

    TravelListCell(createstmt->ColList, "T_CreateStmt ", level+1);
}

static void ShowNodColumnDef(PNode n, char *prompt, int level)
{
    PColumnDef column = (PColumnDef)n;

    ShowBlank(level);
    log("%s columnName:%s type:%s \n", prompt, column->colName, column->colType);
}

static void ShowNodDropStmt(PNode n, char *prompt, int level)
{
    
}

static void ShowNodInsertStmt(PNode n, char *prompt, int level)
{
    PInsertStmt insertstmt = (PInsertStmt)n;

    ShowBlank(level);
    log("{ T_InsertStmt Node: \n");

    ShowBlank(level+1);
    log("table :%s \n", insertstmt->tableName);

    TravelListCell(insertstmt->attrNameList, "attrNameList ", level+1);
    TravelListCell(insertstmt->valuesList, "values", level+1);

    ShowBlank(level);
    log("} %s T_InsertStmt Node end\n", prompt);
}

static void ShowNodAttrName(PNode n, char *prompt, int level)
{
    PAttrName stmt = (PAttrName)n;

    ShowBlank(level);
    log("%s T_AttrName Node: attrname :%s \n", prompt, stmt->attrName);
}

static void ShowNodValuesData(PNode n, char *prompt, int level)
{
    PValuesData stmt = (PValuesData)n;

    ShowBlank(level);
    log("T_ValuesData Node:  \n");

    ShowNode((PNode)stmt->valueNode, "valuesNode", level+1);
}

static void ShowNodSelectStmt(PNode n, char *prompt, int level)
{
    PSelectStmt stmt = (PSelectStmt)n;

    ShowBlank(level);
    log("{  %s T_SelectStmt Node begin: \n", prompt);

    ShowBlank(level+1);
    log("selectAll:%d \n", stmt->selectAll);

    TravelListCell(stmt->targetlist, "targetlist ", level+1);
    TravelListCell(stmt->relrange, "relrange ", level+1);
    TravelListCell(stmt->fromList, "fromList ", level+1);
    TravelListCell(stmt->whereList, "whereList ", level+1);
    TravelListCell(stmt->groupList, "groupList ", level+1);
    TravelListCell(stmt->sortList, "sortList ", level+1);
    TravelListCell(stmt->limitClause, "limitClause ", level+1);

    ShowBlank(level);
    log("}  %s T_SelectStmt Node end:   \n", prompt);
}

static void ShowNodTableRefName(PNode n, char *prompt, int level)
{
    PTableRefName stmt = (PTableRefName)n;

    ShowBlank(level);
    log("%s T_TableRefName Node: table ref name:%s \n", prompt, stmt->tblRefName);
}

static void ShowNodColumnRef(PNode n, char *prompt, int level)
{
    PColumnRef var = (PColumnRef)n;

    ShowBlank(level+1);
    log("%s ColumnRef field: %s \n", prompt, var->field);
}

static void ShowNodResTarget(PNode n, char *prompt, int level)
{
    PResTarget var = (PResTarget)n;

    ShowBlank(level);
    log("{ PResTarget begin: \n");

    ShowBlank(level+1);
    log("name: %s \n", var->name);

    ShowBlank(level+1);
    log("all: %d \n", var->all);

    ShowNode((PNode)var->val, "val", level+1);

    ShowBlank(level);
    log("} PResTarget end : \n");
}

static void ShowNodConstValue(PNode n, char *prompt, int level)
{
    PConstValue var = (PConstValue)n;

    ShowBlank(level);
    log("%s ConstValue: \n", prompt);

    ShowBlank(level);
    log("vt: %d \n", var->vt);

    switch(var->vt)
    {
        case VT_POINTER:
            ShowBlank(level);
            log("string %s\n", var->val.pData);
            break;
	    case VT_INT:
            ShowBlank(level);
            log("int %d\n", var->val.iData);
            break;
	    case VT_CHAR:
            ShowBlank(level);
            log("char %c\n", var->val.cData);
            break;
	    case VT_DOUBLE:
            ShowBlank(level);
            log("double %lf\n", var->val.dData);
            break;
	    case VT_FLOAT:
            ShowBlank(level);
            log("float %f\n", var->val.fData);
            break;
        default:
            ShowBlank(level);
            log("error data type\n");
            break;
    }
    
    ShowBlank(level);
    log("isnull: %d \n", var->isnull);
}

static void ShowNodA_Expr(PNode n, char *prompt, int level)
{
    PA_Expr var = (PA_Expr)n;

    ShowBlank(level);
    log("{ A_Expr begin:  \n");

    ShowBlank(level+1);
    log("name:%s \n", var->name);

    ShowBlank(level+1);
    log("exprType:%d \n", var->exprType);

    ShowBlank(level+1);
    log("left expr  \n");
    ShowNode((PNode)var->lexpr, "left expr", level+1);

    ShowBlank(level+1);
    log("right expr  \n");
    ShowNode((PNode)var->rexpr, "right expr", level+1);

    ShowBlank(level);
    log("}  A_Expr end:  \n");
}

static void ShowNodBoolExpr(PNode n, char *prompt, int level)
{
    PBoolExpr var = (PBoolExpr)n;

    ShowBlank(level);
    log("{ %s BoolExpr begin: \n", prompt);

    ShowBlank(level);
    log("boolop: %d \n", var->boolop);

    TravelListCell(var->args, " ", level+1);

    ShowBlank(level);
    log("} %s BoolExpr end:  \n", prompt);
}

static void ShowNodAlias(PNode n, char *prompt, int level)
{
    PAlias var = (PAlias)n;

    ShowBlank(level);
    log("%s Alias:%s \n", prompt, var->aliasname);
}

static void ShowNodRangeVar(PNode n, char *prompt, int level)
{
    PRangeVar var = (PRangeVar)n;

    ShowBlank(level);
    log("{  RangeVar begin \n");

    ShowBlank(level+1);
    log("relname:%s \n", var->relname);

    ShowNode((PNode)var->alias, prompt, level+1);

    ShowBlank(level);
    log("}  RangeVar end");
}

static void ShowNodQuery(PNode n, char *prompt, int level)
{
    PQuery var = (PQuery)n;

    ShowBlank(level);
    log("{  Query begin \n");

    ShowBlank(level+1);
    log("commandType:%d \n", var->commandType);

    ShowBlank(level+1);
    log("queryId:%d \n", var->queryId);    

    TravelListCell((PList)var->targetList, "targetList", level+1);
    TravelListCell((PList)var->rtable, "rtable", level+1);
    TravelListCell((PList)var->joinTree, "joinTree", level+1);

    ShowBlank(level);
    log("}  Query end\n");
}

static void ShowNodRangTbl(PNode n, char *prompt, int level)
{
    PRangTblEntry var = (PRangTblEntry)n;

    ShowBlank(level);
    log("{  T_RangTblEntry begin \n");

    ShowBlank(level+1);
    log("rindex:%d \n", var->rindex);

    ShowBlank(level+1);
    log("relkind:%d \n", var->relkind);
    
    TravelListCell((PList)var->targetList, "targetList", level+1);
    TravelListCell((PList)var->ValueList, "ValueList", level+1);

    ShowBlank(level);
    log("}  T_RangTblEntry end \n");
}

static void ShowNodJoin(PNode n, char *prompt, int level)
{
    PJoinEntry var = (PJoinEntry)n;

    ShowBlank(level);
    log("{  T_JoinEntry begin \n");

    ShowBlank(level+1);
    log("rindex:%d \n", var->rindex);
    ShowBlank(level+1);
    log("isJoin:%d \n", var->isJoin);

    ShowBlank(level+1);
    log("joinOp:%d \n", var->joinOp);

    TravelListCell((PList)var->targetList, "targetList", level+1);
    ShowNode((PNode)var->lefttree, "lefttree", level+1);
    ShowNode((PNode)var->righttree, "righttree", level+1);

    ShowBlank(level);
    log("}  T_JoinEntry end \n");
}

static void ShowNodMerger(PNode n, char *prompt, int level)
{
    PMergerEntry var = (PMergerEntry)n;

    ShowBlank(level);
    log("{  %s T_MergerEntry begin \n", prompt);

    ShowBlank(level+1);
    log("rindex:%d \n", var->rindex);
    ShowBlank(level+1);
    log("isJoin:%d \n", var->isJoin);

    ShowBlank(level+1);
    log("mergeType:%d \n", var->mergeType);

    TravelListCell((PList)var->targetList, "targetList", level+1);

    ShowNode((PNode)var->lefttree, "lefttree", level+1);

    ShowNode((PNode)var->righttree, "righttree", level+1);

    ShowBlank(level);
    log("} %s T_MergerEntry end \n", prompt);
}

static void ShowNodExprEntry(PNode n, char *prompt, int level)
{
    PExprEntry var = (PExprEntry)n;

    ShowBlank(level);
    log("{  T_ExprEntry begin \n");

    ShowBlank(level+1);
    log("rindex:%d \n", var->rindex);
    ShowBlank(level+1);
    log("op:%d \n", var->op);

    TravelListCell((PList)var->targetList, "targetList", level+1);
    ShowNode((PNode)var->lefttree, "lefttree", level+1);
    ShowNode((PNode)var->righttree, "righttree", level+1);

    ShowBlank(level);
    log("}  T_ExprEntry end \n");
}


static void ShowNodTargetEntry(PNode n, char *prompt, int level)
{
    PTargetEntry var = (PTargetEntry)n;

    ShowBlank(level);
    log("{  T_TargetEntry begin \n");

    ShowBlank(level+1);
    log("rindex:%d \n", var->rindex);

    ShowNode((PNode)var->colRef, "colRef", level+1);

    ShowBlank(level);
    log("}  T_TargetEntry end \n");
}

static void ShowNodPlan(PNode n, char *prompt, int level)
{
    PPlan var = (PPlan)n;

    ShowBlank(level);
    log("{  T_Plan begin \n");

    ShowBlank(level+1);
    log("commandType:%d \n", var->commandType);

    ShowBlank(level+1);
    log("commandType:%d \n", var->planLevel);

    ShowNode((PNode)var->leftplan, "leftplan", level+1);
    ShowNode((PNode)var->rightplan, "rightplan", level+1);

    ShowBlank(level);
    log("}  T_Plan end \n");
}

static void ShowNodNestLoop(PNode n, char *prompt, int level)
{
    PNestLoop var = (PNestLoop)n;

    ShowBlank(level);
    log("{ %s T_NestLoop begin \n", prompt);

    ShowBlank(level+1);
    log("isJoin:%d \n", var->isJoin);

    ShowBlank(level+1);
    log("mergeType:%d \n", var->mergeType);

    ShowNode((PNode)var->leftplan, "leftplan", level+1);
    ShowNode((PNode)var->rightplan, "rightplan", level+1);

    ShowNode((PNode)var->expr, "expr", level+1);
    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    log("} %s T_NestLoop end \n", prompt);
}

static void ShowNodSeqScan(PNode n, char *prompt, int level)
{
    PSeqScan var = (PSeqScan)n;

    ShowBlank(level);
    log("{ %s T_SeqScan begin \n", prompt);

    ShowNode((PNode)var->rangTbl, "rangTbl", level+1);
    ShowNode((PNode)var->expr, "expr", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    log("} %s  T_SeqScan end \n", prompt);
}

static void ShowNodValueScan(PNode n, char *prompt, int level)
{
    PValueScan var = (PValueScan)n;

    ShowBlank(level);
    log("{ %s T_ValueScan begin \n", prompt);

    ShowNode((PNode)var->rangTbl, "rangTbl", level+1);

    ShowNode((PNode)var->expr, "expr", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    log("} %s  T_ValueScan end \n", prompt);
}

static void ShowNodModifyTbl(PNode n, char *prompt, int level)
{
    PModifyTbl var = (PModifyTbl)n;

    ShowBlank(level);
    log("{ %s T_ModifyTbl begin \n", prompt);

    ShowNode((PNode)var->leftplan, "leftplan", level+1);
    ShowNode((PNode)var->rightplan, "rightplan", level+1);

    ShowNode((PNode)var->rangTbl, "rangTbl", level+1);

    ShowBlank(level);
    log("} %s T_ModifyTbl end \n", prompt);
}

static void ShowNodProjectTbl(PNode n, char *prompt, int level)
{
    PProjectTbl var = (PProjectTbl)n;

    ShowBlank(level);
    log("{ %s T_ProjectTbl begin \n", prompt);

    ShowNode((PNode)var->subplan, "subplan", level+1);

    ShowNode((PNode)var->rtable, "rtable", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    log("} %s T_ProjectTbl end \n", prompt);
}

static void ShowNodQueryTbl(PNode n, char *prompt, int level)
{
    PQueryTbl var = (PQueryTbl)n;

    ShowBlank(level);
    log("{ %s T_QueryTbl begin \n", prompt);

    ShowNode((PNode)var->subplan, "subplan", level+1);

    ShowNode((PNode)var->rtable, "rtable", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    log("} %s T_QueryTbl end \n", prompt);
}


static void ShowNodList(PNode node, char *prompt, int level)
{
    TravelListCell((PList)node, prompt, level);
}

static void ShowNode(PNode n, char *prompt, int level)
{
    if(NULL == n)
    {
        ShowBlank(level);
        log("%s NULL\n", prompt);
        return;
    }

    switch(n->type)
    {
        case T_List:
            ShowNodList(n, prompt, level);
        break;
        
        case T_CreateStmt:
            ShowNodCreateStmt(n, prompt, level);
        break;
        
        case T_ColumnDef:
            ShowNodColumnDef(n, prompt, level);
        break;
        
        case T_DropStmt:
            ShowNodDropStmt(n, prompt, level);
        break;
        
        case T_InsertStmt:
            ShowNodInsertStmt(n, prompt, level);
        break;
        
        case T_AttrName:
            ShowNodAttrName(n, prompt, level);
        break;
        
        case T_ValuesData:
            ShowNodValuesData(n, prompt, level);
        break;
        
        case T_SelectStmt:
            ShowNodSelectStmt(n, prompt, level);
        break;
        
        case T_TableRefName:
            ShowNodTableRefName(n, prompt, level);
        break;

        case T_ResTarget:
            ShowNodResTarget(n, prompt, level);
        break;
        case T_ColumnRef:
            ShowNodColumnRef(n, prompt, level);
        break;
        case T_ConstValue:
            ShowNodConstValue(n, prompt, level);
        break;
        case T_A_Expr:
            ShowNodA_Expr(n, prompt, level);
        break;

        case T_BoolExpr:
		    ShowNodBoolExpr(n, prompt, level);
        break;
        case T_RangeVar:
            ShowNodRangeVar(n, prompt, level);
        break;
        case T_Alias:
            ShowNodAlias(n, prompt, level);
        break;
        case T_RangTblEntry:
            ShowNodRangTbl(n, prompt, level);
        break;

        case T_Query:
            ShowNodQuery(n, prompt, level);
        break;
        case T_MergerEntry:
            ShowNodMerger(n, prompt, level);
        break;
        case T_JoinEntry:
            ShowNodJoin(n, prompt, level);
        break;
        case T_ExprEntry:
            ShowNodExprEntry(n, prompt, level);
        break;

        case T_TargetEntry:
            ShowNodTargetEntry(n, prompt, level);
        break;
        case T_Plan:
            ShowNodPlan(n, prompt, level);
        break;
        case T_NestLoop:
            ShowNodNestLoop(n, prompt, level);
        break;
        case T_SeqScan:
            ShowNodSeqScan(n, prompt, level);
        break;	
        case T_ValueScan:
            ShowNodValueScan(n, prompt, level);
        break;
        case T_ModifyTbl:
            ShowNodModifyTbl(n, prompt, level);
        break;
        case T_ProjectTbl:
            ShowNodProjectTbl(n, prompt, level);
        break;

        case T_QueryTbl:
            ShowNodQueryTbl(n, prompt, level);
        break;
        
        default:
            ShowBlank(level);
            log("%s show proc not define, type is [%d]. \n", prompt, n->type);
        break;
    }
}

static void TravelListCell(PList list, char *prompt, int level)
{
    PListCell tmpCell = NULL;
    PList l = (PList)list;
    
    if(NULL == l)
    {
        ShowBlank(level);
        log("%s NULL tree\n", prompt);
        return;
    }

    if(level == 1)
    {
        log("{ %s  \n", prompt);
    }

    /* list node show */
    ShowBlank(level);
    log("{ begin %s T_List Node \n", prompt);

    ShowBlank(level+1);
    log("length:%d \n", l->length);

    /* list cell node show */
    for(tmpCell = l->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)(tmpCell->value.pValue);
        switch(node->type)
        {
            case T_List:
                /* list node show */
                TravelListCell((PList)node, prompt, level+1);
            break;

            default:
                ShowNode(node, prompt, level+1);
            break;
        }
    }

    ShowBlank(level);
    log("}  %s T_List end \n", prompt);

    if(level == 1)
    {
        if(level == 1)
        {
            log("} %s  \n", prompt);
        }
    }
    return;
}

void travelParserTree(PList list, char *prompt)
{
    TravelListCell((PList)list, prompt, 1);
    return ;
}