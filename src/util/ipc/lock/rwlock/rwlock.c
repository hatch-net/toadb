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
#include "atom.h"

#include <errno.h>

#define hat_rwlock_debug(...) 
//#define hat_rwlock_debug(...)  log_report(LOG_DEBUG, __VA_ARGS__) 

int InitRWLock(PRWLockInfo lock, LockBranch branch)
{
    if(NULL == lock)
        return -1;

    pthread_rwlock_init(&lock->rwlock, NULL);

    lock->lockMode = RWLock_NULL;
    
    lock->branch = branch;
    lock->rlockCnt = 0;

    return 0;
}

/*
 * return values:
 * 0 , not acquired, will be waiting.
 * 1, acquired success
 * -1, other error
*/
static int TryAcquireRWLock(PRWLockInfo lock, RWLockMode mode)
{
    int ret = 0;

    if(NULL == lock)
        return -1;
    
    hat_rwlock_debug("Try Aquire begin lock %p mode:%d - %d ,reqmode:%d", lock, lock->lockMode, lock->rlockCnt, mode);

    if(mode == RWLock_READ)
    {
        ret = pthread_rwlock_tryrdlock(&lock->rwlock); // 读者加读锁
    }
    else if(mode == RWLock_WRITE)
    {
        ret = pthread_rwlock_trywrlock(&lock->rwlock); // 写者加写锁
    }
    else 
    {
        return -1;
    }

    if(lock->lockMode != mode)
        lock->lockMode = mode;

    if(lock->lockMode == RWLock_READ)
        atomic_fetch_add(&lock->rlockCnt, 1);

    hat_rwlock_debug("try Aquire end lock %p mode:%d, reqmode:%d cnt:%d ret:%d", lock, lock->lockMode, mode, lock->rlockCnt, ret);

    if(ret == 0)
    {
        /* acquired */
        return 1;
    }
    
    if(ret == EBUSY)
    {
        return 0;
    }

    return -1;
}

int AcquireRWLock(PRWLockInfo lock, RWLockMode mode) 
{
    int ret = 0;

    if(NULL == lock)
        return -1;
    
    hat_rwlock_debug("Aquire begin lock %p mode:%d - %d ,reqmode:%d", lock, lock->lockMode, lock->rlockCnt, mode);

RETRY:
    if(mode == RWLock_READ)
    {
        ret = pthread_rwlock_rdlock(&lock->rwlock); // 读者加读锁
    }
    else if(mode == RWLock_WRITE)
    {
        ret = pthread_rwlock_wrlock(&lock->rwlock); // 写者加写锁
    }
    else 
    {
        return -1;
    }

    if(ret != 0)
    {
        if(EAGAIN == ret)
        {
            hat_error("retry lock %p mode:%d failure ret[%d] erro[%d]", lock, mode, ret, errno);
            goto RETRY;
        }

        if(EDEADLK == ret)
        {
            hat_error("deadlock %p mode:%d ret[%d] error[%d]", lock, mode, ret, errno);
        }

        hat_error("lock %p mode:%d failure ret[%d] erro[%d]", lock, mode, ret, errno);
        return -1;
    }

    if(lock->lockMode != mode)
        lock->lockMode = mode;

    if(lock->lockMode == RWLock_READ)
        atomic_fetch_add(&lock->rlockCnt, 1);

    hat_rwlock_debug("Aquire end  lock %p mode:%d, reqmode:%d cnt:%d", lock, lock->lockMode, mode, lock->rlockCnt);
    return 0;
}

int ReleaseRWLock(PRWLockInfo lock, RWLockMode mode)
{
    int ret = 0;
    if(NULL == lock)
        return -1;

    hat_rwlock_debug("release begin lock %p mode:%d - %d ,reqmode:%d", lock, lock->lockMode, lock->rlockCnt, mode);
    if((lock->lockMode != mode) && (mode != RWLock_NULL))
    {
        ; // hat_error("lock %p mode:%d , release mode:%d cnt:%d", lock, lock->lockMode, mode, lock->rlockCnt);
    }

    /* TODO: record lock mode, check mode consisdence. */    
    if(lock->lockMode == RWLock_WRITE)
        lock->lockMode = RWLock_NULL;
    else 
    {
        if(atomic_fetch_sub(&lock->rlockCnt, 1) == 0)
            lock->lockMode = RWLock_NULL;
    }
    ret = pthread_rwlock_unlock(&lock->rwlock); // 释放写锁  

    if(ret != 0)
    {
        hat_error("unlock %p mode:%d failure ret %d [%d]", lock, mode, ret, errno);
        return -1;
    }

    hat_rwlock_debug("release end  lock %p mode:%d, reqmode:%d cnt:%d", lock, lock->lockMode, mode, lock->rlockCnt);
    return 0;
}

int DestroyRWLock(PRWLockInfo lock)
{
    if(NULL == lock)
        return -1;
    pthread_rwlock_destroy(&lock->rwlock); // 销毁读写锁

    return 0;
}

int ReleaseRWLockEx(PRWLockInfo lock, RWLockMode mode, const char *fun, int line)
{
    hat_rwlock_debug("ReleaseRWLockEx lock %p mode:%d readcnt:%d branch:%d reqmode:%d [%s][%d]", 
                        lock, 
                        lock->lockMode, lock->rlockCnt, lock->branch, 
                        mode, fun, line);
    return ReleaseRWLock(lock, mode);
}

int AcquireRWLockEx(PRWLockInfo lock, RWLockMode mode, const char *fun, int line)
{
    hat_rwlock_debug("AcquireRWLockEx lock %p mode:%d readcnt:%d branch:%d reqmode:%d [%s][%d]", 
                        lock, 
                        lock->lockMode, lock->rlockCnt, lock->branch, 
                        mode, fun, line);
    return AcquireRWLock(lock, mode);
}

int TryAcquireRWLockEx(PRWLockInfo lock, RWLockMode mode, const char *fun, int line)
{
    hat_rwlock_debug("TryAcquireRWLockEx lock %p mode:%d readcnt:%d branch:%d reqmode:%d [%s][%d]", 
                        lock, 
                        lock->lockMode, lock->rlockCnt, lock->branch, 
                        mode, fun, line);
    return TryAcquireRWLock(lock, mode);
}