/*
 *	toadb tables common defines  
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_TABLE_COM_H_H
#define HAT_TABLE_COM_H_H

#include "dataTypes.h"
#include "public.h"

#define NAME_MAX_LEN  64
#define PAGE_MAX_SIZE (4096)
#define PAGE_VERSION (0x2B3C)

#define INVALID_PAGE_NUM    0
#define INVALID_SEGMENT_NUM 0
#define IsInvalidPageNum(pageno) ((pageno) == INVALID_PAGE_NUM)

#define PAGE_HEAD_PAGE_NUM 1
#define PAGE_EXTENSION_MAX_NUM 512


#pragma pack(push, 1)

typedef enum FORK_NUM 
{
    MAIN_FORK,
    GROUP_FORK,
    MAX_FORK
}ForkType;



typedef enum PageType
{
    PAGE_HEADER     = 0x01,
    PAGE_DATA       = 0x02,
    PAGE_EXTDATA    = 0x04,
    PAGE_UNDO       = 0x08,
    PAGE_MAX_TYPE
}PageType;

typedef enum PageOp
{
    PAGE_NEW,
    PAGE_EXIST
}PageOp;


#define ITEM_OFFSET(item, page) ((char*)(item) - (char*)(page))
#define ITEM_SIZE (sizeof(ItemData))

#define ITEM_END_CHECK(item, page) (((char*)(item) - (char*)(page)) >= (page)->dataOffset)
#define GET_ITEM(offset, page) ((PItemData)((char*)(page) + offset))

#define GET_ITEM_BY_INDEX(index, page) ((PItemData)((char*)((page)->item) + index * ITEM_SIZE))
#define GET_ITEM_OFFSET_BY_INDEX(index) (index * ITEM_SIZE)

/* current item, dataOffset shift to starting of next item */
#define GET_ITEM_INDEX(page) (((page)->dataOffset - PAGE_DATA_HEADER_SIZE) / ITEM_SIZE)

/* start from 0 */
#define GET_CUR_ITEM_INDEX(item, page) ((ITEM_OFFSET(item, page) - PAGE_DATA_HEADER_SIZE) / ITEM_SIZE)



typedef struct RowID
{
    unsigned long long id;
}RowID;

typedef struct ItemData
{
    int offset;
    int len;            /* high bit is flag mask */
    RowID rowid;    /* todo: */
}ItemData, *PItemData;

/* high 4 bit is  flag */
#define ITEM_FLAG_MASK      0xF0000000
#define ITEM_VALID_MASK     0x80000000      
#define ITEM_REDIRECT_MASK  0x40000000      

/* 
 * valid flag, item used. 
 * redirect flag, include valid flag. this item is only item, and data is stored other page,
 *     then item->offset is pageno, item->len is itemindex, which position data page.
 */

#define GetItemValid(data)      (data & ITEM_VALID_MASK)
#define GetItemRedirect(data)   (data & ITEM_REDIRECT_MASK)
#define GetItemSize(data)       (data & ~ITEM_FLAG_MASK)
#define SetItemSize(old, data)       (((old) & ITEM_FLAG_MASK) | ((data) & ~ITEM_FLAG_MASK) )


#define STORAGE_TYPE_SHIFT          4
#define STORAGE_TYPE_MASK           (0x000000F0)
#define GET_STORAGE_TYPE_SHIFT(sty) (((sty) & STORAGE_TYPE_MASK) >> STORAGE_TYPE_SHIFT)
#define SET_STORAGE_TYPE_SHIFT(sty) ((sty) << STORAGE_TYPE_SHIFT & STORAGE_TYPE_MASK)

#define PAGE_TYPE_MASK              (0x0000000F)
#define GET_PAGE_TYPE(pgt)          ((pgt) & PAGE_TYPE_MASK)
#define SET_PAGE_TYPE(pgt, type)          ((pgt) & ~PAGE_TYPE_MASK | ((type) & PAGE_TYPE_MASK))

typedef struct PageOffset
{
    int segno;
    int pageno;
}PageOffset, *PPageOffset;

typedef struct PageHeader
{
    int pageVersion;
    int pageType;           /* low 4bit pagetype, 4bit storagetype, .. */
    int pageNum;
}PageHeader, *PPageHeader;

#define PAGE_DATA_HEADER_SIZE   sizeof(PageDataHeader)
#define MAX_ROW_DATA_SIZE (PAGE_MAX_SIZE - (PAGE_DATA_HEADER_SIZE + ITEM_SIZE))
#define ITEM_DATA_START_OFFSET (PAGE_DATA_HEADER_SIZE)
#define PAGE_DATA_MAX_OFFSET    (PAGE_MAX_SIZE)

/* file manage by pages units.
 * page maxsize 
 * first page: pageheader --- table info -- columninfo ...
 * seconde page: pageheader --- row data ... 
 */
typedef struct PageDataHeader
{
    PageHeader header;
    int dataOffset;     /* offset from this structure. */
    int dataEndOffset;  /* the same above */
    int pageCnt;        /* only head page record */
    PageOffset undoPage;    /* undo data page offset */
    PageOffset extPage;     /* extension data page offset */
    ItemData item[FLEXIBLE_SIZE];
}PageDataHeader, *PPageDataHeader;

typedef struct ColumnDefInfo
{
    char colName[NAME_MAX_LEN];
    valueType type;
    int attrIndex;          /* start from 0 */
    int options;
}ColumnDefInfo, *PColumnDefInfo;

typedef struct TableMetaInfo
{
    int tableId;
    int tableType;
    char tableName[NAME_MAX_LEN];
    int colNum;                             /* attribute num, start from 0 */
    ColumnDefInfo column[FLEXIBLE_SIZE];
}TableMetaInfo, *PTableMetaInfo;

typedef struct RowPosition
{
    PageOffset pageIndex;
    int        itemIndex;
}RowPosition, *PRowPosition;

/*
 * file store tuple with two parts:
 * item    ---  | offset | length | other |
 * TableRowData ---  | TableRowData(num = n) | RowColumnData1 | ... | RowColumnDatan | 
 */
typedef struct RowColumnData
{
    int size;                   /* RowColumnData size, include size,attrindex,data */
    int attrindex;              /* start from 0 */
    char data[FLEXIBLE_SIZE];
}RowColumnData, *PRowColumnData;

typedef struct TableRowData
{
    int size;           /* PRowColumnData array total size */
    int num;            /* column num of row data */
    PRowColumnData columnData[FLEXIBLE_SIZE];
}TableRowData, *PTableRowData;

/* 
 * when deform rowdata to memory, use this structure.
 * and write to file only the part of rowsData.
*/
typedef struct RowData
{
    RowPosition     rowPos;
    ItemData        item;
    TableRowData    rowsData;
}RowData, *PRowData;

#define ROW_DATA_HEADER_SIZE (sizeof(RowData))
#define MIN_ROW_SIZE (sizeof(TableRowData)+sizeof(RowColumnData))
#define MIN_DATA_SIZE (sizeof(RowColumnData))


#pragma pack(pop)

#endif