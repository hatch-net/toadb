/*
 *	table files 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_TFILE_H_H
#define HAT_TFILE_H_H

#include "list.h"

/* 
 * filename 
 * table name : table file
 *            : grp_tablename
 */
#define GROUP_FILE_PRE  "grp_"

#define FILE_PATH_MAX_LEN   1024
#define FILE_NAME_MAX_LEN   64
#define FLEXIBLE_SIZE 

/* default 1GB (1*1024*1024*1024) 4Byte */
#define FILE_SEGMENT_MAX_SIZE (0x40000000) 

#define PAGE_MAX_SIZE (4096)

#define FILE_SEGMENT_MAX_PAGE (FILE_SEGMENT_MAX_SIZE/PAGE_MAX_SIZE)

#define PAGE_VERSION (0x2B3C)

#define PAGE_HEAD_PAGE_NUM 1
#define PAGE_EXTENSION_MAX_NUM 512

typedef enum PageType
{
    PAGE_HEADER = 0x01,
    PAGE_DATA   = 0x02,
    PAGE_UNDO   = 0x04,
    PAGE_MAX_TYPE
}PageType;

typedef enum PageOp
{
    PAGE_NEW,
    PAGE_EXIST
}PageOp;

typedef enum FORK_NUM 
{
    MAIN_FORK,
    GROUP_FORK,
    MAX_FORK
}ForkType;

#define INVALID_PAGE_NUM            0 

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

#define ITEM_DATA_START_OFFSET (sizeof(PageDataHeader))
#define PAGE_DATA_HEADER_SIZE   sizeof(PageDataHeader)
#define PAGE_DATA_MAX_OFFSET    (PAGE_MAX_SIZE)

#define ITEM_END_CHECK(item, page) (((char*)(item) - (char*)(page)) >= (page)->dataOffset)
#define GET_ITEM(offset, page) ((PItemData)((char*)(page) + offset))
#define ITEM_OFFSET(item, page) ((char*)(item) - (char*)(page))
#define ITEM_SIZE (sizeof(ItemData))

typedef struct ItemData
{
    int offset;
    int len;            /* high bit |32 valid| */
    long long rowid;    /* todo: */
}ItemData, *PItemData;

/* high 32 bit is valid flag */
#define ITEM_VALID_MASK     0x80000000
#define GetItemValid(data)  (data & ITEM_VALID_MASK)
#define GetItemSize(data)   (data & ~ITEM_VALID_MASK)

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
    ItemData item[FLEXIBLE_SIZE];
}PageDataHeader, *PPageDataHeader;
#define MAX_ROW_DATA_SIZE (PAGE_MAX_SIZE - (PAGE_DATA_HEADER_SIZE + ITEM_SIZE))

#define STORAGE_TYPE_SHIFT          4
#define STORAGE_TYPE_MASK           (0xF0)
#define GET_STORAGE_TYPE_SHIFT(sty) (((sty) & STORAGE_TYPE_MASK) >> STORAGE_TYPE_SHIFT)
#define SET_STORAGE_TYPE_SHIFT(sty) ((sty) << STORAGE_TYPE_SHIFT & STORAGE_TYPE_MASK)

typedef struct VFVec
{
    DList list;
    int forkNum;
    int fd;
}VFVec, *PVFVec;

typedef struct StorageManagerInfo
{
    int version; 
    int storageType;
    PVFVec vfhead;
    PVFVec vpos;
}sgmrInfo, *PsgmrInfo;



int CreateTableFile(char *filename, int mode);
int OpenTableFile(char *filename, int mode);
int DeleteTableFile(char *filename);

/* raw operations interface */
int smgrOpen(char *filename);
int smgrClose(int fd);
int smgrFlush(int fd, char *buffer, int offnum);
int smgrRelease(PsgmrInfo sgmrInfo);

char* smgrReadPage(int fd, int offset, int size);

/* object id update */
int GetObjectId();
void SetObjectId(int id);

PVFVec SearchVF(PVFVec head, ForkType forknum);
PVFVec DeleteVF(PVFVec head, PVFVec node);

int smgr_create(char *fileName, ForkType forkNum);
PVFVec smgr_open(PsgmrInfo smgrInfo, char *fileName, ForkType forkNum);

PPageHeader smgr_read(PVFVec vfInfo, PPageOffset pageOffset);
int smgr_write(PVFVec vfInfo, PPageOffset pageOffset, PPageHeader page);


#endif