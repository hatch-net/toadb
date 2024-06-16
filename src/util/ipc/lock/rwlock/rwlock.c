/*
 *	toadb rwlock  
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

#include "rwlock.h"
#include "public.h"

int InitRWLock(PRWLockInfo lock)
{
    if(NULL == lock)
        return -1;

    pthread_rwlock_init(&lock->rwlock, NULL);

    lock->lockMode = RWLock_NULL;
    return 0;
}

int AcquireRWLock(PRWLockInfo lock, RWLockMode mode) 
{
    if(NULL == lock)
        return -1;
    
    if(lock->lockMode != RWLock_NULL)
    {
        hat_error("lock %p mode:%d , Aquire mode:%d", lock, lock->lockMode, mode);
        return -1;
    }

    if(mode == RWLock_READ)
    {
        pthread_rwlock_rdlock(&lock->rwlock); // 读者加读锁
    }
    else if(mode == RWLock_WRITE)
    {
        pthread_rwlock_wrlock(&lock->rwlock); // 写者加写锁
    }
    else 
    {
        return -1;
    }

    lock->lockMode = mode;
    return 0;
}

int ReleaseRWLock(PRWLockInfo lock, RWLockMode mode)
{
    if(NULL == lock)
        return -1;

    if((lock->lockMode != mode) && (mode != RWLock_NULL))
    {
        hat_error("lock %p mode:%d , release mode:%d", lock, lock->lockMode, mode);
    }

    /* TODO: record lock mode, check mode consisdence. */
    pthread_rwlock_unlock(&lock->rwlock); // 释放写锁  
    lock->lockMode = RWLock_NULL;
    return 0;
}

int DestroyRWLock(PRWLockInfo lock)
{
    if(NULL == lock)
        return -1;
    pthread_rwlock_destroy(&lock->rwlock); // 销毁读写锁

    return 0;
}