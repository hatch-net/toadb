/*
 *	toadb ipc  
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
#include "ipc.h"
#include "public.h"


#include <sys/shm.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>


int sharemid = -1;
sem_t *readsem = NULL;
sem_t *writesem = NULL;
sem_t *clientsem = NULL;

static int InitializeShareMem(char **ptr);
static sem_t *InitializeSem(char *name, int initValue);

static int DestorySem(char *name, sem_t *sem);
static int CloseSem(sem_t *sem);

static int WaitSem(sem_t *sem);
static int PostSem(sem_t *sem);

int InitServerSharedEnv(char **ptr)
{
    int ret = 0;

    ret = InitializeShareMem(ptr);
    readsem = InitializeSem(TOADB_SHARED_SEM_READ_ID, 0);
    writesem = InitializeSem(TOADB_SHARED_SEM_WRITE_ID, 1);
    clientsem = InitializeSem(TOADB_SHARED_SEM_CLIENT_ID, 1);

    return ret;
}

int DestorySeverSharedEnv(char **ptr)
{
    DestorySem(TOADB_SHARED_SEM_WRITE_ID,writesem);
    DestorySem(TOADB_SHARED_SEM_READ_ID,readsem);
    DestorySem(TOADB_SHARED_SEM_CLIENT_ID,clientsem);

    // 从共享内存中分离并删除共享内存对象  
    if(NULL != *ptr)
    {
        shmdt(*ptr);  
        shmctl(sharemid, IPC_RMID, NULL);
        *ptr = NULL;
    }
    return 0;
}

int InitClientSharedEnv(char **ptr)
{
    int ret = 0;

    ret = InitializeShareMem(ptr);
    readsem = InitializeSem(TOADB_SHARED_SEM_READ_ID, 0);
    writesem = InitializeSem(TOADB_SHARED_SEM_WRITE_ID, 1);
    clientsem = InitializeSem(TOADB_SHARED_SEM_WRITE_ID, 1);

    return ret;
}

int DestoryClientSharedEnv(char **ptr)
{
    CloseSem(writesem);
    CloseSem(readsem);
    CloseSem(clientsem);

    // 从共享内存中分离并删除共享内存对象  
    if(NULL != *ptr)
    {
        shmdt(*ptr);  

        *ptr = NULL;
    }
    return 0;
}

int WaitClientDataArrive()
{
    WaitSem(readsem);
    return 0;
}

int NotifyClientReadData()
{
    PostSem(writesem);
    return 0;
}

int NotifyServerReadData()
{
    PostSem(readsem);
    return 0;
}

int WaitServerDataArrive()
{
    WaitSem(writesem);
    return 0;
}

int WaitClientControlLock()
{
    WaitSem(clientsem);
    return 0;
}

int ReleaseClientControlLock()
{
    PostSem(clientsem);
    return 0;
}

static int InitializeShareMem(char **ptr)
{
    int sharedMemSize ;
	char *shmem = NULL;

    sharedMemSize = sizeof(ClientSharedInfo)+CLIENT_SHARED_BUFFER_SIZE;

    sharemid = shmget((key_t)(TOADB_SHARED_ID), sharedMemSize, IPC_CREAT | 0666);
    if (sharemid < 0)
    {
        hat_error("sharemid shmget err");
        return -1;
    }
    
    shmem = shmat(sharemid, 0, 0);
    if ( (void*)-1 == shmem)
    {
        hat_error("error in init\n");
        return -1;
    }

    *ptr = shmem;

    return 0;
}

static sem_t *InitializeSem(char *name, int initValue)
{
    sem_t *readsem = NULL;

    readsem = sem_open(name, O_CREAT , 0666, initValue);
    if(SEM_FAILED == readsem)
    {
        hat_error("create sem failure.\n");
        return NULL;
    }

    return readsem;
}

static int DestorySem(char *name, sem_t *sem)
{
    if(NULL == sem)
    {
        return -1;
    }

    sem_close(sem);
    unlink(name);
    return 0;
}


static int CloseSem(sem_t *sem)
{
    if(NULL == sem)
    {
        return -1;
    }

    sem_close(sem);
    return 0;
}

static int WaitSem(sem_t *sem)
{
    if(NULL == sem)
        return -1;
    return sem_wait(sem);
}

static int PostSem(sem_t *sem)
{
    if(NULL == sem)
        return -1;
    return sem_post(sem);
}