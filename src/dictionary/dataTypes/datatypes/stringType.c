/*
 *	toadb string type 
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

#include "stringType.h"
#include "dataTypeProc.h"
#include "hatstring.h"

static PExprDataInfo stringGreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo stringGreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo stringEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo stringLessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static PExprDataInfo stringLessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
static int stringGetSize(PExprDataInfo value);

DataTypeProcs stringDataTypeProcEntry =
{
    DATATYPEPROC_VERSION,
    VT_STRING,
    {
        stringLessEqualOperator,			/* < */
	    stringLessEqualOperator,		/* <= */
	    stringEqualOperator,			/* = */
	    stringGreaterEqualOperator,		/* >= */
	    stringGreaterOperator,		/* > */
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
    stringGetSize       
};

DataTypeProcs varcharDataTypeProcEntry =
{
    DATATYPEPROC_VERSION,
    VT_VARCHAR,
    {
        stringLessEqualOperator,			/* < */
	    stringLessEqualOperator,		/* <= */
	    stringEqualOperator,			/* = */
	    stringGreaterEqualOperator,		/* >= */
	    stringGreaterOperator,		/* > */
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
    stringGetSize          
};

static int stringGetSize(PExprDataInfo value)
{
    char *str = NULL;

    if((NULL == value) || (NULL == value->data) || (NULL == value->data->pData))
        return -1;

    str = value->data->pData;
    
    value->size = hat_strlen(str);

    return value->size;
}

static PExprDataInfo stringGreaterOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data->pData;
    char *str2 = rightvalue->data->pData;
    int len = 0;

    stringGetSize(leftvalue);
    stringGetSize(rightvalue);
    do {
        if(leftvalue->size > rightvalue->size)
        {
            result = HAT_TRUE;
            break;
        }
        else if(leftvalue->size < rightvalue->size)
        {
            // false
            break;
        }
        else 
        {
            len = leftvalue->size;
        }

        if(hat_strncmp(str1,str2, len) > 0)
        {
            result = HAT_TRUE;
            break;
        }
    }while(0);

    return getDataInfo(&result, VT_INT, resExprData);
}

static PExprDataInfo stringGreaterEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data->pData;
    char *str2 = rightvalue->data->pData;
    int len = 0;

    stringGetSize(leftvalue);
    stringGetSize(rightvalue);
    do {
        if(leftvalue->size > rightvalue->size)
        {
            result = HAT_TRUE;
            break;
        }
        else if(leftvalue->size < rightvalue->size)
        {
            // false
            break;
        }
        else 
        {
            len = leftvalue->size;
        }

        if(hat_strncmp(str1,str2, len) >= 0)
        {
            result = HAT_TRUE;
            break;
        }
    }while(0);

    return getDataInfo(&result, VT_INT, resExprData);
}

static PExprDataInfo stringEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data->pData;
    char *str2 = rightvalue->data->pData;
    int len = 0;

    stringGetSize(leftvalue);
    stringGetSize(rightvalue);
    do {
        if(leftvalue->size > rightvalue->size)
        {
            // false
            break;
        }
        else if(leftvalue->size < rightvalue->size)
        {
            // false
            break;
        }
        else 
        {
            len = leftvalue->size;
        }

        if(hat_strncmp(str1,str2, len) == 0)
        {
            result = HAT_TRUE;
            break;
        }
    }while(0);

    return getDataInfo(&result, VT_INT, resExprData);
}

static PExprDataInfo stringLessEqualOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data->pData;
    char *str2 = rightvalue->data->pData;
    int len = 0;

    stringGetSize(leftvalue);
    stringGetSize(rightvalue);
    do {
        if(leftvalue->size > rightvalue->size)
        {
            // false
            break;
        }
        else if(leftvalue->size < rightvalue->size)
        {
            result = HAT_TRUE;
            break;
        }
        else 
        {
            len = leftvalue->size;
        }

        if(hat_strncmp(str1,str2, len) <= 0)
        {
            result = HAT_TRUE;
            break;
        }
    }while(0);

    return getDataInfo(&result, VT_INT, resExprData);
}

static PExprDataInfo stringLessOperator(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData)
{
    int result = HAT_FALSE;
    char *str1 = leftvalue->data->pData;
    char *str2 = rightvalue->data->pData;
    int len = 0;

    stringGetSize(leftvalue);
    stringGetSize(rightvalue);
    do {
        if(leftvalue->size > rightvalue->size)
        {
            // false
            break;
        }
        else if(leftvalue->size < rightvalue->size)
        {
            result = HAT_TRUE;
            break;
        }
        else 
        {
            len = leftvalue->size;
        }

        if(hat_strncmp(str1,str2, len) < 0)
        {
            result = HAT_TRUE;
            break;
        }
    }while(0);

    return getDataInfo(&result, VT_INT, resExprData);
}
