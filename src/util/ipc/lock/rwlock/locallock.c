/*
 *	toadb locallock  
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

#include "locallock.h"
#include "public_types.h"
#include "public.h"
#include "rwlock.h"

#define MAX_LOCAL_LOCK_NUM 8
ThreadLocal LocalLockInfo localRWlockTableInfo[MAX_LOCAL_LOCK_NUM] = {0};
ThreadLocal int localRWlockUsedPos = 0;

static int SearchLocalLockTable(PRWLockInfo lock);
static void RemoveLocalLock(int rmIndex);

int InitializeLocalLock()
{
    int pos = 0;

    for(; pos < localRWlockUsedPos; pos ++)
    {
        if(localRWlockTableInfo[pos].lock != NULL)
        {
            ReleaseLock(localRWlockTableInfo[pos].lock, RWLock_NULL);            

            localRWlockTableInfo[pos].lock = NULL;
            localRWlockTableInfo[pos].lockCnt = 0;
            localRWlockTableInfo[pos].lockMode = RWLock_NULL;
        }
    }
    localRWlockUsedPos = 0;

    return 0;
}

void ReleaseLocalLock()
{
    InitializeLocalLock();
}

/*
 * return values
 * 0 , acquired
 * -1, error
 * -2, dead lock
 */
int AcquireRWLockLocal(PRWLockInfo lock, RWLockMode mode, const char *fun, int line)
{
    int ret = 0;

    if(NULL == lock)
    {
        return -1;
    }

    /* search local lock list */
    if(localRWlockUsedPos >= MAX_LOCAL_LOCK_NUM)
    {
        hat_error("held too many locks.[%s][%d]",fun, line);
        return -1;
    }

    ret = SearchLocalLockTable(lock);
    if(ret >= 0)
    {
        if(localRWlockTableInfo[ret].lockMode == mode)
        {
            localRWlockTableInfo[ret].lockCnt ++;
            /* this lock is acquired already. */
            return 0;
        }
        else 
        {
            hat_error("dead lock %p, mod %d, reqmod %d ocur.[%s][%d]", lock, lock->lockMode, mode, fun, line);

            /* TODO: deadlock process. */
            return -2;
        }
    }

    ret = AcquireRWLockEx(lock, mode, fun, line);
    if(ret == 0)
    {
        localRWlockTableInfo[localRWlockUsedPos].lock = lock;
        localRWlockTableInfo[localRWlockUsedPos].lockMode = mode;
        localRWlockTableInfo[localRWlockUsedPos++].lockCnt = 1;
    }
    return ret;
}

/*
 * 尝试获取锁
 * 返回值：
 * 1 获取锁成功；
 * 0 需要等待；
 * < 0 , other error
 */
int TryAcquireRWLockLocal(PRWLockInfo lock, RWLockMode mode, const char *fun, int line)
{
    int ret = 0;

    if(NULL == lock)
    {
        return -1;
    }

    /* search local lock list */
    if(localRWlockUsedPos >= MAX_LOCAL_LOCK_NUM)
    {
        hat_error("held too many locks.[%s][%d]",fun, line);
        return -1;
    }

    ret = SearchLocalLockTable(lock);
    if(ret >= 0)
    {
        if(localRWlockTableInfo[ret].lockMode == mode)
        {
            localRWlockTableInfo[ret].lockCnt ++;
            /* this lock is acquired already. */
            return 1;
        }
        else 
        {
            hat_error("dead lock %p, mod %d, reqmod %d ocur.[%s][%d]", lock, lock->lockMode, mode, fun, line);

            /* TODO: deadlock process. */
            return -2;
        }
    }

    ret = TryAcquireRWLockEx(lock, mode, fun, line);
    if(ret > 0)
    {
        localRWlockTableInfo[localRWlockUsedPos].lock = lock;
        localRWlockTableInfo[localRWlockUsedPos].lockMode = mode;
        localRWlockTableInfo[localRWlockUsedPos++].lockCnt = 1;
    }
    return ret;
}

int ReleaseRWLockLocal(PRWLockInfo lock, RWLockMode mode, const char *fun, int line)
{
    int ret = 0;
    int rmIndex = 0;

    if(NULL == lock)
    {
        return -1;
    }

    /* search local lock list */
    if(localRWlockUsedPos <= 0)
    {
        hat_error("lock %p mode %d is not held.[%s][%d]", lock, mode, fun, line);
        return -1;
    }

    rmIndex = SearchLocalLockTable(lock);
    if(rmIndex >= 0)
    {
        if((localRWlockTableInfo[rmIndex].lockMode != mode) && (RWLock_NULL != mode))
        {            
            hat_error("lock mode is not equal.[%s][%d]",fun, line);
        }

        if(localRWlockTableInfo[rmIndex].lockCnt > 1)
        {
            localRWlockTableInfo[rmIndex].lockCnt --;
        
            return 0;
        }
    }

    ret = ReleaseRWLockEx(lock, mode, fun, line);
    if((ret == 0) && (rmIndex >= 0))
    {
        RemoveLocalLock(rmIndex);
    }

    return ret;
}

/*
 *  search from locallock list.
 *  return values:
 * -1 , not found;
 * >= 0 , found, list index
 */
static int SearchLocalLockTable(PRWLockInfo lock)
{
    int pos = 0;

    for(; pos < localRWlockUsedPos; pos++)
    {
        if(lock == localRWlockTableInfo[pos].lock)
            return pos;
    }
    
    /* not found */
    return -1;
}

static void RemoveLocalLock(int rmIndex)
{
    int pos = rmIndex;
    
    for(; pos < localRWlockUsedPos - 1; pos++)
    {
        localRWlockTableInfo[pos] =  localRWlockTableInfo[pos+1];
    }
    localRWlockUsedPos -= 1;

    return;
}