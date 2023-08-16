/*
 *	toadb seqscan
 * Copyright (C) 2023-2023, senllang
*/

#include "seqscan.h"
#include "tfile.h"
#include <stdio.h>
#include <string.h>

#define log printf

/*
 * there, we read one row from table, 
 * from tblScan marked offset and index of page.
 * rowdata is store into tblScan.
 * NOTE: only N-Atrr storage mode page 
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

    /* 
     * maybe scan the end of table. 
     * at first time , pageIndex = 0;
    */
    if(tblScan->pageIndex < 0)
        return NULL;

    /* skip first page */
    if(tblScan->pageIndex == 0)
    {
        tblScan->pageIndex = PAGE_HEAD_PAGE_NUM + 1;
        tblScan->page = NULL;
        tblScan->tblInfo = tbl;
    }

    do
    {
        if(NULL == tblScan->page)
        {
            page = GetPageByIndex(tbl, tblScan->pageIndex, MAIN_FORK);
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
        if(tblScan->pageOffset < page->dataEndOffset)
        {
            /* page find. */
            break;
        }

        tblScan->pageIndex++;
        FreeMem(tblScan->page);
        tblScan->page = NULL;
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


PTableRowData SeqScanRawRowForPages(PTableList tbl, PScanState tblScan)
{
    PTableRowData *rawcolrow = NULL;
    PTableRowData rawrow = NULL;
    int index = 0;

    if(NULL == tbl || tblScan == NULL)
    {
        log("ScanTableForRawRow argments is invalid\n");
        return NULL;
    }

    rawcolrow = (PTableRowData *)AllocMem(sizeof(PTableRowData) * tblScan->scanPageInfo.pageListNum);
    /* second get row data */
    for(index = 0; index < tblScan->scanPageInfo.pageListNum; index++)
    {
        tblScan->scanPageInfo.searchPageList[index].page = tblScan->scanPageInfo.pageList[index];
        rawcolrow[index] = GetRowDataFromPage(tbl, &(tblScan->scanPageInfo.searchPageList[index]));
        if(NULL == rawcolrow[index])
        {
            break;
        }
    }

    if(index != tblScan->scanPageInfo.pageListNum)
    {
        rawrow = NULL;
        goto END;
    }

    /* using colrow form one rawrow. */
    if(tblScan->scanPageInfo.pageListNum > 1)
    {
        rawrow = FormCol2RowData(rawcolrow, tblScan->scanPageInfo.pageListNum);
    }
    else
    {
        rawrow = rawcolrow[0];
        goto ENDONE;
    }
END:
    /* error ocur */
    for(index = 0; index < tblScan->scanPageInfo.pageListNum; index++)
    {
        if(NULL != rawcolrow[index])
            FreeMem(rawcolrow[index]);
    }

ENDONE:
    FreeMem(rawcolrow);
    
    return rawrow;
}

int ScanTable(PTableList tbl, PScanState tblScan)
{
    int num =0, grouprownum = 0, groupcount = 0;;
    int pageNum = 0;
    PTableRowData rawRow = NULL;
    PDLCell cell = NULL;
    PGroupItemData groupItem = NULL;
    PSearchPageInfo groupSearchPage;
    PPageDataHeader *pagelist = NULL;
    
    if(NULL == tbl || NULL == tblScan)
    {
        log("ScanTable argments is invalid\n");
        return -1;
    }
    
    groupSearchPage = &(tblScan->scanPageInfo.groupPageInfo);
    memset(groupSearchPage, 0x00, sizeof(SearchPageInfo));

    /*
     *  table group  search item per page, if NULL when the end of file. 
     */
    do
    {
        /* find one group, until return NULL when reach the end of group file. */
        if(NULL == groupItem)
        {
            groupItem = GetGroupInfo(tbl, groupSearchPage);
            if(NULL == groupItem)
                break; 
            grouprownum = 0;
            groupcount ++;
            /*
            * page list of per group, every attr will has multiple pages.
            */
            pagelist = GetGroupMemberPages(tbl, groupItem, &pageNum);
            if(NULL == pagelist)
                break;

            /* record search page list */
            tblScan->scanPageInfo.pageList = pagelist;
            tblScan->scanPageInfo.pageListNum = pageNum;
            tblScan->scanPageInfo.searchPageList = (PSearchPageInfo)AllocMem(pageNum * sizeof(SearchPageInfo));
        }

        /* search row from group pages. and all attributes form one row. */
        rawRow = SeqScanRawRowForPages(tbl, tblScan);
        if(NULL == rawRow)
        {
            ReleasePageList(pagelist, pageNum);
            pagelist = NULL;

            FreeMem(tblScan->scanPageInfo.searchPageList);
            tblScan->scanPageInfo.searchPageList = NULL;

            FreeMem(groupItem);
            groupItem = NULL;   
            continue;
        }

        /* add to tblScan */
        AddCellToListTail(&(tblScan->rows), rawRow);
        num ++;
        grouprownum++;
    }while(1); /* until this table end. */

    if(NULL != groupItem)
    {
        FreeMem(groupItem);
    }
    
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
    PScanState scanTbl = NULL;
    int ret = 0;

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

/* 
 * Initialize scanState ,which is alive untile scan end. 
 * add this table to the scan table list. 
 */
PScanState AddScanStateNode(PTableList tblInfo, PScan scan)
{
    PScanState scanState = NULL;

    if(scan == NULL)
        return NULL;

    scanState = (PScanState)AllocMem(sizeof(ScanState));

    AddCellToListTail(((PDList*)&(scan->list)), (void *)scanState);
    memset(scanState, 0x00, sizeof(ScanState));

    scanState->tblInfo = tblInfo;
    scanState->columnNum = tblInfo->tableDef->colNum;

    return scanState;
}

PScanState GetScanState(PTableList tblInfo, PScanState scanStateHead)
{
    PScanState scanState = NULL;

    return scanState;
}

