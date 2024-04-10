/*
 *	toadb seqscan
 * Copyright (C) 2023-2023, senllang
*/

#include "seqscan.h"
#include "tfile.h"
#include "node.h"
#include "execNode.h"
#include "queryNode.h"

#include <stdio.h>
#include <string.h>
#include "public.h"

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
        hat_log("ScanTableForRawRow argments is invalid\n");
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
        ReleasePage(tblScan->page);
        tblScan->page = NULL;
    } while (1);

     /* second get row data */
    rawrow = DeFormRowData(page, tblScan->pageOffset);
    if(NULL == rawrow)
    {
        ReleasePage(tblScan->page);
        tblScan->page = NULL;
    }
    else 
    {
        /* notice: here size is include column pointer, must be except from pageOffset. */
        tblScan->pageOffset += rawrow->size - rawrow->num * sizeof(PRowColumnData);
    }
    
    return rawrow;
}

/*
 * sequence scan all rows in the pages of the group. 
 */
PTableRowData SeqScanRawRowForPages(PTableList tbl, PScanState tblScan)
{
    PTableRowData *rawcolrow = NULL;
    PTableRowData rawrow = NULL;
    int index = 0;

    if(NULL == tbl || tblScan == NULL)
    {
        hat_log("ScanTableForRawRow argments is invalid\n");
        return NULL;
    }

    rawcolrow = (PTableRowData *)AllocMem(sizeof(PTableRowData) * tblScan->scanPageInfo.pageListNum);
    /* second get row data, column data is in the every group member pages. */
    for(index = 0; index < tblScan->scanPageInfo.pageListNum; index++)
    {
        tblScan->scanPageInfo.searchPageList[index].page = tblScan->scanPageInfo.pageList[index];
        rawcolrow[index] = GetRowDataFromPage(tbl, &(tblScan->scanPageInfo.searchPageList[index]));
        if(NULL == rawcolrow[index])
        {
            break;
        }
    }

    /* error or end ocur */
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

#if 0
int ScanTable(PTableList tbl, PScanState tblScan)
{
    int num =0; 
    int pageNum = 0;
    PTableRowData rawRow = NULL;
    PDLCell cell = NULL;
    PGroupItemData groupItem = NULL;
    PSearchPageInfo groupSearchPage;
    PPageDataHeader *pagelist = NULL;
    int found = 0;
#ifdef DEBUG_GROUP_SCAN    
    int grouprownum = 0, groupcount = 0;
#endif 

    if(NULL == tbl || NULL == tblScan)
    {
        hat_log("ScanTable argments is invalid\n");
        return -1;
    }
    
    groupSearchPage = &(tblScan->scanPageInfo.groupPageInfo);
    memset(groupSearchPage, 0x00, sizeof(SearchPageInfo));
    
    groupItem = tblScan->scanPageInfo.groupItem;

    /*
     *  table group  search item per page, if NULL when the end of file. 
     */
    do
    {
        /* find one group, until return NULL when reach the end of group file. */
        if(NULL == groupItem)
        {
            found = GetGroupInfo(tbl, groupSearchPage, groupItem);
            if(found <= 0)
                break; 
#ifdef DEBUG_GROUP_SCAN                
            grouprownum = 0;
            groupcount ++;
#endif            
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
#ifdef DEBUG_GROUP_SCAN        
        grouprownum++;
#endif        
    }while(1); /* until this table end. */

    if(NULL != groupItem)
    {
        FreeMem(groupItem);
    }
    
    tblScan->rowNum = num;
    return num;
}

/*
 * 每次从一张表中扫描符合条件的row，并且记录当前扫描位置
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
        hat_log("select table failure.\n");
        return -1;
    }

    /* this table scanState added to scan struct of query. */
    scanTbl = InitScanState(tblInfo, scan);

    ret = ScanTable(tblInfo, scanTbl);

    return ret;
}
#endif 

PTableRowData ExecSeqscanNode(PExecState eState)
{
    return SeqscanNext(eState);
}

int ExecReSeqScan(PExecState eState)
{
    int pageNum = 0;
    PPageDataHeader *pagelist = NULL;

    PSeqScanState seqScanStateNode = (PSeqScanState)eState->subPlanStateNode;
    PScanState tblScan = seqScanStateNode->scanState;

    PSearchPageInfo groupSearchInfo = &(tblScan->scanPostionInfo->groupPageInfo);

    /* reset group search infomation */
    if(NULL != groupSearchInfo->page)
    {
        ReleasePage(groupSearchInfo->page);
        groupSearchInfo->page = NULL;
        
        groupSearchInfo->item_offset = 0;
        groupSearchInfo->pageNum = PAGE_HEAD_PAGE_NUM+1;
        groupSearchInfo->group_id = 0;
    }

    /* reset page list infomation */
    pageNum = tblScan->scanPostionInfo->pageListNum;
    pagelist = tblScan->scanPostionInfo->pageList;

    if(NULL != pagelist)
    {
        ReleasePageList(pagelist, pageNum);
        tblScan->scanPostionInfo->pageReset = HAT_TRUE;
    }

    return 0;
}

PTableRowData ExecSeqscanNodeEnd(PExecState eState)
{
    return SeqscanEnd(eState);
}

/*
*  table group  search item per page, 
*  if return NULL when the end of file. 
*/
PTableRowData SeqscanNext(PExecState eState)
{
    int pageNum = 0;
    PTableRowData rawRow = NULL;
    PPageDataHeader *pagelist = NULL;

    PSeqScan plan = (PSeqScan)eState->subPlanNode;
    PSeqScanState seqScanStateNode = (PSeqScanState)eState->subPlanStateNode;
    PScanState tblScan = seqScanStateNode->scanState;

    int found = 0;

    if(NULL == tblScan->scanPostionInfo)
    {
        hat_error("table scanPostionInfo is NULL\n");
        return NULL;
    }

    pageNum = tblScan->scanPostionInfo->pageListNum;
    pagelist = tblScan->scanPostionInfo->pageList;
    found = 1 - tblScan->scanPostionInfo->pageReset;

    do
    {
        /* maybe first time, or next group. */
        if (found <= 0)
        {
            /*
            * page list of per group, every attr will has multiple pages.
            */
            found = GetGroupMemberPagesOpt(tblScan->tblInfo, 
                                            tblScan->scanPostionInfo);
            if (found <= 0)
                break;

            tblScan->scanPostionInfo->pageReset = found;
        }

        /* search row from group pages. and all attributes form one row. */
        rawRow = SeqScanNextColumnOpt(tblScan->tblInfo, tblScan);
        if (NULL != rawRow)
        {
            /* found one row data */
            tblScan->rowNum = 1;
            break;
        }

        /* this group completed to scanning, then release postion infomation. */
        ReleasePageList(pagelist, pageNum);         

        found = HAT_FALSE;
    }while(found <= 0);

    return rawRow;
}

PTableRowData SeqscanEnd(PExecState eState)
{
    return NULL;
}


/*
 * sequence scan all rows in the pages of the group. 
 * return rows maybe not all columns, which is specified by caller.
 */
PTableRowData SeqScanNextColumnOpt(PTableList tbl, PScanState tblScan)
{
    PTableRowData *rawcolrow = NULL;
    PTableRowData rawrow = NULL;
    int index = 0;

    if(NULL == tbl || tblScan == NULL)
    {
        hat_log("ScanTableForRawRow argments is invalid\n");
        return NULL;
    }

    /* column num which we want is not all the table defined. */
    rawcolrow = (PTableRowData *)AllocMem(sizeof(PTableRowData) * tblScan->scanPostionInfo->pageListNum);

    /* second get row data, column data is in the every group member pages. */
    for(index = 0; index < tblScan->scanPostionInfo->pageListNum; index++)
    {
        tblScan->scanPostionInfo->searchPageList[index].page = tblScan->scanPostionInfo->pageList[index];

        if(tblScan->scanPostionInfo->pageReset == HAT_TRUE)
            tblScan->scanPostionInfo->searchPageList[index].item_offset = 0;

        rawcolrow[index] = GetRowDataFromPage(tbl, &(tblScan->scanPostionInfo->searchPageList[index]));
        if(NULL == rawcolrow[index])
        {
            break;
        }
    }

    if(tblScan->scanPostionInfo->pageReset == HAT_TRUE)
        tblScan->scanPostionInfo->pageReset = HAT_FALSE;

    /* error or end ocur */
    if(index != tblScan->scanPostionInfo->pageListNum)
    {
        goto END;
    }

    /* using colrow form one rawrow. */
    if(tblScan->scanPostionInfo->pageListNum > 1)
    {
        rawrow = FormCol2RowData(rawcolrow, tblScan->scanPostionInfo->pageListNum);
    }
    else
    {
        rawrow = rawcolrow[0];
    }
     
    return (PTableRowData)TransFormScanRowData(rawrow, tblScan);
END:
    /* error ocur */
    if(NULL != rawcolrow)
        FreeMem(rawcolrow);
    return NULL;
}

/* 
 * Initialize search position at the first time.
 */
PScanPageInfo InitScanPositionInfo(PExecState eState)
{
    PSeqScanState seqScanStateNode = (PSeqScanState)eState->subPlanStateNode;
    PSeqScan seqScanPlaneNode = (PSeqScan)eState->subPlanNode;
    PRangTblEntry rte = (PRangTblEntry)seqScanPlaneNode->rangTbl;

    PScanState tblScan = seqScanStateNode->scanState;
    PScanPageInfo scanPositionInfo = NULL;

    PList targetList = seqScanPlaneNode->targetList;
    foreachWithSize_define_Head;
    int colIndex = 0;
    int colNum = 0;
    int memScan = 0, colindexSize = 0, groupItemSize = 0;
    int searchPageInfoSize = 0, pageListSize = 0;

    if((NULL == tblScan->tblInfo) || (NULL == targetList))
    {
        hat_error("table metadata is NULL\n");
        return NULL;
    }

    colNum = tblScan->tblInfo->tableDef->colNum;
    /* scanPositionInfo */
    memScan = sizeof(ScanPageInfo);
    
    /* colindexList */
    colindexSize = sizeof(int) * colNum; 

    /* searchPageList which space is over or equal real needed. */
    searchPageInfoSize = sizeof(SearchPageInfo) * colNum;

    /* pageList, num same to searchPageInfo */
    pageListSize = sizeof(PPageDataHeader) * colNum;

    /* groupItem */
    groupItemSize = sizeof(GroupItemData) 
                    + (sizeof(MemberData) + sizeof(PageOffset)) * colNum;

    scanPositionInfo = (PScanPageInfo)AllocMem(
                                memScan + colindexSize 
                                + searchPageInfoSize + pageListSize 
                                + groupItemSize);
    
    /* Column list we want will be initialize. */
    scanPositionInfo->colindexList = (int *)((char*)scanPositionInfo + memScan);
    foreachWithSize(targetList, tmpCell, listLen)
    {
        PTargetEntry targetEntry = (PTargetEntry)GetCellNodeValue(tmpCell);
        
        if(targetEntry->rindex != rte->rindex)
            continue;
        
        PResTarget restarget = (PResTarget)targetEntry->colRef;
        PColumnRef colDef = (PColumnRef)restarget->val;
        
        scanPositionInfo->colindexList[colIndex] = GetAttrIndex(tblScan->tblInfo, colDef->field);

        colIndex++;
    } 

    scanPositionInfo->pageListNum = colIndex;
    if(colIndex > colNum)
    {
        hat_error("init pagelist, request %d ,but table only %d columns.\n", colIndex, colNum);
        return NULL;
    }

    /* searchPageInfo space */
    memScan += colindexSize;
    scanPositionInfo->searchPageList = (PSearchPageInfo)((char*)scanPositionInfo + memScan);

    memScan += searchPageInfoSize;
    scanPositionInfo->pageList = (PPageDataHeader *)((char*)scanPositionInfo + memScan);
    
    /* 
     * group item save space. 
     * total size of groupitemdata+ N * (memberdata,pageoffset)
     */
    memScan += pageListSize;
    scanPositionInfo->groupItem = (PGroupItemData)((char*)scanPositionInfo + memScan);

    /* 
     * initialize member values.
     */
    scanPositionInfo->groupPageInfo.pageNum = PAGE_HEAD_PAGE_NUM+1;
    scanPositionInfo->pageReset = HAT_TRUE;

    return scanPositionInfo;
}