/*
 *	toadb buffer 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_BUFFER_H_H
#define HAT_BUFFER_H_H

#include "tables.h"
#include "node.h"
#include "list.h"

typedef struct TableList
{
    DList list;
    PPageDataHeader tableInfo;
    PTableMetaInfo tableDef;
    int tbl_fd;
}TableList, *PTableList;

typedef enum ColumnTypeEN
{
    INT,
    INTEGER,
    VARCHAR,
    CHAR,
    BOOL,
    INVALID
}EnColumnType;

extern DList* g_TblList;

void *AllocMem(unsigned int size);
int FreeMem(void *pMem);

PTableList GetTableInfo(char *filename);
PTableList SearchTblInfo(char *filename);
int ReleaseTblInfoResource();

PPageDataHeader GetSpacePage(PTableList tblInfo, int size, PageOp op);
PPageDataHeader GetPageByIndex(PTableList tblInfo, int index);

PPageDataHeader ExtensionTbl(PTableList tblInfo, int num);
int InitPage(char *page, int flag);

PTableRowData FormRowData(PTableMetaInfo tblMeta, PInsertStmt stmt);
PTableRowData DeFormRowData(PPageDataHeader page, int pageffset);

int WriteRowData(PTableList tblInfo, PPageDataHeader page, PTableRowData row);
PPageDataHeader ReadPage(PTableList tblInfo, int index);

int CloseTable(PTableList tbl);



#endif