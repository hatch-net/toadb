/*
 *	toadb scan
 * Copyright (C) 2023-2023, senllang
*/

#include "scan.h"
#include "public.h"
#include "memStack.h"

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
    scanState->currentRowData = NULL;
    
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
    char *pMem = NULL;
    int memSize = 0;

    if(NULL == rowData)
    {
        return NULL;
    }

    memSize = sizeof(RowDataPosition) + sizeof(TableRowDataPosition) + sizeof(PRowDataPosition) + sizeof(ScanTableRowData);
    pMem = AllocMem(memSize);

    rowDataPosition = (PRowDataPosition)pMem;
    rowDataPosition->rowData = rowData;

    memSize = sizeof(RowDataPosition);
    tblRowDataPos = (PTableRowDataPosition)(pMem + memSize);
    tblRowDataPos->rowNum = 1;
    tblRowDataPos->rowDataPosition[0] = rowDataPosition;    

    memSize += sizeof(TableRowDataPosition) + sizeof(PRowDataPosition);
    scanTblRowData = (PScanTableRowData)(pMem + memSize);
    scanTblRowData->tableNum = 1;       /* only scan maybe on one table. */
    scanTblRowData->tableRowData = tblRowDataPos;    

    if(NULL != scanState)
    {
        tblRowDataPos->tblInfo = scanState->tblInfo;
        tblRowDataPos->rindex = scanState->rindex;
        rowDataPosition->scanPostionInfo = scanState->scanPostionInfo;
    }
    
    return scanTblRowData;
}

int ReleaseRowData(PScanTableRowData scanRowData)
{
    PTableRowData rowData = NULL;

    if(NULL == scanRowData)
        return -1;

    rowData = scanRowData->tableRowData->rowDataPosition[0]->rowData;

    if(NULL != rowData)
        FreeMem(GetAddrByMember(rowData, rowsData, RowData));

    FreeMem(scanRowData);
    return 0;
}

PTableRowDataPosition GetTblRowDataPosition(PScanTableRowData scanTblRow, PTableList tblInfo, int rindex)
{
    PTableRowDataPosition *ptblRowPosition = NULL;
    PTableRowDataPosition result = NULL;
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
            
        if((tblInfo == (*ptblRowPosition)->tblInfo) && (rindex == (*ptblRowPosition)->rindex))
        {
            break;
        }

        ptblRowPosition++;
    }

    result = *ptblRowPosition;
    if(tblIndex >= scanTblRow->tableNum)
        result = NULL;
    return result;
}


PRowColumnData GetColRowDataEx(PTableRowDataPosition tblRowPosition, PColumnRef colDef, PAttrDataPosition attrPosData, int rowIndex, int *found, int setUsed)
{
    PRowColumnData colData = NULL;
    PHeapItemData heapItem = NULL;

    int *colIndexArr = NULL;
    int attrIndex = -1;
    
    int pageno = -1, pageOffset = -1;
    int i = 0;

    if((rowIndex < 0) || (rowIndex >= tblRowPosition->rowNum))
    {
        return NULL;
    }

    /* column index of table metadata. */
    attrIndex = colDef->attrIndex;
    if(attrIndex < 0)
    {
        return NULL;
    }

    /* coldata is already readed? */
    colIndexArr = tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->colindexList;
    for(i = 0; i < tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->pageListNum; i++)
    {
        if(attrIndex == colIndexArr[i])
        {
            colData = tblRowPosition->rowDataPosition[rowIndex]->rowData->columnData[i];

            if(setUsed)
                tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->colInusedList[i] = HAT_TRUE;
            
            if(NULL != attrPosData)
            {
                attrPosData->headItem.pageno.pageno = tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->pageList[i]->header.pageNum;
                attrPosData->headItem.itemOffset = tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->searchPageList->item_offset;
                attrPosData->headItem.itemData = *((PItemData)GET_ITEM(attrPosData->headItem.itemOffset, 
                                            tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->pageList[i]));
            }

            *found = HAT_TRUE;
            return colData;
        }
    }
    
    /* below code, it maybe run repeatly at one column data, which not in rang table's target list. */
    /* coldata read from page. */
    if(tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->pageListNum > 0)
        pageOffset = tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->searchPageList->item_offset;

    pageno = GetPageNoFromGroupInfo(&(tblRowPosition->rowDataPosition[rowIndex]->scanPostionInfo->groupPageInfo), attrIndex);

    if(NULL != attrPosData)
        heapItem = &(attrPosData->headItem);
        
    colData = GetRowDataFromPageByIndexEx(tblRowPosition->tblInfo, pageno, pageOffset, heapItem);

    *found = HAT_FALSE;
    return colData;
}


/*
 * 输入值为当前列的值
 * 从行数据中获取一列的值
 * 反之回值结构
 */
Data * TranslateRawColumnData(PRowColumnData colData, PColumnRef colDef, Data *dataValue)
{
    Data *pvalue = NULL;	 

    pvalue = dataValue;

    switch(colDef->vt)
    {
        case VT_INT:
        case VT_INTEGER:
        {
            int *tmp = NULL;

            tmp = (int *)(colData->data);
            pvalue->iData = *tmp;
        }
        break;

        case VT_VARCHAR:
        case VT_STRING:
        {
            int len = colData->size;
            pvalue->pData = (void*)AllocMem(len);
            memcpy(pvalue->pData, colData->data, len);
        }
        break;

        case VT_CHAR:
        {
            /* char from input, which is string, only length is 1. */
            int len = sizeof(char) + 1;
            pvalue->pData = (void*)AllocMem(len);
            memset(pvalue->pData, 0x00, len);

            memcpy(pvalue->pData, colData->data, len-1);
        }
        break;

        case VT_DOUBLE:
        case VT_FLOAT:
        {
            float *tmp = (float *)(colData->data);
            pvalue->fData = *tmp;
        }
        break;
        
        case VT_BOOL:                
            pvalue->iData = colData->data[0] == 'T' ? 1:0;
        break;

        default:
            hat_log("translate column type is not found. ");
            /* TODO resource release. */
            return NULL;
        }

    return pvalue;
}