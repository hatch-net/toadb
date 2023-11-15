/*
 *	toadb scan
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_SCAN_H_H
#define HAT_SCAN_H_H

#include "list.h"
#include "tables.h"
#include "buffer.h"
#include "portal.h"

typedef struct ScanPageInfo
{
    SearchPageInfo groupPageInfo;       /* group search postion, page */
    PSearchPageInfo searchPageList;     /* data page postion of current search  */
    PPageDataHeader *pageList;          /* group member pages array */
    int *colindexList;                  /* columns index array */
    int pageListNum;                    /* group member page number */
}ScanPageInfo, *PScanPageInfo;

/*
 * record per table raw rowdata
 */
typedef struct Scan
{
    PDList list;    /* dlis cell which value member is ScanState */
    PPortal portal; /* unuse */
}Scan, *PScan;


/* per table record the scan state before the table end. */
typedef struct ScanState
{
    PTableList tblInfo;
    PScan scanInfo;
    int columnNum;      /* query column of current table */
    int rowNum;
    PDList rows;        /* storage rowdata which searched. */
    
    /////old below
    int pageIndex;      /* page index of table file */
    int pageOffset;     /* offset of page */
    PPageDataHeader page;
    ScanPageInfo scanPageInfo;  /* old data member */
    /////old above
    
    PScanPageInfo scanPostionInfo;
    PColumnDef columnDefArr;
}ScanState, *PScanState;

typedef struct RowDataPosition
{
    PTableRowData rowData;
    PScanPageInfo scanPostionInfo;
}RowDataPosition, *PRowDataPosition;

typedef struct TableRowDataPosition
{
    PRowDataPosition rowDataPosition;
    int rowNum;
    PTableList tblInfo;
}TableRowDataPosition, *PTableRowDataPosition;

typedef struct ScanTableRowData
{
    PTableRowDataPosition tableRowData;
    int tableNum;
}ScanTableRowData, *PScanTableRowData;

// #define GetScanState(ptbl) ((PScanState)((unsigned long)ptbl - GetOffsetSize(tblInfo, ScanState)))

PScanState InitScanState(PTableList tblInfo, PScan scan);
PScanState GetScanState(PTableList tblInfo, PScanState scanStateHead);

PScanTableRowData TransFormScanRowData(PTableRowData rowData, PScanState scanState);

PTableRowDataPosition GetTblRowDataPosition(PScanTableRowData scanTblRow, PTableList tblInfo);

#endif
