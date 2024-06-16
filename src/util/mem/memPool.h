/*
 *	toadb memory pool 
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
#ifndef HAT_MEM_POOL_H_H
#define HAT_MEM_POOL_H_H

#include "memStack.h"
#include "public.h"
#include "spinlock.h"


#define MEMORY_POOL_MANAGER_VERSION (0x0B10)
#define MEMORY_POOL_PAGE_SIZE (4096)
#define MEMORY_POOL_PAGE_ALIG (0xFFFFE000)
#define MEMORY_POOL_BLOCK_HEADER_SIZE (sizeof(MemBlock))

#ifdef MEM_DEBUG
#define MEMORY_POOL_FREE_PAGE_SIZE (MEMORY_POOL_PAGE_SIZE + DEBUG_MEM_BARRIER_SIZE)
#define MEMORY_POOL_MIN_ALLOC_SIZE (MEMORY_POOL_PAGE_SIZE - MEMPAGE_INFO_LEN - DEBUG_MEM_BARRIER_SIZE)
#else 
#define MEMORY_POOL_FREE_PAGE_SIZE (MEMORY_POOL_PAGE_SIZE)
#define MEMORY_POOL_MIN_ALLOC_SIZE (MEMORY_POOL_PAGE_SIZE - MEMPAGE_INFO_LEN)
#endif

#pragma pack(push, 1)

typedef struct MemPoolManagerContext
{

    DList           memFreeList;
    int             version;
    unsigned long   totalSize;    /* 已经使用的动态内存大小 */
    SPINLOCK        lock;         /* protected this structure. */
}MemPoolManagerContext, *PMemPoolManagerContext;

/* 
 * MemAlloc will alloc a MemBlock, and return MemBlock->ptr for user.
 */
#define GetMemBlockHeader(memptr) (GetAddrByMember(memptr, ptr, MemBlock))
typedef struct MemBlock 
{
    int size;                   /* memblock size + ptr[] size */
    PMemPageInfo memPage;       /* current memPageInfo pointer */
    char ptr[];
}MemBlock, *PMemBlock;

#pragma pack(pop)

int InitializeMemPool();
int DestroyMemPool();

int InitializePoolContext(PMemPoolContextInfo mContext);

MemPtr AllocFromMemPool(unsigned int size);
int ReleaseToMemPool(MemPtr mem);

PMemPoolContextInfo NewPoolMemContext();
int ReleaseMemContext(PMemPoolContextInfo mpContextInfo);

#endif 