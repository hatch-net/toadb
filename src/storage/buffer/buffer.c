/*
 *	toadb buffer 
 * Copyright (C) 2023-2023, senllang
*/

#include "buffer.h"
#include "exectable.h"
#include "tfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#define log printf

DList* g_TblList = NULL;

#define DEBUG_MEM_BARRIER 0xBE
#define DEBUG_MEM_TYPE_SIZE (sizeof(int))
#define DEBUG_MEM_BARRIER_SIZE (3 * DEBUG_MEM_TYPE_SIZE)

void *AllocMem(unsigned int size)
{
    char *pMem = NULL;

#ifdef MEM_DEBUG
    char *start = NULL;
    
    /* prev and after */
    size += DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE;
#endif
    pMem = (char *)malloc(size);
    if(NULL == pMem)
    {
        log("alloc mem %d failure. \n", size);
        exit(-1);
    }
#ifdef MEM_DEBUG
    start = pMem;
    /* prev */
    memset(pMem, DEBUG_MEM_BARRIER, DEBUG_MEM_TYPE_SIZE);
    pMem += DEBUG_MEM_TYPE_SIZE;

    *((unsigned int*)pMem)= size;
    pMem += DEBUG_MEM_TYPE_SIZE;

    memset(pMem, DEBUG_MEM_BARRIER, DEBUG_MEM_TYPE_SIZE);
    pMem += DEBUG_MEM_TYPE_SIZE;


    /* after */
    size -= DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE;
    memcpy(pMem+size, start, DEBUG_MEM_BARRIER_SIZE);
    
    /* zero user space */
    memset(pMem, 0x00, size);
#endif
    
    return (void *)pMem;
}

int FreeMem(void *pMem)
{
    if(NULL == pMem)
    {
        return 0;
    }

#ifdef MEM_DEBUG
    pMem -= DEBUG_MEM_BARRIER_SIZE;
    memset(pMem+DEBUG_MEM_TYPE_SIZE+DEBUG_MEM_TYPE_SIZE, 0x00, DEBUG_MEM_TYPE_SIZE);
#endif
    free(pMem);

    return 0;
}

/*
 * table metadata load from table and group files. 
 * filename: table name, is as table file. 
 * PTableList: return pointer of metadata structure.
 */
PTableList GetTableInfo(char *filename)
{
    PTableList thisTbl = NULL;
    char pagebuffer[PAGE_MAX_SIZE] = {0};
    PPageDataHeader pageheader = (PPageDataHeader)pagebuffer;
    PTableMetaInfo tableinfo = (PTableMetaInfo) (pagebuffer + sizeof(PageDataHeader));
    PVFVec pos = NULL;
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

    if(thisTbl->sgmr == NULL)
    {
        thisTbl->sgmr = (PsgmrInfo)AllocMem(sizeof(sgmrInfo));
        memset(thisTbl->sgmr, 0x00, sizeof(sgmrInfo));
        thisTbl->sgmr->version = 0x01;

        pos = smgr_open(thisTbl->sgmr, filename, MAIN_FORK);
    }

    if(NULL == pos)
    {
        log("table %s is not found, maybe create table first.\n", filename);
        return NULL;
    }
    thisTbl->tbl_fd = pos->fd;
  
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
    //if(size > PAGE_MAX_SIZE)
    
    thisTbl->tableDef = (PTableMetaInfo)AllocMem(size);
    memcpy(thisTbl->tableDef, tableinfo, size);

    thisTbl->rel = (PRelation)AllocMem(sizeof(Relation));
    thisTbl->rel->relid = thisTbl->tableDef->tableId;
    thisTbl->rel->relType = thisTbl->tableDef->tableType;
    thisTbl->sgmr->storageType = thisTbl->tableDef->tableType;
    
    /* update global varial */
    SetObjectId(thisTbl->rel->relid);

    /* linker this table info */
    thisTbl->list.prev = g_TblList->next->prev;
    thisTbl->list.next = g_TblList->next;
    g_TblList->next = &(thisTbl->list);

    /* load group metadata info */
    TableOpen(thisTbl, GROUP_FORK);
    return thisTbl;
}

int ReleaseTblInfo(PTableList tblInfo)
{
    if(NULL == tblInfo)
        return -1;
    
    /* linker dettach this table info */
    tblInfo->list.prev->next = tblInfo->list.next;
    tblInfo->list.next->prev = tblInfo->list.prev;
    
    /* release resource */
    if(tblInfo->tbl_fd > 0)
        smgrClose(tblInfo->tbl_fd);

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
            log("attr and values is not match. \n");
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

    rawRows->num = pagerow->num;
    rawRows->size = pagerow->size;

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
        if(size <= 0 || size > MAX_ROW_DATA_SIZE)
        {
            FreeMem(rawRows);
            rawRows = NULL;
            log("found bad tuple size %d\n", size);
            break;
        }
        /* data is null */
        rawRows->columnData[index] = (PRowColumnData)AllocMem(size);
        memcpy(rawRows->columnData[index], pageBuf + pageBufOffset, size);

        pageBufOffset += size;
    }

    return rawRows;
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
        log("row data size %d is oversize page size %d\n", size, PAGE_MAX_SIZE);
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
        FreeMem(page);
    }

    if(page == NULL && op == PAGE_NEW)
    {
        /* only group file */
        page = ExtensionTbl(tblInfo, 1, forkNum);
    }

    return page;
}

/*
 * 按指定的page index获取对应的page
*/
PPageDataHeader GetPageByIndex(PTableList tblInfo, int index, ForkType forkNum)
{
    PPageDataHeader page = NULL;

    /*
    * we will not check, it only test by filesystem . 
    */
    //if(index > tblInfo->tableInfo->header.pageNum)
    //    return page;
    
    page = ReadPage(tblInfo, index, forkNum);
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


/*
 * format of row data in the page 
 * two part : item and data; 
 * first part item: dataOffset -> ItemData
 * seconde part data:  dataEndOffset -> TableRowData
 * 
 * |tablerowDataHead|column                               |..|           column|
 * |total size, num | column total size, attr index, data |..| ..              |
 */
int WriteRowItemData(PTableList tblInfo, PPageDataHeader page, PTableRowData row)
{
    int offset = 0;
    int columnIndex = 0;
    char *newRowBuffer = NULL;
    int size = 0;
    ItemData item = {0};

    /* form item data */
    item.len = sizeof(TableRowData) + row->size;
    item.offset = page->dataEndOffset - item.len;
    item.rowid = 0;                                         /* not used */

    item.len |= ITEM_VALID_MASK;

    /* rowdata insert into page buffer */
    newRowBuffer = (char *)page + item.offset;   
    
    /* rowdata header */
    size = sizeof(TableRowData);
    memcpy(newRowBuffer, row, size);
    newRowBuffer += size;

    for(columnIndex = 0; columnIndex < row->num; columnIndex++)
    {
        /* column header and data */
        memcpy(newRowBuffer, row->columnData[columnIndex], row->columnData[columnIndex]->size);
        newRowBuffer += row->columnData[columnIndex]->size;
    }

    /* item data insert into page buffer */
    newRowBuffer = (char *)page + page->dataOffset;
    memcpy(newRowBuffer, &item, sizeof(ItemData));

    /* page buffer offset positon reset */
    page->dataOffset += sizeof(ItemData);
    page->dataEndOffset = item.offset;

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

    pageNew->header.pageVersion = PAGE_VERSION;
    pageNew->header.pageType = PAGE_DATA;
    // pageNew->header.pageNum = tblInfo->tableInfo->header.pageNum + 1;

    pageNew->dataOffset = PAGE_DATA_HEADER_SIZE;
    pageNew->dataEndOffset = PAGE_DATA_MAX_OFFSET;
    return 0;
}

PPageDataHeader ExtensionTbl(PTableList tblInfo, int num, ForkType forkNum)
{
    PPageDataHeader pageNew = NULL;
    PPageDataHeader pageHeader = NULL;
    char *page = NULL;
    int i = 0;
    PVFVec vfInfo = NULL;
    PageOffset pageoffset = {0,0};

    if(num <= 0 || num > PAGE_EXTENSION_MAX_NUM)
    {
        log("num will be extension is invalid.\n");
        return NULL;
    }

    switch(forkNum)
    {
        case MAIN_FORK:
            pageHeader = tblInfo->tableInfo;
            vfInfo = (PVFVec)AllocMem(sizeof(VFVec));
            vfInfo->fd = tblInfo->tbl_fd;
            break;
        case GROUP_FORK:
            pageHeader = &(tblInfo->groupInfo->pageheader);
            vfInfo = smgr_open(tblInfo->sgmr, tblInfo->tableDef->tableName, forkNum);
            break;
        default:
            return NULL;
        break;
    }

    /* blank space page write to the end of tblfile. */
    page = (char *)AllocMem(PAGE_MAX_SIZE);

    (void)InitPage(page, PAGE_DATA);
    pageNew = (PPageDataHeader)page;

    for(; i < num; i++)
    {
        pageNew->header.pageNum = pageHeader->pageCnt + 1;

        pageoffset.pageno = pageNew->header.pageNum;
        smgr_write(vfInfo, &pageoffset, &(pageNew->header));

        pageHeader->pageCnt += 1;
    }

    /* write to header page */
    UpdateMetaData(tblInfo, forkNum);

    /* tail page is first use. */
    return pageNew;
}

int FlushBuffer(PTableList tblInfo, char *buffer)
{
    PPageDataHeader pageheader = (PPageDataHeader)buffer;
    int ret = smgrFlush(tblInfo->tbl_fd, buffer, pageheader->header.pageNum);
    return ret;
}

int UpdateMetaData(PTableList tblInfo, ForkType forkNum)
{
    PPageDataHeader page = NULL;

    /* read full page */
    page = ReadPage(tblInfo, PAGE_HEAD_PAGE_NUM, forkNum);

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

    FreeMem(page);
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

PPageDataHeader ReadPage(PTableList tblInfo, int index, ForkType forkNum)
{
    PPageDataHeader page = NULL;
    PageOffset pageoffset;

    if(index <= 0)
    {
        return NULL;
    }

    pageoffset.pageno = index;
    page = (PPageDataHeader)TableRead(tblInfo->sgmr, &pageoffset, forkNum);
    return page;
}

int WritePage(PTableList tblInfo, PPageDataHeader page, ForkType forkNum)
{
    int ret = 0;

    ret = TableWrite(tblInfo->sgmr, (PPageHeader)page, forkNum);
    return ret;
}

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
        log("invalid parameters \n");
        return NULL;
    }

    /* init group file */
    if(NULL == tblInfo->groupInfo)
    {
        ret = TableOpen(tblInfo, GROUP_FORK);
        if(ret < 0)
        {
            return NULL;
        }
    }

    /* find group info, item scan from page  */
    pageNum.pageno = PAGE_HEAD_PAGE_NUM + 1;

    page = (PPageDataHeader)TableRead(tblInfo->sgmr, &pageNum, GROUP_FORK);
    while (page != NULL)
    {
        if(NULL == gItem)
            gItem = (PGroupItem)page->item;
        
        /* next page */
        if(ITEM_END_CHECK(gItem, page))
        {
            FreeMem(page);

            pageNum.pageno ++;
            page = (PPageDataHeader)TableRead(tblInfo->sgmr, &pageNum, GROUP_FORK);

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

/* 
 * get group info from group file, which is same name prefix as relation file.
 * We will create group file at the first time. 
 * 
 * group itemdata will be returned one by one from second page to the end of file.
 * returning NULL when file end.
 */
PGroupItemData GetGroupInfo(PTableList tblInfo, PSearchPageInfo searchInfo)
{
    PTableList tblGroup = NULL;
    char groupFileName[FILE_NAME_MAX_LEN] = {0};
    PPageDataHeader page = NULL;
    PGroupItem gItem = NULL;
    PGroupItemData gItemData = NULL;
    PageOffset pageNum = {0,0};
    int ret = 0;

    if(NULL == tblInfo)
    {
        log("invalid parameters \n");
        return NULL;
    }

    page = searchInfo->page;
    if(NULL == page)
    {
        /* find group info, item scan from page  */
        pageNum.pageno = PAGE_HEAD_PAGE_NUM + 1;

        page = (PPageDataHeader)TableRead(tblInfo->sgmr, &pageNum, GROUP_FORK);
        searchInfo->page = page;        
    }
    else 
    {
        gItem = (PGroupItem)GET_ITEM(searchInfo->item_offset, page);
        gItem += 1;
        searchInfo->pageNum = searchInfo->page->header.pageNum;
    }

    while (page != NULL)
    {
        if(NULL == gItem)
            gItem = (PGroupItem)page->item;
        
        /* next page */
        if(ITEM_END_CHECK(gItem, page))
        {
            searchInfo->pageNum = page->header.pageNum;
            FreeMem(page);

            pageNum.pageno = searchInfo->pageNum + 1;
            page = (PPageDataHeader)TableRead(tblInfo->sgmr, &pageNum, GROUP_FORK);
            searchInfo->page = page;

            gItem = NULL;            
            continue;
        }

        if(GetItemValid(gItem->len))
        {
            gItemData = (PGroupItemData)AllocMem(sizeof(GroupItemData) + GetItemSize(gItem->len));

            /* item infomation */
            memcpy(&(gItemData->ItemData), gItem, sizeof(GroupItem));

            /* member page infomation */
            memcpy(gItemData->memberData, (char*)page + gItem->offset, GetItemSize(gItem->len));

            gItemData->pagePos.pageno = page->header.pageNum;

            searchInfo->group_id = gItem->groupid;
            searchInfo->item_offset = ITEM_OFFSET(gItem, page);
            searchInfo->pageNum = page->header.pageNum;

            return gItemData;
        }

        /* next item */
        gItem += 1;
    }
    
    return NULL;
}

/* 
 * free space check every group one by one, 
 * and inner group pages , from one column pages to another. 
 */
int GetSpaceGroupPage(PTableList tblInfo, PTableRowData insertdata, PageOp op, PPageDataHeader *pageList)
{
    int groupId = FIRST_GROUP_ID;
    PGroupItemData groupItem = NULL;
    int colNum = 0;
    PPageDataHeader pageHeader = NULL;
    SearchPageInfo searchPage ; 

    if((NULL == tblInfo) || (NULL == insertdata) || (NULL == pageList))
    {
        log("invalid parameters \n");
        return -1;
    }

    memset(&searchPage, 0x00, sizeof(searchPage));

    /* will extension space, until error ocur. */
    for(; ; groupId++)
    {
        /* find one group */
        groupItem = GetGroupInfo(tblInfo, &searchPage);
        if(NULL == groupItem)
            break;

        /* check free space */
       /* if(GetGroupMemberNum(groupItem) != insertdata->num)
        {
            log("insert num%d, group metadata member num %d is not equal. \n", insertdata->num, colNum);
            break;
        }
        */

        for(colNum = 0; colNum < insertdata->num; colNum ++)
        {
            pageHeader = GetFreeSpaceMemberPage(tblInfo, 
                                        insertdata->columnData[colNum]->size + sizeof(TableRowData) + sizeof(ItemData), 
                                        groupItem,
                                        op, 
                                        colNum);
            if(NULL == pageHeader)
            {
                /* this group not enogh space */
                ReleasePageListMember(pageList, insertdata->num);
                break;
            }

            pageList[colNum] = pageHeader;
        }

        if(colNum == insertdata->num)
            break;   

        FreeMem(groupItem);   
        groupItem = NULL;
    }

    if(groupItem != NULL)
    {
        FreeMem(groupItem);
        groupItem = NULL;
    }

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
    int pageIndex = 0;
    int subPageNum = 0;
    PMemberData memberData = NULL;
    int offset = 0;

    /* calculator memberdata offset */
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
            break;
        }

        if(HasFreeSpace(page, size))
        {
            /* Enough space is found. */
            break;
        }
        
        /* todo: bufferpool */
        FreeMem(page);
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

    for(; pageIndex != INVALID_PAGE_NUM; )
    {
        page = GetPageByIndex(tblInfo, pageIndex, MAIN_FORK);
        if(NULL == page)
        {
            break;
        }

        pagelist[subPageNum] = page;

        /* next page as column link */
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

        log("group page invalid. groupmember[%d], pagefind[%d-%d]\n", memberData->memNum, subPageNum, pageIndex);
    }

    return pagelist;
}

int ReleasePageList(PPageDataHeader *pagelist, int num)
{
    int pageIndex = 0;
    
    while(pagelist[pageIndex] != NULL)
    {
        FreeMem(pagelist[pageIndex]);

        if(++pageIndex >= num)
            break;
    }
    
    FreeMem(pagelist);

    return pageIndex;
}

int ReleasePageListMember(PPageDataHeader *pagelist, int num)
{
    int pageIndex = 0;
    
    while(pagelist[pageIndex] != NULL)
    {
        FreeMem(pagelist[pageIndex]);
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
        log("col rows form invalid parameters. \n");
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

PTableRowData FormCol2RowData(PTableRowData *colRows, int colNum)
{
    PTableRowData rowData = NULL;
    int rowDataSize = sizeof(TableRowData) + sizeof(PRowColumnData) * colNum;
    int index = 0;

    if(NULL == colRows || NULL == colRows[0])
    {
        log("colrow to rowdata invliad argments\n");
        return NULL;
    }

    rowData = (PTableRowData)AllocMem(rowDataSize);
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
    size = sizeof(GroupItemData) + num * (sizeof(MemberData) +sizeof(PageOffset));
    groupItemData = (PGroupItemData)AllocMem(size);
    
    groupItemData->ItemData.groupid = tblInfo->groupInfo->groupInfo.group_id + 1;
    size = num * (sizeof(MemberData) + sizeof(PageOffset));
    groupItemData->ItemData.len = size | ITEM_VALID_MASK;

    pageno = lastpage->header.pageNum - num + 1;
    if(pageno <= PAGE_HEAD_PAGE_NUM)
    {
        log("page number is not enough. realnum:%d , request:%d \n",lastpage->header.pageNum, num);
        FreeMem(groupItemData);
        return -1;
    }

    /* member pageoffset specify */   
    size = sizeof(MemberData) + sizeof(PageOffset);
    pmData = groupItemData->memberData;
    for(i = 0; i < num; i++, pageno++)
    {
        pmData = (PMemberData)((char*)(groupItemData->memberData) + i * size);
        pmData->colIndex = i; /* colindex start from 0 */
        pmData->memNum = 1;
        pmData->member[0].pageno = pageno;
    }

    /* get free space page */
    size += sizeof(GroupItem) + GetItemSize(groupItemData->ItemData.len); 
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

    FreeMem(page);
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
        log("invalid parameters \n");
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

            /* deform row */
            rowData = DeFormRowData(page, Item->offset);
            return rowData;
        }

        /* next item */
        Item += 1;
    }
    
    return NULL;
}