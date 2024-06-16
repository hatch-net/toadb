/*
 *	toadb semphore
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

#include "semphore.h"
#include "public.h"
#include "shareMem.h"
#include "hatstring.h"

#include <errno.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */

#include <unistd.h> /* unlink */

PSemLock InitializeSem(char *name, int initValue, PSemLock sem)
{
    int ret = 0;
    if(NULL == sem)
    {
        return NULL;
    }

    /* multiple threads in one process. */
    ret = sem_init(&sem->semLock, 0, initValue);
    if(ret < 0)
    {
        hat_error("create sem failure.");
        return NULL;
    }

    hat_strncpy(sem->name, name, sizeof(sem->name));

    return sem;
}

int DestorySem(char *name, PSemLock sem)
{
    if(NULL == sem)
    {
        return -1;
    }

    /* uname semaphore. */
    sem_destroy(&sem->semLock);

    return 0;
}


int CloseSem(PSemLock sem)
{
    if(NULL == sem)
    {
        return -1;
    }

    /* uname semaphore. */
    sem_destroy(&sem->semLock);

    return 0;
}

int WaitSem(PSemLock sem)
{
    int ret = 0;

    if(NULL == sem)
    {
        hat_error("sem_wait null");
        return -1;
    }

    ret = sem_wait(&sem->semLock);
    if(ret < 0)
    {
        hat_error("sem_wait error[%d]", errno);
    }
    return ret; 
}

int PostSem(PSemLock sem)
{
    if(NULL == sem)
    {
        return -1;
    }
    return sem_post(&sem->semLock);
}