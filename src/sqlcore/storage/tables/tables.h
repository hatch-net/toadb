/*
 *	toadb tables 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_TABLES_H_H
#define HAT_TABLES_H_H



#include "tablecom.h"
#include "nsm.h"
#include "pax.h"

typedef struct TableList *PTableList;

typedef enum StorageType
{
    ST_NSM  = 0x01,
    ST_PAX  = 0x02,
    ST_NUM
}StorageType;



/* create table file and initialize first page with metadata. */
int TableCreate(PTableList tblInfo, ForkType forkNum);
int TableFileInit(PTableList tblInfo);

int TableOpen(PTableList tblInfo, char *tableName, ForkType forkNum);

int TableDrop(PTableList tblInfo);

int TableRead(PTableList tblInfo, PPageOffset pageoffset, ForkType forkNum, char *page);
int TableWrite(PTableList tblInfo, PPageHeader page, ForkType forkNum);

/* object id update */
int GetObjectId();

/* 
 * page tuple operator 
*/
int InsertRowData(PPageDataHeader page, PRowData rowData, PItemData item);
int InsertItemData(PPageDataHeader page, PItemData item);

int ReplaceRowData(PPageDataHeader page, PRowData rowData, PItemData item);
int ReplaceItemData(PPageDataHeader page, PItemData item, int itemIndex);

PRowColumnData ReadRowData(PPageDataHeader page,  int offset);
int ReleaseRowDataColumns(PTableRowData rowData);

#endif