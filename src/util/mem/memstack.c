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
#include "memStack.h"

/* PList */
#include "node.h"       
#include "config_pub.h"

#include "public.h"
#include "public_types.h"
#include "memPool.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define hat_debug_memstack(...) 

/* 
 * 内存上下文的列表，总的内存上下文由g_MemList记录，
 * 每个列表节点为一个内存上下文；
 * 
 * 当前正在使用的内存上下文由g_CurrentContext记录；
 * 
 * g_MemValidCheckData是记录内存合法性检查的中间数据；
 */
static DList g_MemList = {0};
static PDList g_headMemList = &g_MemList;

static ThreadLocal PMemContextNode g_CurrentContext = NULL;
static PMemValidCheckData g_MemValidCheckData = NULL;

static PMemContextNode NewMMContext(char *contextName);

static void MemDestroyContextNode(PMemContextNode node);
static void MemDestroyMemNode(PMemNode node);

static void MemMangerInsert(void *ptr, int size);

static void MemValidCheck();
static void MemValidCheckContextNode(PMemContextNode node);
static void MemValidCheckMemNode(PMemNode node);
static void MemValidCheckBlock(char *ptr, int size);
static int MemCheckFlag(char *ptr, int size);


static void InitMMContextNode(PMemContextNode context, char *contextName);

/* For debug */
static void MemMangerShow();
static void MemContextNodeShow(PMemContextNode context);

void *AllocMemWithPool(unsigned int size, char *file, int line)
{
    char *pMem = NULL;
    int total = size;
    PMemNode start = NULL;

    /* alloc memNode struct and user size. */
    //total += MEM_NODE_SIZE;
    pMem = (char *)AllocFromMemPool(total);
    if(NULL == pMem)
    {
        hat_log("alloc mem %d from pool failure. ", size);
        exit(-1);
    }

    /* not used MemNode wrapper 2024/4/9 */
    //start = (PMemNode)pMem;

#ifdef MEM_DEBUG
    /* prev and after */
    total += DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE;
#endif

#ifdef MEM_MANAGER
    /* memnode is not inserted to context list. */
    // MemMangerInsert(start, total);
#endif
    
    /* zero user space */
    //pMem = start->ptr;
    /* memory strip is already reset by the context. */
    // memset(pMem, 0x00, size);

    hat_debug("alloc mem %p, pos[%s:%d] ", pMem, file, line);
    return (void *)pMem;
}

int FreeMemWithPool(void *pMem, char *file, int line)
{
    if(NULL == pMem)
        return -1;
    hat_debug("relase mem %p, pos[%s:%d] ", pMem, file, line);
    return ReleaseToMemPool(pMem);
}

/* common call */
void *AllocMemExt(unsigned int size, char *file, int line)
{
    char *pMem = NULL;
    int total = size;
    char *start = NULL;

#ifdef MEM_DEBUG
    /* prev and after */
    size += DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE;
#endif

    pMem = (char *)malloc(size);
    if(NULL == pMem)
    {
        hat_log("alloc mem %d failure. ", size);
        exit(-1);
    }

    start = pMem;
    total = size;

#ifdef MEM_DEBUG
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

    hat_debug("User Memory Alloc[%p] size[%d] [%s][%d]", start, total, file, line);
#endif

#ifdef MEM_MANAGER
    MemMangerInsert(start, total);
#endif
    
    /* zero user space */
    memset(pMem, 0x00, size);

    return (void *)pMem;
}

/* common call */
int FreeMemExt(void *pMem, char *file, int line)
{
    unsigned int size = 0;

    if(NULL == pMem)
    {
        return 0;
    }

#ifndef  MEM_MANAGER
    pMem += DEBUG_MEM_TYPE_SIZE;
    size = *((unsigned int*)(pMem+DEBUG_MEM_TYPE_SIZE));
    hat_debug("User Memory free[%p] size[%d] [%s][%d]", pMem, size, file, line);

    /* while no memory manger */
    free(pMem);
#endif 
    return 0;
}

/*
 * Initialize Memory context manager.
 * It is called before used memAlloc. 
 */
void MemMangerInit()
{
    PMemContextNode MainContext = NULL; 
    INIT_DLIST_NODE(g_MemList);

    /* initialize memPool first */
    InitializeMemPool();

    MainContext = NewMMContext("MainContext");
    
    /* first context must be initialize at the beginning before using. */
    g_CurrentContext = MainContext;
    AddDListTail(&g_headMemList, (PDList)MainContext);

#ifdef MEM_MANAGER_VALID_CHECK
    g_MemValidCheckData = (PMemValidCheckData)AllocMem(sizeof(MemValidCheckData));
#endif 
}


/* 
 * Destory Memory Context Manager.
 * 
 * Here will release all memory which is already allocated. 
 */
void MemMangerDestroy()
{
    PDList tailNode = g_headMemList->prev;
    PDList headNode = g_headMemList;
    PMemContextNode pCxt = NULL;
    PDList preNode = NULL;

    preNode = tailNode;

    while((preNode != NULL) && (preNode != headNode))
    {
        tailNode = preNode;
        preNode = preNode->prev;

        pCxt = (PMemContextNode)tailNode;
        if(T_MemContextNode == pCxt->type)
        {
            DelDListNode(&headNode, tailNode);
            MemDestroyContextNode((PMemContextNode)pCxt);
        }
    }

    INIT_DLIST_NODE(g_MemList);

    if(NULL != g_MemValidCheckData)
        FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(g_MemValidCheckData));

    DestroyMemPool();
}

/*
 * ptr 实际申请的地址 
 * size 实际申请的大小
*/
static void MemMangerInsert(void *ptr, int size)
{
    PMemNode mNode = (PMemNode)ptr;
    PDList head = &(g_CurrentContext->subList);
    
    mNode->memSize = size;
    mNode->type = T_MemNode;
    INIT_DLIST_NODE(mNode->memList);

    AddDListTail(&head, (PDList)&(mNode->memList));
    
#ifdef MEM_MANAGER_VALID_CHECK    
    MemValidCheck();
#endif 
}

PMemContextNode MemMangerSwitchContext(PMemContextNode oldContext)
{
    PMemContextNode old = g_CurrentContext;
    g_CurrentContext = oldContext;

    hat_debug("currentContext:%p.", g_CurrentContext);
#ifdef MEM_MANAGER_SHOW    
    MemMangerShow();
#endif     
    return old;
}

PMemContextNode MemMangerGetCurrentContext()
{
    return g_CurrentContext;
}

void MemMangerSetCurrentContext(PMemContextNode context)
{
    if(NULL == context)
        return;
        
    g_CurrentContext = context;
}

/*
 * New memory context, and switch to this new context. 
 * return old memory context. 
 * Context address that is used address， real address process by DEBUG_MEM_ADDRESS_ADAPTER.
 */
PMemContextNode MemMangerNewContext(char *contextName)
{
    PMemContextNode old = g_CurrentContext;
    PMemContextNode MainContext = NewMMContext(contextName);
    PDList head = &(g_CurrentContext->subList);

    hat_debug_memstack("1currentContext:%p.", g_CurrentContext);

    /* MainContext->memList link to parent. */
    AddDListTail(&head, (PDList)MainContext);

    g_CurrentContext = MainContext;
    hat_debug_memstack("2currentContext:%p.", g_CurrentContext);
#ifdef MEM_MANAGER_VALID_CHECK    
    MemValidCheck();
#endif 

#ifdef MEM_MANAGER_SHOW
    MemMangerShow();
#endif 
    return old;
}

void MemMangerDeleteContext(PMemContextNode preContext, PMemContextNode delContext)
{
    PDList head = &(preContext->subList);

    /* 先将context 从memory manager List分离 */
    DelDListNode(&head, (PDList)delContext);

    /* 释放memory context */
    MemDestroyContextNode(delContext);
}

static PMemContextNode NewMMContext(char *contextName)
{
    PMemPoolContextInfo memPoolContext = NULL;
    PMemContextNode context = NULL;
    
    /* first get a memory context. */
    memPoolContext = NewPoolMemContext();

    context = &(memPoolContext->contextNode);
    InitMMContextNode(context, contextName);

    return context;
}

static void InitMMContextNode(PMemContextNode context, char *contextName)
{
    snprintf(context->contextName,  MEM_CONTEXT_NAME_LEN, contextName);
    context->type = T_MemContextNode;
    INIT_DLIST_NODE(context->memList);
    context->subList.prev = context->subList.next = &(context->subList);
}


static void MemDestroyContextNode(PMemContextNode context)
{
    PDList tailNode = NULL;
    PDList preNode = NULL;
    PDList headNode = NULL;
    PMemContextNode pCxt = NULL;

    if(NULL == context)
    {
        return ;
    }

    tailNode = context->subList.prev;
    headNode = &(context->subList);
    preNode = tailNode;

    while((preNode != NULL) && (preNode != headNode))
    {
        tailNode = preNode;
        preNode = preNode->prev;

        /* search all list */
        pCxt = (PMemContextNode)tailNode;
        switch(pCxt->type)
        {
            case T_MemContextNode:
                MemDestroyContextNode(pCxt);
            break;
            case T_MemNode:
            break;
            default:
            hat_error("destroyContext %s addr:%p node type:%d node addr:%p ", 
                        context->contextName, context, pCxt->type, pCxt);
            break;
        }
    }

    hat_debug_memstack("destroyContext %s addr:%p ", context->contextName, context);

    ReleaseMemContext((PMemPoolContextInfo)context);
}

/* It isnot used when memory pool enable. */
static void MemDestroyMemNode(PMemNode node)
{
    if(NULL != node)
    {
        if(NULL != node->ptr)
        {
            /* release MemNode value. */
            FreeMem(node->ptr);
        }

        /* release PMemNode structure. */
        FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(node));;
    }
}

/* 
 * All memory Context sanity checking. 
 */
static void MemValidCheck()
{
    PDList tailNode = NULL;
    PDList preNode = NULL;
    PDList headNode = NULL;
    PMemContextNode pCxt = NULL;

    /* not initialized */
    if(NULL == g_MemValidCheckData)
        return;

    tailNode = g_headMemList->prev;
    headNode = g_headMemList;
    preNode = tailNode;

    while((preNode != NULL) && (preNode != headNode))
    {
        tailNode = preNode;
        preNode = preNode->prev;

        /* search all list */
        pCxt = (PMemContextNode)tailNode;
        switch(pCxt->type)
        {
            case T_MemContextNode:
                MemValidCheckContextNode(pCxt);
            break;
            case T_MemNode:
                MemValidCheckMemNode((PMemNode)pCxt);
            break;
            default:
            break;
        }
    }
}

/* 
 * one memory Context sanity checking. 
 */
static void MemValidCheckContextNode(PMemContextNode context)
{
    PDList tailNode = NULL;
    PDList preNode = NULL;
    PDList headNode = NULL;
    PMemContextNode pCxt = NULL;
    char *ptr = NULL;

    if(NULL == context)
    {
        return ;
    }

    tailNode = context->subList.prev;
    headNode = &(context->subList);
    preNode = tailNode;


    while((preNode != NULL) && (preNode != headNode))
    {
        tailNode = preNode;
        preNode = preNode->prev;

        /* search all list */
        pCxt = (PMemContextNode)tailNode;
        switch(pCxt->type)
        {
            case T_MemContextNode:
                MemValidCheckContextNode(pCxt);
            break;
            case T_MemNode:
                MemValidCheckMemNode((PMemNode)pCxt);
            break;
            default:
            break;
        }
    }

    hat_debug_memstack("Memory context[%s] ,address %p", context->contextName, context);
}

static void MemValidCheckMemNode(PMemNode node)
{
    char *ptr = NULL;

    hat_debug_memstack("Memory Node[%p] , value address[%p] size[%d]", node, node->ptr, node->memSize);

    // ptr = (char *)node;
    // MemValidCheckBlock((ptr), node->memSize);
}

static void MemValidCheckBlock(char *ptr, int size)
{
    int index = 0;
    int ret = 0;

    do 
    {
        if(NULL == ptr)
        {
            ret = -4;
            break;
        }

        if(size <= (DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE))
        {
            ret = -1;
            break;
        }

        /* pre head is specified DEBUG_MEM_BARRIER , which size of DEBUG_MEM_BARRIER_SIZE. */
        if((ret = MemCheckFlag(ptr, size)) != 0)
        {
            break;
        }
;

        /* after head is specified DEBUG_MEM_BARRIER , which size of DEBUG_MEM_BARRIER_SIZE. */
        ptr += size - DEBUG_MEM_BARRIER_SIZE;
        if((ret = MemCheckFlag(ptr, size)) != 0)
        {
            break;
        }
    }while(0);

    if(ret != 0)
        hat_error("Momery check failure, maybe be damaged. retcode[%d] ptr[%p] size[%d]", ret, ptr, size);
}


static int MemCheckFlag(char *ptr, int size)
{
    int index = 0;
    int ret = 0;

    do 
    {
        /* pre head is specified DEBUG_MEM_BARRIER , which size of DEBUG_MEM_TYPE_SIZE. */
        for(index = 0; index < DEBUG_MEM_TYPE_SIZE; index ++)
        {
            if(ptr[index] != (char)DEBUG_MEM_BARRIER)
            {
                ret = -2;
                break;
            }
        }
        if(ret != 0)
            break;

        /* middle is size */
        ptr += DEBUG_MEM_TYPE_SIZE;
        index = *((int*)(ptr));
        if(size != index)
        {
            hat_log("size not equal record size %d, real size: %d", size, index);
            ret = -5;
            break;
        }

        /* after head is specified DEBUG_MEM_BARRIER , which size of DEBUG_MEM_TYPE_SIZE. */
        ptr += DEBUG_MEM_TYPE_SIZE;
        for(index = 0; index < DEBUG_MEM_TYPE_SIZE; index ++)
        {
            if(ptr[index] != (char)DEBUG_MEM_BARRIER)
            {
                ret = -3;
                break;
            }
        }
    }while(0);

    return ret;
}


/* 
 * Show Memory Context .
 * 
 * only for debug.
 */
static void MemMangerShow()
{
    PDList tailNode = NULL;
    PDList preNode = NULL;
    PDList headNode = NULL;
    PMemContextNode pCxt = NULL;

    tailNode = g_headMemList->prev;
    headNode = g_headMemList;
    preNode = tailNode;
        
    hat_debug1("MemManagerShow Start----------------------------");

    while((preNode != NULL) && (preNode != headNode))
    {
        tailNode = preNode;
        preNode = preNode->prev;

        /* search all list */
        pCxt = (PMemContextNode)tailNode;
        switch(pCxt->type)
        {
            case T_MemContextNode:
                MemContextNodeShow(pCxt);
            break;
            default:
            hat_debug1("MemManagerShow top has memnode. ");
            break;
        }
    }

    hat_debug1("MemManagerShow End----------------------------");
}

static void MemContextNodeShow(PMemContextNode context)
{
    PDList tailNode = NULL;
    PDList preNode = NULL;
    PDList headNode = NULL;
    PMemContextNode pCxt = NULL;
    PMemNode memNode = NULL;
    int memCnt = 0;

    if(NULL == context)
    {
        return ;
    }

    tailNode = context->subList.prev;
    headNode = &(context->subList);
    preNode = tailNode;
        
    hat_debug1("ContextNode Begin Name:%s Type:%d Addr:%p ", context->contextName, context->type, context);

    while((preNode != NULL) && (preNode != headNode))
    {
        tailNode = preNode;
        preNode = preNode->prev;

        /* search all list */
        pCxt = (PMemContextNode)tailNode;
        switch(pCxt->type)
        {
            case T_MemContextNode:
                MemContextNodeShow(pCxt);
            break;
            case T_MemNode:
                memNode = (PMemNode)pCxt;
                memCnt++;
                hat_debug1("MemNode index:%d Type:%d memSize:%d  memAddr:%p ",
                        memCnt, memNode->type, memNode->memSize, memNode);
            break;
            default:
                hat_debug1("unknown node index:%d Type:%d  memAddr:%p ",
                        memCnt, pCxt->type,  pCxt);
            break;
        }
    }

    hat_debug1("ContextNode END Name:%s Type:%d Addr:%p  ", 
            context->contextName, context->type, context);
}


