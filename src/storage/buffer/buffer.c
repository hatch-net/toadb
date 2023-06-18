/*
 *	toadb buffer 
 * Copyright (C) 2023-2023, senllang
*/

#include "buffer.h"
#include "exectable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#define log printf

DList* g_TblList = NULL;

void *AllocMem(unsigned int size)
{
    char *pMem = NULL;

    pMem = (char *)malloc(size);
    if(NULL == pMem)
    {
        log("alloc mem %d failure. \n", size);
        exit(-1);
    }

    return (void *)pMem;
}

int FreeMem(void *pMem)
{
    if(NULL == pMem)
    {
        return 0;
    }

    free(pMem);

    return 0;
}

PTableList GetTableInfo(char *filename)
{
    PTableList thisTbl = NULL;
    char pagebuffer[PAGE_MAX_SIZE] = {0};
    PPageDataHeader pageheader = (PPageDataHeader)pagebuffer;
    PTableMetaInfo tableinfo = (PTableMetaInfo) (pagebuffer + sizeof(PageDataHeader));
    int size = 0;

    if(NULL == filename)
    {
        return NULL;
    }

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

    if(NULL != thisTbl && thisTbl->tbl_fd != 0)
        return thisTbl;
        
    /* table info is not find, or table file is not opened. */
    if(NULL == thisTbl)
    {
        thisTbl = (PTableList)AllocMem(sizeof(TableList));
        memset(thisTbl, 0x00, sizeof(TableList));
    }

    if(thisTbl->tbl_fd == 0)
    {
        thisTbl->tbl_fd = OpenTableFile(filename, 0666);
    }

    if(thisTbl->tbl_fd <= 0)
    {
        log("table %s is not found, maybe create table first.\n", filename);
        return NULL;
    }
  
    /* read first page */
    lseek(thisTbl->tbl_fd, 0, SEEK_SET);
    read(thisTbl->tbl_fd, pagebuffer, PAGE_MAX_SIZE);

    /* initial table metadata */  
    if(NULL != thisTbl->tableInfo)
        FreeMem(thisTbl->tableInfo);

    if(NULL != thisTbl->tableDef)
        FreeMem(thisTbl->tableDef);

    thisTbl->tableInfo = (PPageDataHeader)AllocMem(sizeof(PageDataHeader));
    memcpy(thisTbl->tableInfo, pageheader, sizeof(PageDataHeader));

    size = sizeof(TableMetaInfo) + tableinfo->colNum * sizeof(ColumnDefInfo);
    thisTbl->tableDef = (PTableMetaInfo)AllocMem(size);
    memcpy(thisTbl->tableDef, tableinfo, size);

    /* linker this table info */
    thisTbl->list.prev = g_TblList->next->prev;
    thisTbl->list.next = g_TblList->next;
    g_TblList->next = &(thisTbl->list);

    return thisTbl;
}

PTableList SearchTblInfo(char *filename)
{
    PTableList tbl = NULL;
    PTableList temp = NULL;

    while(g_TblList->next != NULL && g_TblList->next != g_TblList)
    {
        temp = (PTableList)g_TblList->next;
        if(strcmp(filename, temp->tableDef->tableName) == 0)
        {
            tbl = temp;
            break;
        }
    }

    return tbl;
}

int ReleaseTblInfoResource()
{
    if(g_TblList == NULL)
        return -1;

    /* CloseTable */
    return 0;
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
    memset(rawRows, 0x00, size);

    rawRows->num = tblMeta->colNum;
    rawRows->size += size;

    /* rawRow fill values order by tblMeta, which is null when stmt isnot input. */
    for(index = 0; index < rawRows->num; index++)
    {
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
            log("attr and values is not match. \n");
            /* TODO resource release. */
            return rawRows;
        }

        switch(colDef[index].type)
        {
            case INT:
            case INTEGER:
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
            case VARCHAR:
            {
                int len = strlen(attrData->value.pData)+1;

                size += len;
                rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
                rawRows->columnData[index]->size = size;
                rawRows->columnData[index]->attrindex = index;

                memcpy(rawRows->columnData[index]->data, attrData->value.pData, len);
            }
                break;
            case CHAR:
                size += sizeof(char);
                rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
                rawRows->columnData[index]->size = size;
                rawRows->columnData[index]->attrindex = index;
                memcpy(rawRows->columnData[index]->data, attrData->value.pData, sizeof(char));
                break;
            case BOOL:
                size += sizeof(char);
                rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
                rawRows->columnData[index]->size = size;
                rawRows->columnData[index]->attrindex = index;
                rawRows->columnData[index]->data[0] = attrData->value.iData != 0?'T':'F';
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

/*
 * 从磁盘page中解析一行数据
 * 根据表定义的列，与传入的数据进行匹配，按定义的顺序依次存放数据内容
 * tblMeta中有表的定义：列名，列类型
 * stmt中有对应列和值
 */
PTableRowData DeFormRowData(PPageDataHeader page, int pageffset)
{
    PTableRowData rawRows = NULL;
    PTableRowData pagerow = NULL;
    char *pageBuf = (char *)page;
    int size = 0;
    int *pval = NULL;
    int index = 0;
    int pageBufOffset = 0;
    
    /* num of total column */
    pagerow = (PTableRowData)(pageBuf + pageffset);

    size = pagerow->num * sizeof(PRowColumnData) + sizeof(TableRowData);
    rawRows = (PTableRowData)AllocMem(size);
    memset(rawRows, 0x00, size);

    rawRows->num = pagerow->num;
    rawRows->size = size;

    /*
    * format of row data in the page 
    * |tablerowDataHead|column                               |..|           column|
    * |total size, num | column total size, attr index, data |..| ..              |
    */
    pageBuf = (char *)pagerow;
    pageBufOffset = sizeof(TableRowData);
    /* rawRow fill values order by tblMeta, which is null when stmt isnot input. */
    for(index = 0; index < rawRows->num; index++)
    {
        pval = (int*)(pageBuf + pageBufOffset);
        size = *pval;
        
        /* data is null */
        rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
        memcpy(rawRows->columnData[index], pageBuf + pageBufOffset, size);

        rawRows->size += size;
        pageBufOffset += size;
    }

    return rawRows;
}


/*
 *  按指定大小，查找空闲空间
 *  顺次查找表文件，检查表文件块上的剩余空间;
 *  根据op指定，找不到时，是否需要新建块，还是只在存在的块中查找。
 */
PPageDataHeader GetSpacePage(PTableList tblInfo, int size, PageOp op)
{
    PPageDataHeader page = NULL;
    int pageIndex = PAGE_HEAD_PAGE_NUM+1;  /* first data page start */

    if(size + PAGE_DATA_HEADER_SIZE >= PAGE_MAX_SIZE)
    {
        log("row data size %d is oversize page size %d\n", size, PAGE_MAX_SIZE);
        return page;
    }

    /* sesearch all table file ,from current page. */
    for(; ; pageIndex++)
    {
        page = GetPageByIndex(tblInfo, pageIndex);
        if(NULL == page)
            break;

        if(page->dataEndOffset+size < PAGE_MAX_SIZE)
        {
            break;
        }
    }

    if(page == NULL && op == PAGE_NEW)
    {
        page = ExtensionTbl(tblInfo, 1);
    }

    return page;
}

/*
 * 按指定的page index获取对应的page
*/
PPageDataHeader GetPageByIndex(PTableList tblInfo, int index)
{
    PPageDataHeader page = NULL;

    if(index > tblInfo->tableInfo->header.pageNum)
        return page;

    page = ReadPage(tblInfo, index);
    return page;
}

PPageDataHeader ReadPage(PTableList tblInfo, int index)
{
    PPageDataHeader page = NULL;
    int readSize = 0;

    if(index <= 0)
    {
        return NULL;
    }

    page = (PPageDataHeader)AllocMem(PAGE_MAX_SIZE);

    /* read page */
    lseek(tblInfo->tbl_fd, (index-1) * PAGE_MAX_SIZE, SEEK_SET);
    readSize = read(tblInfo->tbl_fd, (char *)page, PAGE_MAX_SIZE);

    if(readSize < PAGE_MAX_SIZE)
    {
        FreeMem(page);
        return NULL;
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
    offset = PAGE_MAX_SIZE * (page->header.pageNum - 1);
    lseek(tblInfo->tbl_fd, offset, SEEK_SET);
    writeSize = write(tblInfo->tbl_fd, (char *)page, PAGE_MAX_SIZE);

    if(writeSize != PAGE_MAX_SIZE)
    {
        log("write page falure. real size %d is writed into page index %d.\n", writeSize, page->header.pageNum);
    }

    /* end resource release here. */
    FreeMem(row);
    FreeMem(page);   
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

    pageNew->header.pageVersion = PAGE_VERSION;
    pageNew->header.pageType = PAGE_DATA;
    // pageNew->header.pageNum = 0x01;

    pageNew->dataOffset = PAGE_DATA_HEADER_SIZE;
    pageNew->dataEndOffset = PAGE_DATA_HEADER_SIZE;
    return 0;
}

PPageDataHeader ExtensionTbl(PTableList tblInfo, int num)
{
    PPageDataHeader pageNew = NULL;
    PPageDataHeader pageHeader = NULL;
    char page[PAGE_MAX_SIZE] = {0};
    int offset = 0;
    int i = 0;
    int writeSize = 0;

    if(num <= 0 || num > PAGE_EXTENSION_MAX_NUM)
    {
        log("num will be extension is invalid.\n");
        return NULL;
    }

    /* blank space page write to the end of tblfile. */
    (void)InitPage(page, PAGE_DATA);
    pageNew = (PPageDataHeader)page;

    for(; i < num; i++)
    {
        pageNew->header.pageNum = tblInfo->tableInfo->header.pageNum + 1;

        offset = PAGE_MAX_SIZE * tblInfo->tableInfo->header.pageNum;        
        lseek(tblInfo->tbl_fd, offset, SEEK_SET);
        writeSize = write(tblInfo->tbl_fd, (char *)page, PAGE_MAX_SIZE);
        if(writeSize != PAGE_MAX_SIZE)
        {
            log("table is extened failure, maybe is not enough space.\n");
            return NULL;
        }

        tblInfo->tableInfo->header.pageNum += 1;
    }

    /* tail page is first use. */
    pageNew = (PPageDataHeader)AllocMem(PAGE_MAX_SIZE);
    memcpy(pageNew, page, PAGE_MAX_SIZE);

    /* write to header page */
    lseek(tblInfo->tbl_fd, 0, SEEK_SET);
    read(tblInfo->tbl_fd, (char *)page, PAGE_MAX_SIZE);

    pageHeader = (PPageDataHeader)page;
    pageHeader->header.pageNum = tblInfo->tableInfo->header.pageNum;

    lseek(tblInfo->tbl_fd, 0, SEEK_SET);
    writeSize = write(tblInfo->tbl_fd, (char *)page, PAGE_MAX_SIZE);
    if(writeSize != PAGE_MAX_SIZE)
    {
        log("table is extened failure, maybe is not enough space.\n");
        return NULL;
    }

    return pageNew;
}