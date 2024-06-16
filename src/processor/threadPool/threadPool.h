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

#ifndef HAT_THREADPOOL_H_H
#define HAT_THREADPOOL_H_H

#include "semphore.h"
#include "memStack.h"

typedef enum WORKER_STATE
{
    TW_IDLE,
    TW_RUNNING,
    TW_UNKNOWN
}WS_STATE;

typedef struct ThreadTaskInfo
{
    unsigned int taskId;
    int clientfd;
    
}ThreadTaskInfo, *PThreadTaskInfo;

typedef int (*TaskProcess)(PThreadTaskInfo taskInfo);

typedef struct ThreadWorkerInfo 
{
    volatile int tw_id;
    unsigned int tw_threadid;
    volatile int tw_state;

    SemLock taskIdleLock;
    TaskProcess taskEntry;
    PMemContextNode ThreadMemoryContext;  /* init thread top memory */
    ThreadTaskInfo taskContext; 
}ThreadWorkerInfo, *PThreadWorkerInfo;

typedef struct ThreadPoolInfo 
{
    int threadMaxNum;
    int threadInitNum;
    int threadExteNum;
    unsigned int taskNextId;
    
    int threadListNum;
    int idleWorkerNum;
    PThreadWorkerInfo workerInfoList[];
}ThreadPoolInfo, *PThreadPoolInfo;


int InitThreadPool(int max, int init, int add);
int DestoryWorkerThread(PThreadWorkerInfo workerInfo);
int StopThreadPool();

PThreadWorkerInfo CreeateWorkerThread(int index);
int DestoryWorkerThread(PThreadWorkerInfo workerInfo);

int ShutDownWorkerThread(PThreadWorkerInfo workerInfo);
int ShutDownWorkerThreadImmidately(PThreadWorkerInfo workerInfo);

int PushTask(int clientfd, TaskProcess taskProc);

int CriticalBegin();
int CriticalEnd();
int SetCancelPoint();

#endif
