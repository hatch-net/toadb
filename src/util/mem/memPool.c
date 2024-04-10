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

/* 
 * 
 */

#include "memPool.h"
#include "public.h"
#include "memStack.h"

#include <string.h>



/* 内存上下文管理 */
static PMemPoolManagerContext g_memPoolMangerContext = NULL;


/* inner call */
static void *MM_AllocMem(unsigned int size);
static int MM_FreeMem(void *pMem);

static PMemPoolContextInfo GetMemPoolCurrentContext();

static int AddPoolFreeList(PMemPageInfo mPage);
static MemPage GetFreeMemPage();
static MemPage NewMemPage(int size);

/* memory page */
static PMemPageInfo InitMemPage(MemPage newPage, int size);
static int ReleaseMemPage(PMemPageInfo mPage);
static MemPtr AllocFromMemPage(PMemPageInfo mPage, int size);
static int ReleaseToMemPage(PMemBlock memb);

static MemPtr AllocFromMemContext(int size, PMemPoolContextInfo mpContextInfo);

static PDList AddMemPageNode(PDList list, PMemPageInfo node);
static int AddMemPageToContext(PMemPageInfo memPage, PMemPoolContextInfo context, int isSwitchCurrent);

static int AddNewPageToContext(PMemPoolContextInfo poolContext);

int InitializeMemPool()
{
    g_memPoolMangerContext = MM_AllocMem(sizeof(MemPoolManagerContext));
    g_memPoolMangerContext->version = MEMORY_POOL_MANAGER_VERSION;
    g_memPoolMangerContext->totalSize = 0;
    INIT_DLIST_NODE(g_memPoolMangerContext->memFreeList);

    return 0;
}

int DestroyMemPool()
{
    PDList tmpNode = NULL;
    PDList headNode = &(g_memPoolMangerContext->memFreeList);
    int end = 1;

    if(NULL == g_memPoolMangerContext)
        return 0;
    
    /* travel free list to release to os, which reverse travel. */
    do {
        tmpNode = g_memPoolMangerContext->memFreeList.prev;
        
        if(tmpNode->prev == headNode)
            end = 0;

        DelDListNode(&headNode, tmpNode);
        MM_FreeMem(tmpNode);
    }while(end > 0);        
 
    g_memPoolMangerContext = NULL;
}

/* 
 * 从空闲列表中获取内存块
 */
static MemPage GetFreeMemPage()
{
    PDList headNode = NULL;
    PMemPageInfo mPage = NULL;

    if(NULL == g_memPoolMangerContext)
    {
        return NULL;
    }

    if(g_memPoolMangerContext->totalSize <= 0)
        return 0;

    headNode = &(g_memPoolMangerContext->memFreeList);
    mPage = (PMemPageInfo)PopDListTailNode(&headNode);

    g_memPoolMangerContext->totalSize -= mPage->memPageSize;
    return (MemPage)mPage;
}

/*
 * 将空闲空间添加到空闲列表中
 */
static int AddPoolFreeList(PMemPageInfo mPage)
{
    PDList headNode = NULL;
    if(NULL == g_memPoolMangerContext)
    {
        MM_FreeMem(mPage);
        return -1;
    }

    headNode = &(g_memPoolMangerContext->memFreeList);
    AddDListTail(&headNode, (PDList)mPage);

    g_memPoolMangerContext->totalSize += mPage->memPageSize;

    hat_debug1("memory pool free size:%d \n", g_memPoolMangerContext->totalSize);
    return 0;
}

static MemPage NewMemPage(int size)
{
    MemPage mpage = NULL;
    
    mpage = MM_AllocMem(size);
    
    return mpage;
}


/* 
 * alloc from memory manager pool.
 * steps follow:
 * oversize , alloc and add to context;
 * alloc from context free space;
 * find space page from free list, and add to context, then do pre step2.
 * new mempage,and add to context, then do pre step2.
 * 
 */
MemPtr AllocFromMemPool(unsigned int size)
{
    MemPtr mem = NULL;
    PMemPoolContextInfo currentMemContext = NULL;
    PMemPageInfo currentMemPage = NULL;
    MemPage newPage = NULL;
    unsigned int align_size = 0;

    currentMemContext = GetMemPoolCurrentContext();
    if(NULL == currentMemContext)
    {
        hat_error("current memory context is NULL.\n");
        return NULL;
    }

    /* add memblock header size */
    size += MEMORY_POOL_BLOCK_HEADER_SIZE;

    /* oversize */
    if(size >= MEMORY_POOL_MIN_ALLOC_SIZE)
    {
        align_size = (size + MEMPAGE_INFO_LEN + ~MEMORY_POOL_PAGE_ALIG - 1) & MEMORY_POOL_PAGE_ALIG;
        newPage = NewMemPage(align_size);

        currentMemPage = InitMemPage(newPage, align_size);
        AddMemPageToContext(currentMemPage, currentMemContext, 0); 

        mem = AllocFromMemPage(currentMemPage, size);
        return mem;
    }

    do {
        mem = AllocFromMemContext(size, currentMemContext);
        if(NULL != mem)
            break;

        AddNewPageToContext(currentMemContext);     
    }while(1);

    return mem;
}

static int AddNewPageToContext(PMemPoolContextInfo poolContext)
{
    PMemPageInfo currentMemPage = NULL;
    MemPage newPage = NULL;

    newPage = GetFreeMemPage();
    if(NULL == newPage)
    {
        newPage = NewMemPage(MEMORY_POOL_PAGE_SIZE);
    }        

    currentMemPage = InitMemPage(newPage, MEMORY_POOL_PAGE_SIZE);
        
    /* add to context, and continue to alloc from context. */
    AddMemPageToContext(currentMemPage, poolContext, 1);      

    return 0;
}

/*
 * first create a Context, as follow step 
 * get a memory page;
 * alloc context structure;
 * add page to this new context;
 */
PMemPoolContextInfo NewPoolMemContext()
{
    PMemPoolContextInfo mContext = NULL;
    PMemPageInfo currentMemPage = NULL;
    MemPage newPage = NULL;

    newPage = GetFreeMemPage();
    if(NULL == newPage)
    {
        newPage = NewMemPage(MEMORY_POOL_PAGE_SIZE);
    } 

    currentMemPage = InitMemPage(newPage, MEMORY_POOL_PAGE_SIZE);

    mContext = (PMemPoolContextInfo)AllocFromMemPage(currentMemPage, MEMPOOL_CONTEXTINFO_SIZE);
    InitializePoolContext(mContext);

    AddMemPageToContext(currentMemPage, mContext, 1);   
    return mContext;
}

int InitializePoolContext(PMemPoolContextInfo mContext)
{
    PMemPageListInfo mPageList = &(mContext->memPageNode);

    INIT_DLIST_NODE(mPageList->list);
    mPageList->memTotalSize = 0;
    mPageList->currUsePage = NULL;

    return 0;
}

int ReleaseToMemPool(MemPtr mem)
{
    return ReleaseToMemPage(GetMemBlockHeader(mem));
}


static int AddMemPageToContext(PMemPageInfo memPage, PMemPoolContextInfo context, int isSwitchCurrent)
{
    PMemPageListInfo memPageList = &(context->memPageNode);

    memPageList->memTotalSize += memPage->memPageSize;       
    
    /* add to mempage list */
    AddMemPageNode(&(memPageList->list), memPage);
    if(isSwitchCurrent)
    {
        memPageList->currUsePage = memPage;
    }

    memPage->memHead = (MemPage)memPageList;
    return 0;
}

/*
 * 
 */
static MemPtr AllocFromMemContext(int size, PMemPoolContextInfo mpContextInfo)
{
    PMemPageInfo currentMemPage = NULL;
    MemPtr mem = NULL;

    if((size <= 0) || (NULL == mpContextInfo))
        return NULL;

    /* first checking free space have enough. */
    currentMemPage = mpContextInfo->memPageNode.currUsePage;
    if(size <= GetMemPageFreeSpaceSize(currentMemPage))
    {
        mem = AllocFromMemPage(currentMemPage, size);
    }

    return mem;
}

/* 
 * every context sublist has one node at least, 
 * which is storaged current context data.
 */
int ReleaseMemContext(PMemPoolContextInfo mpContextInfo)
{
    PMemPageListInfo mPageList = &(mpContextInfo->memPageNode);
    PMemPageInfo mPage = NULL;    

    PDList tailNode = mPageList->list.prev;
    PDList preNode = tailNode;
    PDList headNode = &(mPageList->list);
    PDList owner = headNode->next;

    while((preNode != NULL) && (preNode != owner))
    {
        tailNode = preNode;
        preNode = preNode->prev;
        
        DelDListNode(&headNode, tailNode);
        ReleaseMemPage((PMemPageInfo)tailNode);
    }

    /* 
     * headNode which storage memory is the first memory page. 
     */
    ReleaseMemPage((PMemPageInfo)owner);
    return 0;
}

/* 
 * initialize new mempage, we will reset to zero at the first.
 */
static PMemPageInfo InitMemPage(MemPage newPage, int size)
{
    PMemPageInfo pageInfo = NULL;
    
    memset(newPage, 0x00, size);

    pageInfo = (PMemPageInfo)newPage;
    pageInfo->memHead = NULL;
    pageInfo->memPageSize = size;
    pageInfo->useOffset = MEMPAGE_INFO_LEN;
    pageInfo->freeSize = size - MEMPAGE_INFO_LEN;
    pageInfo->releaseSize = MEMPAGE_INFO_LEN;
    
    INIT_DLIST_NODE(pageInfo->list);

    return pageInfo;
}

static PDList AddMemPageNode(PDList list, PMemPageInfo node)
{
    PDList newHead = NULL;

    AddDListTail(&list, (PDList)node);
    return list;
}

static MemPtr AllocFromMemPage(PMemPageInfo mPage, int size)
{
    PMemBlock memb = NULL;

    mPage->freeSize -= size;
    memb = (PMemBlock)((char *)mPage + mPage->useOffset);
    mPage->useOffset += size;

    memb->memPage = mPage;
    memb->size = size;

    hat_debug("alloc memblock %p, mem %p, memPage %p \n", memb, memb->ptr, memb->memPage);
    return (MemPtr)(memb->ptr);
}

static int ReleaseMemPage(PMemPageInfo mPage)
{
    if(mPage->memPageSize > MEMORY_POOL_FREE_PAGE_SIZE)
    {
        MM_FreeMem(mPage);
        return 1;
    }

    /* add to free list */
    AddPoolFreeList(mPage);
    return 0;
}

static int ReleaseToMemPage(PMemBlock memb)
{
    PMemPageListInfo memPageList = NULL;
    PMemPageInfo memPage = memb->memPage;
    DList *header = NULL;
    int ret = 0;

    hat_debug("release memblock %p, mem %p, memPage %p \n", memb, memb->ptr, memb->memPage);

    memPage->releaseSize += memb->size;

    if(IsMemPageEmpty(memPage))
    {
        /* release mempage to free list */
        memPageList = (PMemPageListInfo)memPage->memHead;
        header = &(memPageList->list);
        DelDListNode(&header, &(memPage->list));

        ret = ReleaseMemPage(memPage);
    }
    return ret;
}

static PMemPoolContextInfo GetMemPoolCurrentContext()
{
    PMemPoolContextInfo memPoolContext = NULL;

    memPoolContext = (PMemPoolContextInfo)MemMangerGetCurrentContext();

    return memPoolContext;
}


/* memory manager inner call */
static void *MM_AllocMem(unsigned int size)
{
    char *pMem = NULL;

#ifdef MEM_DEBUG
    char *start = NULL;
    int total = size;
    /* prev and after */
    size += DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE;
#endif

    pMem = (char *)malloc(size);
    if(NULL == pMem)
    {
        hat_log("alloc mem %d failure. \n", size);
        exit(-1);
    }

#ifdef MEM_DEBUG
    start = pMem;
    total = size;

    /* prev */
    memset(pMem, DEBUG_MEM_BARRIER, DEBUG_MEM_TYPE_SIZE);
    pMem += DEBUG_MEM_TYPE_SIZE;

    *((unsigned int*)pMem)= size;
    pMem += DEBUG_MEM_TYPE_SIZE;

    memset(pMem, DEBUG_MEM_BARRIER, DEBUG_MEM_TYPE_SIZE);
    pMem += DEBUG_MEM_TYPE_SIZE;


    /* after */
    size -= DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE;
    memcpy(pMem+size, start, DEBUG_MEM_BARRIER_SIZE);
    
    /* zero user space */
    memset(pMem, 0x00, size);

    hat_debug("Memory manager Alloc[%p] size[%d]\n", start, total);
#endif
    
    return (void *)pMem;
}

/* momory manager inner call */
static int MM_FreeMem(void *pMem)
{
    char *freeMem = NULL;
    int size = 0;

    if(NULL == pMem)
    {
        return 0;
    }

#ifdef  MEM_DEBUG
    freeMem = pMem;
    freeMem -= DEBUG_MEM_BARRIER_SIZE;
    size = *((unsigned int*)(freeMem+DEBUG_MEM_TYPE_SIZE));
    hat_debug("User Memory free[%p] size[%d]\n", pMem, size);

    pMem = freeMem;
#endif 

    hat_debug("Memory manager free address %p\n", pMem);
    free(pMem);

    return 0;
}