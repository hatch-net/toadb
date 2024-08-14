/*
 *	toadb buffer 
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

#ifndef HAT_BUFFER_H_H
#define HAT_BUFFER_H_H


#include "parserNode.h"
#include "relCache.h"

typedef enum EStat_RET
{
    ESTAT_ERORR     =   0,
    ESTAT_SUCESS    =   1,
    ESTAT_NO_SPACE  =   2,
    ESTAT_UNKNOW
}EStat_RET;

typedef struct ScanPageInfo  *PScanPageInfo;

typedef struct GroupItemData
{
    PageOffset pagePos;
    GroupItem ItemData;
    MemberData memberData[FLEXIBLE_SIZE];
}GroupItemData, *PGroupItemData;

typedef struct HeapItemData
{
    PageOffset pageno;
    int        itemOffset;
    ItemData   itemData;
}HeapItemData, *PHeapItemData;

typedef struct SearchPageInfo
{
    PPageDataHeader page;
    int item_offset;
    int group_id;
    int pageNum; 
}SearchPageInfo, *PSearchPageInfo;

/* exec form rowdata */
typedef struct AttrDataPosition
{
    HeapItemData headItem;
    TableRowData rowData;
}AttrDataPosition, *PAttrDataPosition;

/* exec form update rowdata */
typedef struct TableRowDataWithPos
{
    int size;           /* row data size */
    int num;            /* column num of row data */
    PAttrDataPosition attrDataPos[FLEXIBLE_SIZE];
}TableRowDataWithPos, *PTableRowDataWithPos;

#define GetGroupMemberNum(gItemData) (GetItemSize(gItemData->ItemData.len) / sizeof(MemberData) - 1)

#define GroupItemDataMemOffset ((unsigned long)(((PGroupItemData)(0))->memberData))

#define HasFreeSpace(pageHeader, size) ((size) < (pageHeader->dataEndOffset - pageHeader->dataOffset))


int CreateBufferPool(int pageNum);
int DestroyBufferPool();

/* get specified page */
PPageDataHeader GetPageByIndex(PTableList tblInfo, int index, ForkType forkNum);
int ReleasePage(PPageDataHeader page);
int ReleaseAllResourceOwner();

/* find free space page */
PPageDataHeader GetSpacePage(PTableList tblInfo, int size, PageOp op, ForkType forkNum);
PPageDataHeader GetSpaceSpecifyPage(PTableList tblInfo, int size, PageOp op, ForkType forkNum, int startPageIndex, int pageType);

/* find free space group page */
int GetSpaceGroupPage(PTableList tblInfo, PTableRowData insertdata, PageOp op, PPageDataHeader *pageList, PScanPageInfo scanInfo);

// PGroupItemData FindGroupInfo(PTableList tblInfo, int groupId);
int GetGroupInfo(PTableList tblInfo, PSearchPageInfo searchInfo, PGroupItemData gItemData);

int GetPageNoFromGroupInfo(PSearchPageInfo groupInfo, int AttrIndex);
int GetGroupMemberPageNo(PMemberData memDataPos, int index);

PPageDataHeader GetFreeSpaceMemberPage(PTableList tblInfo, int size, PGroupItemData item, PageOp op, int colIndex);
PPageDataHeader* GetGroupMemberPages(PTableList tblInfo, PGroupItemData item, int *pageNum);
int GetGroupMemberPagesOpt(PTableList tblInfo, PScanPageInfo scanPageInfo);

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
// PTableRowData GetRowDataFromPage(PTableList tblInfo, PSearchPageInfo searchInfo);
PRowColumnData GetRowDataFromPageEx(PTableList tblInfo, PSearchPageInfo searchInfo);

//PTableRowData GetRowDataFromPageByIndex(PTableList tblInfo, int pageIndex, int rowIndex, PItemData itemData);
PRowColumnData GetRowDataFromPageByIndexEx(PTableList tblInfo, int pageIndex, int pageOffset, PHeapItemData heapItemData);

//PTableRowData GetRowDataFromExtPage(PTableList tblInfo, int pageno, int itemIndex);
PRowColumnData GetRowDataFromExtPageEx(PPageDataHeader page, int itemIndex, PHeapItemData heapItemData);

/* form row data and deform column data */
PTableRowData FormRowData(PTableMetaInfo tblMeta, PInsertStmt stmt);

// PTableRowData DeFormRowData(PPageDataHeader page, int pageffset);
PRowData DeFormRowDatEx(PPageDataHeader page, int pageffset);
PRowColumnData DeFormRowDatEx2(PPageDataHeader page, int pageffset);

/* form group row data, row array return */
//PTableRowData FormColRowsData(PTableRowData insertdata);
PTableRowData FormCol2RowData(PTableRowData *colRows, int colNum);
int FormCol2RowDataEx(PRowColumnData *colRows, int colNum, PRowData rowsData);
int FormCol2RowDataEx2(PRowColumnData *colRows, int colNum, PRowData rowsData);

PTableRowData FormColData2RowData(PRowColumnData colRows);


int UpdateMetaData(PTableList tblInfo, ForkType forkNum);
int UpdateGroupMetaData(PTableList tblInfo, PGroupPageHeader page);
int UpdateTableMetaData(PTableList tblInfo, PPageDataHeader page);

/* buffer operator */
int InsertRowDataWithGroup(PTableList tblInfo, PTableRowData insertdata, PScanPageInfo scanPageInfo);

int WriteRowData(PTableList tblInfo, PPageDataHeader page, PTableRowData row);
int WriteRowItemData(PTableList tblInfo, PPageDataHeader page, PTableRowData row);
int WriteRowItemDataWithHeader(PTableList tblInfo, PPageDataHeader page, PRowData row);

int WriteRowOnePage(PTableList tblInfo, PPageDataHeader page, PRowData row);
int WriteRowDataOnly(PTableList tblInfo, PPageDataHeader page, PRowData row, PItemData oldItem);

PPageDataHeader ReadPage(PTableList tblInfo, int index, ForkType forkNum);
int WritePage(PTableList tblInfo, PPageDataHeader page, ForkType forkNum);
int FlushPage(PTableList tblInfo, PPageDataHeader page, ForkType forkNum);

#define LockPage(page, mode) LockBufferEx(page, mode, __FUNCTION__, __LINE__)
#define UnLockPage(page, mode) UnLockBufferEx(page, mode, __FUNCTION__, __LINE__)
int LockBufferEx(PPageDataHeader page, int mode, const char *fun, int line);
int UnLockBufferEx(PPageDataHeader page, int mode, const char *fun, int line);

#endif