/*
 *	toadb tables 
 * Copyright (C) 2023-2023, senllang
*/

#include "tables.h"
#include "buffer.h"
#include "tfile.h"
#include "public.h"
#include "memStack.h"
#include "toadmain.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define hat_tables_debug(...)  
// #define hat_tables_debug(...) log_report(LOG_DEBUG, __VA_ARGS__)

static int TableInfoInit(PTableList tblInfo);

int TableOpen(PTableList tblInfo, char *tableName, ForkType forkNum)
{
    PVFVec vfInfo = NULL;
    PageOffset pageoffset = {1,1};
    PPageHeader page = NULL;
    int metadata_size = 0;
    int ret = 0;

    /* add vfinfo linker */
    if(NULL == tblInfo->sgmr)
    {
        tblInfo->sgmr = (PsgmrInfo)AllocMem(sizeof(sgmrInfo));
        tblInfo->sgmr->version = SMGR_VERSION;
        tblInfo->sgmr->vfhead = NULL;
        tblInfo->sgmr->vfend = NULL;
    }

    vfInfo = smgr_open(tblInfo->sgmr, tableName, forkNum);
    if(NULL == vfInfo)
    {
        //hat_log("open table %s-%d failure.", tblInfo->tableDef->tableName, forkNum);
        return -1;
    }

    ret = TableInfoInit(tblInfo);
    if(ret < 0)
    {
        hat_error("[%s][%d]table %s information initialize failure.", __FUNCTION__,__LINE__,tableName);
        return -1;
    }

    return 0;
}

static int TableInfoInit(PTableList tblInfo)
{
    PageOffset pageoffset = {1,1};
    PPageDataHeader page = NULL;
    PTableMetaInfo tblMeta = NULL;
    int size = 0;
    int ret = 0;

    tblInfo->tableInfo = (PPageDataHeader)AllocMem(PAGE_MAX_SIZE);
    pageoffset.pageno = PAGE_HEAD_PAGE_NUM;

    ret = TableRead(tblInfo, &pageoffset, MAIN_FORK, (char *)tblInfo->tableInfo);
    if(ret < 0)
    {
        // hat_log("[%s][%d]read table %s-%d first page failure.", __FUNCTION__,__LINE__,tblInfo->tableDef->tableName, MAIN_FORK);
        return -1;
    }

    page = tblInfo->tableInfo;
    tblMeta = (PTableMetaInfo)((char*)page + PAGE_DATA_HEADER_SIZE);
    size = sizeof(TableMetaInfo) + tblMeta->colNum * sizeof(ColumnDefInfo);    

    tblInfo->tableDef = (PTableMetaInfo)AllocMem(size);
    memcpy(tblInfo->tableDef, tblMeta, size);

    tblInfo->rel = (PRelation)AllocMem(sizeof(Relation));
    tblInfo->rel->relid = tblInfo->tableDef->tableId;
    tblInfo->rel->relType = tblInfo->tableDef->tableType;
    tblInfo->sgmr->storageType = tblInfo->tableDef->tableType;
    
    /* group file */
    tblInfo->groupInfo = (PGroupPageHeader)AllocMem(PAGE_MAX_SIZE);

    ret = TableRead(tblInfo, &pageoffset, GROUP_FORK, (char *)tblInfo->groupInfo);
    if (ret < 0)
    {
        hat_error("[%s][%d]read table %s-%d first page failure.", __FUNCTION__,__LINE__,tblInfo->tableDef->tableName, GROUP_FORK);
        return -1;
    }

    return 0;
}

int TableCreate(PTableList tblInfo, ForkType forkNum)
{
    PVFVec vfInfo = NULL;
    PageOffset pageoffset = {1,1};
    PPageHeader page = NULL;
    int metadata_size = 0;

    /* add vfinfo linker */
    if(NULL == tblInfo->sgmr)
    {
        tblInfo->sgmr = (PsgmrInfo)AllocMem(sizeof(sgmrInfo));
        tblInfo->sgmr->version = 0x01;
        tblInfo->sgmr->storageType = tblInfo->tableDef->tableType;
        tblInfo->sgmr->vfhead = NULL;
        tblInfo->sgmr->vfend = NULL;
    }

    vfInfo = smgr_create(tblInfo->sgmr, tblInfo->tableDef->tableName, forkNum);
    if(NULL == vfInfo)
    {
        smgrRelease(tblInfo->sgmr);
        tblInfo->sgmr = NULL;
        return -1;
    }

    return 0;
}

static int TableFileMainInit(PTableList tblInfo)
{
    char pagebuffer[PAGE_MAX_SIZE] = {0};
    PPageDataHeader pageheader = (PPageDataHeader)pagebuffer;
    PTableMetaInfo tblMetaInfo = (PTableMetaInfo)(pagebuffer + PAGE_DATA_HEADER_SIZE);

    PTableMetaInfo tblDef = tblInfo->tableDef;
    int size = 0;
    int ret = 0;

    /* initialize table infomastion page */
    pageheader->header.pageVersion = PAGE_VERSION;
    pageheader->header.pageType = PAGE_HEADER | tblDef->tableType;
    pageheader->header.pageNum = PAGE_HEAD_PAGE_NUM;
    pageheader->pageCnt = PAGE_HEAD_PAGE_NUM;
    
    size = sizeof(TableMetaInfo) + sizeof(ColumnDefInfo) * tblDef->colNum;
    pageheader->dataOffset = PAGE_DATA_HEADER_SIZE;
    pageheader->dataEndOffset = pageheader->dataOffset + size;

    memcpy(tblMetaInfo, tblDef, size);

    /* flush first page */
    ret = TableWrite(tblInfo, (PPageHeader)pageheader, MAIN_FORK);

    /* init dictionary */
    if(NULL == tblInfo->tableInfo)
    {
        tblInfo->tableInfo = (PPageDataHeader)AllocMem(PAGE_MAX_SIZE);
        memcpy(tblInfo->tableInfo, pageheader, PAGE_DATA_HEADER_SIZE+pageheader->dataEndOffset);
    }
    return ret;
}

static int TableFileGroupInit(PTableList tblInfo)
{
    PageOffset pageoffset = {1,1};
    PGroupPageHeader gpage = NULL;
    PPageDataHeader page = tblInfo->tableInfo;
    PTableMetaInfo tableDef = NULL;
    PVFVec vf;
    int metadata_size = 0;
    int ret = 0;

    /* read first page of table file */
    vf = smgr_open(tblInfo->sgmr, tblInfo->tableDef->tableName, MAIN_FORK);
    if (NULL == vf)
    {
        hat_error("open table %s failure.", tblInfo->tableDef->tableName);
        return -1;
    }

    /* first page content is grouppageheader and table metadata */
    gpage = (PGroupPageHeader)AllocMem(PAGE_MAX_SIZE);

    pageoffset.pageno = PAGE_HEAD_PAGE_NUM;
    ret = TableRead(tblInfo, &pageoffset, MAIN_FORK, (char *)gpage);
    if (ret < 0)
    {
        hat_error("read table %s-%d first page failure.", tblInfo->tableDef->tableName, MAIN_FORK);
        return -1;
    }

    /* group info */
    tableDef = (PTableMetaInfo)((char *)(page) + page->dataOffset);
    gpage->groupInfo.group_id = INVALID_GROUP_ID;
    gpage->groupInfo.columnNum = tableDef->colNum;

    /* table metadata, same as table */
    metadata_size = page->dataEndOffset - page->dataOffset;
    memcpy((char *)gpage + GROUP_PAGE_HEADER_SIZE,
                        tableDef,
                        metadata_size);

    gpage->pageheader.dataOffset = GROUP_PAGE_HEADER_SIZE;
    gpage->pageheader.dataEndOffset = GROUP_PAGE_HEADER_SIZE + metadata_size;

    /* flush first page */
    ret = TableWrite(tblInfo, (PPageHeader)gpage, GROUP_FORK);

    /* init dictionary */
    if(NULL == tblInfo->groupInfo)
    {
        tblInfo->groupInfo = gpage;
    }

    return ret; 
}

int TableFileInit(PTableList tblInfo)
{
    int ret = 0;

    ret = TableFileMainInit(tblInfo);
    ret |= TableFileGroupInit(tblInfo);

    return ret; 
}


int TableRead(PTableList tblInfo, PPageOffset pageoffset, ForkType forkNum, char *page)
{
    PsgmrInfo smgrInfo = NULL;
    PVFVec vpos = NULL;
    int ret = 0;

    smgrInfo = tblInfo->sgmr;
    vpos = SearchVF(smgrInfo->vfhead, forkNum);
    if((NULL == vpos) || (NULL == vpos->pfh))
    {
        // open file 
        vpos = smgr_open(tblInfo->sgmr, tblInfo->tableDef->tableName, forkNum);
        if(NULL == vpos)
        {
            hat_error("open table %s-%d failure.", tblInfo->tableDef->tableName, forkNum);
            return -1;
        }
    }

    ret = smgr_read(vpos, pageoffset, page);

    return ret;
}

int TableWrite(PTableList tblInfo, PPageHeader page, ForkType forkNum)
{
    PsgmrInfo smgrInfo = NULL;
    PageOffset pageoffset;
    PVFVec vpos = NULL;
    int ret = 0;

    smgrInfo = tblInfo->sgmr;
    vpos = SearchVF(smgrInfo->vfhead, forkNum);
    if(NULL == vpos)
    {
        // open file 
        vpos = smgr_open(tblInfo->sgmr, tblInfo->tableDef->tableName, forkNum);
        if(NULL == vpos)
        {
            hat_error("open table %s-%d failure.", tblInfo->tableDef->tableName, forkNum);
            return -1;
        }
    }

    pageoffset.pageno = page->pageNum;
    ret = smgr_write(vpos, &pageoffset, page);

    return ret;
}

int TableDrop(PTableList tblInfo)
{
    int ret = 0;
    char filepath[1024] = {0};

    if(NULL == tblInfo)
    {
        hat_error("exec drop table failure. unknow table name.");
        return -1;
    }

    /* invalid all table buffers. */

    /* delete file main fork */
    ret = DeleteTableFile(tblInfo->tableDef->tableName);
    if(0 != ret)
    {
        hat_error("exec drop %s table failure.", tblInfo->tableDef->tableName);
        return -1;
    }

    /* delete file main fork */
    snprintf(filepath, 1024, "%s%s", GROUP_FILE_PRE, tblInfo->tableDef->tableName);
    ret = DeleteTableFile(filepath);
    return ret;
}

int GetObjectId()
{
    return GetAndIncObjectId();
}


int InsertRowData(PPageDataHeader page, PRowData rowData, PItemData item)
{
    PTableRowData tblRowData = NULL;
    //int size = 0;

    /* first rowdata insert into page buffer */
    tblRowData = &(rowData->rowsData);
    //size = sizeof(TableRowData);

    //item->len = SetItemSize(item->len, size + tblRowData->size);
    //item->offset = page->dataEndOffset - size - tblRowData->size;
    item->len = SetItemSize(item->len, tblRowData->size);
    item->offset = page->dataEndOffset - tblRowData->size;

    ReplaceRowData(page, rowData, item);

    return 0;
}

int InsertItemData(PPageDataHeader page, PItemData item)
{
    char *newRowBuffer = NULL;

    /* item data insert into page buffer */
    newRowBuffer = (char *)page + page->dataOffset;
    memcpy(newRowBuffer, item, sizeof(ItemData));

    /* page buffer offset positon reset */
    page->dataOffset += sizeof(ItemData);
    page->dataEndOffset = item->offset;

    return 0;
}

/*
 * rowdata insert into page buffer.
 * and update item len.
 * data ==>  | columndata1 | columndata2 |... columndatan|
 * NOTE, oldlen >= newlen 
 */
int ReplaceRowData(PPageDataHeader page, PRowData rowData, PItemData item)
{
    int columnIndex = 0;
    char *newRowBuffer = NULL;
    PTableRowData tblRowData = &(rowData->rowsData);

    newRowBuffer = (char *)page + item->offset;    
    //memcpy(newRowBuffer, tblRowData, size);
    //newRowBuffer += size;

    for (columnIndex = 0; columnIndex < rowData->rowsData.num; columnIndex++)
    {
        /* column header and data */
        memcpy(newRowBuffer, tblRowData->columnData[columnIndex], tblRowData->columnData[columnIndex]->size);
        newRowBuffer += tblRowData->columnData[columnIndex]->size;
    }

    return 0;
}

int ReplaceItemData(PPageDataHeader page, PItemData item, int itemIndex)
{
    char *newRowBuffer = NULL;

    /* item data replace */
    newRowBuffer = (char *)page + itemIndex * sizeof(ItemData);
    memcpy(newRowBuffer, &item, sizeof(ItemData));

    return 0;
}

/*
 * format of row data in the page
 * |tablerowDataHead|column1                              | column2                             |..| tablerowDataHead|
 * |total size, num | column total size, attr index, data | column total size, attr index, data |..| ..              |
 * nsm storage model, num maybe is greater 1;
 * pax storage model, num always equal 1;
 */
PRowColumnData ReadRowData(PPageDataHeader page,  int offset)
{
    int colSize = 0;
    PRowColumnData colData = NULL;
    char *pageBuf = ((char *)page + offset);

    /*
     * pagerow memory structure:
     * | RowColumnData  |
     */
    colData = (PRowColumnData)pageBuf;
    colSize = colData->size;
    if(colSize > MAX_ROW_DATA_SIZE)
    {
        hat_error("column size error [%d], page[%d-%d].", colSize, page->header.pageNum, offset);
        return NULL;
    }

    colData = (PRowColumnData)AllocMem(colSize);
    
    hat_tables_debug("read columndata address:%p size:%d", colData, colSize);

    memcpy(colData, pageBuf, colSize);
    return colData;
}

/* 
 * release memory of columns, 
 * which is include in this rowData.
 */
int ReleaseRowDataColumns(PTableRowData rowData)
{
    int num = 0;
    if(NULL == rowData)
        return 0;
    
    num = rowData->num;

    for(; num > 0; num--)
    {
        hat_tables_debug("release columndata address:%p index:%d", rowData->columnData[num-1], num-1);

        FreeMem(rowData->columnData[num-1]);
        rowData->columnData[num-1] = NULL;
    }

    rowData->size = 0;
    rowData->num = 0;
    return num;
}