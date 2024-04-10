/*
 *	toadb tuples
 * Copyright (C) 2023-2023, senllang
 */

#include "tuples.h"
#include "queryNode.h"
#include "hatstring.h"

#include <stdio.h>



#define hat_log printf

/*
 * 由values list 形成内存中的tuple
 */
PTableRowData ValuesFormRowData(PTableMetaInfo tblMeta, PNode targetList, PNode valueList)
{
    PTableRowData rawRows = NULL;
    PColumnDefInfo colDef = tblMeta->column;
    PValuesData attrValue = NULL;

    int size = 0;
    int index = 0;
    int attrIndex = 0;

    /* num of total column */
    size = tblMeta->colNum * sizeof(PRowColumnData) + sizeof(TableRowData);
    rawRows = (PTableRowData)AllocMem(size);
    // memset(rawRows, 0x00, size);

    rawRows->num = tblMeta->colNum;
    rawRows->size += size;

    /* rawRow fill values order by tblMeta, which is null when stmt isnot input. */
    for (index = 0; index < rawRows->num; index++)
    {
        size = sizeof(RowColumnData);

        /* checking column which will not input in . */
        attrIndex = GetColumnRefIndexByName(colDef[index].colName, (PList)targetList);   
        if (attrIndex < 0)
        {
            /* data is null */
            rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
            rawRows->columnData[index]->size = size;
            rawRows->columnData[index]->attrindex = index;

            rawRows->size += size;
            /* next colum define */
            continue;
        }

        attrValue = GetDataByIndex(attrIndex, (PList)valueList);
        if (attrValue == NULL)
        {
            hat_log("attr and values is not match. \n");
            /* TODO resource release. */
            for (int i = 0; i < tblMeta->colNum; i++)
            {
                if (rawRows->columnData[i] != NULL)
                    FreeMem(rawRows->columnData[i]);
            }
            FreeMem(rawRows);
            rawRows = NULL;
            return rawRows;
        }

        rawRows->columnData[index] = TransformConstValueRowData((PConstValue)attrValue->valueNode, &colDef[index], index);        
        rawRows->size += rawRows->columnData[index]->size;
    }

    return rawRows;
}

PRowColumnData TransformConstValueRowData(PConstValue constValue, PColumnDefInfo colDef, int attrIndex)
{
    PRowColumnData  columnData = NULL;
    int size = sizeof(RowColumnData);

    /* TODO: the value is different type from table define. */
    if (constValue->vt != colDef->type)
    {
        hat_log("value type of column %s is %d, which is different from define type %d.\n",
                colDef->colName, constValue->vt, colDef->type);
    }

    switch (colDef->type)
    {
    case VT_INT:
    case VT_INTEGER:
    {
        int *tmp = NULL;

        size += sizeof(int);
        columnData = (PRowColumnData)AllocMem(size);
        columnData->size = size;
        columnData->attrindex = attrIndex;

        tmp = (int *)(columnData->data);
        *tmp = constValue->val.iData;
    }
    break;
    case VT_VARCHAR:
    case VT_STRING:
    {
        int len = strlen(constValue->val.pData) + 1; /* end with 0 */

        size += len;
        columnData = (PRowColumnData)AllocMem(size);
        columnData->size = size;
        columnData->attrindex = attrIndex;

        memcpy(columnData->data, constValue->val.pData, len);
    }
    break;
    case VT_CHAR:
    {
        int len = strlen(constValue->val.pData);

        /* avoid store '\0' */
        if (len > 0)
            size += sizeof(char);

        columnData = (PRowColumnData)AllocMem(size);
        columnData->size = size;
        columnData->attrindex = attrIndex;

        /* string type receive from std io */
        if (len > 0)
            memcpy(columnData->data, constValue->val.pData, sizeof(char));
    }
    break;
    case VT_DOUBLE:
    case VT_FLOAT:
    {
        float *tmp = NULL;

        size += sizeof(float);
        columnData = (PRowColumnData)AllocMem(size);
        columnData->size = size;
        columnData->attrindex = attrIndex;

        tmp = (float *)(columnData->data);

        /* digest type received that will be int or float. */
        if (constValue->vt == VT_FLOAT)
        {
            *tmp = constValue->val.fData;
        }
        else if (constValue->vt == VT_INT)
        {
            *tmp = constValue->val.iData;
        }
        hat_log("float type is not match values. \n");
    }
    break;
    case VT_BOOL:
        size += sizeof(char);
        columnData = (PRowColumnData)AllocMem(size);
        columnData->size = size;
        columnData->attrindex = attrIndex;
        columnData->data[0] = constValue->val.iData != 0 ? 'T' : 'F';
        break;
    default:
        hat_log("attr and values type is not match. \n");
        /* TODO resource release. */
        break;
    }

    return columnData;
}

/*
 * 将常量值转为pexpredatainfo结构
 */
PExprDataInfo TransformConstExprValue(PConstValue constValue)
{
    PExprDataInfo exprDataInfo = NULL;
    int size = sizeof(ExprDataInfo) + sizeof(Data);

    exprDataInfo = (PExprDataInfo)AllocMem(size);
    exprDataInfo->data = GetDataPointer(exprDataInfo);

    switch (constValue->vt)
    {
    case VT_INT:
    case VT_INTEGER:
    {
        int *tmp = NULL;

        exprDataInfo->type = VT_INTEGER;
        exprDataInfo->size = sizeof(int);

        tmp = (int *)(exprDataInfo->data);
        *tmp = constValue->val.iData;
    }
    break;
    case VT_VARCHAR:
    case VT_STRING:
    {
        int len = strlen(constValue->val.pData) + 1; /* end with 0 */

        exprDataInfo->type = VT_STRING;
        exprDataInfo->size = len;

        exprDataInfo->data->pData = (char *)AllocMem(len);

        memcpy(exprDataInfo->data->pData, constValue->val.pData, len);
    }
    break;
    case VT_CHAR:
    {
        int len = strlen(constValue->val.pData);

        exprDataInfo->type = VT_CHAR;
        exprDataInfo->size = len;

        /* string type receive from std io */
        exprDataInfo->data->cData = *((char*)(constValue->val.pData));
    }
    break;
    case VT_DOUBLE:
    case VT_FLOAT:
    {
        exprDataInfo->type = VT_FLOAT;
        exprDataInfo->size = sizeof(float);

        /* digest type received that will be int or float. */
        if (constValue->vt == VT_FLOAT)
        {
            exprDataInfo->data->fData = constValue->val.fData;
        }
        else if (constValue->vt == VT_INT)
        {
            exprDataInfo->data->fData = constValue->val.iData;
        }
        hat_log("float type is not match values. \n");
    }
    break;
    case VT_BOOL:
        exprDataInfo->type = VT_FLOAT;
        exprDataInfo->size = sizeof(char);

        exprDataInfo->data->cData = constValue->val.iData != 0 ? 'T' : 'F';
        break;
    default:
        /* TODO resource release. */
        FreeMem(exprDataInfo);
        exprDataInfo = NULL;
        hat_log("attr and values type is not match. \n");
        break;
    }

    return exprDataInfo;
}


/*
 * 输入值为当前列的值
 * 从行数据中获取一列的值
 * 反之回值结构
 */
PRowColumnData transFormExpr2RowColumnData(PExprDataInfo exprData, int attrIndex)
{
    PRowColumnData newColData = NULL;
    int dataSize = 0;

    if(NULL == exprData)
        return NULL;   
    
    switch(exprData->type)
    {
        case VT_INT:
        case VT_INTEGER:
        {
            int *tmp = NULL;
            dataSize = sizeof(int) + sizeof(RowColumnData);
            newColData = (PRowColumnData)AllocMem(dataSize);

            tmp = (int *)(newColData->data);
            *tmp = exprData->data->iData;
            newColData->size = dataSize;
        }
        break;

        case VT_VARCHAR:
        case VT_STRING:
        {
            dataSize = hat_strlen(exprData->data->pData) + 1 + sizeof(RowColumnData);
            newColData = (PRowColumnData)AllocMem(dataSize);
            newColData->size = dataSize;

            memcpy(newColData->data, exprData->data->pData, dataSize);
        }
        break;

        case VT_CHAR:
        {
            /* char from input, which is string, only length is 1. */
            dataSize = sizeof(char) + sizeof(RowColumnData);
            newColData = (PRowColumnData)AllocMem(dataSize);
            newColData->size = dataSize;

            newColData->data[0] = *((char*)(exprData->data->pData));            
        }
        break;

        case VT_DOUBLE:
        case VT_FLOAT:
        {
            float *tmp = (float *)(newColData->data);

            dataSize = sizeof(float) + sizeof(RowColumnData);
            newColData = (PRowColumnData)AllocMem(dataSize);
            newColData->size = dataSize;

            *tmp = exprData->data->fData;    
        }
        break;
        
        case VT_BOOL:          
            dataSize = sizeof(float) + sizeof(RowColumnData);
            newColData = (PRowColumnData)AllocMem(dataSize);
            newColData->size = dataSize;      
            
            newColData->data[0] = exprData->data->iData == 1 ? 'T':'F';
        break;

        default:
            hat_log("translate column type is not found. \n");
            /* TODO resource release. */
            return NULL;
        }

        newColData->attrindex = attrIndex;

    return newColData;
}