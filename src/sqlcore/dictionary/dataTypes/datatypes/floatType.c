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
#include <math.h>

static PExprDataInfo floatGreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo floatGreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo floatEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo floatLessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo floatLessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
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
        NULL,		/* <> */
        NULL,		/* + */
        NULL,		/* - */
        NULL,			/* + */
        NULL,			/* - */
        NULL,		/* * */
        NULL,		/* / */
        NULL, 			/* % */
        NULL            /* bool value */
    },
    getfloatSize

};

static int getfloatSize(PExprDataInfo value)
{
    if(NULL == value)
        return -1;
    
    /* TODO: 2024/1/23 double the same as float   */
    value->size = sizeof(float);

    return value->size;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo floatGreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    float first = leftvalue->data.fData;
    float seconde = rightvalue->data.fData;
    int result = HAT_FALSE;

    if((first - seconde) > FLOAT_EPSILON)
        result =  HAT_TRUE;
    return getDataInfo(&result, VT_INT, resExprData);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo floatGreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    float first = leftvalue->data.fData;
    float seconde = rightvalue->data.fData;
    int result = HAT_FALSE;

    first -= seconde;
    if((first > FLOAT_EPSILON) || fabs(first) < FLOAT_EPSILON)
        result =  HAT_TRUE;
    return getDataInfo(&result, VT_INT, resExprData);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo floatEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    float first = leftvalue->data.fData;
    float seconde = rightvalue->data.fData;
    int result = HAT_FALSE;

    first -= seconde;
    if(fabs(first) < FLOAT_EPSILON)
        result =  HAT_TRUE;
    return getDataInfo(&result, VT_INT, resExprData);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo floatLessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    float first = leftvalue->data.fData;
    float seconde = rightvalue->data.fData;
    int result = HAT_FALSE;

    first -= seconde;
    if((first < -FLOAT_EPSILON) || fabs(first) < FLOAT_EPSILON)
        result =  HAT_TRUE;    
    return getDataInfo(&result, VT_INT, resExprData);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo floatLessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    float first = leftvalue->data.fData;
    float seconde = rightvalue->data.fData;
    int result = HAT_FALSE;

    first -= seconde;
    if(first < -FLOAT_EPSILON)
        result =  HAT_TRUE;    
    return getDataInfo(&result, VT_INT, resExprData);
}