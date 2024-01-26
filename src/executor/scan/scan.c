/*
 *	toadb scan
 * Copyright (C) 2023-2023, senllang
*/

#include "scan.h"
#include "public.h"

#include <stddef.h>
#include <string.h>


/* 
 * Initialize scanState ,which is alive untile scan end. 
 * add this table to the scan table list. 
 */
PScanState InitScanState(PTableList tblInfo, PScan scan)
{
    PScanState scanState = NULL;

    scanState = (PScanState)AllocMem(sizeof(ScanState));
    memset(scanState, 0x00, sizeof(ScanState));
    
    if(scan != NULL)
    {
        AddCellToListTail(((PDList*)&(scan->list)), (void *)scanState);
    }

    if(tblInfo != NULL)
    {
        scanState->columnNum = tblInfo->tableDef->colNum;
    }

    scanState->tblInfo = tblInfo;
    scanState->scanInfo = scan;

    return scanState;
}

PScanState GetScanState(PTableList tblInfo, PScanState scanStateHead)
{
    PScanState scanState = NULL;

    return scanState;
}

/*
 * transform common row to scanState row with position infomation.
 */
PScanTableRowData TransFormScanRowData(PTableRowData rowData, PScanState scanState)
{
    PScanTableRowData scanTblRowData = NULL;
    PTableRowDataPosition tblRowDataPos = NULL;
    PRowDataPosition rowDataPosition = NULL;

    if((NULL == rowData) || (NULL == scanState))
    {
        return NULL;
    }

    rowDataPosition = (PRowDataPosition)AllocMem(sizeof(RowDataPosition));
    rowDataPosition->rowData = rowData;
    rowDataPosition->scanPostionInfo = scanState->scanPostionInfo;

    tblRowDataPos = (PTableRowDataPosition)AllocMem(sizeof(TableRowDataPosition));
    tblRowDataPos->tblInfo = scanState->tblInfo;
    tblRowDataPos->rowDataPosition = rowDataPosition;
    tblRowDataPos->rowNum = 1;

    scanTblRowData = (PScanTableRowData)AllocMem(sizeof(ScanTableRowData));
    scanTblRowData->tableNum = 1;       /* only scan maybe on one table. */
    scanTblRowData->tableRowData = tblRowDataPos;
    

    return scanTblRowData;
}

PTableRowDataPosition GetTblRowDataPosition(PScanTableRowData scanTblRow, PTableList tblInfo)
{
    PTableRowDataPosition *ptblRowPosition = NULL;
    int tblIndex = 0;

    if((NULL == scanTblRow) || (NULL == tblInfo))
    {
        return NULL;
    }

    ptblRowPosition = &(scanTblRow->tableRowData);
    for(tblIndex = 0; tblIndex < scanTblRow->tableNum; tblIndex++)
    {   
        if(NULL == *ptblRowPosition)
            break;
                 
        if(tblInfo == (*ptblRowPosition)->tblInfo)
        {
            break;
        }

        ptblRowPosition++;
    }

    if(tblIndex >= scanTblRow->tableNum)
        *ptblRowPosition = NULL;
    return *ptblRowPosition;
}



/*
 * getting rowdata, specified colmn infomation.
 */
PTableRowData GetColRowData(PTableRowDataPosition tblRowPosition, PColumnRef colDef)
{
    PTableRowData colRowData = NULL;
    
    int *colIndexArr = NULL;
    int attrIndex = -1;
    
    int pageno = -1, pageOffset = -1;
    int i = 0;

    if(tblRowPosition->rowNum <= 0)
    {
        return NULL;
    }

    /* column index of table metadata. */
    attrIndex = GetAttrIndex(tblRowPosition->tblInfo, colDef->field);
    if(attrIndex < 0)
    {
        return NULL;
    }

    /* coldata is already readed? */
    colIndexArr = tblRowPosition->rowDataPosition->scanPostionInfo->colindexList;
    for(i = 0; i < tblRowPosition->rowDataPosition->scanPostionInfo->pageListNum; i++)
    {
        if(attrIndex == colIndexArr[i])
        {
            /* TODO: alread exist */
            colRowData = FormColData2RowData(tblRowPosition->rowDataPosition->rowData->columnData[i]);
            return colRowData;
        }
    }
    
    /* coldata read from page. */
    if(tblRowPosition->rowDataPosition->scanPostionInfo->pageListNum > 0)
        pageOffset = tblRowPosition->rowDataPosition->scanPostionInfo->searchPageList->item_offset;

    pageno = GetPageNoFromGroupInfo(&(tblRowPosition->rowDataPosition->scanPostionInfo->groupPageInfo), attrIndex);

    colRowData = GetRowDataFromPageByIndex(tblRowPosition->tblInfo, pageno, pageOffset);

    return colRowData;
}


/*
 * 输入值为当前列的值
 * 从行数据中获取一列的值
 * 反之回值结构
 */
Data * TranslateRawColumnData(PTableRowData rawRows, PColumnRef colDef)
{
    Data *pvalue = NULL;	 

    pvalue = (Data *)AllocMem(sizeof(Data));

    switch(colDef->vt)
    {
        case VT_INT:
        case VT_INTEGER:
        {
            int *tmp = NULL;

            tmp = (int *)(rawRows->columnData[0]->data);
            pvalue->iData = *tmp;
        }
        break;

        case VT_VARCHAR:
        case VT_STRING:
        {
            int len = rawRows->columnData[0]->size;
            pvalue->pData = (void*)AllocMem(len);
            memcpy(pvalue->pData, rawRows->columnData[0]->data, len);
        }
        break;

        case VT_CHAR:
        {
            /* char from input, which is string, only length is 1. */
            int len = sizeof(char) + 1;
            pvalue->pData = (void*)AllocMem(len);
            memset(pvalue->pData, 0x00, len);

            memcpy(pvalue->pData, rawRows->columnData[0]->data, len-1);
        }
        break;

        case VT_DOUBLE:
        case VT_FLOAT:
        {
            float *tmp = (float *)(rawRows->columnData[0]->data);
            pvalue->fData = *tmp;
        }
        break;
        
        case VT_BOOL:                
            pvalue->iData = rawRows->columnData[0]->data[0] == 'T' ? 1:0;
        break;

        default:
            hat_log("translate column type is not found. \n");
            /* TODO resource release. */
            return NULL;
        }

    return pvalue;
}