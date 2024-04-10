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
    PGroupItemData groupItem;           /* current group item */
    PSearchPageInfo searchPageList;     /* data page postion of current search  */
    PPageDataHeader *pageList;          /* group member pages array */
    int *colindexList;                  /* columns index array */
    int pageListNum;                    /* group member page number */
    
    int pageReset;                      /* seqscan control reset scan */
    int isNoSpace;                      /* when getting freespace group, this group has space, don't to switch group id. */
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
    int rindex;         /* rang table list index, mark scan table, maybe real table is the same, but rindex is special. */
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
    PScanPageInfo scanPostionInfo;
    PTableRowData rowData;
}RowDataPosition, *PRowDataPosition;

typedef struct TableRowDataPosition
{
    int rowNum;
    int rindex;                     /* same as ScanState */
    PTableList tblInfo;
    PRowDataPosition rowDataPosition[];       /* array list */
}TableRowDataPosition, *PTableRowDataPosition;

typedef struct ScanTableRowData
{
    int tableNum;
    PTableRowDataPosition tableRowData;     /* array list */
}ScanTableRowData, *PScanTableRowData;

// #define GetScanState(ptbl) ((PScanState)((unsigned long)ptbl - GetOffsetSize(tblInfo, ScanState)))

PScanState InitScanState(PTableList tblInfo, PScan scan);
PScanState GetScanState(PTableList tblInfo, PScanState scanStateHead);

PScanTableRowData TransFormScanRowData(PTableRowData rowData, PScanState scanState);
int ReleaseRowData(PScanTableRowData scanRowData);

PTableRowDataPosition GetTblRowDataPosition(PScanTableRowData scanTblRow, PTableList tblInfo, int rindex);

/* 从结查rows 中找到对应列的值 */
PTableRowData GetColRowData(PTableRowDataPosition tblRowPosition, PColumnRef colDef, PAttrDataPosition attrPosData, int rowIndex);

/*
 * 输入值为当前列的值,
 * 从行数据中获取一列的值
 * 反之回值结构
 */
Data * TranslateRawColumnData(PTableRowData rowdata, PColumnRef colDef);
#endif
