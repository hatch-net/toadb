/*
 *	toadb seqscan
 * Copyright (C) 2023-2023, senllang
*/

#include "seqscan.h"
#include "tfile.h"
#include "node.h"
#include "execNode.h"
#include "queryNode.h"
#include "memStack.h"
#include "toadmain.h"
#include "undopage.h"

#include <stdio.h>
#include <string.h>
#include "public.h"

// #define hat_debug_seqscan(...) log_report(LOG_DEBUG, __VA_ARGS__) 
#define hat_debug_seqscan(...) 
#define hat_debug_seqscan_mem(...) 

static int ResetColumnInUsed(PScanPageInfo scanPageInfo);

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

    hat_debug_seqscan("Sescan reset.");

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
    hat_debug_seqscan("Sescan reset finish.");
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
        hat_error("table scanPostionInfo is NULL");
        return NULL;
    }

    hat_debug_seqscan("Sescan scan starting.");

    pageNum = tblScan->scanPostionInfo->pageListNum;
    pagelist = tblScan->scanPostionInfo->pageList;
    found = 1 - tblScan->scanPostionInfo->pageReset;

    tblScan->snapshot = eState->snapshot;

    /* release column memory */
    ResetColumnInUsed(tblScan->scanPostionInfo);

    do
    {
        /* maybe first time, or next group. */
        if (found <= 0)
        {
            hat_debug_seqscan("Sescan scan find next page group.");
            /*
            * page list of per group, every attr will has multiple pages.
            */
            found = GetGroupMemberPagesOpt(tblScan->tblInfo, 
                                            tblScan->scanPostionInfo);
            if (found <= 0)
                break;

            tblScan->scanPostionInfo->pageReset = found;
        }

        hat_debug_seqscan("Sescan scan find next row.");
        /* search row from group pages. and all attributes form one row. */
        rawRow = SeqScanNextColumnOpt(tblScan->tblInfo, tblScan);
        if (NULL != rawRow)
        {
            /* found one row data */
            tblScan->rowNum = 1;
            break;
        }

        hat_debug_seqscan("Sescan scan page one group end.");
        
        /* this group completed to scanning, then release postion infomation. */
        ReleasePageList(pagelist, pageNum);         

        found = HAT_FALSE;
    }while(found <= 0);

    hat_debug_seqscan("Sescan scan find one row.");

    return rawRow;
}

PTableRowData SeqscanEnd(PExecState eState)
{
    hat_debug_seqscan("Sescan scan clean.");
    return NULL;
}


/*
 * sequence scan all rows in the pages of the group. 
 * return rows maybe not all columns, which is specified by caller.
 */
#define SCAN_NEXT_ROW               0
#define SCAN_END_OF_FILE            1
#define IGNORE_INVISIBILITY_COLUMNS 2
PTableRowData SeqScanNextColumnOpt(PTableList tbl, PScanState tblScan)
{
    PRowColumnData *rawcolrow = NULL;
    PTableRowData rawrow = NULL;
    PScanTableRowData scantblRowData = NULL;
    TupleVisibility isVisible = TUPLE_VISIBLE;
    int index = 0;
    int flag = SCAN_NEXT_ROW;

    if(NULL == tbl || tblScan == NULL)
    {
        hat_error("ScanTableForRawRow argments is invalid");
        return NULL;
    }

    hat_debug_seqscan("Sescan seach row begin.");

    /* column num which we want is not all the table defined. */
    rawcolrow = tblScan->scanPostionInfo->rowData->rowsData.columnData;

NEXT:
    /* second get row data, column data is in the every group member pages. */
    for(index = 0; index < tblScan->scanPostionInfo->pageListNum; index++)
    {
        tblScan->scanPostionInfo->searchPageList[index].page = tblScan->scanPostionInfo->pageList[index];

        if(tblScan->scanPostionInfo->pageReset == HAT_TRUE)
            tblScan->scanPostionInfo->searchPageList[index].item_offset = 0;

        rawcolrow[index] = GetRowDataFromPageEx(tbl, &(tblScan->scanPostionInfo->searchPageList[index]));
        if(NULL == rawcolrow[index])
        {
            flag = SCAN_END_OF_FILE;
            break;
        }    

        /* we jump over invisibility columns at the rest. */
        if(IGNORE_INVISIBILITY_COLUMNS == flag)
            continue;

        isVisible = TupleVisibilitySatisfy(&rawcolrow[index]->headerData, tblScan->snapshot);
        if(TUPLE_VISIBLE != isVisible)
        {
            /* when this row version is invisible, we continue to search version chain. */
            rawcolrow[index] = CurrentReadVersion(rawcolrow[index], tblScan);
            if(NULL == rawcolrow[index])
            {
                /* skip this rows */
                flag = IGNORE_INVISIBILITY_COLUMNS;
                continue;
            }
        }       
    }

    if(tblScan->scanPostionInfo->pageReset == HAT_TRUE)
        tblScan->scanPostionInfo->pageReset = HAT_FALSE;

    /* error or end ocur */
    if(SCAN_END_OF_FILE == flag)
    {
        return NULL;
    }

    if(IGNORE_INVISIBILITY_COLUMNS == flag)
    {
        /* TODO: release rawcolrows */
        ResetColumnInUsed(tblScan->scanPostionInfo);

        flag = SCAN_NEXT_ROW;
        goto NEXT; 
    }

    hat_debug_seqscan("Sescan find raw row with rolumn data.");

    /* using colrow form one rawrow. */
    FormCol2RowDataEx2(rawcolrow, 
                        tblScan->scanPostionInfo->pageListNum, 
                        tblScan->scanPostionInfo->rowData);
    
    hat_debug_seqscan("Sescan add row to scan structure.");

    scantblRowData = tblScan->scanPostionInfo->scanTableRow;
    scantblRowData->tableNum = 1;
    scantblRowData->tableRowData->rowNum = 1;
    scantblRowData->tableRowData->rindex = tblScan->rindex;
    scantblRowData->tableRowData->tblInfo = tblScan->tblInfo;
    scantblRowData->tableRowData->rowDataPosition[0]->scanPostionInfo = tblScan->scanPostionInfo;
    scantblRowData->tableRowData->rowDataPosition[0]->rowData = &(tblScan->scanPostionInfo->rowData->rowsData);

    return (PTableRowData)scantblRowData;
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
    int searchPageInfoSize = 0, pageListSize = 0, scanRowSize = 0;

    if((NULL == tblScan->tblInfo) || (NULL == targetList))
    {
        hat_error("table metadata is NULL");
        return NULL;
    }

    colNum = tblScan->tblInfo->tableDef->colNum;
    /* scanPositionInfo */
    memScan = sizeof(ScanPageInfo);
    
    /* colindexList and flags */
    colindexSize = sizeof(int) * colNum; 

    /* searchPageList which space is over or equal real needed. */
    searchPageInfoSize = sizeof(SearchPageInfo) * colNum;

    /* pageList, num same to searchPageInfo */
    pageListSize = sizeof(PPageDataHeader) * colNum;

    /* groupItem */
    groupItemSize = sizeof(GroupItemData) 
                    + (sizeof(MemberData) + sizeof(PageOffset)) * colNum;

    scanRowSize =  SCANTBLROW_HEADER_SIZE  // ScanTblRow Size
                    + sizeof(PRowColumnData) * colNum + ROW_DATA_HEADER_SIZE;       // RowData size                 

    scanPositionInfo = (PScanPageInfo)AllocMem(
                                memScan + colindexSize + colindexSize
                                + searchPageInfoSize + pageListSize 
                                + groupItemSize + scanRowSize );
    
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
        hat_error("init pagelist, request %d ,but table only %d columns.", colIndex, colNum);
        return NULL;
    }

    /* colInused flag array */
    memScan += colindexSize;
    scanPositionInfo->colInusedList = (int *)((char*)scanPositionInfo + memScan);

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
     * scanTableRow member's memory
     */
    memScan += groupItemSize ;
    scanPositionInfo->scanTableRow = (PScanTableRowData)((char*)scanPositionInfo + memScan);

    memScan += sizeof(ScanTableRowData);
    scanPositionInfo->scanTableRow->tableRowData = (PTableRowDataPosition)((char*)scanPositionInfo + memScan);

    memScan += sizeof(TableRowDataPosition) + sizeof(PRowDataPosition);
    scanPositionInfo->scanTableRow->tableRowData->rowDataPosition[0] = (PRowDataPosition)((char*)scanPositionInfo + memScan);

    memScan += sizeof(RowDataPosition);
    scanPositionInfo->rowData = (PRowData)((char*)scanPositionInfo + memScan);

    /* 
     * initialize member values.
     */
    scanPositionInfo->groupPageInfo.pageNum = PAGE_HEAD_PAGE_NUM+1;
    scanPositionInfo->pageReset = HAT_TRUE;
    scanPositionInfo->initColNum = colNum;

    return scanPositionInfo;
}

/* release column memory, which is not used. */
extern int runMode;
static int ResetColumnInUsed(PScanPageInfo scanPageInfo)
{
    int *inUsedFlag = NULL;
    PRowColumnData *colData = NULL;
    int index = 0;

    inUsedFlag = scanPageInfo->colInusedList;
    colData = scanPageInfo->rowData->rowsData.columnData;

    for( ; index < scanPageInfo->pageListNum; index++)
    {
        if(HAT_TRUE == inUsedFlag[index])
        {
            /* reset */
            inUsedFlag[index] = HAT_FALSE;

            /* 
             * under the cs_mode, rowdata is not stored in list,
             * which is send to client, so we don't reserve memory space.
             */
            if(runMode != TOADSERV_CS_MODE_SERVER)
                continue;
        }
        
        hat_debug_seqscan_mem("reset column address:%p, index:%d", colData[index], index);
        FreeMem(colData[index]);
    }

    return 0;
}