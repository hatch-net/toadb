/*
 *	toadb buffer pool 
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
*/
#ifndef HAT_BUFFER_POOL_H_H
#define HAT_BUFFER_POOL_H_H

#include "tablecom.h"
#include "rwlock.h"

typedef int BUFFERID;
#define INVLID_BUFFER (-1)
#define BUFFER_MIN_NUM (8)

#pragma pack(push, 1)

typedef struct BufferTag
{
    int databaseId;
    int tableId;
    int segno;
    int pageno;
    ForkType forkNum;
}BufferTag, *PBufferTag;

typedef struct BufferPoolHashValue
{
    BufferTag tag;
    BUFFERID bufferId;
}BufferPoolHashValue, *PBufferPoolHashValue;

#pragma pack(pop)

typedef struct BufferElement
{
    char buffer[PAGE_MAX_SIZE];
}BufferElement, *PBufferElement;

typedef enum BufferLockMode
{
    BUF_NULL,
    BUF_READ,
    BUF_WRITE,
    BUF_UNKNOWN
}BufferLockMode;

typedef enum BufferValidFlag
{
    BVF_INVALID,
    BVF_VLID,
    BVF_TAG,
    BUFFER_CLEAN = 0x00,
    BUFFER_DITRY = 0x01,
    BUFFER_FOUND = 0x00,
    BUFFER_EMPTY = 0x01,
    BUFFER_OTHER = 0x03
}BufferValidFlag;

typedef struct BufferDesc
{
    BufferTag bufferTag;
    unsigned int isValid;
    unsigned int isDirty;
    unsigned int refCnt;
    int usedCnt;
    int isInFreeList;
    RWLockInfo lock;
}BufferDesc, *PBufferDesc;

#define CompareBufferTag(tag1, tag2) (((tag1)->databaseId == (tag2)->databaseId) \
                                    && ((tag1)->tableId == (tag2)->tableId) \
                                    && ((tag1)->segno == (tag2)->segno) \
                                    && ((tag1)->pageno == (tag2)->pageno) \
                                    && ((tag1)->forkNum == (tag2)->forkNum))
#define USE_COUNT_MAX (5)


typedef struct BufferPoolContext
{
    int elementSize;                /* per buffer size */
    int bufferNum;                  /* total buffers number. */
    int *freeList;                  /* first postion is head */
    PBufferElement bufferPool;      /* buffer array. */
    PBufferDesc bufferDesc;         /* buffer describe array, which is corosponding to buffer array. */
}BufferPoolContext, *PBufferPoolContext;

#define Free_List_End (-1)
#define GetBufferDescID(buffPool, desc) ((desc) - (bufferPool)->bufferDesc)
#define GetBufferID(buffPool, buffer) ((buffer) - (bufferPool)->bufferPool)

int InitBufferPool(PBufferPoolContext bufferPool, int pageNum);
int ReleaseBufferPool(PBufferPoolContext bufferPool, int pageNum);

int LockBuffer(PBufferDesc bufferDesc, BufferLockMode mode);
int UnlockBuffer(PBufferDesc bufferDesc, BufferLockMode mode);

int PinBuffer(PBufferPoolContext bufferPool, BUFFERID bufferId);
int unPinBuffer(PBufferPoolContext bufferPool, BUFFERID bufferId);

BUFFERID GetFreeBufferId(PBufferPoolContext bufferPool);
BUFFERID ClockSweep(PBufferPoolContext bufferPool);

int InvalidateBuffer(PBufferPoolContext bufferPool, PBufferDesc bufferDesc);
int ReleaseBufferDesc(PBufferPoolContext bufferPool, PBufferDesc bufferDesc);
int ReleaseBuffer(PBufferPoolContext bufferPool, PBufferElement bufferElemnet);

PBufferTag GetBufferTag(PBufferPoolContext bufferPool, PBufferElement bufferElemnet);

/* flag set/clear */
int SetBuffferDirty(PBufferPoolContext bufferPool, PBufferElement bufferElemnet);
int ClearBufferDirty(PBufferPoolContext bufferPool, PBufferElement bufferElemnet);

int SetBuffferValid(PBufferPoolContext bufferPool, PBufferElement bufferElemnet, int flag);
#endif 
