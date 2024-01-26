/*
 *	toadb char type 
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

#include "floatType.h"
#include "dataTypeProc.h"
#include "math.h"

static HAT_BOOL floatGreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL floatGreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL floatEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL floatLessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL floatLessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static int getfloatSize(PExprDataInfo value);



DataTypeProcs floatDataTypeProcEntry =
{
    DATATYPEPROC_VERSION,
    VT_CHAR,
    {
        floatLessEqualOperator,			/* < */
	    floatLessEqualOperator,		/* <= */
	    floatEqualOperator,			/* = */
	    floatGreaterEqualOperator,		/* >= */
	    floatGreaterOperator,		/* > */
        // != 
    },
    getfloatSize

};

static int getfloatSize(PExprDataInfo value)
{
    if((NULL == value) || (NULL == value->data))
        return -1;
    
    /* TODO: 2024/1/23 double the same as float   */
    value->size = sizeof(float);

    return value->size;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL floatGreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    float first = leftvalue->data->fData;
    float seconde = rightvalue->data->fData;

    if((first - seconde) > FLOAT_EPSILON)
        return HAT_TRUE;
    return HAT_FALSE;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL floatGreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    float first = leftvalue->data->fData;
    float seconde = rightvalue->data->fData;

    first -= seconde;
    if((first > FLOAT_EPSILON) || fabs(first) < FLOAT_EPSILON)
        return HAT_TRUE;
    return HAT_FALSE;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL floatEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    float first = leftvalue->data->fData;
    float seconde = rightvalue->data->fData;

    first -= seconde;
    if(fabs(first) < FLOAT_EPSILON)
        return HAT_TRUE;
    return HAT_FALSE;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL floatLessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    float first = leftvalue->data->fData;
    float seconde = rightvalue->data->fData;

    first -= seconde;
    if((first < -FLOAT_EPSILON) || fabs(first) < FLOAT_EPSILON)
        return HAT_TRUE;    
    return HAT_FALSE;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static HAT_BOOL floatLessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    float first = leftvalue->data->fData;
    float seconde = rightvalue->data->fData;

    first -= seconde;
    if(first < -FLOAT_EPSILON)
        return HAT_TRUE;    
    return HAT_FALSE;
}