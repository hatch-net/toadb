/*
 *	toadb tables 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_TABLES_H_H
#define HAT_TABLES_H_H


#include "tfile.h"
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
int TableCreate(char *fileName, ForkType forkNum);

int TableOpen(PTableList tblInfo, ForkType forkNum);

int TableDrop(PTableList tblInfo);

PPageHeader TableRead(PsgmrInfo smgrInfo, PPageOffset pageoffset, ForkType forkNum);
int TableWrite(PsgmrInfo smgrInfo, PPageHeader page, ForkType forkNum);



#endif