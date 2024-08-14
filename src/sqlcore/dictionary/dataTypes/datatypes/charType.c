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

#include "charType.h"
#include "hatstring.h"

static PExprDataInfo charGreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo charGreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo charEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo charLessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo charLessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static int charGetSize(PExprDataInfo value);


DataTypeProcs charDataTypeProcEntry =
{
    DATATYPEPROC_VERSION,
    VT_CHAR,
    {
        charLessEqualOperator,			/* < */
	    charLessEqualOperator,		/* <= */
	    charEqualOperator,			/* = */
	    charGreaterEqualOperator,		/* >= */
	    charGreaterOperator,		/* > */
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
    charGetSize

};

static int charGetSize(PExprDataInfo value)
{
    if(NULL == value)
        return -1;
    
    value->size = sizeof(char);

    return value->size;
}
static PExprDataInfo charGreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data.pData;
    char *str2 = rightvalue->data.pData;
    int len = sizeof(char);

    if(hat_strncmp(str1,str2, len) > 0)
    {
        result = HAT_TRUE;
    }

    return getDataInfo(&result, VT_INT, resExprData);
}

static PExprDataInfo charGreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data.pData;
    char *str2 = rightvalue->data.pData;
    int len = sizeof(char);

    if(hat_strncmp(str1,str2, len) >= 0)
    {
        result = HAT_TRUE;
    }

    return getDataInfo(&result, VT_INT, resExprData);
}

static PExprDataInfo charEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data.pData;
    char *str2 = rightvalue->data.pData;
    int len = sizeof(char);

    if(hat_strncmp(str1,str2, len) == 0)
    {
        result = HAT_TRUE;
    }

    return getDataInfo(&result, VT_INT, resExprData);
}

static PExprDataInfo charLessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data.pData;
    char *str2 = rightvalue->data.pData;
    int len = sizeof(char);

    if(hat_strncmp(str1,str2, len) <= 0)
    {
        result = HAT_TRUE;
    }

    return getDataInfo(&result, VT_INT, resExprData);
}

static PExprDataInfo charLessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data.pData;
    char *str2 = rightvalue->data.pData;
    int len = sizeof(char);

    if(hat_strncmp(str1,str2, len) < 0)
    {
        result = HAT_TRUE;
    }

    return getDataInfo(&result, VT_INT, resExprData);
}