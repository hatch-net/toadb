/*
 *	toadb seqscan
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_SEQSCAN_H_H
#define HAT_SEQSCAN_H_H

#include "list.h"
#include "tables.h"
#include "buffer.h"
#include "portal.h"

/* per table record the scan state before the table end. */
typedef struct ScanState
{
    PTableList tblInfo;
    int pageIndex;      /* page index of table file */
    int pageOffset;     /* offset of page */
    int columnNum;      /* query column of current table */
    int rowNum;
    PDList rows;
    PPageDataHeader page;
    PColumnDef columnDefArr;
}ScanState, *PScanState;


/*
 * record per table raw rowdata
 */
typedef struct Scan
{
    PDList list;    /* dlis cell which value member is ScanState */
    PPortal portal; /* unuse */
}Scan, *PScan;

/* 
 *  record one row , which is query .
 */
PTableRowData SeqScanRawRow(PTableList tbl, PScanState tblScan);
int ScanTable(PTableList tbl, PScanState tblScan);
int ScanOneTblRows(char *tblName, PScan scanState);

PScanState AddScanStateNode(PTableList tblInfo, PScan scan);

#endif