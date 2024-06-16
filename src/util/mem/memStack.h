/*
 *	toadb memory stack 
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

#ifndef HAT_MEM_STACK_H_H
#define HAT_MEM_STACK_H_H

#include "nodeType.h"
#include "node.h"
#include "list.h"

#define DEBUG_MEM_BARRIER 0xBE
#define DEBUG_MEM_TYPE_SIZE (sizeof(int))
#define DEBUG_MEM_BARRIER_SIZE (3 * DEBUG_MEM_TYPE_SIZE)

#define DEBUG_MEM_ADDRESS_ADAPTER(ptr) (((char*)(ptr)) - DEBUG_MEM_BARRIER_SIZE)

#define AllocMem(size) AllocMemWithPool(size, __FILE__, __LINE__)
#define FreeMem(ptr) FreeMemWithPool(ptr, __FILE__, __LINE__)

//#define AllocMemEx(size) AllocMemExt(size, __FILE__, __LINE__)
//#define FreeMemEx(ptr) FreeMemExt(ptr, __FILE__, __LINE__)

typedef char *MemPage, *MemPtr;

#define MEM_CONTEXT_NAME_LEN (64)
typedef struct MemContextNode
{
    DList memList;  /* parent double linker */
    NodeType type;
    DList subList;  /* children double linker */
    char contextName[MEM_CONTEXT_NAME_LEN];
}MemContextNode, *PMemContextNode;

#define MEM_NODE_SIZE (sizeof(MemNode))
typedef struct MemNode
{
    DList memList;
    NodeType type;
    int memSize;
    char ptr[];
}MemNode, *PMemNode;

typedef struct MemValidCheckData
{
    int pos;
    PList parent;
    PList preCell;
    PList currentCell;
    PList currentContext;
}MemValidCheckData, *PMemValidCheckData;


/* 
 * 当释放时，增加此值，当与pagesize相等时，释放此page到 freelist. 
 * 这只针对标准page，非标page不计数，它只有used 和 unused,释放时直接释放。 
 * 
 *  malloc a memory strip,  | MemPageInfo | MemBlock1 | Memblock2| ... |
 *  user use MemBlock->ptr, and Memblock->mem pointer to Header of MemPageInfo. 
*/
#define MEMPAGE_INFO_LEN (sizeof(MemPageInfo))
typedef struct MemPageInfo
{
    DList list;
    MemPage memHead;        /* pointer to current mempoolcontext */
    int memPageSize;
    int useOffset;
    int freeSize;  
    int releaseSize;         
}MemPageInfo, *PMemPageInfo;

#define GetMemPageFreeSpaceSize(mpage) ((mpage)->memPageSize - (mpage)->useOffset)
#define IsMemPageEmpty(mpage) ((mpage)->releaseSize == (mpage)->memPageSize)

typedef struct MemPageListInfo
{
    DList list;                 /* memPageInfo list */
    unsigned int memTotalSize;        
    PMemPageInfo currUsePage;
}MemPageListInfo, *PMemPageListInfo;


typedef struct MemPoolContextInfo
{   
    MemContextNode contextNode;
    MemPageListInfo memPageNode;
}MemPoolContextInfo, *PMemPoolContextInfo;
#define MEMPOOL_CONTEXTINFO_SIZE (sizeof(MemPoolContextInfo))

void MemMangerInit();
void MemMangerDestroy();

void *AllocMemWithPool(unsigned int size, char *file, int line);
int FreeMemWithPool(void *pMem, char *file, int line);

void *AllocMemExt(unsigned int size, char *file, int line);
int FreeMemExt(void *pMem, char *file, int line);

PMemContextNode MemMangerSwitchContext(PMemContextNode oldContext);
PMemContextNode MemMangerNewContext(char *contextName);
void MemMangerDeleteContext(PMemContextNode preContext, PMemContextNode delContext);
PMemContextNode MemMangerGetCurrentContext();
void MemMangerSetCurrentContext(PMemContextNode context);

PNode MM_CreateNode(int size, NodeType type);


#endif 