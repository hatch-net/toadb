/*
 *	toadb parserNode 
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parserNode.h"
#include "queryNode.h"
#include "planNode.h"

#define hat_log printf
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
    for(int i = level; i > 0; i--)
    {
        hat_log("  ");
    }
}

static void ShowNodCreateStmt(PNode n, char *prompt, int level)
{
    PCreateStmt createstmt = (PCreateStmt)n;

    ShowBlank(level);
    hat_log("T_CreateStmt Node: tablename:%s ", createstmt->tableName);

    TravelListCell(createstmt->ColList, "T_CreateStmt ", level+1);
}

static void ShowNodColumnDef(PNode n, char *prompt, int level)
{
    PColumnDef column = (PColumnDef)n;

    ShowBlank(level);
    hat_log("%s columnName:%s type:%s ", prompt, column->colName, column->colType);
}

static void ShowNodDropStmt(PNode n, char *prompt, int level)
{
    
}

static void ShowNodInsertStmt(PNode n, char *prompt, int level)
{
    PInsertStmt insertstmt = (PInsertStmt)n;

    ShowBlank(level);
    hat_log("{ T_InsertStmt Node: ");

    ShowBlank(level+1);
    hat_log("table :%s ", insertstmt->tableName);

    TravelListCell(insertstmt->attrNameList, "attrNameList ", level+1);
    TravelListCell(insertstmt->valuesList, "values", level+1);

    ShowBlank(level);
    hat_log("} %s T_InsertStmt Node end", prompt);
}

static void ShowNodUpdateStmt(PNode n, char *prompt, int level)
{
    PUpdateStmt updatestmt = (PUpdateStmt)n;

    ShowBlank(level);
    hat_log("{ T_UpdateStmt Node: ");

    ShowNode(updatestmt->relation, "relation", level+1);

    TravelListCell(updatestmt->targetlist, "targetlist ", level+1);
    TravelListCell(updatestmt->fromList, "fromList", level+1);
    TravelListCell(updatestmt->whereList, "whereList", level+1);

    ShowBlank(level);
    hat_log("} %s T_UpdateStmt Node end", prompt);
}

static void ShowNodAttrName(PNode n, char *prompt, int level)
{
    PAttrName stmt = (PAttrName)n;

    ShowBlank(level);
    hat_log("%s T_AttrName Node: attrname :%s ", prompt, stmt->attrName);
}

static void ShowNodValuesData(PNode n, char *prompt, int level)
{
    PValuesData stmt = (PValuesData)n;

    ShowBlank(level);
    hat_log("T_ValuesData Node:  ");

    ShowNode((PNode)stmt->valueNode, "valuesNode", level+1);
}

static void ShowNodSelectStmt(PNode n, char *prompt, int level)
{
    PSelectStmt stmt = (PSelectStmt)n;

    ShowBlank(level);
    hat_log("{  %s T_SelectStmt Node begin: ", prompt);

    ShowBlank(level+1);
    hat_log("selectAll:%d ", stmt->selectAll);

    TravelListCell(stmt->targetlist, "targetlist ", level+1);
    TravelListCell(stmt->relrange, "relrange ", level+1);
    TravelListCell(stmt->fromList, "fromList ", level+1);
    TravelListCell(stmt->whereList, "whereList ", level+1);
    TravelListCell(stmt->groupList, "groupList ", level+1);
    TravelListCell(stmt->sortList, "sortList ", level+1);
    TravelListCell(stmt->limitClause, "limitClause ", level+1);

    ShowBlank(level);
    hat_log("}  %s T_SelectStmt Node end:   ", prompt);
}

static void ShowNodTableRefName(PNode n, char *prompt, int level)
{
    PTableRefName stmt = (PTableRefName)n;

    ShowBlank(level);
    hat_log("%s T_TableRefName Node: table ref name:%s ", prompt, stmt->tblRefName);
}

static void ShowNodColumnRef(PNode n, char *prompt, int level)
{
    PColumnRef var = (PColumnRef)n;

    ShowBlank(level+1);

    if(var->tableName != NULL)
        hat_log("%s ColumnRef field: %s.%s ", prompt, var->tableName, var->field);
    else 
        hat_log("%s ColumnRef field: %s", prompt,  var->field);
}

static void ShowNodResTarget(PNode n, char *prompt, int level)
{
    PResTarget var = (PResTarget)n;

    ShowBlank(level);
    hat_log("{ PResTarget begin: ");

    ShowBlank(level+1);
    hat_log("name: %s ", var->name);

    ShowBlank(level+1);
    hat_log("all: %d ", var->all);

    ShowNode((PNode)var->val, "val", level+1);
    ShowNode((PNode)var->setValue, "setValue", level+1);

    ShowBlank(level);
    hat_log("} PResTarget end : ");
}

static void ShowNodConstValue(PNode n, char *prompt, int level)
{
    PConstValue var = (PConstValue)n;

    ShowBlank(level);
    hat_log("%s ConstValue: ", prompt);

    ShowBlank(level);
    hat_log("vt: %d ", var->vt);

    switch(var->vt)
    {
        case VT_POINTER:
            ShowBlank(level);
            hat_log("string %s", var->val.pData);
            break;
	    case VT_INT:
        case VT_INTEGER:
            ShowBlank(level);
            hat_log("int %d", var->val.iData);
            break;
        case VT_VARCHAR:
        case VT_STRING:
            ShowBlank(level);
            hat_log("string %s", var->val.pData);
            break;
	    case VT_CHAR:
            ShowBlank(level);
            hat_log("char %c", var->val.cData);
            break;
	    case VT_DOUBLE:
            ShowBlank(level);
            hat_log("double %lf", var->val.dData);
            break;
	    case VT_FLOAT:
            ShowBlank(level);
            hat_log("float %f", var->val.fData);
            break;
        default:
            ShowBlank(level);
            hat_log("error data type");
            break;
    }
    
    ShowBlank(level);
    hat_log("isnull: %d ", var->isnull);
}

static void ShowNodA_Expr(PNode n, char *prompt, int level)
{
    PA_Expr var = (PA_Expr)n;

    ShowBlank(level);
    hat_log("{ %s A_Expr begin:  ", prompt);

    ShowBlank(level+1);
    hat_log("name:%s ", var->name);

    ShowBlank(level+1);
    hat_log("exprType:%d ", var->exprType);

    ShowBlank(level+1);
    hat_log("left expr  ");
    ShowNode((PNode)var->lexpr, "left expr", level+1);

    ShowBlank(level+1);
    hat_log("right expr  ");
    ShowNode((PNode)var->rexpr, "right expr", level+1);

    ShowBlank(level);
    hat_log("}  %s A_Expr end:  ", prompt);
}

static void ShowNodBoolExpr(PNode n, char *prompt, int level)
{
    PBoolExpr var = (PBoolExpr)n;

    ShowBlank(level);
    hat_log("{ %s BoolExpr begin: ", prompt);

    ShowBlank(level);
    hat_log("boolop: %d ", var->boolop);

    TravelListCell(var->args, " ", level+1);

    ShowBlank(level);
    hat_log("} %s BoolExpr end:  ", prompt);
}

static void ShowNodAlias(PNode n, char *prompt, int level)
{
    PAlias var = (PAlias)n;

    ShowBlank(level);
    hat_log("%s Alias:%s ", prompt, var->aliasname);
}

static void ShowNodRangeVar(PNode n, char *prompt, int level)
{
    PRangeVar var = (PRangeVar)n;

    ShowBlank(level);
    hat_log("{  RangeVar begin ");

    ShowBlank(level+1);
    hat_log("relname:%s ", var->relname);

    ShowNode((PNode)var->alias, "alias", level+1);

    ShowBlank(level);
    hat_log("}  RangeVar end ");
}

static void ShowNodQuery(PNode n, char *prompt, int level)
{
    PQuery var = (PQuery)n;

    ShowBlank(level);
    hat_log("{  Query begin ");

    ShowBlank(level+1);
    hat_log("commandType:%d ", var->commandType);

    ShowBlank(level+1);
    hat_log("queryId:%d ", var->queryId);    

    TravelListCell((PList)var->targetList, "targetList", level+1);
    TravelListCell((PList)var->rtable, "rtable", level+1);
    TravelListCell((PList)var->joinTree, "joinTree", level+1);
    TravelListCell((PList)var->rtjoinTree, "rtjoinTree", level+1);

    ShowBlank(level);
    hat_log("}  Query end");
}

static void ShowNodRangTbl(PNode n, char *prompt, int level)
{
    PRangTblEntry var = (PRangTblEntry)n;

    ShowBlank(level);
    hat_log("{  T_RangTblEntry begin ");

    ShowBlank(level+1);
    hat_log("rindex:%d ", var->rindex);

    ShowBlank(level+1);
    hat_log("relkind:%d ", var->relkind);

    ShowBlank(level+1);
    hat_log("isScaned:%d ", var->isScaned);    
    
    TravelListCell((PList)var->targetList, "targetList", level+1);
    TravelListCell((PList)var->ValueList, "ValueList", level+1);

    ShowBlank(level);
    hat_log("}  T_RangTblEntry end ");
}

static void ShowNodJoin(PNode n, char *prompt, int level)
{
    PJoinEntry var = (PJoinEntry)n;

    ShowBlank(level);
    hat_log("{  T_JoinEntry begin ");

    ShowBlank(level+1);
    hat_log("rindex:%d ", var->rindex);
    ShowBlank(level+1);
    hat_log("isJoin:%d ", var->isJoin);

    ShowBlank(level+1);
    hat_log("joinOp:%d ", var->joinOp);

    TravelListCell((PList)var->targetList, "targetList", level+1);
    ShowNode((PNode)var->lefttree, "lefttree", level+1);
    ShowNode((PNode)var->righttree, "righttree", level+1);

    ShowBlank(level);
    hat_log("}  T_JoinEntry end ");
}

static void ShowNodMerger(PNode n, char *prompt, int level)
{
    PMergerEntry var = (PMergerEntry)n;

    ShowBlank(level);
    hat_log("{  %s T_MergerEntry begin ", prompt);

    ShowBlank(level+1);
    hat_log("rindex:%d ", var->rindex);
    ShowBlank(level+1);
    hat_log("isJoin:%d ", var->isJoin);

    ShowBlank(level+1);
    hat_log("mergeType:%d ", var->mergeType);

    TravelListCell((PList)var->targetList, "targetList", level+1);

    ShowNode((PNode)var->lefttree, "lefttree", level+1);

    ShowNode((PNode)var->righttree, "righttree", level+1);

    ShowBlank(level);
    hat_log("} %s T_MergerEntry end ", prompt);
}

static void ShowNodExprEntry(PNode n, char *prompt, int level)
{
    PExprEntry var = (PExprEntry)n;

    ShowBlank(level);
    hat_log("{ %s T_ExprEntry begin ", prompt);

    ShowBlank(level+1);
    hat_log("rindex:%d ", var->rindex);
    
    ShowBlank(level+1);
    hat_log("rtNum:%d ", var->rtNum);

    ShowBlank(level+1);
    hat_log("op:%d ", var->op);

    ShowBlank(level+1);
    hat_log("rrindex:%d ", var->rrindex);


    TravelListCell((PList)var->targetList, "targetList", level+1);
    ShowNode((PNode)var->lefttree, "lefttree", level+1);
    ShowNode((PNode)var->righttree, "righttree", level+1);

    ShowBlank(level);
    hat_log("}  %s T_ExprEntry end ", prompt);
}


static void ShowNodTargetEntry(PNode n, char *prompt, int level)
{
    PTargetEntry var = (PTargetEntry)n;

    ShowBlank(level);
    hat_log("{  T_TargetEntry begin ");

    ShowBlank(level+1);
    hat_log("rindex:%d ", var->rindex);

    ShowNode((PNode)var->colRef, "colRef", level+1);

    ShowBlank(level);
    hat_log("}  T_TargetEntry end ");
}

static void ShowNodPlan(PNode n, char *prompt, int level)
{
    PPlan var = (PPlan)n;

    ShowBlank(level);
    hat_log("{  T_Plan begin ");

    ShowBlank(level+1);
    hat_log("commandType:%d ", var->commandType);

    ShowBlank(level+1);
    hat_log("planLevel:%d ", var->planLevel);

    ShowNode((PNode)var->leftplan, "leftplan", level+1);
    ShowNode((PNode)var->rightplan, "rightplan", level+1);

    ShowBlank(level);
    hat_log("}  T_Plan end ");
}

static void ShowNodNestLoop(PNode n, char *prompt, int level)
{
    PNestLoop var = (PNestLoop)n;

    ShowBlank(level);
    hat_log("{ %s T_NestLoop begin ", prompt);

    ShowBlank(level+1);
    hat_log("isJoin:%d ", var->isJoin);

    ShowBlank(level+1);
    hat_log("mergeType:%d ", var->mergeType);

    ShowNode((PNode)var->leftplan, "leftplan", level+1);
    ShowNode((PNode)var->rightplan, "rightplan", level+1);

    ShowNode((PNode)var->expr, "expr", level+1);
    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    hat_log("} %s T_NestLoop end ", prompt);
}

static void ShowNodSeqScan(PNode n, char *prompt, int level)
{
    PSeqScan var = (PSeqScan)n;

    ShowBlank(level);
    hat_log("{ %s T_SeqScan begin ", prompt);

    ShowNode((PNode)var->rangTbl, "rangTbl", level+1);
    ShowNode((PNode)var->expr, "expr", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    hat_log("} %s  T_SeqScan end ", prompt);
}

static void ShowNodValueScan(PNode n, char *prompt, int level)
{
    PValueScan var = (PValueScan)n;

    ShowBlank(level);
    hat_log("{ %s T_ValueScan begin ", prompt);

    ShowNode((PNode)var->rangTbl, "rangTbl", level+1);

    ShowNode((PNode)var->expr, "expr", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    hat_log("} %s  T_ValueScan end ", prompt);
}

static void ShowNodModifyTbl(PNode n, char *prompt, int level)
{
    PModifyTbl var = (PModifyTbl)n;

    ShowBlank(level);
    hat_log("{ %s T_ModifyTbl begin ", prompt);

    ShowNode((PNode)var->leftplan, "leftplan", level+1);
    ShowNode((PNode)var->rightplan, "rightplan", level+1);

    ShowNode((PNode)var->rangTbl, "rangTbl", level+1);

    ShowBlank(level);
    hat_log("} %s T_ModifyTbl end ", prompt);
}

static void ShowNodProjectTbl(PNode n, char *prompt, int level)
{
    PProjectTbl var = (PProjectTbl)n;

    ShowBlank(level);
    hat_log("{ %s T_ProjectTbl begin ", prompt);

    ShowNode((PNode)var->subplan, "subplan", level+1);

    ShowNode((PNode)var->rtable, "rtable", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    hat_log("} %s T_ProjectTbl end ", prompt);
}

static void ShowNodQueryTbl(PNode n, char *prompt, int level)
{
    PQueryTbl var = (PQueryTbl)n;

    ShowBlank(level);
    hat_log("{ %s T_QueryTbl begin ", prompt);

    ShowNode((PNode)var->subplan, "subplan", level+1);

    ShowNode((PNode)var->rtable, "rtable", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    hat_log("} %s T_QueryTbl end ", prompt);
}

static void ShowSelectResult(PNode n, char *prompt, int level)
{
    PSelectResult var = (PSelectResult)n;

    ShowBlank(level);
    hat_log("{ %s T_SelectResult begin ", prompt);

    ShowNode((PNode)var->subplan, "subplan", level+1);
    
    ShowNode((PNode)var->qual, "qual", level+1);

    ShowNode((PNode)var->rtable, "rtable", level+1);

    ShowNode((PNode)var->targetList, "targetList", level+1);

    ShowBlank(level);
    hat_log("} %s T_SelectResult end ", prompt);
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
        hat_log("%s NULL", prompt);
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

        case T_UpdateStmt:
            ShowNodUpdateStmt(n, prompt, level);
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
        case T_SelectResult:
            ShowSelectResult(n, prompt, level);
        break;
        
        default:
            ShowBlank(level);
            hat_log("%s show proc not define, type is [%d]. ", prompt, n->type);
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
        hat_log("%s NULL tree", prompt);
        return;
    }

    if(level == 1)
    {
        hat_log("{ %s  ", prompt);
    }

    /* list node show */
    ShowBlank(level);
    hat_log("{ begin %s T_List Node ", prompt);

    ShowBlank(level+1);
    hat_log("length:%d ", l->length);

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
    hat_log("}  %s T_List end ", prompt);

    if(level == 1)
    {
        if(level == 1)
        {
            hat_log("} %s  ", prompt);
        }
    }
    return;
}

void travelParserTree(PList list, char *prompt)
{
    TravelListCell((PList)list, prompt, 1);
    return ;
}