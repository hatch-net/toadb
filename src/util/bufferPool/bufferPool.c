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

//#define hat_debug_buffPool(...) log_report(LOG_INFO, __VA_ARGS__) 
//#define hat_debug_bufflock(...) log_report(LOG_INFO, __VA_ARGS__) 
#define hat_debug_bufflock(...) 
//#define hat_debug_buffPool_ClockSweep(...) log_report(LOG_INFO, __VA_ARGS__) 
#define hat_debug_buffPool_ClockSweep(...)
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

    InitRWLock(&bufferPool->lock);

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

int ReleaseBuffer(PBufferPoolContext bufferPool, PBufferElement bufferElemnet)
{
    int ret = 0;
    BUFFERID bufferId = GetBufferID(bufferPool, bufferElemnet);

    ret = unPinBuffer(bufferPool, bufferId);

    return ret;
}


PBufferTag GetBufferTag(PBufferPoolContext bufferPool, PBufferElement bufferElemnet)
{
    BUFFERID bufferId = GetBufferID(bufferPool, bufferElemnet);

    return &(bufferPool->bufferDesc[bufferId].bufferTag);
}

PBufferDesc GetBufferDesc(PBufferPoolContext bufferPool, PBufferElement bufferElemnet)
{
    PBufferDesc desc = NULL;
    BUFFERID bufferId = GetBufferID(bufferPool, bufferElemnet);

    desc = &(bufferPool->bufferDesc[bufferId]);
    return desc;
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

    hat_debug_buffPool("release buffer id=%d refCnt=%d, buftag[%d,%d,%d]", 
                                        bufferId, bufferPool->bufferDesc[bufferId].refCnt, 
                                        bufferPool->bufferDesc[bufferId].bufferTag.tableId,
                                        bufferPool->bufferDesc[bufferId].bufferTag.pageno,
                                        bufferPool->bufferDesc[bufferId].bufferTag.forkNum);
    return 0;
}

static int InsertFreeList(PBufferPoolContext bufferPool, BUFFERID id)
{
    AcquireLock(&bufferPool->lock, RWLock_WRITE);
    /* id = 0, not save to this list. */
    if((id > 0) && (bufferPool->bufferDesc[id].isInFreeList == 0))
    {
        bufferPool->freeList[id] = bufferPool->freeList[0];
        bufferPool->freeList[0] = id;

        bufferPool->bufferDesc[id].isInFreeList = 1;
    }
    ReleaseLock(&bufferPool->lock, RWLock_WRITE);
    return 0;
}

BUFFERID GetFreeBufferId(PBufferPoolContext bufferPool)
{
    BUFFERID id = 0;

    AcquireLock(&bufferPool->lock, RWLock_WRITE);

    id = bufferPool->freeList[0];
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

    ReleaseLock(&bufferPool->lock, RWLock_WRITE);
    return id;
}

volatile BUFFERID start = 0;
BUFFERID ClockSweep(PBufferPoolContext bufferPool)
{
    BUFFERID id = start;

RETRY:
    for( ; ; id = (id+1) % bufferPool->bufferNum)
    {
        LockBufDesc(&bufferPool->bufferDesc[id], BUF_READ);

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
        UnLockBufDesc(&bufferPool->bufferDesc[id], BUF_READ);
    }
    UnLockBufDesc(&bufferPool->bufferDesc[id], BUF_READ);
    
    /* loading process protected by the lock, other will wait lock release. */
    LockBufDesc(&bufferPool->bufferDesc[id], BUF_WRITE);

    if((BVF_INVALID == bufferPool->bufferDesc[id].isValid) 
        || ((0 == bufferPool->bufferDesc[id].refCnt) && (0 == bufferPool->bufferDesc[id].usedCnt)))
    {
        hat_debug_buffPool_ClockSweep("clocksweep id=%d start=%d ", id, start);
        start = (id + 1) % bufferPool->bufferNum;
    }
    else 
    {
        UnLockBufDesc(&bufferPool->bufferDesc[id], BUF_WRITE);
        goto RETRY;
    }

    return id;
}

int LockBufferDesc(PBufferDesc buffer, BufferLockMode mode)
{
    return AcquireRWLock(&buffer->lock, mode);
}

int UnlockBufferDesc(PBufferDesc buffer, BufferLockMode mode)
{
    return ReleaseRWLock(&buffer->lock, mode);
}

int LockBufferDescEx(PBufferDesc buffer, BufferLockMode mode, char *fun, int line)
{
    hat_debug_bufflock("LockBufferDescEx desc %p mode %d [%s][%d]", buffer, mode, fun, line);
    return LockBufferDesc(buffer, mode);
}

int UnlockBufferDescEx(PBufferDesc buffer, BufferLockMode mode, char *fun, int line)
{
    hat_debug_bufflock("UnlockBufferDescEx desc %p mode %d [%s][%d]", buffer, mode, fun, line);
    return UnlockBufferDesc(buffer, mode);
}
