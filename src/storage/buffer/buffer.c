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

#include "server_pub.h"
#include "public.h"
#include "config_pub.h"
#include "bufferPool.h"

#include "resourceMgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>


/* 数据字典 */
DList* g_TblList = NULL;

/* 内存缓冲区 */
PBufferPoolContext g_bufferPoolContext = NULL;

static PBufferPoolContext GetBufferPoolContext();

/*
 * table metadata load from table and group files. 
 * filename: table name, is as table file. 
 * PTableList: return pointer of metadata structure.
 */
PTableList GetTableInfo(char *filename)
{
    PTableList thisTbl = NULL;
    PPageDataHeader pageheader = NULL;
    PTableMetaInfo tableinfo = NULL;
    PMemContextNode oldContext = NULL;
    PageOffset pageNum = {1,1};
    PVFVec pos = NULL;
    int size = 0;
    int ret = 0;

    if(NULL == filename)
    {
        return NULL;
    }

    /* MemoryContext switch to Database dictionary memory context. */
    oldContext = MemMangerSwitchContext(dictionaryContext);
    do
    {
        if(NULL == g_TblList)
        {
            g_TblList = (DList*)AllocMem(sizeof(DList));
            g_TblList->prev = g_TblList->next = g_TblList;
            break;
        }

        /* search table info list */
        thisTbl = SearchTblInfo(filename);
    }while(0);

    if(NULL != thisTbl)
    {
        MemMangerSwitchContext(oldContext);
        return thisTbl;
    }

    /* table info is not find, or table file is not opened. */
    if(NULL == thisTbl)
    {
        thisTbl = (PTableList)AllocMem(sizeof(TableList));
        memset(thisTbl, 0x00, sizeof(TableList));

        /* 初始化 */
        ret = TableOpen(thisTbl, filename, MAIN_FORK);
        if(ret < 0)
        {
            hat_log("table %s open failure, maybe create table first.\n", filename);
            return NULL;
        }
    }

    /* linker this table info */
    thisTbl->list.prev = g_TblList->next->prev;
    thisTbl->list.next = g_TblList->next;
    g_TblList->next = &(thisTbl->list);

    MemMangerSwitchContext(oldContext);

    return thisTbl;
}


PTableList GetTableInfoByRel(PRelation rel)
{
    PTableList tbl = NULL;
    PTableList temp = NULL;

    temp = (PTableList)g_TblList->next;
    while(NULL != temp && temp != (PTableList)g_TblList)
    {
        if(RelationCompare(rel, temp->rel))
        {
            tbl = temp;
            break;
        }

        temp = (PTableList)((DList*)temp)->next;
    }

    return tbl;
}


/* 创建表文件及数据字典 */
PTableList CreateTblInfo(PTableMetaInfo tblDef)
{
    PTableList thisTbl = NULL;
    PMemContextNode oldContext = NULL;
    PVFVec pos = NULL;
    int size = 0;
    int ret = 0;

    if(NULL == tblDef)
    {
        return NULL;
    }

    /* MemoryContext switch to Database dictionary memory context. */
    oldContext = MemMangerSwitchContext(dictionaryContext);

    do
    {
        if(NULL == g_TblList)
        {
            g_TblList = (DList*)AllocMem(sizeof(DList));
            g_TblList->prev = g_TblList->next = g_TblList;
            break;
        }

        /* search table info list */
        thisTbl = SearchTblInfo(tblDef->tableName);
    }while(0);

    /* found */
    if(NULL != thisTbl)
    {
        MemMangerSwitchContext(oldContext);

        hat_error("table exitst.\n");
        return NULL;
    }

    /* dictionary initialize */
    thisTbl = (PTableList)AllocMem(sizeof(TableList));
    memset(thisTbl, 0x00, sizeof(TableList));
   
    size = sizeof(TableMetaInfo) + sizeof(ColumnDefInfo) * tblDef->colNum;
    thisTbl->tableDef = (PTableMetaInfo)AllocMem(size);
    memcpy(thisTbl->tableDef, tblDef, size);
    thisTbl->tableDef->tableId = GetObjectId();

    /* create table file */
    ret = TableCreate(thisTbl, MAIN_FORK);
    if(ret < 0)
    {
        goto ERRRET;
    }

    /* create table file */
    ret = TableCreate(thisTbl, GROUP_FORK);
    if(ret < 0)
    {
        goto ERRRET;
    }

    thisTbl->rel = (PRelation)AllocMem(sizeof(Relation));
    thisTbl->rel->relid = thisTbl->tableDef->tableId;
    thisTbl->rel->relType = thisTbl->tableDef->tableType;  

    /* init table file */
    ret = TableFileInit(thisTbl);
    if(ret < 0)
    {
        goto ERRRET;
    }

    /* linker this table info */
    thisTbl->list.prev = g_TblList->next->prev;
    thisTbl->list.next = g_TblList->next;
    g_TblList->next = &(thisTbl->list);

    MemMangerSwitchContext(oldContext);
    return thisTbl;

ERRRET:
    MemMangerSwitchContext(oldContext);

    ReleaseTblInfo(thisTbl);
    return NULL;
}

int ReleaseTblInfo(PTableList tblInfo)
{
    if(NULL == tblInfo)
        return -1;
    
    /* linker dettach this table info */
    if(NULL != tblInfo->list.next && NULL != tblInfo->list.prev)
    {
        tblInfo->list.prev->next = tblInfo->list.next;
        tblInfo->list.next->prev = tblInfo->list.prev;
    }
    
    /* release resource */
    if(NULL != tblInfo->sgmr)
        smgrRelease(tblInfo->sgmr);

    if(NULL != tblInfo->tableInfo)
        FreeMem(tblInfo->tableInfo);

    if(NULL != tblInfo->tableDef)
        FreeMem(tblInfo->tableDef);

    if(NULL != tblInfo->rel)
        FreeMem(tblInfo->rel);

    if(NULL != tblInfo->groupInfo)
        FreeMem(tblInfo->groupInfo);

    FreeMem(tblInfo);
    return 0;
}

PTableList SearchTblInfo(char *filename)
{
    PTableList tbl = NULL;
    PTableList temp = NULL;

    temp = (PTableList)g_TblList->next;
    while(NULL != temp && temp != (PTableList)g_TblList)
    {
        if(strcmp(filename, temp->tableDef->tableName) == 0)
        {
            tbl = temp;
            break;
        }

        temp = (PTableList)((DList*)temp)->next;
    }

    return tbl;
}

int ReleaseAllTblInfoResource()
{
    PTableList temp = NULL;

    if(g_TblList == NULL)
        return -1;

    /* CloseTable */
    while(g_TblList->next != NULL && g_TblList->next != g_TblList)
    {
        temp = (PTableList)g_TblList->next;
        ReleaseTblInfo(temp);
    }

    FreeMem(g_TblList);
    g_TblList = NULL;

    return 0;
}

int CreateBufferPool(int pageNum)
{
    int size = 0;


    if(pageNum <= 0)
        pageNum = 128;
    
    size = sizeof(BufferPoolContext);
    size += pageNum * sizeof(BufferElement);
    size += pageNum * sizeof(BufferDesc);
    size += pageNum * sizeof(int);

    g_bufferPoolContext = AllocMem(size);

    size = InitBufferPool(g_bufferPoolContext, pageNum);
    if(size < 0)
    {
        hat_error("initialize buffer pool faulure.\n");
    }

    return size;
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
    //memset(rawRows, 0x00, size);

    rawRows->num = tblMeta->colNum;
    rawRows->size += size;

    /* rawRow fill values order by tblMeta, which is null when stmt isnot input. */
    for(index = 0; index < rawRows->num; index++)
    {
        /* checking column which will not input in . */
        attrIndex = GetAtrrIndexByName(colDef[index].colName, stmt->attrNameList);
        size = sizeof(RowColumnData);

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

        attrData = GetDataByIndex(attrIndex, stmt->valuesList);
        if(attrData == NULL)
        {
            hat_log("attr and values is not match. \n");
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
                *tmp = attrData->value.iData;
            }
                break;
            case VT_VARCHAR:
            case VT_STRING:
            {
                int len = strlen(attrData->value.pData)+1;

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
                rawRows->columnData[index]->data[0] = attrData->value.iData != 0?'T':'F';
                break;
            default:
                hat_log("attr and values type is not match. \n");
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
    columnDataPos = ((char*)pagerow + size);

    pageBuf = (char *)&(tempRow->rowsData);
    pageBufOffset = sizeof(TableRowData);

    /* rawRow fill values order by tblMeta, which is null when stmt isnot input. */
    for(index = 0; index < pagerow->rowsData.num; index++)
    {
        pval = (int*)(pageBuf + pageBufOffset);
        size = *pval;
        if(size <= 0 || size > MAX_ROW_DATA_SIZE)
        {
            FreeMem(pagerow);
            pagerow = NULL;
            hat_log("found bad tuple size %d\n", size);
            break;
        }

        /* data is null */
        pagerow->rowsData.columnData[index] = (PRowColumnData)(columnDataPos - size);
        memcpy(pagerow->rowsData.columnData[index], pageBuf + pageBufOffset, size);

        pageBufOffset += size;
        columnDataPos -= size;

        offset -= size;
        if(offset < 0)
        {
            hat_error("rowData size %d , space size not enough\n", offset, tempRow->rowsData.size);

            FreeMem(pagerow);
            pagerow = NULL;
            break;
        }
    }

    return pagerow;
}

PTableRowData DeFormRowData(PPageDataHeader page, int pageffset)
{
    PRowData tempRow = NULL;

    tempRow = DeFormRowDatEx(page, pageffset);

    return &(tempRow->rowsData);
}

/*
 *  按指定大小，查找空闲空间
 *  顺次查找表文件，检查表文件块上的剩余空间;
 *  根据op指定，找不到时，是否需要新建块，还是只在存在的块中查找。
 */
PPageDataHeader GetSpacePage(PTableList tblInfo, int size, PageOp op, ForkType forkNum)
{
    PPageDataHeader page = NULL;
    int pageIndex = PAGE_HEAD_PAGE_NUM+1;  /* first data page start */

    if(size + PAGE_DATA_HEADER_SIZE >= PAGE_MAX_SIZE)
    {
        hat_log("row data size %d is oversize page size %d\n", size, PAGE_MAX_SIZE);
        return page;
    }

    /* sesearch all table file ,from current page. */
    for(; ; pageIndex++)
    {
        page = GetPageByIndex(tblInfo, pageIndex, forkNum);
        if(NULL == page)
            break;

        /* freespace check */
        if(HasFreeSpace(page, size))
        {
            break;
        }

        /* todo: bufferpool */
        ReleasePage(page);
    }

    if(page == NULL && op == PAGE_NEW)
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
    PPageDataHeader page = NULL;
    PPageDataHeader extpage = NULL;
    int pageIndex = startPageIndex;  /* first data page start */
    
    if(size + PAGE_DATA_HEADER_SIZE >= PAGE_MAX_SIZE)
    {
        hat_log("row data size %d is oversize page size %d, pagetype:%d \n", size, PAGE_MAX_SIZE, pageType);
        return page;
    }

    /* sesearch all table file ,from current page. */
    do
    {
        /* last page is not release, which is updated when extension new page. */
        ReleasePage(page);  

        page = GetPageByIndex(tblInfo, startPageIndex, forkNum);
        if(NULL == page)
            break;

        if(GET_PAGE_TYPE(page->header.pageType) != pageType)
        {
            /* todo: bufferpool */
            ReleasePage(page);

            hat_error("found error page, page num:%d type:%d ,request type:%d \n", 
                                            page->header.pageNum, 
                                            GET_PAGE_TYPE(page->header.pageType), 
                                            pageType);
            return NULL;
        }

        /* freespace check */
        if(HasFreeSpace(page, size))
        {
            return page;
        }

        /* next extension page. */
        startPageIndex = page->extPage.pageno;      
    }while(!IsInvalidPageNum(startPageIndex));

    /* 
     * Get here, all the pages have not enogh space. 
     * We plan to extending new page. 
    */
    do 
    {
        if(op != PAGE_NEW)
        {
            break;
        }
    
        extpage = ExtensionTbl(tblInfo, 1, forkNum);
        extpage->header.pageType = SET_PAGE_TYPE(extpage->header.pageType, pageType);

        if(page != NULL)
        {
            /* 指定块没有空间时，扩展一个块 */
            page->extPage.pageno = extpage->header.pageNum;

            /* todo, maybe set dirty flag only. */
            WritePage(tblInfo, page, forkNum);

            /* pre block of chain. */
            ReleasePage(page);  
        }

        page = extpage;
    }while(0);

    return page;
}

/*
 * 按指定的page index获取对应的page
*/
PPageDataHeader GetPageByIndex(PTableList tblInfo, int index, ForkType forkNum)
{
    PPageDataHeader page = NULL;

    if(IsInvalidPageNum(index))
    {
        return NULL;
    }

    /*
    * we will not check, it only test by filesystem . 
    */
    //if(index > tblInfo->tableInfo->header.pageNum)
    //    return page;
    
    page = ReadPage(tblInfo, index, forkNum);

    if(NULL != page)
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

    /* first rowdata insert into page buffer */
    newRowBuffer = (char*)page + page->dataEndOffset;    

    /* rowdata header */
    size = sizeof(TableRowData);
    memcpy(newRowBuffer, row, size);
    newRowBuffer += size;
    page->dataEndOffset += size;

    for(columnIndex = 0; columnIndex < row->num; columnIndex++)
    {
        /* column header and data */
        memcpy(newRowBuffer, row->columnData[columnIndex], row->columnData[columnIndex]->size);

        page->dataEndOffset += row->columnData[columnIndex]->size;
        newRowBuffer += row->columnData[columnIndex]->size;
    }

    /* page buffer write to table file. */
    WritePage(tblInfo, page, MAIN_FORK);

    /* end resource release here. */
    FreeMem(row);
    ReleasePage(page);   
    return 0;
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
int WriteRowItemDataWithHeader(PTableList tblInfo, PPageDataHeader page, PRowData row, PRowHeaderData rowHeader)
{
    int offset = 0;
    int columnIndex = 0;
    char *newRowBuffer = NULL;
    int size = 0;
    ItemData item = {0};

    /* form item data */
    item.len = sizeof(RowData) + row->rowsData.size;
    item.offset = page->dataEndOffset - item.len;
    // item.rowid = {0};                                        /* not used */
    item.len |= ITEM_VALID_MASK;

    /* rowdata insert into page buffer */
    newRowBuffer = (char *)page + item.offset;   
    
    size = sizeof(RowData);
    memcpy(newRowBuffer, row, size);
    newRowBuffer += size;

    for(columnIndex = 0; columnIndex < row->rowsData.num; columnIndex++)
    {
        /* column header and data */
        memcpy(newRowBuffer, row->rowsData.columnData[columnIndex], row->rowsData.columnData[columnIndex]->size);
        newRowBuffer += row->rowsData.columnData[columnIndex]->size;
    }

    /* item data insert into page buffer */
    newRowBuffer = (char *)page + page->dataOffset;
    memcpy(newRowBuffer, &item, sizeof(ItemData));

    /* page buffer offset positon reset */
    page->dataOffset += sizeof(ItemData);
    page->dataEndOffset = item.offset;

    /* page buffer write to table file. */
    WritePage(tblInfo, page, MAIN_FORK);

    if(NULL != rowHeader)
    {
        rowHeader->rowPos.itemIndex = GET_ITEM_INDEX(page);
        rowHeader->rowPos.pageIndex.pageno = page->header.pageNum;
    }
    return 0;
}

int WriteRowItemData(PTableList tblInfo, PPageDataHeader page, PTableRowData row)
{
    PRowData rowRealData = NULL;
    int rowSize = 0;
    int ret = 0;

    rowSize = sizeof(RowData) + row->num * sizeof(PRowColumnData);
    rowRealData = (PRowData)AllocMem(rowSize);

    /* rowdata header */
    rowRealData->rowheader.rowPos.itemIndex = GET_ITEM_INDEX(page);
    rowRealData->rowheader.rowPos.pageIndex.pageno = page->header.pageNum;

    rowSize = sizeof(TableRowData) + row->num * sizeof(PRowColumnData);
    memcpy(&(rowRealData->rowsData), row, rowSize);
    
    ret = WriteRowItemDataWithHeader(tblInfo, page, rowRealData, NULL);
    return ret;
}

int WriteRowDataOnly(PTableList tblInfo, PPageDataHeader page, PRowData row, PItemData oldItem)
{
    int offset = 0;
    int columnIndex = 0;
    char *newRowBuffer = NULL;
    int size = 0;

    /* rowdata insert into page buffer */
    newRowBuffer = (char *)page + oldItem->offset;   
    
    size = sizeof(RowData);
    memcpy(newRowBuffer, row, size);
    newRowBuffer += size;

    for(columnIndex = 0; columnIndex < row->rowsData.num; columnIndex++)
    {
        /* column header and data */
        memcpy(newRowBuffer, row->rowsData.columnData[columnIndex], row->rowsData.columnData[columnIndex]->size);
        newRowBuffer += row->rowsData.columnData[columnIndex]->size;
    }

    oldItem->len = SetItemSize(oldItem->len, sizeof(RowData) + row->rowsData.size);
    
    /* page buffer write to table file. */
    WritePage(tblInfo, page, MAIN_FORK);

    return 0;
}

int CloseTable(PTableList tbl)
{
    FreeMem(tbl);
    return 0;
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

    if(num <= 0 || num > PAGE_EXTENSION_MAX_NUM)
    {
        hat_log("num will be extension is invalid.\n");
        return NULL;
    }

    switch(forkNum)
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

    /* blank space page write to the end of tblfile. */
    //page = (char *)AllocMem(PAGE_MAX_SIZE);

    (void)InitPage(page, PAGE_DATA);
    pageNew = (PPageDataHeader)page;

    for(; i < num; i++)
    {
        pageNew->header.pageNum = pageHeader->pageCnt + 1;

        /* low level write to file */
        ret = TableWrite(tblInfo, (PPageHeader)pageNew, forkNum);
        pageHeader->pageCnt += 1;
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

    /* update buffer */
    switch(forkNum)
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
    //PTableMetaInfo tableinfo = NULL;
    page->pageCnt = tblInfo->tableInfo->pageCnt;
    //tableinfo = (PTableMetaInfo) ((char *)page + PAGE_DATA_HEADER_SIZE);
    return 0;
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
    BufferTag  bufferTag;
    
    PTableList otherTblInfo = NULL;
    Relation otherRelation = {0};
    PBufferTag otherBufferTag = NULL;

    int found = 0;
    int ret = 0;

    if(index <= 0)
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
    if(BUFFER_FOUND == found)
    {
        return page;
    }

    /* buffer context has other page, which is modified and flush context at the first. */
    if(BUFFER_OTHER == found)
    {
        otherBufferTag = GetBufferTag(GetBufferPoolContext(), (PBufferElement)page);

        otherRelation.databaseId = otherBufferTag->databaseId;
        otherRelation.relid = otherBufferTag->tableId;
        otherRelation.relType = 0;

        /* flush this buffer */
        otherTblInfo = GetTableInfoByRel(&otherRelation);
        FlushPage(otherTblInfo, page, otherBufferTag->forkNum);

        /* assigned this tag to the buffer. */
        memcpy(otherBufferTag, &bufferTag, sizeof(BufferTag));
        SetBuffferValid(GetBufferPoolContext(), (PBufferElement)page, BVF_TAG);
    }

    ret = TableRead(tblInfo, &pageoffset, forkNum, (char *)page);
    if(ret < 0)
    {
        ReleaseBuffer(GetBufferPoolContext(), (PBufferElement)page);
        return NULL;
    }

    SetBuffferValid(GetBufferPoolContext(), (PBufferElement)page, BVF_VLID);
    return page;
}

int ReleasePage(PPageDataHeader page)
{
    if(NULL == page)
        return -1;

    ReleaseBuffer(GetBufferPoolContext(), (PBufferElement)page);
    
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

#if 0
/* 
 * get group info from group file, which is same name prefix as relation file.
 * We will create group file at the first time. 
 */
PGroupItemData FindGroupInfo(PTableList tblInfo, int groupId)
{
    PTableList tblGroup = NULL;
    char groupFileName[FILE_NAME_MAX_LEN] = {0};
    PPageDataHeader page = NULL;
    PGroupItem gItem = NULL;
    PGroupItemData gItemData = NULL;
    PageOffset pageNum = {0,0};
    int ret = 0;

    if(NULL == tblInfo || groupId == INVALID_GROUP_ID)
    {
        hat_log("invalid parameters \n");
        return NULL;
    }

    /* init group file */
    if(NULL == tblInfo->groupInfo)
    {
        hat_log("groupInfo invalid \n");
        return NULL;
    }

    /* find group info, item scan from page  */
    pageNum.pageno = PAGE_HEAD_PAGE_NUM + 1;

    page = GetPageByIndex(tblInfo, pageNum.pageno, GROUP_FORK);
    while (page != NULL)
    {
        if(NULL == gItem)
            gItem = (PGroupItem)page->item;
        
        /* next page */
        if(ITEM_END_CHECK(gItem, page))
        {
            FreeMem(page);

            pageNum.pageno ++;
            page = GetPageByIndex(tblInfo, pageNum.pageno, GROUP_FORK);

            gItem = NULL;            
            continue;
        }

        if(GetItemValid(gItem->len) && (groupId == gItem->groupid))
        {
            gItemData = (PGroupItemData)AllocMem(sizeof(GroupItemData) + GetItemSize(gItem->len));

            /* item infomation */
            memcpy(&(gItemData->ItemData), gItem, sizeof(GroupItem));

            /* member page infomation */
            memcpy(gItemData->memberData, (char*)page + gItem->offset, GetItemSize(gItem->len));

            gItemData->pagePos.pageno = page->header.pageNum;
            return gItemData;
        }

        /* next item */
        gItem += 1;
    }
    
    return NULL;
}
#endif

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
    PageOffset pageNum = {0,0};
    int found = 0;

    if(NULL == tblInfo)
    {
        hat_log("invalid parameters \n");
        return -1;
    }

    page = searchInfo->page;
    if(NULL == page)
    {
        /* find group info, item scan from page  */
        pageNum.pageno = pageNum.pageno = searchInfo->pageNum ;
        
        page = GetPageByIndex(tblInfo, pageNum.pageno, GROUP_FORK);
        searchInfo->page = page;        
    }
    else 
    {
        /* get next group item */
        gItem = (PGroupItem)GET_ITEM(searchInfo->item_offset, page);
        gItem += 1;
        searchInfo->pageNum = searchInfo->page->header.pageNum;
    }

    while (page != NULL)
    {
        /* group item postion is start when switch page. */
        if(NULL == gItem)
        {
            gItem = (PGroupItem)page->item;
        }
        
        /* next page */
        if(ITEM_END_CHECK(gItem, page))
        {
            pageNum.pageno = searchInfo->pageNum + 1;
            gItem = NULL;    
            ReleasePage(page);

            page = GetPageByIndex(tblInfo, pageNum.pageno, GROUP_FORK);
            searchInfo->page = page;

            if(NULL != page)
                searchInfo->pageNum = pageNum.pageno;
            continue;
        }

        if(GetItemValid(gItem->len))
        {
           // gItemData = (PGroupItemData)AllocMem(sizeof(GroupItemData) + GetItemSize(gItem->len));

            /* item infomation */
            memcpy(&(gItemData->ItemData), gItem, sizeof(GroupItem));

            /* member page infomation */
            memcpy(gItemData->memberData, (char*)page + gItem->offset, GetItemSize(gItem->len));

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
    if(NULL == page)
    {
        return -1;      
    }
    
    gItem = (PGroupItem)GET_ITEM(groupInfo->item_offset, page);
    if(ITEM_END_CHECK(gItem, page) || !GetItemValid(gItem->len))
    {
        return -1;
    }

    gMemData =  (PMemberData)((char*)page + gItem->offset);

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

    if((NULL == tblInfo) || (NULL == insertdata) || (NULL == pageList))
    {
        hat_log("invalid parameters \n");
        return -1;
    }

    groupItem = scanInfo->groupItem;
    searchPage = scanInfo->searchPageList;
    isNoSpace = scanInfo->isNoSpace;

    /* will extension space, until error ocur. */
    for(; ; )
    {
        /* find one group */
        if(isNoSpace)
        {
            found = GetGroupInfo(tblInfo, searchPage, groupItem);
            if(found != HAT_TRUE)
                break;

            isNoSpace = HAT_FALSE;
        }

        for(colNum = 0; colNum < insertdata->num; colNum ++)
        {
            pageHeader = GetFreeSpaceMemberPage(tblInfo, 
                                        insertdata->columnData[colNum]->size + sizeof(TableRowData) + sizeof(ItemData), 
                                        groupItem,
                                        op, 
                                        insertdata->columnData[colNum]->attrindex);
            if(NULL == pageHeader)
            {
                /* this group not enogh space */
                ReleasePageListMember(pageList, insertdata->num);
                isNoSpace = HAT_TRUE;
                break;
            }

            pageList[colNum] = pageHeader;
        }

        if(colNum == insertdata->num)
            break;   
    }

    scanInfo->isNoSpace = isNoSpace;

    /* error ocur */
    if(colNum < insertdata->num)
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
    for(pageIndex = 0; pageIndex < colIndex; pageIndex++)
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
    for(; pageIndex != INVALID_PAGE_NUM; )
    {
        page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
        if(NULL == page)
        {
            /* error ocur */
            hat_error("It isn't found group member pageindex: %d\n", pageIndex);
            break;
        }

        if(HasFreeSpace(page, size))
        {
            /* Enough space is found. */
            break;
        }
        
        /* todo: bufferpool */
        ReleasePage(page);
        page = NULL;

        /* next page as column link */
        if(subPageNum < 0)
        {
            break;
        }
        pageIndex = memberData->member[subPageNum--].pageno;
    }

    return page;
}

PPageDataHeader* GetGroupMemberPages(PTableList tblInfo, PGroupItemData item, int *pageNum)
{
    PPageDataHeader page = NULL;
    PMemberData memberData = NULL;
    PPageDataHeader *pagelist = NULL;
    int pageIndex = 0;
    int subPageNum = 0;
    int offset = 0;

    /* calculator memberdata offset */
    memberData = item->memberData;
    if(NULL == memberData)
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
    for(; pageIndex != INVALID_PAGE_NUM; )
    {
        page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
        if(NULL == page)
        {
            break;
        }

        pagelist[subPageNum] = page;

        /* all column finish */
        if(++subPageNum >= *pageNum)
        {
            break;
        }

        memberData = (PMemberData)((char *)(item->memberData) + offset);
        pageIndex = memberData->member[0].pageno;
        offset += sizeof(MemberData) + memberData->memNum * sizeof(PageOffset);
    }

    if(subPageNum != *pageNum)
    {
        /* error ocur */
        pageIndex = ReleasePageList(pagelist, memberData->memNum);
        pagelist = NULL;

        hat_log("group page invalid. groupmember[%d], pagefind[%d-%d]\n", memberData->memNum, subPageNum, pageIndex);
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
    if(NULL == memberData)
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

    for(; pageIndex != INVALID_PAGE_NUM; )
    {
        if(colPageIndex == colIndex[subPageNum])
        {
            page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
            if(NULL == page)
            {
                break;
            }

            pagelist[subPageNum] = page;

            /* next page as column link */
            if(++subPageNum >= scanPageInfo->pageListNum)
            {
                break;
            }
        }

        colPageIndex ++;
        memberData = (PMemberData)((char *)(groupItem->memberData) + offset);
        pageIndex = memberData->member[0].pageno;
        offset += sizeof(MemberData) + memberData->memNum * sizeof(PageOffset);
    }

    if(subPageNum != scanPageInfo->pageListNum)
    {
        /* error ocur */
        pageIndex = ReleasePageList(pagelist, memberData->memNum);
        found = 0;
        //hat_log("opt group page invalid. groupmember[%d], pagefind[%d-%d]\n", memberData->memNum, subPageNum, pageIndex);
    }

END:
    return found;
}

int ReleasePageList(PPageDataHeader *pagelist, int num)
{
    int pageIndex = 0;
    
    while(pagelist[pageIndex] != NULL)
    {
        ReleasePage(pagelist[pageIndex]);
        pagelist[pageIndex] = NULL;

        if(++pageIndex >= num)
            break;
    }
    
    return pageIndex;
}

int ReleasePageListMember(PPageDataHeader *pagelist, int num)
{
    int pageIndex = 0;
    
    while(pagelist[pageIndex] != NULL)
    {
        ReleasePage(pagelist[pageIndex]);
        pagelist[pageIndex] = NULL;

        if(++pageIndex >= num)
            break;
    }
    
    return pageIndex;
}

int GetInvlidRowCount(PPageDataHeader page)
{
    int rowNum = (page->dataOffset - ITEM_DATA_START_OFFSET)/sizeof(ItemData);
    int rowTotal = rowNum; 
    PItemData pItem = &(page->item[0]);

    for(; rowNum > 0; rowNum--,pItem++)
    {
        if(!GetItemValid(pItem->len))
            rowTotal--;
    }

    return rowTotal;
}

/* return column row array, array size is insertdata->num */
PTableRowData FormColRowsData(PTableRowData insertdata)
{
    int colNum = 0;
    PTableRowData colRows = NULL;
    PTableRowData rawRow = NULL;
    int itemsize = sizeof(TableRowData) + sizeof(PRowColumnData);

    if(NULL == insertdata && (insertdata->num <= 0))
    {
        hat_log("col rows form invalid parameters. \n");
        return NULL;
    }
    
    colRows = (PTableRowData)AllocMem(insertdata->num * itemsize);

    /* only one column data in per rowdata */
    for(colNum = 0; colNum < insertdata->num; colNum++)
    {
        rawRow = (PTableRowData)((char *)colRows + colNum * itemsize);
        rawRow->num = 1;
        
        rawRow->columnData[0] = insertdata->columnData[colNum];
        rawRow->size = rawRow->columnData[0]->size;
    }
 
    return colRows;
}

PTableRowData FormColData2RowData(PRowColumnData colRows)
{
    PTableRowData rowData = NULL;
    int rowDataSize = sizeof(TableRowData) + sizeof(PRowColumnData);

    if(NULL == colRows)
    {
        hat_log("coldata to rowdata invliad argments\n");
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

    if(NULL == colRows || NULL == colRows[0])
    {
        hat_log("colrow to rowdata invliad argments\n");
        return NULL;
    }

    rowsData = (PRowData)AllocMem(rowDataSize);
    rowData = &(rowsData->rowsData);

    rowDataSize = 0; /* reuse. row data real size */
    for(index = 0; index < colNum; index++)
    {
        rowDataSize += colRows[index]->size;

        if(colRows[index]->size > 0)
            rowData->columnData[index] = colRows[index]->columnData[0];
        else
        {
            rowData->columnData[index] = NULL;
        }
    }

    /* todo: sort as attribute index, start from 0 */

    rowData->size = rowDataSize;
    rowData->num = colNum;

    return rowData;
}

int InsertGroupItem(PTableList tblInfo, PPageDataHeader lastpage, int num)
{
    PPageDataHeader page = NULL;
    PGroupItemData groupItemData = NULL;
    int size = 0;
    int i = 0;
    int pageno = 0;
    PMemberData pmData = NULL;

    /* form item and data */
    size = sizeof(GroupItemData) + num * (sizeof(MemberData) + sizeof(PageOffset));
    groupItemData = (PGroupItemData)AllocMem(size);
    
    groupItemData->ItemData.groupid = tblInfo->groupInfo->groupInfo.group_id + 1;
    size = num * (sizeof(MemberData) + sizeof(PageOffset));
    groupItemData->ItemData.len = size | ITEM_VALID_MASK;

    pageno = lastpage->header.pageNum - num + 1;
    if(pageno <= PAGE_HEAD_PAGE_NUM)
    {
        hat_log("page number is not enough. realnum:%d , request:%d \n",lastpage->header.pageNum, num);
        FreeMem(groupItemData);
        return -1;
    }

    /* member pageoffset specify */   
    size = sizeof(MemberData) + sizeof(PageOffset);
    pmData = groupItemData->memberData;
    for(i = 0; i < num; i++, pageno++)
    {
        pmData = (PMemberData)((char*)(groupItemData->memberData) + i * size);
        
        /* colindex start from 0 */
        pmData->colIndex = i;   
        pmData->memNum = 1;
        pmData->member[0].pageno = pageno;
    }

    /* get free space page */
    size = sizeof(GroupItem) + GetItemSize(groupItemData->ItemData.len); 
    page = GetSpacePage(tblInfo, size, PAGE_NEW, GROUP_FORK);

    /* put data into page */
    size = GetItemSize(groupItemData->ItemData.len);    /* data size */
    page->dataEndOffset -= size;
    groupItemData->ItemData.offset = page->dataEndOffset;    
    memcpy((char*)page + page->dataEndOffset, groupItemData->memberData, size);   

    /* put item into page */
    memcpy((char*)page + page->dataOffset, &(groupItemData->ItemData), sizeof(GroupItem));
    page->dataOffset += sizeof(GroupItem);
 
    /* write to disk */
    WritePage(tblInfo, page, GROUP_FORK);

    /* update metadata */
    tblInfo->groupInfo->groupInfo.group_id ++;
    UpdateMetaData(tblInfo, GROUP_FORK);

    ReleasePage(page);
    FreeMem(groupItemData);
    
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

/*
 * this function will search rows in the page.
 * return rawrow when found one, next time will continue to search from next item until the end.
*/
PTableRowData GetRowDataFromPage(PTableList tblInfo, PSearchPageInfo searchInfo)
{
    PPageDataHeader page = NULL;
    PItemData Item = NULL;
    PTableRowData rowData = NULL;

    if(NULL == tblInfo || NULL == searchInfo)
    {
        hat_log("invalid parameters \n");
        return NULL;
    }

    page = searchInfo->page;
    if(NULL == page)
    {
        return NULL;        
    }
    
    if(searchInfo->item_offset > 0)
    {
        Item = (PItemData)GET_ITEM(searchInfo->item_offset, page);
        Item += 1; // next item 
    }

    if(NULL == Item)
        Item = (PItemData)page->item;

    while(!ITEM_END_CHECK(Item, page))
    {    
        if(GetItemValid(Item->len))
        {
            searchInfo->item_offset = ITEM_OFFSET(Item, page);
            searchInfo->pageNum = page->header.pageNum;

            if(GetItemRedirect(Item->len))
            {
                rowData = GetRowDataFromExtPage(tblInfo, Item->offset, GetItemSize(Item->len));
            }
            else 
            {
                /* deform row */
                rowData = DeFormRowData(page, Item->offset);
            }
#ifdef SEQSCAN_STEP_ITEM_POSITION
            hat_log("tablename:%s pagenum:%d itemindex:%d item[len:%d offset:%d flage:valid:%0x,redirect:%0x]\n",
                        tblInfo->tableDef->tableName, page->header.pageNum,
                        GET_CUR_ITEM_INDEX(Item, page), GetItemSize(Item->len), Item->offset,
                        GetItemValid(Item->len), GetItemRedirect(Item->len));
#endif             
            return rowData;
        }

        /* next item */
        Item += 1;
    }
    
    return NULL;
}

/*
 * row data deform from extension page.
 */
PTableRowData GetRowDataFromExtPage(PTableList tblInfo, int pageno, int itemIndex)
{
    PPageDataHeader page = NULL;
    PTableRowData rowData = NULL;
    PItemData Item = NULL;

    do 
    {
        page = GetPageByIndex(tblInfo, pageno, MAIN_FORK);
        if(NULL == page)
        {
            break;
        }

        if(GET_PAGE_TYPE(page->header.pageType) != PAGE_EXTDATA)
        {
            break;
        }

        Item = (PItemData)GET_ITEM_BY_INDEX(itemIndex, page);    
        if(!ITEM_END_CHECK(Item, page) && GetItemValid(Item->len))
        {    
            rowData = DeFormRowData(page, Item->offset);
        }

        ReleasePage(page);
    }while(0);

    return rowData;
}

/*
 * Searching rowData, it is pecified by pageindex and rowIndex.
 */
PTableRowData GetRowDataFromPageByIndex(PTableList tblInfo, int pageIndex, int pageOffset, PItemData itemData)
{
    PPageDataHeader page = NULL;
    PItemData Item = NULL;
    PTableRowData rowData = NULL;

    if(NULL == tblInfo)
    {
        hat_log("invalid parameters \n");
        return NULL;
    }

    page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
    if(NULL == page)
    {
        return NULL;        
    }
    
    if(pageOffset > 0)
    {
        Item = (PItemData)GET_ITEM(pageOffset, page);
    }

    if(NULL == Item)
    {
        return NULL;        
    }

    if(!ITEM_END_CHECK(Item, page))
    {    
        if(GetItemValid(Item->len))
        {
            /* deform row */
            rowData = DeFormRowData(page, Item->offset);
            if(NULL != itemData)
                *itemData = *Item;
        }
    }

    ReleasePage(page);

    return rowData;
}

int GetAttrIndex(PTableList tblInfo, char *attrName)
{
    int index = -1;     /* normal start from 0 */
    int col = 0; 

    for(col = 0; col < tblInfo->tableDef->colNum; col++)
    {
        if(strcmp(attrName, tblInfo->tableDef->column[col].colName) == 0)
        {
            index = col;
            break;
        }
    }

    return index;
}

PColumnDefInfo GetAttrDef(PTableList tblInfo, char *attrName)
{
    PColumnDefInfo colDef = NULL;     
    int col = 0;        /* normal start from 0 */

    for(col = 0; col < tblInfo->tableDef->colNum; col++)
    {
        if(strcmp(attrName, tblInfo->tableDef->column[col].colName) == 0)
        {
            colDef = &(tblInfo->tableDef->column[col]);
            break;
        }
    }

    return colDef;
}

int GetGroupMemberPageNo(PMemberData memDataPos, int index)
{
    PMemberData pmData = NULL;

    /* member pageoffset specify, default one pageOffset per member data  */   
    int size = sizeof(MemberData) + sizeof(PageOffset);  

    pmData = (PMemberData)((char*)(memDataPos) + index * size);

    return pmData->member[0].pageno;
}



int ReleaseAllResourceOwner()
{
    int ret = 0;
    PPageDataHeader page = NULL;
    int count = 0;

    ret = PopResourceOwner(&page);
    while(ret > 0)
    {
        ReleasePage(page);
        count++;

        ret = PopResourceOwner(&page);
    }

    if(count > 0)
        hat_error("releaseAllResourceOwner Buffer, this is %d rest. \n", count);
    return ret;
}
