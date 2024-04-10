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



static int InsertFreeList(PBufferPoolContext bufferPool, BUFFERID id);
static BUFFERID GetFreeBufferId(PBufferPoolContext bufferPool);

static BUFFERID SearchBuffer(PBufferPoolContext bufferPool, PBufferTag bufferTag);
static BUFFERID ClockSweep(PBufferPoolContext bufferPool);



int InitBufferPool(PBufferPoolContext bufferPool, int pageNum)
{
    int index = 0;

    if(NULL == bufferPool)
        return -1;
    
    if(pageNum < BUFFER_MIN_NUM)
    {
        hat_error("buffer pool size is invalid.\n");
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
    }
    bufferPool->freeList[pageNum-1] = Free_List_End; 

    return 0;
}

/*
 * found :
 *      1, buffer tag is found in buffer pool;
 *      2, free buffer is returned, and tag was assigned;
 *      3, the buffer which is returned is dirty buffer, which will be flushed at the first,
 *          then assigned tag.
 *      all of above, refcnt is added, releasebuffer to minus.
 */
PBufferElement AllocBuffer(PBufferPoolContext bufferPool, PBufferTag bufferTag, int *found)
{
    BUFFERID bufferId = 0;
    PBufferElement pElemnt = NULL;

    /* search buffer pool */
    bufferId = SearchBuffer(bufferPool, bufferTag);
    if(INVLID_BUFFER != bufferId)
    {
        bufferPool->bufferDesc[bufferId].usedCnt = 
            (bufferPool->bufferDesc[bufferId].usedCnt+1) > USE_COUNT_MAX? USE_COUNT_MAX:(bufferPool->bufferDesc[bufferId].usedCnt+1);
        bufferPool->bufferDesc[bufferId].refCnt += 1;

        *found = BUFFER_FOUND;
        goto RET_BUF;
    }

    /* find free list */
    bufferId = GetFreeBufferId(bufferPool);
    if(INVLID_BUFFER != bufferId)
    {
        bufferPool->bufferDesc[bufferId].usedCnt = USE_COUNT_MAX;
        bufferPool->bufferDesc[bufferId].refCnt = 1;

        *found = BUFFER_EMPTY;
        goto INIT_TAG;
    }

    /* find a victim */
    bufferId = ClockSweep(bufferPool);    
    /* swap one buffer */
    if(INVLID_BUFFER != bufferId)
    {
        bufferPool->bufferDesc[bufferId].usedCnt = USE_COUNT_MAX;
        bufferPool->bufferDesc[bufferId].refCnt = 1;

        if(BUFFER_DITRY == bufferPool->bufferDesc[bufferId].isDirty)
        {
            *found = BUFFER_OTHER;
            goto RET_BUF;
        }

        *found = BUFFER_EMPTY;
    }

INIT_TAG:
    memcpy(&(bufferPool->bufferDesc[bufferId].bufferTag), bufferTag, sizeof(BufferTag));
    bufferPool->bufferDesc[bufferId].isValid = BVF_TAG;
    bufferPool->bufferDesc[bufferId].isDirty = BUFFER_CLEAN;

RET_BUF:
    pElemnt = &(bufferPool->bufferPool[bufferId]);
    return pElemnt;
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

    hat_debug("release buffer id=%d refCnt=%d, buftag[%d,%d,%d]\n", 
                                        bufferId, bufferDesc->refCnt, 
                                        bufferDesc->bufferTag.tableId,
                                        bufferDesc->bufferTag.pageno,
                                        bufferDesc->bufferTag.forkNum);
                                        
    if(bufferDesc->refCnt == 0)
    {
        InsertFreeList(bufferPool, bufferId);
    }
    
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

static BUFFERID GetFreeBufferId(PBufferPoolContext bufferPool)
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

    if(bufferPool->bufferDesc[id].refCnt != 0)
    {
        hat_error("get free buffer %d reference count %d error\n", id, bufferPool->bufferDesc[id].refCnt);
    }

    hat_debug("free buffer id=%d refCnt=%d bufferNum=%d\n", id, bufferPool->bufferDesc[id].refCnt, bufferPool->bufferNum);

    return id;
}

static BUFFERID SearchBuffer(PBufferPoolContext bufferPool, PBufferTag bufferTag)
{
    BUFFERID id = 0;

    for( ; id < bufferPool->bufferNum; id ++)
    {
        if(BVF_VLID != bufferPool->bufferDesc[id].isValid)
            continue;

        hat_debug1("buffer hit id=%d buftag[%d,%d,%d,%d,%d] requestTag[%d,%d,%d,%d,%d]\n", 
                                        id,  
                                        bufferPool->bufferDesc[id].bufferTag.databaseId,
                                        bufferPool->bufferDesc[id].bufferTag.tableId,
                                        bufferPool->bufferDesc[id].bufferTag.segno,
                                        bufferPool->bufferDesc[id].bufferTag.pageno,
                                        bufferPool->bufferDesc[id].bufferTag.forkNum,
                                        bufferTag->databaseId,
                                        bufferTag->tableId,
                                        bufferTag->segno,
                                        bufferTag->pageno,
                                        bufferTag->forkNum);

        if(CompareBufferTag(&(bufferPool->bufferDesc[id].bufferTag), bufferTag))
            break;
    }

    if(id >= bufferPool->bufferNum)
        return INVLID_BUFFER;

    hat_debug("buffer hit id=%d refCnt=%d, buftag[%d,%d,%d]\n", 
                                        id, bufferPool->bufferDesc[id].refCnt, 
                                        bufferPool->bufferDesc[id].bufferTag.tableId,
                                        bufferPool->bufferDesc[id].bufferTag.pageno,
                                        bufferPool->bufferDesc[id].bufferTag.forkNum);
    return id;
}

static BUFFERID start = 0;
static BUFFERID ClockSweep(PBufferPoolContext bufferPool)
{
    BUFFERID id = start;

    for( ; ; id = (id+1) % bufferPool->bufferNum)
    {
        if(BVF_INVALID == bufferPool->bufferDesc[id].isValid)
            break;

        bufferPool->bufferDesc[id].usedCnt = (bufferPool->bufferDesc[id].usedCnt - 1) >= 0? (bufferPool->bufferDesc[id].usedCnt - 1):0;

        if((0 == bufferPool->bufferDesc[id].refCnt) && (0 == bufferPool->bufferDesc[id].usedCnt))
            break;
    }

    hat_debug("clocksweep id=%d start=%d \n", id, start);

    start = id + 1;
    return id;
}

