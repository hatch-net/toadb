/*
 *	toadb tables 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_TABLES_H_H
#define HAT_TABLES_H_H

#define PAGE_MAX_SIZE (1024)
#define PAGE_VERSION (0x2B3C)

#define FLEXIBLE_SIZE
#define NAME_MAX_LEN  64

#define PAGE_HEAD_PAGE_NUM 1
#define PAGE_EXTENSION_MAX_NUM 512

typedef enum PageType
{
    PAGE_HEADER = 0x01,
    PAGE_DATA
}PageType;

typedef enum PageOp
{
    PAGE_NEW,
    PAGE_EXIST
}PageOp;

/* file manage by pages units.
 * page maxsize 
 * first page: pageheader --- table info -- columninfo ...
 * seconde page: pageheader --- row data ... 
 */
typedef struct PageHeader
{
    int pageVersion;
    int pageType;
    int pageNum;
}PageHeader, *PPageHeader;

typedef struct PageDataHeader
{
    PageHeader header;
    int dataOffset;     /* offset from this structure. */
    int dataEndOffset;  /* the same above */
}PageDataHeader, *PPageDataHeader;

#define PAGE_DATA_HEADER_SIZE sizeof(PageDataHeader)

typedef struct ColumnDefInfo
{
    char colName[NAME_MAX_LEN];
    int type;
    int options;
}ColumnDefInfo, *PColumnDefInfo;

typedef struct TableMetaInfo
{
    int tableId;
    char tableName[NAME_MAX_LEN];
    int colNum;
    ColumnDefInfo column[FLEXIBLE_SIZE];
}TableMetaInfo, *PTableMetaInfo;

typedef struct RowColumnData
{
    int size;
    int attrindex;
    char data[FLEXIBLE_SIZE];
}RowColumnData, *PRowColumnData;

typedef struct TableRowData
{
    int size;           /* row data size */
    int num;            /* column num of row data */
    PRowColumnData columnData[FLEXIBLE_SIZE];
}TableRowData, *PTableRowData;


#endif