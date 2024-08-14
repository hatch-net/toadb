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
#include "locallock.h"
#include "transactionControl.h"
#include "toadsignal.h"


#include <pthread.h>
#include <stdlib.h>

int maxThreadWorkerNum = 12;
int initThreadWorkerNum = 12;
int addThreadWorkerNum = 1;

__thread PResourceOnwerPool resourceOnwerPool = NULL;
__thread PMemContextNode resourceMemContext = NULL;
__thread PMemContextNode WorkerTopMemContext = NULL;

extern PMemContextNode topMemContext;

static void ToadExitSigProc (int signo);

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
    /* release held lock. */
    ReleaseLocalLock();
    MemMangerSwitchContext(topMemContext);
    MemMangerDeleteContext(topMemContext, WorkerTopMemContext);
    return 0;
}

int WorkerMain()
{
    int ret = 0;

    signal(SIGPIPE, SIG_IGN);
    hatSignalSet(SIGINT, ToadExitSigProc);
    hatSignalSet(SIGTERM, ToadExitSigProc);
    hatSignalSet(SIGQUIT, ToadExitSigProc);

    /* this is revert order. */
    atexit(DestoryServer);
    atexit(StopThreadPool);

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

    ret = InitTransactContext(maxThreadWorkerNum);
    if(ret < 0)
    {
        return -1;
    }

    ServerLoop();

    //StopThreadPool();
    //DestoryServer();
    return 0;
}

static void ToadExitSigProc (int signo)
{
    hat_log("receive signal %d", signo);
    switch(signo)
    {
        case SIGINT:
        /* 
         * ctrl+c 
         * this signal will terminal current query to continue process. 
         */
        break;
        case SIGQUIT:
        /* 
         * ctrl + /
         * some error ocur,  server will crash.
        */

        break;
        case SIGTERM:
        /* 
         * kill, 
         * query is cancel and server to exit normal. 
         */
        break;
        default:
        break;
    }
    exit(0);
}