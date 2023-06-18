/*
 *	toadb seqscan
 * Copyright (C) 2023-2023, senllang
*/

#include "seqscan.h"

#include <stdio.h>
#include <string.h>

#define log printf

/*
 * there, we read one row from table, 
 * from tblScan marked offset and index of page.
 * rowdata is store into tblScan.
*/
PTableRowData SeqScanRawRow(PTableList tbl, PScanState tblScan)
{
    PTableRowData rowData = NULL;
    PTableRowData rawrow = NULL;
    PPageDataHeader page = NULL;

    if(NULL == tbl || tblScan == NULL)
    {
        log("ScanTableForRawRow argments is invalid\n");
        return NULL;
    }

    /* maybe scan the end of table. */
    if(tblScan->pageIndex < 0)
        return NULL;

    /* skip first page */
    if(tblScan->pageIndex == 0)
    {
        tblScan->pageIndex = PAGE_HEAD_PAGE_NUM+1;
        tblScan->page = NULL;
        tblScan->tblInfo = tbl;
    }

    do
    {
        if(NULL == tblScan->page)
        {
            page = GetPageByIndex(tbl, tblScan->pageIndex);
            if(NULL == page)
            {
                /* scan end of this table. */
                tblScan->pageIndex = -1;    
                return NULL;
            }

            tblScan->page = page;
            tblScan->pageOffset = PAGE_DATA_HEADER_SIZE;
        }

        page = tblScan->page;
        /* search rowdata untile end of file */
        if(tblScan->pageOffset >= page->dataEndOffset)
        {
            tblScan->pageIndex++;
            FreeMem(tblScan->page);
            tblScan->page = NULL;
        }
        else
        {
            /* page find. */
            break;
        }
    } while (1);

     /* second get row data */
    rawrow = DeFormRowData(page, tblScan->pageOffset);
    if(NULL != rawrow)
    {
        /* notice: here size is include column pointer, must be except from pageOffset. */
        tblScan->pageOffset += rawrow->size - rawrow->num * sizeof(PRowColumnData);
    }

    return rawrow;
}

int ScanTable(PTableList tbl, PScanState tblScan)
{
    int num =0;
    PTableRowData rawRow = NULL;
    PDLCell cell = NULL;

    if(NULL == tbl || tblScan == NULL)
    {
        log("ScanTable argments is invalid\n");
        return -1;
    }
    
    do
    {
        rawRow = SeqScanRawRow(tbl, tblScan);
        if(NULL == rawRow)
        {
            break;
        }

        /* add to tblScan */
        AddCellToListTail(&(tblScan->rows), rawRow);
        num ++;
    } while (1);
    
    tblScan->rowNum = num;
    return num;
}

/*
 * 每次从每张表中扫描符合条件的row，并且记录当前扫描位置
 * 将扫描到的row记录在scanState中
*/
int ScanOneTblRows(char *tblName, PScan scan)
{
    PTableList tblInfo = NULL;
    int ret = 0;
    PScanState scanTbl = NULL;

    /* get table information */
    tblInfo = GetTableInfo(tblName);
    if (NULL == tblInfo)
    {
        log("select table failure.\n");
        return -1;
    }

    scanTbl = AddScanStateNode(tblInfo, scan);

    ret = ScanTable(tblInfo, scanTbl);

    return ret;
}

PScanState AddScanStateNode(PTableList tblInfo, PScan scan)
{
    PScanState scanState = NULL;

    if(scan == NULL)
        return NULL;

    scanState = (PScanState)AllocMem(sizeof(ScanState));

    AddCellToListTail(((PDList*)&(scan->list)), (void *)scanState);
    memset(scanState, 0x00, sizeof(ScanState));

    return scanState;
}

PScanState GetScanState(PTableList tblInfo, PScanState scanStateHead)
{
    PScanState scanState = NULL;

    return scanState;
}

