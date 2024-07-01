/*
 *	toadb relation cache 
 *
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
*/

#include "relCache.h"
#include "memStack.h"
#include "public.h"
#include "server_pub.h"
#include <string.h>

// #define hat_relcache_debug(...)   log_report(LOG_DEBUG, __VA_ARGS__) 
#define hat_relcache_debug(...) 

/* 数据字典 */
DList *g_TblList = NULL;
PDictionaryContext dictionInfo = NULL;

/* search table metadata infomation. */
static PTableList SearchTblInfo(char *filename);
static PTableList LoadTblInfo(char *filename);
static PTableList FindTblInfo(char *filename);

static void LockTblInfoContextExclusion();
static void ReleaseLockTblInfoContext();
static void LockTblInfoContextShare();

static PTableList AllocTblInfo()
{
    PTableList tbl = NULL;

    tbl = (PTableList)AllocMem(sizeof(TableList));
    memset(tbl, 0x00, sizeof(TableList));

    InitRWLock(&tbl->tblLock);
    InitRWLock(&tbl->extLock);
    return tbl;
}

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
    PageOffset pageNum = {1, 1};
    PVFVec pos = NULL;
    int size = 0;
    int ret = 0;

    if (NULL == filename)
    {
        return NULL;
    }

    if (NULL == g_TblList)
    {
        return NULL;
    }

    /* search table info list */
    thisTbl = SearchTblInfo(filename);
    if (NULL != thisTbl)
    {
        return thisTbl;
    }

    /* table info is not find, or table file is not opened. */
    thisTbl = LoadTblInfo(filename);

    return thisTbl;
}

int InitTblInfo()
{
    PMemContextNode oldContext = NULL;

    /* MemoryContext switch to Database dictionary memory context. */
    oldContext = MemMangerSwitchContext(dictionaryContext);

    if(NULL == dictionInfo)
    {
        dictionInfo = (PDictionaryContext)AllocMem(sizeof(DictionaryContext));
        dictionInfo->tblList = (DList *)AllocMem(sizeof(DList));

        InitRWLock(&dictionInfo->rwLock);
        
        g_TblList = dictionInfo->tblList;
        g_TblList->prev = g_TblList->next = g_TblList;
    }

    /* init storage manager */
    InitSmgr();
    MemMangerSwitchContext(oldContext);
    return 0;
}

int GetAttrIndex(PTableList tblInfo, char *attrName)
{
    int index = -1; /* normal start from 0 */
    int col = 0;

    for (col = 0; col < tblInfo->tableDef->colNum; col++)
    {
        if (strcmp(attrName, tblInfo->tableDef->column[col].colName) == 0)
        {
            index = col;
            break;
        }
    }

    return index;
}

static PTableList LoadTblInfo(char *filename)
{
    PTableList thisTbl = NULL;
    PMemContextNode oldContext = NULL;
    int ret = 0;

    /* MemoryContext switch to Database dictionary memory context. */
    oldContext = MemMangerSwitchContext(dictionaryContext);
    LockTblInfoContextExclusion();

    /* recheck */
    thisTbl = FindTblInfo(filename); 
    if(NULL != thisTbl)
    {
        goto FOUND;
    } 

    /* table info is not find, or table file is not opened. */
    thisTbl = AllocTblInfo();

    /* 初始化 */
    ret = TableOpen(thisTbl, filename, MAIN_FORK);
    if (ret < 0)
    {
        hat_error("table %s open failure, maybe create table first.", filename);
        goto ERRRET;
    }

    /* linker this table info */
    thisTbl->list.prev = g_TblList->next->prev;
    thisTbl->list.next = g_TblList->next;
    g_TblList->next = &(thisTbl->list);

FOUND:    
    ReleaseLockTblInfoContext();
    MemMangerSwitchContext(oldContext);
    return thisTbl;

ERRRET:
    ReleaseLockTblInfoContext();
    MemMangerSwitchContext(oldContext);

    /* lock release before. */
    ReleaseTblInfo(thisTbl);

    return NULL;
}

PTableList GetTableInfoByRel(PRelation rel)
{
    PTableList tbl = NULL;
    PTableList temp = NULL;

    LockTblInfoContextShare();

    temp = (PTableList)g_TblList->next;
    while (NULL != temp && temp != (PTableList)g_TblList)
    {
        if (RelationCompare(rel, temp->rel))
        {
            tbl = temp;
            break;
        }

        temp = (PTableList)((DList *)temp)->next;
    }
    
    ReleaseLockTblInfoContext();
    return tbl;
}


PColumnDefInfo GetAttrDef(PTableList tblInfo, char *attrName)
{
    PColumnDefInfo colDef = NULL;
    int col = 0; /* normal start from 0 */

    for (col = 0; col < tblInfo->tableDef->colNum; col++)
    {
        if (strcmp(attrName, tblInfo->tableDef->column[col].colName) == 0)
        {
            colDef = &(tblInfo->tableDef->column[col]);
            break;
        }
    }

    return colDef;
}

/* 创建表文件及数据字典 */
PTableList CreateTblInfo(PTableMetaInfo tblDef)
{
    PTableList thisTbl = NULL;
    PMemContextNode oldContext = NULL;
    PVFVec pos = NULL;
    int size = 0;
    int ret = 0;

    if (NULL == tblDef)
    {
        return NULL;
    }

    /* search table info list */
    thisTbl = SearchTblInfo(tblDef->tableName);
    if (NULL != thisTbl)
    {
        /* found */
        hat_error("table exitst.");
        return NULL;
    }

    /* MemoryContext switch to Database dictionary memory context. */
    oldContext = MemMangerSwitchContext(dictionaryContext);
    LockTblInfoContextExclusion();

    /* TODO recheck */
    thisTbl = FindTblInfo(tblDef->tableName);
    if (NULL != thisTbl)
    {
        /* found */
        hat_error("table exitst.");
        thisTbl = NULL;
        goto FOUND;
    }

    /* dictionary initialize */
    thisTbl = AllocTblInfo();

    size = sizeof(TableMetaInfo) + sizeof(ColumnDefInfo) * tblDef->colNum;
    thisTbl->tableDef = (PTableMetaInfo)AllocMem(size);
    memcpy(thisTbl->tableDef, tblDef, size);
    thisTbl->tableDef->tableId = GetObjectId();

    /* create table file */
    ret = TableCreate(thisTbl, MAIN_FORK);
    if (ret < 0)
    {
        goto ERRRET;
    }

    /* create table file */
    ret = TableCreate(thisTbl, GROUP_FORK);
    if (ret < 0)
    {
        goto ERRRET;
    }

    thisTbl->rel = (PRelation)AllocMem(sizeof(Relation));
    thisTbl->rel->relid = thisTbl->tableDef->tableId;
    thisTbl->rel->relType = thisTbl->tableDef->tableType;

    /* init table file */
    ret = TableFileInit(thisTbl);
    if (ret < 0)
    {
        goto ERRRET;
    }

    /* linker this table info */
    thisTbl->list.prev = g_TblList->next->prev;
    thisTbl->list.next = g_TblList->next;
    g_TblList->next = &(thisTbl->list);

    hat_relcache_debug("CreateTblInfo thisTbl:%p thisTbl->list.prev:%p thisTbl->list.next:%p ",thisTbl, thisTbl->list.prev, thisTbl->list.next);
FOUND:
    ReleaseLockTblInfoContext();
    MemMangerSwitchContext(oldContext);

    return thisTbl;

ERRRET:
    ReleaseLockTblInfoContext();
    MemMangerSwitchContext(oldContext);

    ReleaseTblInfo(thisTbl);
    return NULL;
}

int ReleaseTblInfo(PTableList tblInfo)
{
    if (NULL == tblInfo)
        return -1;

    LockTblInfoContextExclusion();

    /* linker dettach this table info */
    if (NULL != tblInfo->list.next && NULL != tblInfo->list.prev)
    {        
        tblInfo->list.prev->next = tblInfo->list.next;
        tblInfo->list.next->prev = tblInfo->list.prev;

        hat_relcache_debug("ReleaseTblInfo tblinfo:%p prev->next:%p next->prev:%p", tblInfo, tblInfo->list.prev->next, tblInfo->list.next->prev);
    }

    ReleaseLockTblInfoContext();

    /* TODO: locktable lock */
    /* release resource */
    if (NULL != tblInfo->sgmr)
        smgrRelease(tblInfo->sgmr);

    if (NULL != tblInfo->tableInfo)
        FreeMem(tblInfo->tableInfo);

    if (NULL != tblInfo->tableDef)
        FreeMem(tblInfo->tableDef);

    if (NULL != tblInfo->rel)
        FreeMem(tblInfo->rel);

    if (NULL != tblInfo->groupInfo)
        FreeMem(tblInfo->groupInfo);

    FreeMem(tblInfo);
    return 0;
}

static PTableList FindTblInfo(char *filename)
{
    PTableList tbl = NULL;
    PTableList temp = NULL;

    temp = (PTableList)g_TblList->next;
    while (NULL != temp && temp != (PTableList)g_TblList)
    {
        hat_relcache_debug("SearchTblInfo tblinfo:%p tblname:%s", temp, temp->tableDef->tableName);
        if (strcmp(filename, temp->tableDef->tableName) == 0)
        {
            tbl = temp;
            break;
        }

        temp = (PTableList)((DList *)temp)->next;
    }

    return tbl;
}

PTableList SearchTblInfo(char *filename)
{
    PTableList tbl = NULL;

    LockTblInfoContextShare();
    tbl = FindTblInfo(filename);    
    ReleaseLockTblInfoContext();

    return tbl;
}

int ReleaseAllTblInfoResource()
{
    PTableList temp = NULL;

    if (g_TblList == NULL)
        return -1;

    /* CloseTable */
    while (g_TblList->next != NULL && g_TblList->next != g_TblList)
    {
        hat_relcache_debug("ReleaseAllTblInfoResource tblinfo:%p tblname:%s", temp, temp->tableDef->tableName);
        temp = (PTableList)g_TblList->next;
        ReleaseTblInfo(temp);
    }

    FreeMem(g_TblList);
    g_TblList = NULL;

    DestroyRWLock(&dictionInfo->rwLock);
    
    FreeMem(dictionInfo);
    dictionInfo = NULL;

    return 0;
}

/* TODO: here need lock to protect. */
char * CreateDictionaryItem(int size)
{
    PMemContextNode oldContext = NULL;
    char *ptr = NULL;

    /* MemoryContext switch to Database dictionary memory context. */
    oldContext = MemMangerSwitchContext(dictionaryContext);
    ptr = (char *)AllocMem(size);
    MemMangerSwitchContext(oldContext);

    return ptr;
}

static void LockTblInfoContextExclusion()
{
    if(NULL == dictionInfo)
        return ;

    AcquireLock(&dictionInfo->rwLock, RWLock_WRITE);
}

static void LockTblInfoContextShare()
{
    if(NULL == dictionInfo)
        return ;

    AcquireLock(&dictionInfo->rwLock, RWLock_READ);
}

static void ReleaseLockTblInfoContext()
{
    if(NULL == dictionInfo)
        return ;

    ReleaseRWLock(&dictionInfo->rwLock, RWLock_NULL);
}

void StartExtensionLock(PTableList tblInfo, char *fun, int line)
{
    if(NULL == tblInfo)
        return;
    
    AcquireRWLockEx(&tblInfo->extLock, RWLock_WRITE, fun, line);
}


void EndExtensionLock(PTableList tblInfo, char *fun, int line)
{
    if(NULL == tblInfo)
        return;
    
    ReleaseRWLockEx(&tblInfo->extLock, RWLock_WRITE, fun, line);
}