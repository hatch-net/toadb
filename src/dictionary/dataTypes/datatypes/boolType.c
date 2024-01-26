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



static HAT_BOOL boolEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
static HAT_BOOL boolNotEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
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
        boolEqualOperator,      // !=  NOT_EQUAL
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
static HAT_BOOL boolEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
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
static HAT_BOOL boolNotEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue)
{
    int first = leftvalue->data->iData;
    int seconde = rightvalue->data->iData;

    if(first != seconde)
        return HAT_TRUE;
    return HAT_FALSE;
}