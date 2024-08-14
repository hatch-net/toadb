/*
 *	toadb expression process
 *
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
#include "execExpre.h"
#include "expreCompute.h"
#include "public.h"
#include "server_pub.h"
#include "queryNode.h"
#include "string.h"

static PExprDataInfo ComputeBoolExprNode(PNode node, PExecState eState);
static PExprDataInfo ComputeExpreNode(PNode node, PExecState eState);
static PExprDataInfo GetExpreOperatorValue(PNode node, PExecState eState);
static PExprDataInfo GetExprDataColumnValue(PNode node, PExecState eState);
static PExprDataInfo GetExprDataConstValue(PNode node, PExecState eState);

static PExprDataInfo ProcessQualExprNode(PNode node, PExecState eState);

/* 
 * 计算表达式结果
 * 结果有两种：真，返回大于0的值；假，返回0值；
 */
PExprDataInfo ExecSelectQual(PNode node, PExecState eState)
{
    PExprDataInfo result = NULL;

    result = ProcessQualExprNode(node, eState);

    return result;
}

static PExprDataInfo ProcessQualExprNode(PNode node, PExecState eState)
{
    PNode pnode = eState->subPlanNode;
    PExprDataInfo resultExprData = NULL;

    /* 条件为NULL时，表达式为真 */
    if(NULL == node)
        return NULL;
    
    switch(node->type)
    {
        case T_MergerEntry:
            /* 记录当前节点 */
            eState->subPlanNode = node;
            resultExprData = ComputeBoolExprNode(node, eState);
        break;
        case T_ExprEntry:
            /* 记录当前节点 */
            eState->subPlanNode = node;
            resultExprData = ComputeExpreNode(node, eState);
        break;
        case T_ColumnRef:
        case T_ConstValue:
        {
            resultExprData = GetExpreOperatorValue(node, eState);
        }
        break;
        default:
            hat_error("select qual error node type(%d)", node->type);
        break;
    }

    eState->subPlanNode = pnode;
   
    return resultExprData;
}

/*
 * 计算布尔表达式结果 
 * and/or/not 递归计算
 */
static PExprDataInfo ComputeBoolExprNode(PNode node, PExecState eState)
{
    PMergerEntry mergerNode = (PMergerEntry)node;
    PSelectState pst = (PSelectState)eState->subPlanStateNode;
    ExprDataInfo leftResult = {0}, rightResult = {0};
    int result = 0;
    int boolType = mergerNode->mergeType;       /* and,or,not */  

    /* 递归获取结果 */
    if(NULL != mergerNode->lefttree)
    {
        ExecSelectQual(mergerNode->lefttree,eState);
        
        leftResult = *(pst->selectExpreData->resultExpreData);
    }

    if(NULL != mergerNode->righttree)
    {
        ExecSelectQual(mergerNode->righttree,eState);

        rightResult = *(pst->selectExpreData->resultExpreData);
    }

    /* 计算结果 */
    switch(boolType)
    {
        case AND_EXPR:
            if(getDataBool(&leftResult) && getDataBool(&rightResult))
                result = 1;
            else 
                result = 0;
        break;
        case OR_EXPR:
            if(getDataBool(&leftResult) || getDataBool(&rightResult))
                result = 1;
            else 
                result = 0;
        break;
        case NOT_EXPR:
            result = getDataBool(&leftResult) ? 0 : 1;
        break;
        default:
            hat_error("bool expr not support (%d)", boolType);
        break;
    }
    
    /* result save to resultExpreData. */
    return getDataInfo(&result, VT_INT, pst->selectExpreData->resultExpreData);
}

/*
 * 计算简单表达式结果 
 * =、>=, >, =, < , <=, != ,也会有嵌套表达式，递归计算
 */
static PExprDataInfo ComputeExpreNode(PNode node, PExecState eState)
{
    PSelectState pst = (PSelectState)eState->subPlanStateNode;
    PExprEntry simpleExprNode = (PExprEntry)node;
    ExprDataInfo leftvalue = {0}, rightvalue = {0};
    int exprType = simpleExprNode->op;
    PExprDataInfo result = 0;

    /* 获取表达式两端的值与类型 */
    if(NULL != simpleExprNode->lefttree)
    {
        ProcessQualExprNode(simpleExprNode->lefttree, eState);

        /* result is save to local. */
        leftvalue = *(pst->selectExpreData->resultExpreData);
    }

    if(NULL != simpleExprNode->righttree)
    {
        ProcessQualExprNode(simpleExprNode->righttree, eState);

        /* result is save to local. */
        rightvalue = *(pst->selectExpreData->resultExpreData);
    }
    

    /* 计算表达式结果，调用类型通用计算函数 */
    result = ComputeExpr(&leftvalue, &rightvalue, pst->selectExpreData->resultExpreData, exprType);

    return result;
}

/* 
 * get column or const value.
 * result is saved in pselectplanState->selectExpreData->resultExpreData.
 */
static PExprDataInfo GetExpreOperatorValue(PNode node, PExecState eState)
{    
    PExprDataInfo exprValue = NULL;

    if(NULL == node)
        return NULL;

    switch(node->type)
    {
        case T_ColumnRef:
            exprValue = GetExprDataColumnValue(node, eState);
        break;
        case T_ConstValue:
            exprValue = GetExprDataConstValue(node, eState);
        break;
        default:
        break;
    }

    return exprValue;
}

/* 
 * 表达式为列属性名
 * 从该列的查询结果中获取该列的value
 */
static PExprDataInfo GetExprDataColumnValue(PNode node, PExecState eState)
{
    PExprEntry simpleExprNode = (PExprEntry)eState->subPlanNode;
    PSelectState pst = (PSelectState)eState->subPlanStateNode;
    PScanTableRowData rowData = (PScanTableRowData)pst->resultRow;
    PList rtable = pst->rtable;

    PColumnRef columnRefNode = (PColumnRef)node;
    PTableRowDataPosition tblRowPosition = NULL;
    PRowColumnData colData = NULL;
    PExprDataInfo exprValue = NULL;

    PRangTblEntry rte = NULL;
    int rindex = -1;
    int found = HAT_FALSE;

    /* 找到对应的表信息  */
    if(node == simpleExprNode->lefttree)
        rindex = simpleExprNode->rindex;
    else
        rindex = simpleExprNode->rrindex;

    rte = (PRangTblEntry)GetCellValueByIndex(rtable, rindex);
    if(NULL == rte)
    {
        hat_error("qual Rang table not founded.");
        return NULL;
    }

    /* 根据表元数据定义，找到对应的表的查询行 */
    tblRowPosition = GetTblRowDataPosition(rowData, rte->tblInfo, rte->rindex);
    if(NULL == tblRowPosition)
    {
        hat_error("select rowdata position not founded.");
        return NULL;
    }
    
    /* 
     * 根据列的定义，找到对应列的信息进行投影，得到该表列的投影字段数组
     */
    colData = GetColRowDataEx(tblRowPosition, columnRefNode, NULL, 0, &found, HAT_FALSE);
    if(NULL == colData)
    {
        hat_error("column rowdata not founded.");
        return NULL;
    }    

    exprValue = pst->selectExpreData->resultExpreData;
    TranslateRawColumnData(colData, columnRefNode, &exprValue->data);

    exprValue->type = columnRefNode->vt;
    exprValue->size = colData->size;       

    /* only for express */
    if(HAT_FALSE == found)
        FreeMem(colData);
    return exprValue;
}

/* 
 * 表达式为常量
 * 将它组成对应数据类型
 */
static PExprDataInfo GetExprDataConstValue(PNode node, PExecState eState)
{
    PSelectState pst = (PSelectState)eState->subPlanStateNode;
    PConstValue pcv = (PConstValue)node;
    PExprDataInfo exprValue = NULL;

    exprValue = pst->selectExpreData->resultExpreData;
    exprValue->type = pcv->vt;
    memcpy(&exprValue->data , &(pcv->val), sizeof(Data));
    exprValue->size = -1;       

    if(pcv->isnull)
        exprValue->size = 0;

    return exprValue;
}

