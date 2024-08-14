/*
 *	toadb data types
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

#include "dataTypes.h"
#include "string.h"
#include "hatstring.h"
#include "memStack.h"
#include <math.h>

char *ColumnType[VT_MAX] = 
{
    "UNKNOWN",
	"pointer",
    "int",
	"integer",
	"varchar",
    "string",
	"char",
    "double",
	"float",
    "bool"
};

int GetColumnType(char *typename)
{
    int index = -1;

    for(int i = 0; i < VT_MAX; i++)
    {
        if(strcmp(typename, ColumnType[i]) == 0)
            index = i;
    }

    return index;
}

int getDataTypeSize(void *pval, valueType type)
{
    int valsize = 0;
    switch (type) 
    {  
        case VT_POINTER:  
            valsize = sizeof(pval);
            break;  
        case VT_INT:              
        case VT_INTEGER:  
            valsize = sizeof(int);
            break;  
        case VT_VARCHAR:  
        case VT_STRING:  
            valsize = hat_strlen(pval);
            break;  
        case VT_CHAR:  
            valsize = sizeof(char); 
            break;  
        case VT_DOUBLE:  
            valsize = sizeof(double);  
            break;  
        case VT_FLOAT:  
            valsize = sizeof(float);   
            break;  
        case VT_BOOL:  
            valsize = sizeof(char); 
            break;  
        default:   
            break;  
    }
    return valsize;
}

PExprDataInfo getDataInfo(void *pval, valueType type, PExprDataInfo resExprData)
{
    PExprDataInfo result = resExprData;

    if((NULL == pval) || (NULL == result))
        return NULL;

    result->type = type;

    switch (type) 
    {  
        case VT_POINTER:  
            result->size = getDataTypeSize(pval, type);
            result->data.pData = pval;
            break;  
        case VT_INT:              
        case VT_INTEGER:  
            result->size = getDataTypeSize(pval, type);
            result->data.iData = *((int *)pval);
            break;  
        case VT_VARCHAR:  
        case VT_STRING:  
            result->size = getDataTypeSize(pval, type);
            result->data.pData = pval;
            break;  
        case VT_CHAR:  
            result->size = getDataTypeSize(pval, type);
            result->data.cData = *((char *)pval);
            break;  
        case VT_DOUBLE:  
            result->size = getDataTypeSize(pval, type);
            result->data.dData = *((double *)pval);  
            break;  
        case VT_FLOAT:  
            result->size = getDataTypeSize(pval, type);
            result->data.fData = *((float *)pval); 
            break;  
        case VT_BOOL:  
            result->size = getDataTypeSize(pval, type);
            result->data.iData = *((char *)pval); 
            break;  
        default:   
            break;  
    }
    return result;
}

HAT_BOOL getDataBool(PExprDataInfo dataInfo)
{
    HAT_BOOL result = HAT_FALSE;

    if(NULL == dataInfo)
        return HAT_TRUE;

    switch (dataInfo->type) 
    {  
        case VT_POINTER:  
            if(dataInfo->data.pData != NULL)
                result = HAT_TRUE;
            break;  
        case VT_INT:              
        case VT_INTEGER:  
            if(dataInfo->data.iData != 0)
                result = HAT_TRUE;
            break;  
        case VT_VARCHAR:  
        case VT_STRING:  
            if((dataInfo->data.pData != NULL) && (dataInfo->size > 0))
                result = HAT_TRUE;
            break;  
        case VT_CHAR:  
            if(dataInfo->data.cData != '\0')
                result = HAT_TRUE;
            break;  
        case VT_DOUBLE:  
            if(fabs(dataInfo->data.dData) > FLOAT_EPSILON)
                result = HAT_TRUE;  
            break;  
        case VT_FLOAT:  
            if(fabs(dataInfo->data.fData) > FLOAT_EPSILON)
                result = HAT_TRUE;  
            break;  
        case VT_BOOL:  
            if(dataInfo->data.iData != 0)
                result = HAT_TRUE;
            break;  
        default:   
            break;  
    }

    return result;
}