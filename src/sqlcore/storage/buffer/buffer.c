/*
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
 *
 * toadb buffer
 *
 */

#include "buffer.h"
#include "exectable.h"
#include "tfile.h"
#include "scan.h"

#include "public.h"
#include "config_pub.h"
#include "bufferPool.h"
#include "memStack.h"
#include "resourceMgr.h"
#include "hashtab.h"
#include "atom.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

// #define hat_bufpool_debug(...) log_report(LOG_DEBUG, __VA_ARGS__) 
#define hat_bufpool_debug(...) 
// #define hat_bufcontext_debug(...)  log_report(LOG_DEBUG, __VA_ARGS__)
#define hat_bufcontext_debug(...)

//#define hat_buflock_debug(...) log_report(LOG_DEBUG, __VA_ARGS__) 
#define hat_buflock_debug(...)

/* 内存缓冲区 */
PBufferPoolContext g_bufferPoolContext = NULL;

/* 数据块缓冲区 hashtable */
PHashTableInfo g_bufferHashTable = NULL;

static PBufferPoolContext GetBufferPoolContext();

static PBufferElement AllocBuffer(PBufferPoolContext bufferPool, PBufferTag bufferTag, int *found);
static BUFFERID SearchBuffer(PBufferPoolContext bufferPool, PBufferTag bufferTag);
static int CleanPageBuffer(PPageDataHeader page, BUFFERID bufferId, PBufferDesc bufDesc) ;

int CreateBufferPool(int pageNum)
{
    int size = 0;

    if (pageNum <= 0)
        pageNum = 128;

    size = sizeof(BufferPoolContext);
    size += pageNum * sizeof(BufferElement);
    size += pageNum * sizeof(BufferDesc);
    size += pageNum * sizeof(int);

    g_bufferPoolContext = AllocMem(size);

    size = InitBufferPool(g_bufferPoolContext, pageNum);
    if (size < 0)
    {
        hat_error("initialize buffer pool faulure.");
    }

    g_bufferHashTable = HashTableCreate(4, pageNum, pageNum, sizeof(BufferPoolHashValue), sizeof(BufferTag), MemMangerGetCurrentContext(), "bufferHashTable");
    if(NULL == g_bufferHashTable)
    {
        return -1;
    }

    return size;
}

int DestroyBufferPool()
{
    ReleaseBufferPool(g_bufferPoolContext, g_bufferPoolContext->bufferNum);

    DestroyHashTable(g_bufferHashTable);
    return 0;
}

static PBufferPoolContext GetBufferPoolContext()
{
    return g_bufferPoolContext;
}

/*
 * 组装一行数据
 * 根据表定义的列，与传入的数据进行匹配，按定义的顺序依次存放数据内容
 * tblMeta中有表的定义：列名，列类型
 * stmt中有对应列和值
 */
PTableRowData FormRowData(PTableMetaInfo tblMeta, PInsertStmt stmt)
{
    PTableRowData rawRows = NULL;
    PColumnDefInfo colDef = tblMeta->column;
    PValuesData attrData = NULL;
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
        /* checking column which will not input in . */
        attrIndex = GetAtrrIndexByName(colDef[index].colName, stmt->attrNameList);
        size = sizeof(RowColumnData);

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

        attrData = GetDataByIndex(attrIndex, stmt->valuesList);
        if (attrData == NULL)
        {
            hat_log("attr and values is not match. ");
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

        switch (colDef[index].type)
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
            *tmp = attrData->value.iData;
        }
        break;
        case VT_VARCHAR:
        case VT_STRING:
        {
            int len = strlen(attrData->value.pData) + 1;

            size += len;
            rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
            rawRows->columnData[index]->size = size;
            rawRows->columnData[index]->attrindex = index;

            memcpy(rawRows->columnData[index]->data, attrData->value.pData, len);
        }
        break;
        case VT_CHAR:
            size += sizeof(char);
            rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
            rawRows->columnData[index]->size = size;
            rawRows->columnData[index]->attrindex = index;
            memcpy(rawRows->columnData[index]->data, attrData->value.pData, sizeof(char));
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
            *tmp = attrData->value.fData;
        }
        break;
        case VT_BOOL:
            size += sizeof(char);
            rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
            rawRows->columnData[index]->size = size;
            rawRows->columnData[index]->attrindex = index;
            rawRows->columnData[index]->data[0] = attrData->value.iData != 0 ? 'T' : 'F';
            break;
        default:
            hat_log("attr and values type is not match. ");
            /* TODO resource release. */
            return rawRows;
        }

        rawRows->size += size;
    }

    return rawRows;
}

/*
 * 从磁盘page中解析一行数据
 * 根据表定义的列，与传入的数据进行匹配，按定义的顺序依次存放数据内容
 * tblMeta中有表的定义：列名，列类型
 * stmt中有对应列和值
 */
PRowData DeFormRowDatEx(PPageDataHeader page, int pageffset)
{
    PRowData tempRow = NULL;
    PRowData pagerow = NULL;
    char *pageBuf = (char *)page;
    char *columnDataPos = NULL;
    int size = 0, offset = 0;
    int *pval = NULL;
    int index = 0;
    int pageBufOffset = 0;

    /* num of total column */
    tempRow = (PRowData)(pageBuf + pageffset);

    /*
     * format of row data in the page
     * |tablerowDataHead|column1                              | column2                             |..| tablerowDataHead|
     * |total size, num | column total size, attr index, data | column total size, attr index, data |..| ..              |
     * nsm storage model, num maybe is greater 1;
     * pax storage model, num always equal 1;
     */
    size = tempRow->rowsData.num * sizeof(PRowColumnData) + sizeof(RowData) + tempRow->rowsData.size;
    pagerow = (PRowData)AllocMem(size);

    /*
     * pagerow memory structure:
     * | RowData ,sizeof(RowData) | num * PRowColumnData | num * RowColumnData , total of size is rowData.size |
     * rowcolumnData order is reverse.
     */
    memcpy(pagerow, tempRow, sizeof(RowData));

    offset = tempRow->rowsData.size;
    columnDataPos = ((char *)pagerow + size);

    pageBuf = (char *)&(tempRow->rowsData);
    pageBufOffset = sizeof(TableRowData);

    /* rawRow fill values order by tblMeta, which is null when stmt isnot input. */
    for (index = 0; index < pagerow->rowsData.num; index++)
    {
        pval = (int *)(pageBuf + pageBufOffset);
        size = *pval;
        if (size <= 0 || size > MAX_ROW_DATA_SIZE)
        {
            FreeMem(pagerow);
            pagerow = NULL;
            hat_log("found bad tuple size %d", size);
            break;
        }

        /* data is null */
        pagerow->rowsData.columnData[index] = (PRowColumnData)(columnDataPos - size);
        memcpy(pagerow->rowsData.columnData[index], pageBuf + pageBufOffset, size);

        pageBufOffset += size;
        columnDataPos -= size;

        offset -= size;
        if (offset < 0)
        {
            hat_error("rowData size %d , space size not enough", offset, tempRow->rowsData.size);

            FreeMem(pagerow);
            pagerow = NULL;
            break;
        }
    }

    return pagerow;
}

/*
 * only for pax mode
 * one tuple has one clumns data.
 */
PRowColumnData DeFormRowDatEx2(PPageDataHeader page, int pageffset)
{
    PRowColumnData colData = NULL;

    colData = ReadRowData(page, pageffset);

    return colData;
}

#if 0
PTableRowData DeFormRowData(PPageDataHeader page, int pageffset)
{
    PRowData tempRow = NULL;

    tempRow = DeFormRowDatEx(page, pageffset);

    return &(tempRow->rowsData);
}
#endif 

/*
 *  按指定大小，查找空闲空间
 *  顺次查找表文件，检查表文件块上的剩余空间;
 *  根据op指定，找不到时，是否需要新建块，还是只在存在的块中查找。
 */
PPageDataHeader GetSpacePage(PTableList tblInfo, int size, PageOp op, ForkType forkNum)
{
    PPageDataHeader page = NULL;
    int pageIndex = PAGE_HEAD_PAGE_NUM + 1; /* first data page start */

    if (size + PAGE_DATA_HEADER_SIZE >= PAGE_MAX_SIZE)
    {
        hat_log("row data size %d is oversize page size %d", size, PAGE_MAX_SIZE);
        return NULL;
    }

    /* sesearch all table file ,from current page. */
    for (;; pageIndex++)
    {
        page = GetPageByIndex(tblInfo, pageIndex, forkNum);
        if (NULL == page)
            break;

        LockPage(page, BUF_READ);
        /* freespace check */
        if (HasFreeSpace(page, size))
        {
            UnLockPage(page, BUF_READ);
            break;
        }
        UnLockPage(page, BUF_READ);

        /* todo: bufferpool */
        ReleasePage(page);
    }

    if (page == NULL && op == PAGE_NEW)
    {
        /* only group file */
        page = ExtensionTbl(tblInfo, 1, forkNum);
    }

    return page;
}

/*
 * free space will be check specified page and it's extension page .
 * if page not found, new page with op;
 * if page is no enough free space, extension next page.
 */
PPageDataHeader GetSpaceSpecifyPage(PTableList tblInfo, int size, PageOp op, ForkType forkNum, int startPageIndex, int pageType)
{
    PPageDataHeader firstpage = NULL;
    PPageDataHeader page = NULL;
    PPageDataHeader extpage = NULL;
    int pageIndex = 0;

    if (size + PAGE_DATA_HEADER_SIZE >= PAGE_MAX_SIZE)
    {
        hat_log("row data size %d is oversize page size %d, pagetype:%d ", size, PAGE_MAX_SIZE, pageType);
        return page;
    }

    /* sesearch all table file ,from current page. */
    do
    {
        page = GetPageByIndex(tblInfo, startPageIndex, forkNum);
        if (NULL == page)
            break;

        LockPage(page, BUF_READ);

        /* first page is original data type. */
        if((firstpage == NULL) && (PAGE_UNDO == pageType))
        {
            firstpage = page;
            startPageIndex = page->undoPage.pageno;
            continue;
        }

        if (GET_PAGE_TYPE(page->header.pageType) != pageType)
        {
            /* todo: bufferpool */
            UnLockPage(page, BUF_READ);
            ReleasePage(page);
            
            if(NULL != firstpage)
            {
                UnLockPage(firstpage, BUF_READ);
                ReleasePage(firstpage);
            }

            hat_error("found error page, page num:%d type:%d ,request type:%d ",
                      page->header.pageNum,
                      GET_PAGE_TYPE(page->header.pageType),
                      pageType);
            return NULL;
        }

        /* freespace check */
        if (HasFreeSpace(page, size))
        {
            UnLockPage(page, BUF_READ);
            if(NULL != firstpage)
            {
                UnLockPage(firstpage, BUF_READ);
                ReleasePage(firstpage);
            }
            return page;
        }

        /*
         * PAGE_DATA extension page type is PAGE_EXTDATA.
         * extension page of PAGE_UNDO is PAGE_UNDO.
         */
        if (PAGE_DATA == pageType)
        {
            pageType = PAGE_EXTDATA;

            /* next extension/undo page. */
            startPageIndex = page->extPage.pageno;
        }
        else 
        {
            /* 
             * here , follow code will be excutor. 
             * head of extension page chain have free space, 
             * accordingly, other extension page don't need check.
            */
            pageIndex++;
        }

        /* TODO: first page will locked until ext/undo ended of adding. */
        if (NULL == firstpage)
        {
            firstpage = page;
        }
        else
        {
            /* first page is not release, which is updated when extension new page. */
            if(page != firstpage)
            {
                UnLockPage(page, BUF_READ);
                ReleasePage(page);
            }
        }

        /* next page search once more. */
        pageIndex++;
    } while (!IsInvalidPageNum(startPageIndex) && (pageIndex < 2));

    /* TODO: we find freespace list at first. */

    /*
     * Get here, all the pages have not enogh space.
     * We plan to extending new page.
     */
    if ((op == PAGE_NEW) && (NULL == extpage))
    {
        StartExtensionTbl(tblInfo);

        extpage = ExtensionTbl(tblInfo, 1, forkNum);
        if(NULL == extpage)
        {
            EndExtensionTbl(tblInfo);
            return NULL;
        }

        LockPage(extpage, BUF_WRITE);
        extpage->header.pageType = SET_PAGE_TYPE(extpage->header.pageType, pageType);
        UnLockPage(extpage, BUF_WRITE);
    }

EXT_NEW:
    if ((firstpage != NULL) && (extpage != NULL))
    {        
        UnLockPage(firstpage, BUF_READ);

        /* page is locked in order that avoid to dead lock.  */
        LockPage(firstpage, BUF_WRITE);
        LockPage(extpage, BUF_WRITE);

        extpage->header.pageType = SET_PAGE_TYPE(extpage->header.pageType, pageType);

        /* 指定块没有空间时，扩展一个块, 这里使用头插法。  */
        if (PAGE_EXTDATA == pageType)
        {
            extpage->extPage.pageno = firstpage->extPage.pageno;
            firstpage->extPage.pageno = extpage->header.pageNum;
        }
        else // PAGE_UNDO
        {
            extpage->undoPage.pageno = firstpage->undoPage.pageno;
            firstpage->undoPage.pageno = extpage->header.pageNum;
        }

        /* todo, maybe set dirty flag only. */
        WritePage(tblInfo, firstpage, forkNum);
        
        UnLockPage(extpage, BUF_WRITE);
        UnLockPage(firstpage, BUF_WRITE);

        EndExtensionTbl(tblInfo);
        
        /* pre block of chain. */
        ReleasePage(firstpage);
    }
    else if(firstpage != NULL)
    {
        UnLockPage(firstpage, BUF_READ);

        /* pre block of chain. */
        ReleasePage(firstpage);
    }
    else 
    {
        /* nothing */
        ;
    }

    return extpage;
}

/*
 * 按指定的page index获取对应的page
 */
PPageDataHeader GetPageByIndex(PTableList tblInfo, int index, ForkType forkNum)
{
    PPageDataHeader page = NULL;

    if (IsInvalidPageNum(index))
    {
        return NULL;
    }

    /*
     * we will not check, it only test by filesystem .
     */
    // if(index > tblInfo->tableInfo->header.pageNum)
    //     return page;

    page = ReadPage(tblInfo, index, forkNum);

    if (NULL != page)
    {
        PushResourceOwner(page, 1);
    }
    return page;
}

/*
 * format of row data in the page
 * |tablerowDataHead|column                               |..|           column|
 * |total size, num | column total size, attr index, data |..| ..              |
 */
int WriteRowData(PTableList tblInfo, PPageDataHeader page, PTableRowData row)
{
    int offset = 0;
    int columnIndex = 0;
    char *newRowBuffer = NULL;
    int writeSize = 0;
    int size = 0;

    LockPage(page, BUF_WRITE);
    /* first rowdata insert into page buffer */
    newRowBuffer = (char *)page + page->dataEndOffset;

    /* rowdata header */
    size = sizeof(TableRowData);
    memcpy(newRowBuffer, row, size);
    newRowBuffer += size;
    page->dataEndOffset += size;

    for (columnIndex = 0; columnIndex < row->num; columnIndex++)
    {
        /* column header and data */
        memcpy(newRowBuffer, row->columnData[columnIndex], row->columnData[columnIndex]->size);

        page->dataEndOffset += row->columnData[columnIndex]->size;
        newRowBuffer += row->columnData[columnIndex]->size;
    }

    /* page buffer write to table file. */
    WritePage(tblInfo, page, MAIN_FORK);
    UnLockPage(page, BUF_WRITE);

    /* end resource release here. */
    FreeMem(row);

    return 0;
}

static int LockGroupPages(PScanPageInfo scanPageInfo, int num)
{
    int ColNum = 0;
    PPageDataHeader *pageList = NULL;

    pageList = scanPageInfo->pageList;
    for(ColNum = 0; ColNum < num; ColNum++)
    {
        LockPage(pageList[ColNum], BUF_WRITE);
    }
    return 0;
}

static int UnLockGroupPages(PScanPageInfo scanPageInfo, int num)
{
    int ColNum = 0;
    PPageDataHeader *pageList = NULL;

    pageList = scanPageInfo->pageList;
    for(ColNum = 0; ColNum < num; ColNum++)
    {
        UnLockPage(pageList[ColNum], BUF_WRITE);
    }
    return 0;
}

static int FreeSpaceCheck(PScanPageInfo scanPageInfo, PTableRowData insertdata)
{
    int columnRowSize = 0;
    int columnRowHeaderSize = sizeof(TableRowData) + sizeof(ItemData);
    int ColNum = 0;
    PPageDataHeader *pageList = NULL;

    pageList = scanPageInfo->pageList;
    for(ColNum = 0; ColNum < insertdata->num; ColNum++)
    {
        columnRowSize = insertdata->columnData[ColNum]->size + columnRowHeaderSize;
        if (!HasFreeSpace(pageList[ColNum], columnRowSize))
        {
            return -1;
        }
    }

    return 0;
}

static int WriteRowOnePage(PTableList tblInfo, PPageDataHeader page, PRowData row)
{
    int ret = 0;

    ret = InsertRowData(page, row, &(row->item));
 
    /* form item data */
    // item.rowid = {0};                                        /* not used */
    row->item.len |= ITEM_VALID_MASK;


    ret |= InsertItemData(page, &(row->item));

    /* page buffer write to table file. */
    WritePage(tblInfo, page, MAIN_FORK);

    row->rowPos.itemIndex = GET_ITEM_INDEX(page) - 1;
    row->rowPos.pageIndex.pageno = page->header.pageNum;

    return ret;
}

int InsertRowDataWithGroup(PTableList tblInfo, PTableRowData insertdata, PScanPageInfo scanPageInfo)
{
    PPageDataHeader *pageList = NULL;
    PRowData rowData = NULL;
    int ColNum = 0;
    int ret = 0;

    pageList = scanPageInfo->pageList;
    rowData = scanPageInfo->rowData;

    /* lock group pages */
    LockGroupPages(scanPageInfo, insertdata->num);

    /* recheck free space */
    ret = FreeSpaceCheck(scanPageInfo, insertdata);
    if(ret < 0)
    {
        ret = ESTAT_NO_SPACE;
        goto ENERET;
    }

    /* insert rows into pages. */
    rowData->rowsData.num = 1;
    for(ColNum = 0; ColNum < insertdata->num; ColNum++)
    {
        rowData->rowsData.columnData[0] = insertdata->columnData[ColNum];
        rowData->rowsData.size = insertdata->columnData[ColNum]->size;

        ret = WriteRowOnePage(tblInfo, pageList[ColNum], rowData);
    }

    ret = ESTAT_SUCESS;
ENERET:
    UnLockGroupPages(scanPageInfo, insertdata->num);
    return ret;
}

/*
 * format of row data in the page
 * two part : item and data;
 * first part item: dataOffset -> ItemData
 * seconde part data:  dataEndOffset -> TableRowData
 *
 * |tablerowDataHead|column                               |..|           column|
 * | RowData | total size, num | column total size, attr index, data |..| ..              |
 */
int WriteRowItemDataWithHeader(PTableList tblInfo, PPageDataHeader page, PRowData row)
{
    int ret = 0;

    LockPage(page, BUF_WRITE);
    ret = WriteRowOnePage(tblInfo, page, row);
    UnLockPage(page, BUF_WRITE);

    return ret;
}

/*
 * insert command, only generate tablerowData.
 */
int WriteRowItemData(PTableList tblInfo, PPageDataHeader page, PTableRowData row)
{
    RowData tmpRowData;
    int ret = 0;

    memcpy(&(tmpRowData.rowsData), row, sizeof(TableRowData));
    ret = WriteRowItemDataWithHeader(tblInfo, page, &tmpRowData);
    return ret;
}

/* 
 * update on the same page, then item not rewrite.
 * item->len is old data size, real data size is written data, 
 * which in order to next update compute space.
 */
int WriteRowDataOnly(PTableList tblInfo, PPageDataHeader page, PRowData row, PItemData oldItem)
{
    int ret = 0;

    LockPage(page, BUF_WRITE);
    ret = ReplaceRowData(page, row, oldItem);

    /* page buffer write to table file. */
    WritePage(tblInfo, page, MAIN_FORK);
    UnLockPage(page, BUF_WRITE);

    return ret;
}

int InitPage(char *page, int flag)
{
    PPageDataHeader pageNew = (PPageDataHeader)page;

    memset(page, 0x00, PAGE_MAX_SIZE);

    pageNew->header.pageVersion = PAGE_VERSION;
    pageNew->header.pageType = PAGE_DATA;
    // pageNew->header.pageNum = tblInfo->tableInfo->header.pageNum + 1;
    pageNew->undoPage.segno = INVALID_SEGMENT_NUM;
    pageNew->undoPage.pageno = INVALID_PAGE_NUM;
    pageNew->extPage = pageNew->undoPage;

    pageNew->dataOffset = PAGE_DATA_HEADER_SIZE;
    pageNew->dataEndOffset = PAGE_DATA_MAX_OFFSET;
    return 0;
}

PPageDataHeader ExtensionTbl(PTableList tblInfo, int num, ForkType forkNum)
{
    PPageDataHeader pageNew = NULL;
    PPageDataHeader pageHeader = NULL;
    char page[PAGE_MAX_SIZE] = {0};
    int i = 0, ret = 0;

    if (num <= 0 || num > PAGE_EXTENSION_MAX_NUM)
    {
        hat_log("num will be extension is invalid.");
        return NULL;
    }

    switch (forkNum)
    {
    case MAIN_FORK:
        pageHeader = tblInfo->tableInfo;
        break;
    case GROUP_FORK:
        pageHeader = &(tblInfo->groupInfo->pageheader);
        break;
    default:
        return NULL;
        break;
    }

    (void)InitPage(page, PAGE_DATA);
    pageNew = (PPageDataHeader)page;

    for (; i < num; i++)
    {
        pageNew->header.pageNum = atomic_fetch_add(&pageHeader->pageCnt, 1) + 1;

        /* low level write to file */
        ret = TableWrite(tblInfo, (PPageHeader)pageNew, forkNum);
    }

    /* write to header page */
    UpdateMetaData(tblInfo, forkNum);

    /* tail page is first use, load to buffer pool. */
    pageNew = GetPageByIndex(tblInfo, pageNew->header.pageNum, forkNum);

    return pageNew;
}

int UpdateMetaData(PTableList tblInfo, ForkType forkNum)
{
    PPageDataHeader page = NULL;

    /* read full page */
    page = GetPageByIndex(tblInfo, PAGE_HEAD_PAGE_NUM, forkNum);

    LockPage(page, BUF_WRITE);

    /* update buffer */
    switch (forkNum)
    {
    case MAIN_FORK:
        UpdateTableMetaData(tblInfo, page);
        break;
    case GROUP_FORK:
        UpdateGroupMetaData(tblInfo, (PGroupPageHeader)page);
        break;
    default:
        break;
    }

    /* write header page */
    WritePage(tblInfo, page, forkNum);
    
    UnLockPage(page, BUF_WRITE);
    ReleasePage(page);
    return 0;
}

int UpdateGroupMetaData(PTableList tblInfo, PGroupPageHeader page)
{
    page->groupInfo.group_id = tblInfo->groupInfo->groupInfo.group_id;
    page->pageheader.pageCnt = tblInfo->groupInfo->pageheader.pageCnt;
    return 0;
}

int UpdateTableMetaData(PTableList tblInfo, PPageDataHeader page)
{
    // PTableMetaInfo tableinfo = NULL;
    page->pageCnt = tblInfo->tableInfo->pageCnt;
    // tableinfo = (PTableMetaInfo) ((char *)page + PAGE_DATA_HEADER_SIZE);
    return 0;
}


/*
 * found :
 *      1, buffer tag is found in buffer pool;
 *      2, free buffer is returned, and tag was assigned;
 *      3, the buffer which is returned is dirty buffer, which will be flushed at the first,
 *          then assigned tag.
 *      4, when tag_valid, buffer is locked until setting valid tag .
 *      all of above, refcnt is added, releasebuffer to minus.
 */
static PBufferElement AllocBuffer(PBufferPoolContext bufferPool, PBufferTag bufferTag, int *found)
{
    BUFFERID bufferId = 0;
    PBufferElement pElemnt = NULL;
    PHashElement hashEntry = NULL;
    HASHKEY key = 0;
    BufferPoolHashValue hashValue;
    PBufferPoolHashValue entryValue = NULL;
    int ret = 0;

REFIND:
    /* search buffer pool */
    bufferId = SearchBuffer(bufferPool, bufferTag);
FIND_RECHECK:            
    if(INVLID_BUFFER != bufferId)
    {
        LockBufDesc(&bufferPool->bufferDesc[bufferId], BUF_WRITE);
        PinBuffer(bufferPool, bufferId);

        /*
         *  recheck tag and isvalid. 
         *  before lock and pin, this buffer may be sweep out by other worker.
         */
        if((bufferTag->databaseId == bufferPool->bufferDesc[bufferId].bufferTag.databaseId) 
                && (bufferTag->tableId == bufferPool->bufferDesc[bufferId].bufferTag.tableId) 
                && (bufferTag->segno == bufferPool->bufferDesc[bufferId].bufferTag.segno) 
                && (bufferTag->pageno == bufferPool->bufferDesc[bufferId].bufferTag.pageno) 
                && (bufferTag->forkNum == bufferPool->bufferDesc[bufferId].bufferTag.forkNum))
        {
            /*
            * invalid -- del, refind
            * valid  -- normal
            * tag_valid -- loading
            */
            if(bufferPool->bufferDesc[bufferId].isValid != BVF_INVALID)
            {
                if(bufferPool->bufferDesc[bufferId].isValid == BVF_TAG)
                {
                    hat_bufpool_debug("empty1 page:%p, bufid:%d tag(%d-%d-%d) desc(%d-%d-%d)", 
                        &(bufferPool->bufferPool[bufferId]), bufferId, 
                        bufferTag->tableId, bufferTag->pageno, bufferTag->forkNum, 
                        bufferPool->bufferDesc[bufferId].isValid, bufferPool->bufferDesc[bufferId].refCnt, bufferPool->bufferDesc[bufferId].usedCnt);

                    /* 
                    * other is loading, we must wait a lot, or we loading it. 
                    * locking until setting valid.
                    */
                    *found = BUFFER_EMPTY;
                }
                else 
                {
                    hat_bufpool_debug("found page:%p, bufid:%d tag(%d-%d-%d) desc(%d-%d-%d)", 
                        &(bufferPool->bufferPool[bufferId]), bufferId, 
                        bufferTag->tableId, bufferTag->pageno, bufferTag->forkNum, 
                        bufferPool->bufferDesc[bufferId].isValid, bufferPool->bufferDesc[bufferId].refCnt, bufferPool->bufferDesc[bufferId].usedCnt);

                    *found = BUFFER_FOUND;
                    UnLockBufDesc(&bufferPool->bufferDesc[bufferId], BUF_WRITE);
                }

                goto RET_BUF;
            }
        }          

        /* invalid, this buffer is sweeping out. */
        unPinBuffer(bufferPool, bufferId);
        UnLockBufDesc(&bufferPool->bufferDesc[bufferId], BUF_WRITE);
    }
    
    /* find free list */
    bufferId = GetFreeBufferId(bufferPool);
    if(INVLID_BUFFER != bufferId)
    {
        LockBufDesc(&bufferPool->bufferDesc[bufferId], BUF_WRITE);
        PinBuffer(bufferPool, bufferId);

        *found = BUFFER_EMPTY;
        goto INIT_TAG;
    }

    /*
     * not found, replace buffer returned.
     * buffer context has other page, which is modified,
     * please flush context at the first. 
     */
    /* find a victim */
    bufferId = ClockSweep(bufferPool);    
    /* swap one buffer */
    if(INVLID_BUFFER != bufferId)
    {
        /* loading process protected by the lock, other will wait lock release. */
        PinBuffer(bufferPool, bufferId);

        /* flush ,  oldtag was deleted from hashtable*/
        CleanPageBuffer((PPageDataHeader)&(bufferPool->bufferPool[bufferId]), bufferId, &bufferPool->bufferDesc[bufferId]);

        /* init tag */
        *found = BUFFER_EMPTY;
    }
    else 
    {
        hat_error("buffer pool is not enogh, victim is not found.");
        return NULL;
    }

    hat_bufpool_debug("empty page:%p, bufid:%d tag(%d-%d-%d) desc(%d-%d-%d)", 
                &(bufferPool->bufferPool[bufferId]), bufferId, 
                bufferTag->tableId, bufferTag->pageno, bufferTag->forkNum, 
                bufferPool->bufferDesc[bufferId].isValid, bufferPool->bufferDesc[bufferId].refCnt, bufferPool->bufferDesc[bufferId].usedCnt);
INIT_TAG:
    
    /* recheck */
    if(BVF_INVALID == bufferPool->bufferDesc[bufferId].isValid)
        bufferPool->bufferDesc[bufferId].isValid = BVF_TAG;
    else
    {
        /* this buffer is used by other worker, we find again. */
        unPinBuffer(bufferPool, bufferId);
        UnLockBufDesc(&bufferPool->bufferDesc[bufferId], BUF_WRITE);
        goto REFIND;
    }

    memcpy(&(bufferPool->bufferDesc[bufferId].bufferTag), bufferTag, sizeof(BufferTag));
    bufferPool->bufferDesc[bufferId].isDirty = BUFFER_CLEAN;
    
    /* new buffer tag, which will add to hashtable. */
    hashValue.bufferId = bufferId;
    hashValue.tag = *bufferTag;
    key = g_bufferHashTable->getHashKey((char*)bufferTag, sizeof(BufferTag));

    /* must be enough. */
    hashEntry = HashGetFreeEntry(g_bufferHashTable, key, (char*)&hashValue, &ret);
    if(NULL == hashEntry)
    {
        unPinBuffer(bufferPool, bufferId);
        UnLockBufDesc(&bufferPool->bufferDesc[bufferId], BUF_WRITE);

        ShowHashTableValues(g_bufferHashTable, bufferPool);
        
        goto REFIND;
    }

    if(HASH_FIND == ret)
    {
        /* 
         * this tag has inserted by other worker,
         * thus the empty buffer release currently, and pin/lock new buffer.
        */
        unPinBuffer(bufferPool, bufferId);
        UnLockBufDesc(&bufferPool->bufferDesc[bufferId], BUF_WRITE);

        entryValue = GetEntryValue(hashEntry);
        bufferId = entryValue->bufferId;

        hat_bufpool_debug("insert(find) hash entry:%p bufid:%d tag(%d-%d-%d)", 
                hashEntry, bufferId, bufferTag->tableId, bufferTag->pageno, bufferTag->forkNum);

        /* recheck */
        goto FIND_RECHECK;
    }

    hat_bufpool_debug("insert hash entry:%p bufid:%d tag(%d-%d-%d)", 
                hashEntry, bufferId, bufferTag->tableId, bufferTag->pageno, bufferTag->forkNum);

RET_BUF:
    pElemnt = &(bufferPool->bufferPool[bufferId]);
    return pElemnt;
}

static BUFFERID SearchBuffer(PBufferPoolContext bufferPool, PBufferTag bufferTag)
{
    BUFFERID id = INVLID_BUFFER;
    PHashElement entry = NULL;
    HASHKEY key = 0;
    PBufferPoolHashValue value = NULL;
    int partition = 0;

    key = g_bufferHashTable->getHashKey((char*)bufferTag, sizeof(BufferTag));

    partition = GetPartitionIndex(key, g_bufferHashTable);
    HashLockPartition(g_bufferHashTable, partition);

    entry = HashFindEntry(g_bufferHashTable, key, (char*)bufferTag, partition);
    if(entry == NULL)
    {
        HashLockPartitionRelease(g_bufferHashTable, partition);
        return INVLID_BUFFER;
    }

    value = (PBufferPoolHashValue)GetEntryValue(entry);
    id = value->bufferId;

    HashLockPartitionRelease(g_bufferHashTable, partition);

    hat_bufpool_debug("SearchBuffer tableId:%d pagenum:(%d-%d-%d) hitbuffer:%d key:%0x partition:%d", 
                    bufferTag->tableId, bufferTag->segno, bufferTag->pageno, bufferTag->forkNum, id, key, partition);

    return id;
}

/* 
 * flush , oldtag was deleted from hashtable
 */
static int CleanPageBuffer(PPageDataHeader page, BUFFERID bufferId, PBufferDesc bufDesc) 
{
    PBufferTag otherBufferTag = NULL;
    Relation otherRelation = {0};
    PTableList otherTblInfo = NULL;
    HASHKEY key = 0;
    BufferPoolHashValue hashValue;
    int ret = 0;

    otherBufferTag = GetBufferTag(GetBufferPoolContext(), (PBufferElement)page);

    otherRelation.databaseId = otherBufferTag->databaseId;
    otherRelation.relid = otherBufferTag->tableId;
    otherRelation.relType = 0;

    /* flush this buffer */
    if(BUFFER_DITRY == bufDesc->isDirty) 
    {
        otherTblInfo = GetTableInfoByRel(&otherRelation);
        FlushPage(otherTblInfo, page, otherBufferTag->forkNum);
    }
    
    /* INVALID TAG */
    bufDesc->isValid = BVF_INVALID;
    hat_bufpool_debug("CleanPageBuffer page:%p, tag(%d-%d-%d) bufferId:%d desc(%d-%d-%d)", 
                page, otherBufferTag->tableId, otherBufferTag->pageno, otherBufferTag->forkNum, 
                bufferId,
                bufDesc->isValid, bufDesc->refCnt, bufDesc->usedCnt);

    /*delete from hashtable */
    hashValue.tag = *otherBufferTag;
    hashValue.bufferId = bufferId;
    key = g_bufferHashTable->getHashKey((char *)otherBufferTag, sizeof(BufferTag));
    ret = HashDeleteEntry(g_bufferHashTable, key, (char *)&hashValue);
        
    return ret;
}

/*
 * index -> pageno
 *
 * find bufferPool, which maybe has this page buffer.
 * otherwise, alloc buffer from buffer pool, and read from file.
 */
PPageDataHeader ReadPage(PTableList tblInfo, int index, ForkType forkNum)
{
    PPageDataHeader page = NULL;
    PageOffset pageoffset;
    BufferTag bufferTag;
    PBufferDesc desc = NULL;

    int found = 0;
    int ret = 0;

    if (index <= 0)
    {
        return NULL;
    }

    pageoffset.pageno = index;

    bufferTag.databaseId = 0;
    bufferTag.tableId = tblInfo->tableDef->tableId;
    bufferTag.segno = 0;
    bufferTag.pageno = index;
    bufferTag.forkNum = forkNum;

    page = (PPageDataHeader)AllocBuffer(GetBufferPoolContext(), &bufferTag, &found);
    if ((BUFFER_FOUND == found) || (NULL == page))
    {
        /* debug check . */
        if((NULL != page) && (page->header.pageNum != bufferTag.pageno))
        {
            desc = GetBufferDesc(GetBufferPoolContext(), (PBufferElement)page);
            hat_error("find bad page %p, pagenum:%d, reqnum:%d forknum:%d tbl:%s  desc(%d-%d-%d)", 
                page, page->header.pageNum, bufferTag.pageno, forkNum, tblInfo->tableDef->tableName, 
                desc->isValid, desc->refCnt, desc->usedCnt);
        }
        return page;
    }

    ret = TableRead(tblInfo, &pageoffset, forkNum, (char *)page);
    if (ret < 0)
    {
        desc = GetBufferDesc(GetBufferPoolContext(), (PBufferElement)page);
        hat_bufpool_debug("page:%p read failure, tag(%d-%d-%d) desc(%d-%d-%d)", 
                page, bufferTag.tableId, bufferTag.pageno, bufferTag.forkNum, desc->isValid, desc->refCnt, desc->usedCnt);
        
        ReleaseBuffer(GetBufferPoolContext(), (PBufferElement)page);
        UnLockPage(page, BUF_WRITE);
        return NULL;
    }

    hat_bufpool_debug("page:%p readed, tag(%d-%d-%d)", 
                page, bufferTag.tableId, bufferTag.pageno, bufferTag.forkNum);

    /* debug check. */
    if(page->header.pageNum != bufferTag.pageno)
    {
        hat_error("find2 bad page %p, pagenum:%d, reqnum:%d forknum:%d tbl:%s", 
                page, page->header.pageNum, bufferTag.pageno, forkNum, tblInfo->tableDef->tableName);
    }

    SetBuffferValid(GetBufferPoolContext(), (PBufferElement)page, BVF_VLID);
    UnLockPage(page, BUF_WRITE);

    return page;
}

int ReleasePage(PPageDataHeader page)
{
    if (NULL == page)
        return -1;

    LockBufDesc(GetBufferDesc(GetBufferPoolContext(), (PBufferElement)page), BUF_WRITE);
    ReleaseBuffer(GetBufferPoolContext(), (PBufferElement)page);
    UnLockBufDesc(GetBufferDesc(GetBufferPoolContext(), (PBufferElement)page), BUF_WRITE);

    ReleaseResourceOwner(page, 1);

    return 0;
}

int WritePage(PTableList tblInfo, PPageDataHeader page, ForkType forkNum)
{
    int ret = 0;

    ret = TableWrite(tblInfo, (PPageHeader)page, forkNum);

    /* cancelling the dirty flag */
    ClearBufferDirty(GetBufferPoolContext(), (PBufferElement)page);
    return ret;
}

int FlushPage(PTableList tblInfo, PPageDataHeader page, ForkType forkNum)
{
    int ret = 0;

    ret = TableWrite(tblInfo, (PPageHeader)page, forkNum);

    /* cancelling the dirty flag */
    ClearBufferDirty(GetBufferPoolContext(), (PBufferElement)page);
    return ret;
}


/*
 * get group info from group file, which is same name prefix as relation file.
 * We will create group file at the first time.
 *
 * group itemdata will be returned one by one from second page to the end of file.
 * returning NULL when file end.
 */
int GetGroupInfo(PTableList tblInfo, PSearchPageInfo searchInfo, PGroupItemData gItemData)
{
    PTableList tblGroup = NULL;
    char groupFileName[FILE_NAME_MAX_LEN] = {0};
    PPageDataHeader page = NULL;
    PGroupItem gItem = NULL;
    PageOffset pageNum = {0, 0};
    int found = 0;

    if (NULL == tblInfo)
    {
        hat_log("invalid parameters ");
        return -1;
    }

    page = searchInfo->page;
    if (NULL == page)
    {
        /* find group info, item scan from page  */
        pageNum.pageno = searchInfo->pageNum;

        page = GetPageByIndex(tblInfo, pageNum.pageno, GROUP_FORK);
        searchInfo->page = page;
        if(NULL == page)
            return -1;

        LockPage(page, BUF_READ);
    }
    else
    {
        LockPage(page, BUF_READ);

        /* get next group item */
        gItem = (PGroupItem)GET_ITEM(searchInfo->item_offset, page);
        gItem += 1;
        searchInfo->pageNum = searchInfo->page->header.pageNum;
    }

    while (page != NULL)
    {
        /* group item postion is start when switch page. */
        if (NULL == gItem)
        {
            gItem = (PGroupItem)page->item;
        }

        /* next page */
        if (ITEM_END_CHECK(gItem, page))
        {
            pageNum.pageno = searchInfo->pageNum + 1;
            gItem = NULL;

            UnLockPage(page, BUF_READ);
            ReleasePage(page);

            page = GetPageByIndex(tblInfo, pageNum.pageno, GROUP_FORK);
            searchInfo->page = page;

            if (NULL != page)
            {
                searchInfo->pageNum = pageNum.pageno;
                LockPage(page, BUF_READ);
            }
            continue;
        }

        if (GetItemValid(gItem->len))
        {
            // gItemData = (PGroupItemData)AllocMem(sizeof(GroupItemData) + GetItemSize(gItem->len));

            /* item infomation */
            memcpy(&(gItemData->ItemData), gItem, sizeof(GroupItem));

            /* member page infomation */
            memcpy(gItemData->memberData, (char *)page + gItem->offset, GetItemSize(gItem->len));

            gItemData->pagePos.pageno = page->header.pageNum;

            searchInfo->group_id = gItem->groupid;
            searchInfo->item_offset = ITEM_OFFSET(gItem, page);

            /* found */
            found = 1;
            break;
        }

        /* next item */
        gItem += 1;
    }

    if(page != NULL)
        UnLockPage(page, BUF_READ);

    return found;
}

/*
 * page No. of column Index is search from specified group.
 * groupInfo, group data is already readed, group id is specified.
 */
int GetPageNoFromGroupInfo(PSearchPageInfo groupInfo, int AttrIndex)
{
    PPageDataHeader page = NULL;
    PGroupItem gItem = NULL;
    PMemberData gMemData = NULL;

    int pageno = -1;

    page = groupInfo->page;
    if (NULL == page)
    {
        return -1;
    }

    gItem = (PGroupItem)GET_ITEM(groupInfo->item_offset, page);
    if (ITEM_END_CHECK(gItem, page) || !GetItemValid(gItem->len))
    {
        return -1;
    }

    gMemData = (PMemberData)((char *)page + gItem->offset);

    /* default is first member */
    pageno = GetGroupMemberPageNo(gMemData, AttrIndex);

    return pageno;
}

/*
 * free space check every group one by one,
 * and inner group pages , from one column pages to another.
 */
int GetSpaceGroupPage(PTableList tblInfo, PTableRowData insertdata, PageOp op, PPageDataHeader *pageList, PScanPageInfo scanInfo)
{
    int colNum = 0;
    PPageDataHeader pageHeader = NULL;
    PGroupItemData groupItem = NULL;
    PSearchPageInfo searchPage = NULL;
    int found = HAT_FALSE;
    int isNoSpace = HAT_FALSE;

    if ((NULL == tblInfo) || (NULL == insertdata) || (NULL == pageList))
    {
        hat_log("invalid parameters ");
        return -1;
    }

    groupItem = scanInfo->groupItem;
    searchPage = scanInfo->searchPageList;
    isNoSpace = scanInfo->isNoSpace;

    /* will extension space, until error ocur. */
    for (;;)
    {
        /* find one group */
        if (isNoSpace)
        {
            found = GetGroupInfo(tblInfo, searchPage, groupItem);
            if (found != HAT_TRUE)
                break;

            isNoSpace = HAT_FALSE;
        }

        for (colNum = 0; colNum < insertdata->num; colNum++)
        {
            pageHeader = GetFreeSpaceMemberPage(tblInfo,
                                                insertdata->columnData[colNum]->size + sizeof(TableRowData) + sizeof(ItemData),
                                                groupItem,
                                                op,
                                                insertdata->columnData[colNum]->attrindex);
            if (NULL == pageHeader)
            {
                /* this group not enogh space */
                ReleasePageListMember(pageList, insertdata->num);
                isNoSpace = HAT_TRUE;
                break;
            }

            pageList[colNum] = pageHeader;
        }

        if (colNum == insertdata->num)
            break;
    }

    scanInfo->isNoSpace = isNoSpace;

    /* error ocur */
    if (colNum < insertdata->num)
        return -1;

    return 0;
}

/*
 * check column pages which have enough free space to insert.
 * row count < Max column count, maybe extension page in the group.
 */
PPageDataHeader GetFreeSpaceMemberPage(PTableList tblInfo, int size, PGroupItemData item, PageOp op, int colIndex)
{
    PPageDataHeader page = NULL;
    PMemberData memberData = NULL;
    int pageIndex = 0;
    int subPageNum = 0;
    int offset = 0;

    /* calculator memberdata offset
       column0: | MemberDataHeader| memNum | pageOffset0 | pageOffset1 | ... | pageOffsetmemNum |
       column1: | MemberDataHeader| memNum | pageOffset0 | pageOffset1 | ... | pageOffsetmemNum |
       multiple pages for columns in one group.
    */
    memberData = item->memberData;
    for (pageIndex = 0; pageIndex < colIndex; pageIndex++)
    {
        offset += sizeof(MemberData) + memberData->memNum * sizeof(PageOffset);
        memberData = (PMemberData)((char *)(item->memberData) + offset);
    }

    /*
     * we will search all subpage of this column.
     * every memberData may be serval pages.
     */
    subPageNum = memberData->memNum - 1;

    /* sesearch all table file ,from current page. */
    pageIndex = memberData->member[subPageNum--].pageno;
    for (; pageIndex != INVALID_PAGE_NUM;)
    {
        page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
        if (NULL == page)
        {
            /* error ocur */
            hat_error("It isn't found group member pageindex: %d, MAIN_FORK, tblName:%s", pageIndex, tblInfo->tableDef->tableName);
            break;
        }

        LockPage(page, BUF_READ);
        if (HasFreeSpace(page, size))
        {
            UnLockPage(page, BUF_READ);
            /* Enough space is found. */
            break;
        }
        UnLockPage(page, BUF_READ);

        /* todo: bufferpool */
        ReleasePage(page);
        page = NULL;

        /* next page as column link */
        if (subPageNum < 0)
        {
            break;
        }
        pageIndex = memberData->member[subPageNum--].pageno;
    }

    return page;
}

PPageDataHeader *GetGroupMemberPages(PTableList tblInfo, PGroupItemData item, int *pageNum)
{
    PPageDataHeader page = NULL;
    PMemberData memberData = NULL;
    PPageDataHeader *pagelist = NULL;
    int pageIndex = 0;
    int subPageNum = 0;
    int offset = 0;

    /* calculator memberdata offset */
    memberData = item->memberData;
    if (NULL == memberData)
    {
        return NULL;
    }

    /*
     * we will search all subpage of this column.
     * every memberData may be serval pages.
     */
    subPageNum = tblInfo->tableDef->colNum;
    pagelist = (PPageDataHeader *)AllocMem(sizeof(PPageDataHeader) * subPageNum);
    *pageNum = subPageNum;

    /* sesearch all table file ,from current page. */
    subPageNum = 0;
    pageIndex = memberData->member[0].pageno; /* default one member */
    offset = sizeof(MemberData) + memberData->memNum * sizeof(PageOffset);

    /*
     * NOTE: every memberData has one member page default.
     */
    for (; pageIndex != INVALID_PAGE_NUM;)
    {
        page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
        if (NULL == page)
        {
            break;
        }

        pagelist[subPageNum] = page;

        /* all column finish */
        if (++subPageNum >= *pageNum)
        {
            break;
        }

        memberData = (PMemberData)((char *)(item->memberData) + offset);
        pageIndex = memberData->member[0].pageno;
        offset += sizeof(MemberData) + memberData->memNum * sizeof(PageOffset);
    }

    if (subPageNum != *pageNum)
    {
        /* error ocur */
        pageIndex = ReleasePageList(pagelist, memberData->memNum);
        pagelist = NULL;

        hat_log("group page invalid. groupmember[%d], pagefind[%d-%d]", memberData->memNum, subPageNum, pageIndex);
    }

    return pagelist;
}

int GetGroupMemberPagesOpt(PTableList tblInfo, PScanPageInfo scanPageInfo)
{
    PPageDataHeader *pagelist = NULL;
    PGroupItemData groupItem = NULL;

    PPageDataHeader page = NULL;
    PMemberData memberData = NULL;

    PSearchPageInfo groupSearchInfo = &(scanPageInfo->groupPageInfo);
    int *colIndex = scanPageInfo->colindexList;

    int pageIndex = 0;
    int subPageNum = 0;
    int offset = 0;
    int colPageIndex = 0;
    int found = 0;

    pagelist = scanPageInfo->pageList;
    groupItem = scanPageInfo->groupItem;
    found = GetGroupInfo(tblInfo, groupSearchInfo, groupItem);
    if (found <= 0)
        goto END;

    /* calculator memberdata offset */
    memberData = groupItem->memberData;
    if (NULL == memberData)
    {
        goto END;
    }

    /*
     * we will search all subpage of this column.
     * every memberData may be serval pages.
     */

    /* sesearch all table file ,from current page. */
    pageIndex = memberData->member[0].pageno; /* default one member */
    offset = sizeof(MemberData) + memberData->memNum * sizeof(PageOffset);

    for (; pageIndex != INVALID_PAGE_NUM;)
    {
        if (colPageIndex == colIndex[subPageNum])
        {
            page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
            if (NULL == page)
            {
                break;
            }

            pagelist[subPageNum] = page;

            /* next page as column link */
            if (++subPageNum >= scanPageInfo->pageListNum)
            {
                break;
            }
        }

        colPageIndex++;
        memberData = (PMemberData)((char *)(groupItem->memberData) + offset);
        pageIndex = memberData->member[0].pageno;
        offset += sizeof(MemberData) + memberData->memNum * sizeof(PageOffset);
    }

    if (subPageNum != scanPageInfo->pageListNum)
    {
        /* error ocur */
        pageIndex = ReleasePageList(pagelist, memberData->memNum);
        found = 0;
        // hat_log("opt group page invalid. groupmember[%d], pagefind[%d-%d]", memberData->memNum, subPageNum, pageIndex);
    }

END:
    return found;
}

int ReleasePageList(PPageDataHeader *pagelist, int num)
{
    int pageIndex = 0;

    while (pagelist[pageIndex] != NULL)
    {
        ReleasePage(pagelist[pageIndex]);
        pagelist[pageIndex] = NULL;

        if (++pageIndex >= num)
            break;
    }

    return pageIndex;
}

int ReleasePageListMember(PPageDataHeader *pagelist, int num)
{
    int pageIndex = 0;

    while (pagelist[pageIndex] != NULL)
    {
        ReleasePage(pagelist[pageIndex]);
        pagelist[pageIndex] = NULL;

        if (++pageIndex >= num)
            break;
    }

    return pageIndex;
}

int GetInvlidRowCount(PPageDataHeader page)
{
    int rowNum = (page->dataOffset - ITEM_DATA_START_OFFSET) / sizeof(ItemData);
    int rowTotal = rowNum;
    PItemData pItem = &(page->item[0]);

    for (; rowNum > 0; rowNum--, pItem++)
    {
        if (!GetItemValid(pItem->len))
            rowTotal--;
    }

    return rowTotal;
}


PTableRowData FormColData2RowData(PRowColumnData colRows)
{
    PTableRowData rowData = NULL;
    int rowDataSize = sizeof(TableRowData) + sizeof(PRowColumnData);

    if (NULL == colRows)
    {
        hat_log("coldata to rowdata invliad argments");
        return NULL;
    }

    rowData = (PTableRowData)AllocMem(rowDataSize);

    rowData->columnData[0] = colRows;
    rowData->size = colRows->size;
    rowData->num = 1;

    return rowData;
}

PTableRowData FormCol2RowData(PTableRowData *colRows, int colNum)
{
    PTableRowData rowData = NULL;
    PRowData rowsData = NULL;
    int rowDataSize = sizeof(PRowColumnData) * colNum + sizeof(RowData);
    int index = 0;

    if (NULL == colRows || NULL == colRows[0])
    {
        hat_log("colrow to rowdata invliad argments");
        return NULL;
    }

    rowsData = (PRowData)AllocMem(rowDataSize);
    rowData = &(rowsData->rowsData);

    rowDataSize = 0; /* reuse. row data real size */
    for (index = 0; index < colNum; index++)
    {
        rowDataSize += colRows[index]->size;

        if (colRows[index]->size > 0)
            rowData->columnData[index] = colRows[index]->columnData[0];
        else
        {
            rowData->columnData[index] = NULL;
        }
    }

    rowData->size = rowDataSize;
    rowData->num = colNum;

    return rowData;
}

int FormCol2RowDataEx(PRowColumnData *colRows, int colNum, PRowData rowsData)
{
    PTableRowData rowData = NULL;
    int rowDataSize = 0;
    int index = 0;

    if (NULL == colRows || NULL == colRows[0])
    {
        hat_log("colrow to rowdata invliad argments");
        return -1;
    }

    rowData = &(rowsData->rowsData);

    for (index = 0; index < colNum; index++)
    {
        rowDataSize += colRows[index]->size;

        if (colRows[index]->size > 0)
            rowData->columnData[index] = colRows[index];
        else
        {
            rowData->columnData[index] = NULL;
        }
    }

    rowData->size = rowDataSize;
    rowData->num = colNum;

    return index;
}

/*
 * columns data put in position, then init the tablerowdata structure.
 */
int FormCol2RowDataEx2(PRowColumnData *colRows, int colNum, PRowData rowsData)
{
    PTableRowData rowData = NULL;
    int rowDataSize = 0;
    int index = 0;

    if (NULL == colRows || NULL == colRows[0])
    {
        hat_log("colrow to rowdata invliad argments");
        return -1;
    }

    rowData = &(rowsData->rowsData);

    for (index = 0; index < colNum; index++)
    {
        rowDataSize += colRows[index]->size;
    }

    rowData->size = rowDataSize;
    rowData->num = colNum;

    return index;
}

int InsertGroupItem(PTableList tblInfo, PPageDataHeader lastpage, int num)
{
    PPageDataHeader page = NULL;
    PGroupItemData groupItemData = NULL;
    int size = 0;
    int i = 0;
    int pageno = 0;
    PMemberData pmData = NULL;

    LockPage(lastpage, BUF_READ);
    pageno = lastpage->header.pageNum - num + 1;
    UnLockPage(lastpage, BUF_READ);

    if (pageno <= PAGE_HEAD_PAGE_NUM)
    {
        hat_log("page number is not enough. realnum:%d , request:%d ", lastpage->header.pageNum, num);
        return -1;
    }

    /* form item and data */
    size = sizeof(GroupItemData) + num * (sizeof(MemberData) + sizeof(PageOffset));
    groupItemData = (PGroupItemData)AllocMem(size);

    groupItemData->ItemData.groupid = atomic_fetch_add(&tblInfo->groupInfo->groupInfo.group_id, 1) + 1;
    size = num * (sizeof(MemberData) + sizeof(PageOffset));
    groupItemData->ItemData.len = size | ITEM_VALID_MASK;

    /* member pageoffset specify */
    size = sizeof(MemberData) + sizeof(PageOffset);
    pmData = groupItemData->memberData;
    for (i = 0; i < num; i++, pageno++)
    {
        pmData = (PMemberData)((char *)(groupItemData->memberData) + i * size);

        /* colindex start from 0 */
        pmData->colIndex = i;
        pmData->memNum = 1;
        pmData->member[0].pageno = pageno;
    }

    /* get free space page */
    size = sizeof(GroupItem) + GetItemSize(groupItemData->ItemData.len);
    page = GetSpacePage(tblInfo, size, PAGE_NEW, GROUP_FORK);

    LockPage(page, BUF_WRITE);

    /* put data into page */
    size = GetItemSize(groupItemData->ItemData.len); /* data size */
    page->dataEndOffset -= size;
    groupItemData->ItemData.offset = page->dataEndOffset;
    memcpy((char *)page + page->dataEndOffset, groupItemData->memberData, size);

    /* put item into page */
    memcpy((char *)page + page->dataOffset, &(groupItemData->ItemData), sizeof(GroupItem));
    page->dataOffset += sizeof(GroupItem);

    /* write to disk */
    WritePage(tblInfo, page, GROUP_FORK);

    UnLockPage(page, BUF_WRITE);
    ReleasePage(page);
    FreeMem(groupItemData);

    /* update metadata */
    UpdateMetaData(tblInfo, GROUP_FORK);

    return 0;
}

int UpdateGroupMember(PTableList tblInfo, PPageDataHeader lastpage, int num, PGroupItemData item)
{
#if 0 
    PPageDataHeader page = NULL;
    PItemData groupItemData = NULL;
    PPageOffset pageoffset = NULL;
    int size = 0;
    int i = 0;

    /* form item and data */
    size = sizeof(GroupItem) + sizeof(MemberData) + num * sizeof(PageOffset);
    groupItemData = (PItemData)AllocMem(size);
    
    groupItemData->ItemData.groupid = tblInfo->groupInfo->groupInfo.group_id + 1;
    size = sizeof(MemberData) + num * sizeof(PageOffset);
    groupItemData->ItemData.len = size | ITEM_VALID_MASK;
    tblInfo->groupInfo->groupInfo.group_id ++;

    /* member pageoffset specify */
    groupItemData->memberData.head.pageno = lastpage->pageno;
    pageoffset = groupItemData->memberData.member;
    
    for(i = 0; i < num; i++,pageoffset++)
    {
        pageoffset->pageno = groupItemData->memberData.head.pageno;
        groupItemData->memberData.head.pageno -= 1;
    }

    /* get free space page */
    size += sizeof(GroupItem);
    page = GetSpacePage(tblInfo, size, PAGE_NEW, GROUP_FORK);

    /* put into page */
    size = GetItemSize(groupItemData->ItemData.len);
    groupItemData->ItemData.offset = page->dataEndOffset - size;
    memcpy((char*)page + dataOffset, groupItemData->ItemData, sizeof(GroupItem));

    memcpy((char*)page + groupItemData->ItemData.offset, groupItemData->memberData, size);
    
    WritePage(tblInfo, page, GROUP_FORK);

    /* update metadata */
    WritePage(tblInfo, tblInfo->groupInfo, GROUP_FORK);

    FreeMem(page);
    FreeMem(groupItemData);
#endif
    return 0;
}


PRowColumnData GetRowDataFromPageEx(PTableList tblInfo, PSearchPageInfo searchInfo)
{
    PPageDataHeader page = NULL;
    PItemData Item = NULL;
    PRowColumnData colData = NULL;

    if (NULL == tblInfo || NULL == searchInfo)
    {
        hat_log("invalid parameters ");
        return NULL;
    }

    page = searchInfo->page;
    if (NULL == page)
    {
        return NULL;
    }

    LockPage(page, BUF_READ);

    if (searchInfo->item_offset > 0)
    {
        Item = (PItemData)GET_ITEM(searchInfo->item_offset, page);
        Item += 1; // next item
    }

    if (NULL == Item)
        Item = (PItemData)page->item;

    hat_bufcontext_debug("search item");

    while (!ITEM_END_CHECK(Item, page))
    {
        if (GetItemValid(Item->len))
        {
            searchInfo->item_offset = ITEM_OFFSET(Item, page);
            searchInfo->pageNum = page->header.pageNum;

            if (GetItemRedirect(Item->len))
            {
                hat_bufcontext_debug("search item, this is redirect item.");
                colData = GetRowDataFromExtPageEx(page, GET_CUR_ITEM_INDEX(Item, page), NULL);
            }
            else
            {
                /* deform row */
                colData = DeFormRowDatEx2(page, Item->offset);
            }
#ifdef SEQSCAN_STEP_ITEM_POSITION
            hat_debug("tablename:%s pagenum:%d itemindex:%d item[len:%d offset:%d flage:valid:%0x,redirect:%0x]",
                    tblInfo->tableDef->tableName, page->header.pageNum,
                    GET_CUR_ITEM_INDEX(Item, page), GetItemSize(Item->len), Item->offset,
                    GetItemValid(Item->len), GetItemRedirect(Item->len));
#endif
            hat_bufcontext_debug("search item, found valid one.");

            UnLockPage(page, BUF_READ);
            return colData;
        }

        hat_bufcontext_debug("search item, next item.");

        /* next item */
        Item += 1;
    }

    hat_bufcontext_debug("search item, but not found.");
    UnLockPage(page, BUF_READ);
    
    return NULL;
}


/*
 * page was already locked .
 */
PRowColumnData GetRowDataFromExtPageEx(PPageDataHeader page, int itemIndex, PHeapItemData heapItemData)
{
    PRowColumnData colData = NULL;
    PItemData Item = NULL;

    do
    {
        if (NULL == page)
        {
            break;
        }

        if (GET_PAGE_TYPE(page->header.pageType) != PAGE_EXTDATA)
        {
            break;
        }

        Item = (PItemData)GET_ITEM_BY_INDEX(itemIndex, page);
        if (!ITEM_END_CHECK(Item, page) && GetItemValid(Item->len))
        {
            colData = DeFormRowDatEx2(page, Item->offset);

            if (NULL != heapItemData)
            {
                heapItemData->itemData = *Item;
                heapItemData->pageno.pageno = page->header.pageNum;
                heapItemData->itemOffset = GET_ITEM_OFFSET_BY_INDEX(itemIndex);
            }
        }
    } while (0);

    return colData;
}

PRowColumnData GetRowDataFromPageByIndexEx(PTableList tblInfo, int pageIndex, int pageOffset, PHeapItemData heapItemData)
{
    PPageDataHeader page = NULL;
    PItemData Item = NULL;
    PRowColumnData colData = NULL;

    if (NULL == tblInfo)
    {
        hat_log("invalid parameters ");
        return NULL;
    }

    page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
    if (NULL == page)
    {
        return NULL;
    }

    LockPage(page, BUF_READ);

    if (pageOffset > 0)
    {
        Item = (PItemData)GET_ITEM(pageOffset, page);
    }

    if (NULL == Item)
    {
        goto ENDRET;
    }

    if (!ITEM_END_CHECK(Item, page))
    {
        if (GetItemValid(Item->len))
        {
            if (GetItemRedirect(Item->len))
            {
                colData = GetRowDataFromExtPageEx(page, GET_CUR_ITEM_INDEX(Item, page), heapItemData);
            }
            else
            {
                /* deform row */
                colData = DeFormRowDatEx2(page, Item->offset);
                if (NULL != heapItemData)
                {
                    heapItemData->itemData = *Item;
                    heapItemData->pageno.pageno = pageIndex;
                    heapItemData->itemOffset = pageOffset;
                }
            }
        }
    }

ENDRET:
    UnLockPage(page, BUF_READ);
    ReleasePage(page);

    return colData;
}

int GetGroupMemberPageNo(PMemberData memDataPos, int index)
{
    PMemberData pmData = NULL;

    /* member pageoffset specify, default one pageOffset per member data  */
    int size = sizeof(MemberData) + sizeof(PageOffset);

    pmData = (PMemberData)((char *)(memDataPos) + index * size);

    return pmData->member[0].pageno;
}

int ReleaseAllResourceOwner()
{
    int ret = 0;
    PPageDataHeader page = NULL;
    int count = 0;

    ret = PopResourceOwner(&page);
    while (ret > 0)
    {
        ReleasePage(page);
        count++;

        ret = PopResourceOwner(&page);
    }

    //if (count > 0)
        // hat_error("releaseAllResourceOwner Buffer, this is %d rest. ", count);
    return ret;
}


int LockBuffer(PPageDataHeader page, int mode)
{
    int ret = 0;
    PBufferDesc desc = NULL;

    if(NULL == page)
        return -1;

    desc = GetBufferDesc(GetBufferPoolContext(), (PBufferElement)page);
    ret = LockBufDesc(desc, (BufferLockMode)mode);
    return ret;
}

int UnLockBuffer(PPageDataHeader page, int mode)
{
    int ret = 0;
    PBufferDesc desc = NULL;
    
    if(NULL == page)
        return -1;

    desc = GetBufferDesc(GetBufferPoolContext(), (PBufferElement)page);
    ret = UnLockBufDesc(desc, (BufferLockMode)mode);
    return ret;
}

int LockBufferEx(PPageDataHeader page, int mode, char *fun, int line)
{
    hat_buflock_debug("LockBufferEx page %p mode %d [%s][%d]", page, mode, fun, line);
    return LockBuffer(page, mode);
}

int UnLockBufferEx(PPageDataHeader page, int mode, char *fun, int line)
{
    hat_buflock_debug("UnLockBufferEx page %p mode %d [%s][%d]", page, mode, fun, line);
    return UnLockBuffer(page, mode);
}