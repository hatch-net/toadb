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

static HAT_BOOL int32GreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL int32GreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL int32EqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL int32LessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL int32LessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
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
            // != 
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
        // != 
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
static HAT_BOOL int32GreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;

    if(first > seconde)
        return HAT_TRUE;
    return HAT_FALSE;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL int32GreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;

    if(first >= seconde)
        return HAT_TRUE;
    return HAT_FALSE;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL int32EqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;

    if(first == seconde)
        return HAT_TRUE;
    return HAT_FALSE;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL int32LessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;

    if(first <= seconde)
        return HAT_TRUE;    
    return HAT_FALSE;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL int32LessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;

    if(first < seconde)
        return HAT_TRUE;    
    return HAT_FALSE;
}