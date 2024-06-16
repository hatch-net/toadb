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

#include "bufferPool.h"
#include "public.h"


#include <string.h>

// #define hat_debug_buffPool(...) log_report(LOG_INFO, __VA_ARGS__) 
#define hat_debug_buffPool(...) 

static int InsertFreeList(PBufferPoolContext bufferPool, BUFFERID id);

int InitBufferPool(PBufferPoolContext bufferPool, int pageNum)
{
    int index = 0;

    if(NULL == bufferPool)
        return -1;
    
    if(pageNum < BUFFER_MIN_NUM)
    {
        hat_error("buffer pool size is invalid.");
        return -1;
    }

    bufferPool->elementSize = sizeof(BufferElement);
    bufferPool->bufferNum = pageNum;

    bufferPool->bufferDesc = (PBufferDesc)(bufferPool + 1);
    bufferPool->bufferPool = (PBufferElement)(bufferPool->bufferDesc + bufferPool->bufferNum);
    bufferPool->freeList = (int *)(bufferPool->bufferPool + bufferPool->bufferNum);

    /* initialize array value */
    for(index = 0; index < pageNum; index ++)
    {
        bufferPool->freeList[index] = index + 1;
        bufferPool->bufferDesc[index].isValid = BVF_INVALID;
        bufferPool->bufferDesc[index].isDirty = BUFFER_CLEAN;

        InitRWLock(&bufferPool->bufferDesc[index].lock);
    }
    bufferPool->freeList[pageNum-1] = Free_List_End; 

    return 0;
}

int ReleaseBufferPool(PBufferPoolContext bufferPool, int pageNum)
{    
    int index = 0;

    if(NULL == bufferPool)
        return -1;

    for(index = 0; index < pageNum; index ++)
    {
        DestroyRWLock(&bufferPool->bufferDesc[index].lock);
    }

    return 0;
}



int InvalidateBuffer(PBufferPoolContext bufferPool, PBufferDesc bufferDesc)
{
    BUFFERID bufferId = GetBufferDescID(bufferPool, bufferDesc);

    bufferPool->bufferDesc[bufferId].isValid = BVF_INVALID;

    InsertFreeList(bufferPool, bufferId);

    return 0;
}

int ReleaseBufferDesc(PBufferPoolContext bufferPool, PBufferDesc bufferDesc)
{
    BUFFERID bufferId = GetBufferDescID(bufferPool, bufferDesc);

    if(bufferDesc->refCnt > 0)
    {
        bufferDesc->refCnt -= 1;
    }

    hat_debug_buffPool("release buffer id=%d refCnt=%d, buftag[%d,%d,%d]", 
                                        bufferId, bufferDesc->refCnt, 
                                        bufferDesc->bufferTag.tableId,
                                        bufferDesc->bufferTag.pageno,
                                        bufferDesc->bufferTag.forkNum);
 // let clocksweep to do.                                      
 //   if(bufferDesc->refCnt == 0)
 //   {
 //       InsertFreeList(bufferPool, bufferId);
 //   }
    
    return 1;
}

int ReleaseBuffer(PBufferPoolContext bufferPool, PBufferElement bufferElemnet)
{
    BUFFERID bufferId = GetBufferID(bufferPool, bufferElemnet);
    int ret = 0;

    ret = ReleaseBufferDesc(bufferPool, &(bufferPool->bufferDesc[bufferId]));
    return ret;
}


PBufferTag GetBufferTag(PBufferPoolContext bufferPool, PBufferElement bufferElemnet)
{
    BUFFERID bufferId = GetBufferID(bufferPool, bufferElemnet);

    return &(bufferPool->bufferDesc[bufferId].bufferTag);
}

int SetBuffferDirty(PBufferPoolContext bufferPool, PBufferElement bufferElemnet)
{
    BUFFERID bufferId = GetBufferID(bufferPool, bufferElemnet);

    bufferPool->bufferDesc[bufferId].isDirty = BUFFER_DITRY;
    return 0;
}

int ClearBufferDirty(PBufferPoolContext bufferPool, PBufferElement bufferElemnet)
{
    BUFFERID bufferId = GetBufferID(bufferPool, bufferElemnet);

    bufferPool->bufferDesc[bufferId].isDirty = BUFFER_CLEAN;
    return 0;
}

int SetBuffferValid(PBufferPoolContext bufferPool, PBufferElement bufferElemnet, int flag)
{
    BUFFERID bufferId = GetBufferID(bufferPool, bufferElemnet);

    bufferPool->bufferDesc[bufferId].isValid = flag;
    return 0;
}


int PinBuffer(PBufferPoolContext bufferPool, BUFFERID bufferId)
{
    /* It set to max value, which is avoid when new block replace repeatly. */
    bufferPool->bufferDesc[bufferId].usedCnt = USE_COUNT_MAX;
    //    (bufferPool->bufferDesc[bufferId].usedCnt+1) > USE_COUNT_MAX? USE_COUNT_MAX:(bufferPool->bufferDesc[bufferId].usedCnt+1);
    
    bufferPool->bufferDesc[bufferId].refCnt += 1;

    return 0;
}

int unPinBuffer(PBufferPoolContext bufferPool, BUFFERID bufferId)
{
    if(bufferPool->bufferDesc[bufferId].refCnt > 0)
    {
        bufferPool->bufferDesc[bufferId].refCnt -= 1;
    }

    return 0;
}

static int InsertFreeList(PBufferPoolContext bufferPool, BUFFERID id)
{
    /* id = 0, not save to this list. */
    if((id > 0) && (bufferPool->bufferDesc[id].isInFreeList == 0))
    {
        bufferPool->freeList[id] = bufferPool->freeList[0];
        bufferPool->freeList[0] = id;

        bufferPool->bufferDesc[id].isInFreeList = 1;
    }
    return 0;
}

BUFFERID GetFreeBufferId(PBufferPoolContext bufferPool)
{
    BUFFERID id = bufferPool->freeList[0];

    while(id != Free_List_End)
    {
        id = bufferPool->freeList[0];
        bufferPool->freeList[0] = bufferPool->freeList[id];
        bufferPool->bufferDesc[id].isInFreeList = 0;

        if(bufferPool->bufferDesc[id].refCnt == 0)
            break;
    }

    if(id != Free_List_End)
    {
        hat_debug_buffPool("free buffer id=%d refCnt=%d bufferNum=%d", id, bufferPool->bufferDesc[id].refCnt, bufferPool->bufferNum);
    }

    return id;
}



static BUFFERID start = 0;
BUFFERID ClockSweep(PBufferPoolContext bufferPool)
{
    BUFFERID id = start;

    for( ; ; id = (id+1) % bufferPool->bufferNum)
    {
        hat_debug_buffPool("clocksweep id=%d usedCnt=%d isValid=%d refCnt=%d", 
                    id, 
                    bufferPool->bufferDesc[id].usedCnt, 
                    bufferPool->bufferDesc[id].isValid,
                    bufferPool->bufferDesc[id].refCnt);

        if(BVF_INVALID == bufferPool->bufferDesc[id].isValid)
            break;

        bufferPool->bufferDesc[id].usedCnt = (bufferPool->bufferDesc[id].usedCnt) > 0? (bufferPool->bufferDesc[id].usedCnt - 1):0;

        /* after usedCnt minus one, current circle is find one. */
        if((0 == bufferPool->bufferDesc[id].refCnt) && (0 == bufferPool->bufferDesc[id].usedCnt))
            break;
    }

    hat_debug_buffPool("clocksweep id=%d start=%d ", id, start);

    start = (id + 1) % bufferPool->bufferNum;
    return id;
}

int LockBuffer(PBufferDesc bufferDesc, BufferLockMode mode)
{
    if(NULL == bufferDesc)
        return -1;

    return AcquireRWLock(&bufferDesc->lock, mode);
}

int UnlockBuffer(PBufferDesc bufferDesc, BufferLockMode mode)
{
    if(NULL == bufferDesc)
        return -1;

    return ReleaseRWLock(&bufferDesc->lock, mode);
}