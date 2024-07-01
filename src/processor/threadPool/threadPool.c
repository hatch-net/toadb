/*
 *	toadb threadPool 
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

#include "threadPool.h"
#include "public.h"
#include "server_pub.h"
#include "shareMem.h"
#include "workermain.h"
#include "public_types.h"


#include <stdio.h>
#include <pthread.h>
#include <errno.h>

#define hat_debug1_threadpool(...)   log_report(LOG_DEBUG, __VA_ARGS__) 

PThreadPoolInfo ProcessWorkerThreadInfo = NULL;
static ThreadLocal PThreadWorkerInfo workerContextInfo = NULL;

static int BeginWorkerThread(PThreadWorkerInfo workerInfo);
static int FinishWorkerThread(PThreadWorkerInfo workerInfo);

static void* threadEntry(void *arg);

int InitThreadPool(int max, int init, int add)
{
    int maxSize = max * sizeof(PThreadWorkerInfo);
    int index = 0;

    if((max <= 0) || (init <= 0) || (add < 0))
    {
        return -1;
    }

    /* switch to top memcontext. */
    MemMangerSwitchContext(topMemContext);

    ProcessWorkerThreadInfo = (PThreadPoolInfo)SMemAlloc(sizeof(ThreadPoolInfo) + maxSize);
    ProcessWorkerThreadInfo->threadExteNum = add;
    ProcessWorkerThreadInfo->threadInitNum = init;
    ProcessWorkerThreadInfo->threadMaxNum = max;
    ProcessWorkerThreadInfo->taskNextId = 0;

    hat_debug1_threadpool("init threadpool  starting.");
    for(index = 0; index < init; index++)
    {
        /* TODO: initialize work to init number. */
        ProcessWorkerThreadInfo->workerInfoList[index] = CreeateWorkerThread(index);
        if(NULL == ProcessWorkerThreadInfo->workerInfoList[index])
            return -1;
    }

    ProcessWorkerThreadInfo->threadListNum = init;
    ProcessWorkerThreadInfo->idleWorkerNum = init;

    hat_log("InitThreadPool succeed. ");
    return 0;
}

#define WORK_NAME_LEN 32
PThreadWorkerInfo CreeateWorkerThread(int index)
{
    int ret = 0;
    char workerName[WORK_NAME_LEN];
    PThreadWorkerInfo worker = (PThreadWorkerInfo)SMemAlloc(sizeof(ThreadWorkerInfo));
    pthread_t threadId;
    PMemContextNode oldContext;

    worker->tw_id = index + 1;
    worker->taskEntry = NULL;
    
    snprintf(workerName, WORK_NAME_LEN, "worker-%d",index);

    hat_debug1_threadpool("init thread %s starting.", workerName);

    /* initialize memory pool context. */
    oldContext = MemMangerNewContext(workerName);
    worker->ThreadMemoryContext = MemMangerSwitchContext(oldContext);

    if(NULL == InitializeSem(workerName, 0, &worker->taskIdleLock))
    {
        return NULL;
    }

    hat_debug1_threadpool("create2 thread %s starting.", workerName);
    ret = pthread_create(&threadId, NULL, threadEntry, (void *)worker);
    if (ret != 0) 
    {
        return NULL;
    }
    worker->tw_threadid = (unsigned int)threadId;
    worker->tw_state = TW_IDLE;

    hat_debug1_threadpool("create thread %s succeed.", workerName);
    return worker;
}

int DestoryWorkerThread(PThreadWorkerInfo workerInfo)
{
    int* ret = 0;

    if(NULL == workerInfo)
        return 0;
    
    if(workerInfo->tw_threadid > 0)
    {
        pthread_join((pthread_t)&workerInfo->tw_threadid, (void **)&ret);
        hat_log("thread %u exit (%d) ", workerInfo->tw_id, *ret);
    }

    DestorySem(workerInfo->taskIdleLock.name, &workerInfo->taskIdleLock);

    return 0;
}

int StopThreadPool()
{
    int index = 0;
    if(NULL == ProcessWorkerThreadInfo)
        return 0;

    for(; index < ProcessWorkerThreadInfo->threadListNum - 1; index ++)
    {
        ShutDownWorkerThread(ProcessWorkerThreadInfo->workerInfoList[index]);

        DestoryWorkerThread(ProcessWorkerThreadInfo->workerInfoList[index]);
    }

    SMemFree(ProcessWorkerThreadInfo);
    ProcessWorkerThreadInfo = NULL;
    return 0;
}

int ShutDownWorkerThread(PThreadWorkerInfo workerInfo)
{
    int ret = 0;

    if(NULL == workerInfo)
        return -1;
    
    hat_log("worker-%d shutdown... ", workerInfo->tw_id);
    workerInfo->tw_id = -1;
    
    return 0;
}

int ShutDownWorkerThreadImmidately(PThreadWorkerInfo workerInfo)
{    
    int ret = 0;

    if(NULL == workerInfo)
        return -1;
    
    ret = pthread_cancel(workerInfo->tw_threadid);
    if (ret != 0)
        hat_error("worker-%d shutdown failure.", workerInfo->tw_id);
    hat_log("worker-%d shutdown immediately. ", workerInfo->tw_id);
    return 0;
}

int IsIdleWorker(PThreadWorkerInfo worker)
{
    hat_debug1_threadpool("IsIdleWorker threadid:%u - workerid:%d - state:%d ", worker->tw_threadid, worker->tw_id, worker->tw_state);

    if(TW_IDLE == worker->tw_state)
    {
        return 1;
    }

    return 0;
}

int GetIdleWorker()
{
    int index = ProcessWorkerThreadInfo->threadListNum - 1;

    for(; index >= 0; index --)
    {
        if(IsIdleWorker(ProcessWorkerThreadInfo->workerInfoList[index]))
            return index;
    }

    /* TODO: not found, exlarge workerPool. */

    return -1;
}

int PushTask(int clientfd, TaskProcess taskProc)
{
    int workerIndex = 0;
    PThreadWorkerInfo idleWorker = NULL;
    PMemContextNode oldContext = NULL;    
    
    workerIndex = GetIdleWorker();
    if(workerIndex < 0)
    {
        hat_error("ThreadWorkers are not in idle state.");
        return -1;
    }

    idleWorker = ProcessWorkerThreadInfo->workerInfoList[workerIndex];

    idleWorker->taskContext.taskId = ProcessWorkerThreadInfo->taskNextId++;
    idleWorker->taskContext.clientfd = clientfd;
    idleWorker->taskEntry = taskProc;

    ProcessWorkerThreadInfo->idleWorkerNum--;

    hat_log("PushTask-%d cliendfd[%d] taskid:%d ", workerIndex, clientfd, idleWorker->taskContext.taskId);
    PostSem(&idleWorker->taskIdleLock);

    return 0;
}

int CriticalBegin()
{
    int s = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (s != 0)
        hat_error("pthread_setcancelstate disable fail.");
}

int CriticalEnd()
{
    int s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        hat_error("pthread_setcancelstate enable fail.");
}

int SetCancelPoint()
{
    pthread_testcancel();
    return 0;
}

static int BeginWorkerThread(PThreadWorkerInfo workerInfo)
{
    workerInfo->tw_state = TW_RUNNING;
}

static int FinishWorkerThread(PThreadWorkerInfo workerInfo)
{
    workerInfo->tw_state = TW_IDLE;
    return 0;
}

static void* threadEntry(void *arg)
{
    PThreadWorkerInfo workerInfo = (PThreadWorkerInfo)arg;
    int ret = 0;

    if(NULL == workerInfo)
        return NULL;
    
    workerContextInfo = workerInfo;

    /* initialize currentContext. */
    MemMangerSetCurrentContext(workerContextInfo->ThreadMemoryContext);

    hat_log("thread %d entered... ", workerContextInfo->tw_id);

    while(workerContextInfo->tw_id > 0)
    {
        ret = WaitSem(&workerContextInfo->taskIdleLock);
        if(ret < 0)
        {
            hat_error("worker %d waitsem cancel[%d]!", workerContextInfo->tw_id, errno);
            break;
        }

        hat_log("worker %d task running.", workerContextInfo->tw_id);

        BeginWorkerThread(workerContextInfo);

        ret = InitWorker();
        if(ret < 0)
            break;
        hat_log("worker %d task init finish.", workerContextInfo->tw_id);

        if(NULL != workerContextInfo->taskEntry)
        {
            workerContextInfo->taskEntry(&workerInfo->taskContext);

            hat_log("worker %d finished.", workerContextInfo->tw_id);
        }        

        FinishWorkerThread(workerContextInfo);
        ExitWorker();
    }

    return NULL;
}
