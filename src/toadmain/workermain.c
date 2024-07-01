/*
 *	toadb workermain 
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

#include "workermain.h"
#include "memStack.h"
#include "public.h"
#include "resourceMgr.h"
#include "threadPool.h"
#include "servprocess.h"

#include <pthread.h>


int maxThreadWorkerNum = 16;
int initThreadWorkerNum = 16;
int addThreadWorkerNum = 1;

__thread PResourceOnwerPool resourceOnwerPool = NULL;
__thread PMemContextNode resourceMemContext = NULL;
__thread PMemContextNode WorkerTopMemContext = NULL;

extern PMemContextNode topMemContext;

/* 
 * function is called in thread loop.
 */
int InitWorker()
{
    PMemContextNode oldContext = NULL;    
    
    /* initialize memory pool context. */
    oldContext = MemMangerNewContext("memoryWorker");

    CreateResourceOwnerPool();

    WorkerTopMemContext = MemMangerSwitchContext(oldContext);
    
    /* end switch to worker memcontext. */
    MemMangerSwitchContext(WorkerTopMemContext);
    return 0;
}

/* 
 * function is called in thread loop.
 */
int ExitWorker() 
{
    MemMangerSwitchContext(topMemContext);
    MemMangerDeleteContext(topMemContext, WorkerTopMemContext);
    return 0;
}

int WorkerMain()
{
    int ret = 0;

    ret = InitializeServer();
    if(ret < 0)
    {
        return -1;
    }

    ret = InitThreadPool(maxThreadWorkerNum, initThreadWorkerNum, addThreadWorkerNum);
    if(ret < 0)
    {
        return -1;
    }

    ServerLoop();

    StopThreadPool();
    DestoryServer();
    return 0;
}

