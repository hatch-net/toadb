/*
 *	toadb tuples 
 * Copyright (C) 2023-2023, senllang
 */

#include "tuples.h"
#include "queryNode.h"

#include <stdio.h>
#include <string.h>

#define log printf

/*
 * 由values list 形成内存中的tuple
 */
PTableRowData ValuesFormRowData(PTableMetaInfo tblMeta, PNode targetList, PNode valueList)
{
    PTableRowData rawRows = NULL;
    PColumnDefInfo colDef = tblMeta->column;
    
    PValuesData attrValue = NULL;
    PConstValue attrData = NULL;      /* node type of values list */

    int size = 0;
    int index = 0;
    int attrIndex = 0;
    
    /* num of total column */
    size = tblMeta->colNum * sizeof(PRowColumnData) + sizeof(TableRowData);
    rawRows = (PTableRowData)AllocMem(size);
    //memset(rawRows, 0x00, size);

    rawRows->num = tblMeta->colNum;
    rawRows->size += size;

    /* rawRow fill values order by tblMeta, which is null when stmt isnot input. */
    for(index = 0; index < rawRows->num; index++)
    {
        size = sizeof(RowColumnData);
        
        /* checking column which will not input in . */
        attrIndex = GetColumnRefIndexByName(colDef[index].colName, (PList)targetList);
        if(attrIndex < 0)
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
        if(attrValue == NULL)
        {
            log("attr and values is not match. \n");
            /* TODO resource release. */
            for(int i = 0; i < tblMeta->colNum; i++)
            {
                if(rawRows->columnData[i] != NULL)
                    FreeMem(rawRows->columnData[i]);
            }
            FreeMem(rawRows);
            rawRows = NULL;
            return rawRows;
        }

        attrData = (PConstValue)attrValue->valueNode;
        
        /* TODO: the value is different type from table define. */
        if(attrData->vt != colDef[index].type)
        {
            log("value type of column %s is %d, which is different from define type %d.\n", 
                    colDef[index].colName, attrData->vt, colDef[index].type);
        }

        switch(colDef[index].type)
        {
            case VT_INT:
            case VT_INTEGER:
            {
                int *tmp = NULL;

                size += sizeof(int);
                rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
                rawRows->columnData[index]->size = size;
                rawRows->columnData[index]->attrindex = index;

                tmp = (int *)(rawRows->columnData[index]->data);
                *tmp = attrData->val.iData;
            }
                break;
            case VT_VARCHAR:
            case VT_STRING:
            {
                int len = strlen(attrData->val.pData)+1;

                size += len;
                rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
                rawRows->columnData[index]->size = size;
                rawRows->columnData[index]->attrindex = index;

                memcpy(rawRows->columnData[index]->data, attrData->val.pData, len);
            }
                break;
            case VT_CHAR:
                size += sizeof(char);
                rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
                rawRows->columnData[index]->size = size;
                rawRows->columnData[index]->attrindex = index;
                rawRows->columnData[index]->data[0] = attrData->val.cData ;
                break;
            case VT_DOUBLE:
            case VT_FLOAT:
            {
                float *tmp = NULL;

                size += sizeof(float);
                rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
                rawRows->columnData[index]->size = size;
                rawRows->columnData[index]->attrindex = index;

                tmp = (float *)(rawRows->columnData[index]->data);
                *tmp = attrData->val.fData;
            }
                break;
            case VT_BOOL:
                size += sizeof(char);
                rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
                rawRows->columnData[index]->size = size;
                rawRows->columnData[index]->attrindex = index;
                rawRows->columnData[index]->data[0] = attrData->val.iData != 0?'T':'F';
                break;
            default:
                log("attr and values type is not match. \n");
                /* TODO resource release. */
                return rawRows;
        }

        rawRows->size += size;
    }

    return rawRows;
}