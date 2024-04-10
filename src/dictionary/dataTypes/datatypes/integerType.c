/*
 *	toadb integer type 
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

#include "integerType.h"
#include "dataTypeProc.h"

/* operation type define in A_A_Expr_Op_Type */
static PExprDataInfo int32GreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32GreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32EqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32LessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32LessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32NotEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);

static PExprDataInfo int32PositiveOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32NegtiveOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);

static PExprDataInfo int32PlusOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32MinusOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32MultiOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32DivisionOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static PExprDataInfo int32ModOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);

static int getIntSize(PExprDataInfo value);

DataTypeProcs integerDataTypeProcEntry =
{
    DATATYPEPROC_VERSION,
    VT_INTEGER,
    {
        int32LessEqualOperator,			/* < */
	    int32LessEqualOperator,		/* <= */
	    int32EqualOperator,			/* = */
	    int32GreaterEqualOperator,		/* >= */
	    int32GreaterOperator,		/* > */
        int32NotEqualOperator,			/* <> */
	    int32PositiveOperator,		/* + */
        int32NegtiveOperator,		/* - */
        int32PlusOperator,			/* + */
        int32MinusOperator,			/* - */
        int32MultiOperator,		/* * */
        int32DivisionOperator,		/* / */
        int32ModOperator 			/* % */
    },
    getIntSize          
};

DataTypeProcs intDataTypeProcEntry =
{
    DATATYPEPROC_VERSION,
    VT_INT,
    {
        int32LessEqualOperator,			/* < */
	    int32LessEqualOperator,		/* <= */
	    int32EqualOperator,			/* = */
	    int32GreaterEqualOperator,		/* >= */
	    int32GreaterOperator,		/* > */
        int32NotEqualOperator,			/* <> */
	    int32PositiveOperator,		/* + */
        int32NegtiveOperator,		/* - */
        int32PlusOperator,			/* + */
        int32MinusOperator,			/* - */
        int32MultiOperator,		/* * */
        int32DivisionOperator,		/* / */
        int32ModOperator 			/* % */
    },
    getIntSize

};

static int getIntSize(PExprDataInfo value)
{
    if((NULL == value) || (NULL == value->data))
        return -1;
    
    value->size = sizeof(int);

    return value->size;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32GreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;
    
    if(first > seconde)
        result = HAT_TRUE;
    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32GreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;

    if(first >= seconde)
        result = HAT_TRUE;
    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32EqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;

    if(first == seconde)
        result = HAT_TRUE;
    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32LessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;

    if(first <= seconde)
        result = HAT_TRUE;    
    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32LessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;

    if(first < seconde)
        result = HAT_TRUE;    
    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32NotEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;

    if(first != seconde)
        result = HAT_TRUE;    
    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32PositiveOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int result = first >= 0 ? first : first * -1;
    
    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32NegtiveOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int result = first > 0 ? first * -1 : first;
    
    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32PlusOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = first + seconde;

    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32MinusOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = first - seconde;

    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32MultiOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = first * seconde;

    return getDataInfo(&result, VT_INT);
}


/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32DivisionOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = first / seconde;

    return getDataInfo(&result, VT_INT);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo int32ModOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = first % seconde;

    return getDataInfo(&result, VT_INT);
}