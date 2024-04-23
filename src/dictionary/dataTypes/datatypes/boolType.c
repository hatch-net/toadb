/*
 *	toadb bool type 
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
#include "boolType.h"



static PExprDataInfo boolEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo boolNotEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static int getboolSize(PExprDataInfo value);

DataTypeProcs boolDataTypeProcEntry =
{
    DATATYPEPROC_VERSION,
    VT_BOOL,
    {
        NULL,			        /* < */
	    NULL,		            /* <= */
	    boolEqualOperator,			/* = */
	    NULL,		            /* >= */
	    NULL,		            /* > */
        boolNotEqualOperator,      // !=  NOT_EQUAL
	    NULL,		/* + */
        NULL,		/* - */
        NULL,			/* + */
        NULL,			/* - */
        NULL,		/* * */
        NULL,		/* / */
        NULL, 			/* % */
        NULL            /* bool value */
    },
    getboolSize
};

static int getboolSize(PExprDataInfo value)
{
    if((NULL == value) || (NULL == value->data))
        return -1;
    
    /* memory store as integer  */
    value->size = sizeof(int);

    return value->size;
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo boolEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;

    if(first == seconde)
        result = HAT_TRUE;
    return getDataInfo(&result, VT_INT, resExprData);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo boolNotEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;

    if(first != seconde)
        result = HAT_TRUE;
    return getDataInfo(&result, VT_INT, resExprData);
}

/* 
 * 第一操作数的类型一定是匹配的；
 * 第二操作数的类型不同时，会转为第一操作数对应的类型 
*/
static PExprDataInfo boolValueOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;
    int result = HAT_FALSE;

    if(first != seconde)
        result = HAT_TRUE;
    return getDataInfo(&result, VT_INT, resExprData);
}
