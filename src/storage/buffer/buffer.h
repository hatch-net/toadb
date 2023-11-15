/*
 *	toadb buffer 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_BUFFER_H_H
#define HAT_BUFFER_H_H

#include "tables.h"
#include "parserNode.h"
#include "list.h"
#include "relation.h"
#include "memStack.h"

typedef struct ScanPageInfo  *PScanPageInfo;

typedef struct TableList
{
    DList list;
    PRelation rel;
    PPageDataHeader tableInfo;
    PTableMetaInfo tableDef;
    PGroupPageHeader groupInfo;
    int tbl_fd;
    PsgmrInfo sgmr;
}TableList, *PTableList;

typedef struct GroupItemData
{
    PageOffset pagePos;
    GroupItem ItemData;
    MemberData memberData[FLEXIBLE_SIZE];
}GroupItemData, *PGroupItemData;

typedef struct HeapItemData
{
    PageOffset pagePos;
    ItemData ItemData;
}HeapItemData, *PHeapItemData;

typedef struct SearchPageInfo
{
    PPageDataHeader page;
    int item_offset;
    int group_id;
    int pageNum; 
}SearchPageInfo, *PSearchPageInfo;

#define GetGroupMemberNum(gItemData) (GetItemSize(gItemData->ItemData.len) / sizeof(MemberData) - 1)

#define GroupItemDataMemOffset ((unsigned long)(((PGroupItemData)(0))->memberData))

#define HasFreeSpace(pageHeader, size) ((size) < (pageHeader->dataEndOffset - pageHeader->dataOffset))

/* global dictionary list */
extern DList* g_TblList;


/* search table metadata infomation, if NULL, load from file. */
PTableList GetTableInfo(char *filename);

/* search table metadata infomation. */
PTableList SearchTblInfo(char *filename);

/* search table metadata, return index of attribute Name position. if not found, return -1 . */
int GetAttrIndex(PTableList tblInfo, char *attrName);

/* search table metadata, if not found, return NULL . */
PColumnDefInfo GetAttrDef(PTableList tblInfo, char *attrName);

int ReleaseTblInfo(PTableList tblInfo);
int ReleaseAllTblInfoResource();

/* get specified page */
PPageDataHeader GetPageByIndex(PTableList tblInfo, int index, ForkType forkNum);

/* find free space page */
PPageDataHeader GetSpacePage(PTableList tblInfo, int size, PageOp op, ForkType forkNum);

/* find free space group page */
int GetSpaceGroupPage(PTableList tblInfo, PTableRowData insertdata, PageOp op, PPageDataHeader *pageList);

PGroupItemData FindGroupInfo(PTableList tblInfo, int groupId);
PGroupItemData GetGroupInfo(PTableList tblInfo, PSearchPageInfo searchInfo);
int GetPageNoFromGroupInfo(PSearchPageInfo groupInfo, int AttrIndex);
int GetGroupMemberPageNo(PMemberData memDataPos, int index);

PPageDataHeader GetFreeSpaceMemberPage(PTableList tblInfo, int size, PGroupItemData item, PageOp op, int colIndex);
PPageDataHeader* GetGroupMemberPages(PTableList tblInfo, PGroupItemData item, int *pageNum);
PPageDataHeader* GetGroupMemberPagesOpt(PTableList tblInfo, PScanPageInfo scanPageInfo);

/* insert one group info into group file */
int InsertGroupItem(PTableList tblInfo, PPageDataHeader lastpage, int num);

int ReleasePageList(PPageDataHeader *pagelist, int num);
int ReleasePageListMember(PPageDataHeader *pagelist, int num);

int UpdateGroupMember(PTableList tblInfo, PPageDataHeader lastpage, int num, PGroupItemData item);

int GetInvlidRowCount(PPageDataHeader page);

/* page extension and init */
PPageDataHeader ExtensionTbl(PTableList tblInfo, int num, ForkType forkNum);
int InitPage(char *page, int flag);

/* main table fork page row */
PTableRowData GetRowDataFromPage(PTableList tblInfo, PSearchPageInfo searchInfo);
PTableRowData GetRowDataFromPageByIndex(PTableList tblInfo, int pageIndex, int rowIndex);

/* form row data and deform column data */
PTableRowData FormRowData(PTableMetaInfo tblMeta, PInsertStmt stmt);
PTableRowData DeFormRowData(PPageDataHeader page, int pageffset);

/* form group row data, row array return */
PTableRowData FormColRowsData(PTableRowData insertdata);
PTableRowData FormCol2RowData(PTableRowData *colRows, int colNum);
PTableRowData FormColData2RowData(PRowColumnData colRows);

int UpdateMetaData(PTableList tblInfo, ForkType forkNum);
int UpdateGroupMetaData(PTableList tblInfo, PGroupPageHeader page);
int UpdateTableMetaData(PTableList tblInfo, PPageDataHeader page);

int CloseTable(PTableList tbl);


/* buffer operator */
int FlushBuffer(PTableList tblInfo, char *buffer);
int WriteRowData(PTableList tblInfo, PPageDataHeader page, PTableRowData row);
int WriteRowItemData(PTableList tblInfo, PPageDataHeader page, PTableRowData row);



PPageDataHeader ReadPage(PTableList tblInfo, int index, ForkType forkNum);
int WritePage(PTableList tblInfo, PPageDataHeader page, ForkType forkNum);


#endif