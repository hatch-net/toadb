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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "public.h"


#define DEBUG_MEM_BARRIER 0xBE
#define DEBUG_MEM_TYPE_SIZE (sizeof(int))
#define DEBUG_MEM_BARRIER_SIZE (3 * DEBUG_MEM_TYPE_SIZE)

#define DEBUG_MEM_ADDRESS_ADAPTER(ptr) (((char*)(ptr)) - DEBUG_MEM_BARRIER_SIZE)

#define MM_NewNode(type) (type *)MM_CreateNode(sizeof(type),T_##type)

static PList g_MemList = NULL;
static PMemContextNode g_CurrentContext = NULL;
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

static PNode MM_CreateNode(int size, NodeType type);
static PList MM_AppendNode(PList list, PNode node);

/* For debug */
static void MemMangerShow();
static void MemContextNodeShow(PMemContextNode context);


/* common call */
void *AllocMem(unsigned int size)
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
        hat_log("alloc mem %d failure. \n", size);
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

    hat_debug("User Memory Alloc[%p] size[%d]\n", start, total);
#endif

#ifdef MEM_MANAGER
    MemMangerInsert(start, total);
#endif
    
    /* zero user space */
    memset(pMem, 0x00, size);

    return (void *)pMem;
}

/* common call */
int FreeMem(void *pMem)
{
    if(NULL == pMem)
    {
        return 0;
    }

#if 0
    pMem -= DEBUG_MEM_BARRIER_SIZE;
    memset(pMem+DEBUG_MEM_TYPE_SIZE+DEBUG_MEM_TYPE_SIZE, 0x00, DEBUG_MEM_TYPE_SIZE);
#endif

#ifndef  MEM_MANAGER
    /* while no memory manger */
    free(pMem);
#endif 
    return 0;
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

    hat_debug("Memory Alloc[%p] size[%d]\n", start, total);
#endif
    
    return (void *)pMem;
}

/* momory manager inner call */
static int MM_FreeMem(void *pMem)
{
    if(NULL == pMem)
    {
        return 0;
    }

    hat_debug("Memory free address %p\n", pMem);
    free(pMem);

    return 0;
}

/*
 * Initialize Memory context manager.
 * It is called before used memAlloc. 
 */
void MemMangerInit()
{
    PMemContextNode MainContext = NewMMContext("MainContext");

    g_MemList = MM_AppendNode(g_MemList, (PNode)MainContext);
    g_CurrentContext = MainContext;
}

/*
 *
 */
static PList MM_AppendNode(PList list, PNode node)
{
    PListCell tmpCell = NULL;

    tmpCell = (PListCell)MM_AllocMem(sizeof(ListCell));
    tmpCell->next = NULL;
    tmpCell->value.pValue = node;

    if(NULL == list)
    {
        list = (PList)MM_NewNode(List);
        list->length = 0;
    }

    list = AppendCellNode(list, tmpCell);

    return list;
}

/* 
 * Destory Memory Context Manager.
 * 
 * Here will release all memory which is already allocated. 
 */
void MemMangerDestroy()
{
    PListCell tmpCell = NULL;
    PNode node = NULL;
    char *ptr = NULL;

    /* search all list */
    for(tmpCell = g_MemList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)GetCellNodeValue(tmpCell);
        switch(node->type)
        {
            case T_MemContextNode:
                MemDestroyContextNode((PMemContextNode)node);
            break;
            case T_MemNode:
                MemDestroyMemNode((PMemNode)node);
            break;
            default:
            break;
        }

        MM_FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(tmpCell));
    }

    if(NULL != g_MemList)
        MM_FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(g_MemList));

    if(NULL != g_MemValidCheckData)
        MM_FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(g_MemValidCheckData));
}

/*
 * ptr 实际申请的地址 
 * size 实际申请的大小
*/
static void MemMangerInsert(void *ptr, int size)
{
    PMemNode context = MM_NewNode(MemNode);
    context->memSize = size;
    context->ptr = ptr;
    g_CurrentContext->memList = MM_AppendNode(g_CurrentContext->memList, (PNode)context);

#ifdef MEM_MANAGER_VALID_CHECK    
    MemValidCheck();
#endif 
}

PMemContextNode MemMangerSwitchContext(PMemContextNode oldContext)
{
    PMemContextNode old = g_CurrentContext;
    g_CurrentContext = oldContext;
#ifdef MEM_MANAGER_SHOW    
    MemMangerShow();
#endif     
    return old;
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
    g_CurrentContext->memList = MM_AppendNode(g_CurrentContext->memList, (PNode)MainContext);

    g_CurrentContext = MainContext;

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
    /* 先将context 从memory manager List分离 */
    preContext->memList = DelListNode(preContext->memList, (PNode)delContext);

    /* 释放memory context */
    MemDestroyContextNode(delContext);
}

static PMemContextNode NewMMContext(char *contextName)
{
    PMemContextNode context = MM_NewNode(MemContextNode);

    snprintf(context->contextName,  MEM_CONTEXT_NAME_LEN, contextName);
    context->memList = (PList)NULL;

    return context;
}

static void MemDestroyContextNode(PMemContextNode context)
{
    PListCell tmpCell = NULL;
    PNode node = NULL;

    if(NULL == context)
    {
        return ;
    }

    if(NULL == context->memList)
    {
        MM_FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(context));
        return;
    }

    /* search all list */
    for(tmpCell = context->memList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)GetCellNodeValue(tmpCell);
        switch(node->type)
        {
            case T_MemContextNode:
                MemDestroyContextNode((PMemContextNode)node);
            break;
            case T_MemNode:
                MemDestroyMemNode((PMemNode)node);
            break;
            default:
            break;
        }

        /* release ListCell struct. */
        MM_FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(tmpCell));
    }

    /* release List struct */
    MM_FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(context->memList));

    /* release memory context struct */
    MM_FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(context));
}

static void MemDestroyMemNode(PMemNode node)
{
    if(NULL != node)
    {
        if(NULL != node->ptr)
        {
            /* release MemNode value. */
            MM_FreeMem(node->ptr);
        }

        /* release PMemNode structure. */
        MM_FreeMem(DEBUG_MEM_ADDRESS_ADAPTER(node));;
    }
}

/* 
 * All memory Context sanity checking. 
 */
static void MemValidCheck()
{
    PListCell tmpCell = NULL;
    PNode node = NULL;

    if(NULL == g_MemValidCheckData)
        g_MemValidCheckData = (PMemValidCheckData)MM_AllocMem(sizeof(MemValidCheckData));

    /* search all list */
    for(tmpCell = g_MemList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)GetCellNodeValue(tmpCell);
        switch(node->type)
        {
            case T_MemContextNode:
                MemValidCheckContextNode((PMemContextNode)node);
            break;
            case T_MemNode:
                MemValidCheckMemNode((PMemNode)node);
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
    PListCell tmpCell = NULL;
    PNode node = NULL;
    char *ptr = NULL;

    //hat_debug("Memory context[%s] ,address %p\n", context->contextName, context);

    /* memory context is empty. */
    if(NULL == context->memList)
    {
        return ;
    }

    /* search all list */
    for(tmpCell = context->memList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)GetCellNodeValue(tmpCell);
        switch(node->type)
        {
            case T_MemContextNode:
                MemValidCheckContextNode((PMemContextNode)node);
            break;
            case T_MemNode:
                MemValidCheckMemNode((PMemNode)node);
            break;
            default:
            break;
        }
    }

    /* 检查memory context manager使用的内存 */
    ptr = (char *)context - DEBUG_MEM_BARRIER_SIZE;
    MemValidCheckBlock((ptr), sizeof(MemContextNode) + DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE);
}

static void MemValidCheckMemNode(PMemNode node)
{
    char *ptr = NULL;

    //hat_debug("Memory Node[%p] , value address[%p] size[%d]\n", node, node->ptr, node->memSize);
    MemValidCheckBlock(node->ptr, node->memSize);

    ptr = (char *)node - DEBUG_MEM_BARRIER_SIZE;
    MemValidCheckBlock((ptr), sizeof(MemNode) + DEBUG_MEM_BARRIER_SIZE + DEBUG_MEM_BARRIER_SIZE);
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
        hat_error("Momery check failure, maybe be damaged. retcode[%d] ptr[%p] size[%d]\n", ret, ptr, size);
}

static PNode MM_CreateNode(int size, NodeType type)
{
    PNode node = MM_AllocMem(size);
    if(NULL == node)
    {
        hat_log("list create, not enough memory.\n");
        exit(1);
    }
    hat_debug("CreateNode node:%p size:%d \n", node, size);

    node->type = type;

    return node;
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
    PListCell tmpCell = NULL;
    PNode node = NULL;
    char *ptr = NULL;

    hat_debug1("MemManagerShow Start----------------------------\n");
    /* search all list */
    for(tmpCell = g_MemList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)GetCellNodeValue(tmpCell);
        switch(node->type)
        {
            case T_MemContextNode:
                MemContextNodeShow((PMemContextNode)node);
            break;
            default:
            hat_debug1("MemManagerShow top has memnode. \n");
            break;
        }
    }
    hat_debug1("MemManagerShow End----------------------------\n");
}

static void MemContextNodeShow(PMemContextNode context)
{
    PListCell tmpCell = NULL;
    PNode node = NULL;
    PMemNode memNode = NULL;
    int memCnt = 0;

    if(NULL == context)
    {
        return ;
    }

    hat_debug1("ContextNode Begin Name:%s Type:%d Addr:%p \n", context->contextName, context->type, DEBUG_MEM_ADDRESS_ADAPTER(context));

    if(NULL == context->memList)
    {
        hat_debug1("ContextNode END Name:%s Type:%d Addr:%p \n", context->contextName, context->type, DEBUG_MEM_ADDRESS_ADAPTER(context));
        return;
    }

    /* search all list */
    for(tmpCell = context->memList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)GetCellNodeValue(tmpCell);
        switch(node->type)
        {
            case T_MemContextNode:
                MemContextNodeShow((PMemContextNode)node);
            break;
            case T_MemNode:
                memNode = (PMemNode)node;
                memCnt++;
                hat_debug1("MemNode index:%d Type:%d memSize:%d nodeAddr:%p memAddr:%p \n",
                        memCnt, memNode->type, memNode->memSize, DEBUG_MEM_ADDRESS_ADAPTER(memNode), memNode->ptr);
            break;
            default:
            break;
        }
    }

    hat_debug1("ContextNode END Name:%s Type:%d Addr:%p node:%d \n", 
            context->contextName, context->type, DEBUG_MEM_ADDRESS_ADAPTER(context), context->memList->length);
}

