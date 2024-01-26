/*
 *	toadb execSelect 
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

#include "execSelect.h"
#include "public.h"
#include "server_pub.h"
#include "planNode.h"
#include "execNode.h"
#include "queryNode.h"
#include "expreCompute.h"


static int ProcessSelectQual(PNode node, PExecState eState);
static int ComputeBoolExprNode(PNode node, PExecState eState);
static int ComputeExpreNode(PNode node, PExecState eState);
static PExprDataInfo GetExpreOperatorValue(PNode node, PExecState eState);
static PExprDataInfo GetExprDataColumnValue(PNode node, PExecState eState);
static PExprDataInfo GetExprDataConstValue(PNode node, PExecState eState);

/*
 * 对扫描结果进行条件表达式选择过滤
 * 条件表达式也是以树形式记录，需要递归处理 
 */
PTableRowData ExecSelect(PExecState eState)
{
    PSelectResult psr = (PSelectResult)eState->subPlanNode;
    PSelectState pst = (PSelectState)eState->subPlanStateNode;
    PList qual = psr->qual;
    PTableRowData rowData = NULL;
    PListCell tmpCell = NULL;
    PNode qualNode = NULL;
    int result = 0;

    for(; ;)
    {
        /* 获取一条记录 */
        eState->subPlanNode = (PNode)psr->subplan;
        eState->subPlanStateNode = (PNode)pst->subplanState;
        rowData = ExecNodeProc(eState);
        if(NULL == rowData)
        {
            /* 如果为空，说明已经扫描结束，退出 */
            break;
        }

        if(NULL == qual)
            return rowData;

        /* 记录结果，用于表达式计算 */
        pst->resultRow = (PNode)rowData;
        eState->subPlanNode = (PNode)psr;
        eState->subPlanStateNode = (PNode)pst;

        /* 查询结果记录在pst中 */
        for(tmpCell = qual->head; tmpCell != NULL; tmpCell = tmpCell->next)
        {
            PNode qualNode = GetCellNodeValue(tmpCell);
            result = ProcessSelectQual(qualNode, eState);

            /* 
             * 当有多个表达式时，各表达式之间是or的关系，
             * 只要有一个为真时，最终结果为真
            */
            if(result)
                break;
        }

        /* 
         * 表达式为真，返回当前扫描结果; 
         * 表达式为假，获取下一条数据
         */
        if(result)
            return rowData;
    } 
    
    return NULL;
}

/* 
 * 计算表达式结果
 * 结果有两种：真，返回大于0的值；假，返回0值；
 */
static int ProcessSelectQual(PNode node, PExecState eState)
{
    int result = 0;
    PNode pnode = eState->subPlanNode;

    /* 记录当前节点 */
    eState->subPlanNode = node;

    /* 条件为NULL时，表达式为真 */
    if(NULL == node)
        return 1;
    
    switch(node->type)
    {
        case T_MergerEntry:
            result = ComputeBoolExprNode(node, eState);
        break;
        case T_ExprEntry:
            result = ComputeExpreNode(node, eState);
        break;
        default:
            hat_error("select qual error node type(%d)\n", node->type);
        break;
    }

    eState->subPlanNode = pnode;

    return result;
}

/*
 * 计算布尔表达式结果 
 * and/or/not 递归计算
 */
static int ComputeBoolExprNode(PNode node, PExecState eState)
{
    PMergerEntry mergerNode = (PMergerEntry)node;

    int result = 0, leftResult = 0, rightResult = 0;
    int boolType = mergerNode->mergeType;       /* and,or */

    /* 递归获取结果 */
    if(NULL != mergerNode->lefttree)
    {
        leftResult = ProcessSelectQual(mergerNode->lefttree,eState);
    }

    if(NULL != mergerNode->righttree)
    {
        rightResult = ProcessSelectQual(mergerNode->righttree,eState);
    }

    /* 计算结果 */
    switch(boolType)
    {
        case AND_EXPR:
            if((leftResult == 0) || (rightResult == 0))
                result = 0;
            else 
                result = 1;
        break;
        case OR_EXPR:
            if((leftResult == 1) || (rightResult == 1))
                result = 1;
            else 
                result = 0;
        break;
        case NOT_EXPR:
            result = leftResult == 1 ? 0 : 1;
        break;
        default:
            hat_error("bool expr not support (%d)\n", boolType);
        break;
    }
    
    return result;
}

/*
 * 计算简单表达式结果 
 * =、>=, >, =, < , <=, != ,也会有嵌套表达式，递归计算
 */
static int ComputeExpreNode(PNode node, PExecState eState)
{
    PExprEntry simpleExprNode = (PExprEntry)node;
    PExprDataInfo leftvalue = NULL, rightvalue = NULL;
    int exprType = simpleExprNode->op;
    int result = 0;

    /* 获取表达式两端的值与类型 */
    if((simpleExprNode->lefttree->type != T_ColumnRef) && (simpleExprNode->lefttree->type != T_ConstValue))
    {
        hat_error("Expre left not support qual neither column nor constvalue.\n");
        return 0;
    }
    else
    {
        leftvalue = GetExpreOperatorValue(simpleExprNode->lefttree, eState);
    }

    if((simpleExprNode->righttree->type != T_ColumnRef) && (simpleExprNode->righttree->type != T_ConstValue))
    {
        hat_error("Expre right not support qual neither column nor constvalue.\n");
        return 0;
    }
    else
    {
        rightvalue = GetExpreOperatorValue(simpleExprNode->righttree, eState);
    }
    

    /* 计算表达式结果，调用类型通用计算函数 */
    result = ComputeBoolExpr(leftvalue, rightvalue, exprType);

    return result;
}

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
    PTableRowData rowData = (PTableRowData)pst->resultRow;
    PList rtable = pst->rtable;

    PColumnRef columnRefNode = (PColumnRef)node;
    PTableRowDataPosition tblRowPosition = NULL;
    PTableRowData rawcolrow = NULL;
    PExprDataInfo exprValue = NULL;

    PRangTblEntry rte = NULL;
    int rindex = -1;

    /* 找到对应的表信息  */
    if(node == simpleExprNode->lefttree)
        rindex = simpleExprNode->rindex;
    else
        rindex = simpleExprNode->rrindex;

    rte = (PRangTblEntry)GetCellValueByIndex(rtable, rindex);
    if(NULL == rte)
    {
        hat_error("qual Rang table not founded.\n");
        return NULL;
    }

    /* 根据表元数据定义，找到对应的表的查询行 */
    tblRowPosition = GetTblRowDataPosition((PScanTableRowData)rowData, rte->tblInfo);
    if(NULL == tblRowPosition)
    {
        hat_error("select rowdata position not founded.\n");
        return NULL;
    }
    
    /* 
     * 根据列的定义，找到对应列的信息进行投影，得到该表列的投影字段数组
     */
    rawcolrow = GetColRowData(tblRowPosition, columnRefNode);
    if(NULL == rawcolrow)
    {
        hat_error("column rowdata not founded.\n");
        return NULL;
    }    

    exprValue = (PExprDataInfo)AllocMem(sizeof(ExprDataInfo));
    exprValue->type = columnRefNode->vt;
    exprValue->data = TranslateRawColumnData(rawcolrow, columnRefNode);
    exprValue->size = rawcolrow->size;       

    return exprValue;
}

/* 
 * 表达式为常量
 * 将它组成对应数据类型
 */
static PExprDataInfo GetExprDataConstValue(PNode node, PExecState eState)
{
    PConstValue pcv = (PConstValue)node;
    PExprDataInfo exprValue = NULL;

    exprValue = (PExprDataInfo)AllocMem(sizeof(ExprDataInfo));
    exprValue->type = pcv->vt;
    exprValue->data = &(pcv->val);
    exprValue->size = -1;       

    if(pcv->isnull)
        exprValue->size = 0;

    return exprValue;
}