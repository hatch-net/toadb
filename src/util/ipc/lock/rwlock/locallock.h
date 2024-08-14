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

#ifndef HAT_LOCALLOCK_H_H
#define HAT_LOCALLOCK_H_H

#include <pthread.h>  
#include <unistd.h>  

typedef enum RWLockMode 
{
    RWLock_NULL,
    RWLock_READ,
    RWLock_WRITE
}RWLockMode;

typedef enum LockBranch
{
    LB_BUFFER_BLOCKS,
    LB_BUFFER_POOL,
     LB_RELCACHE_TABLES,
    LB_RELCACHE_TBLS_EXTENSION,
    LB_DICTIONARY_INFO,
    LB_SMGR_INFO,
    LB_VFS_INFO

}LockBranch;

typedef struct RWLockInfo 
{
    pthread_rwlock_t rwlock; 
    RWLockMode lockMode;

    /* below debug infomations */    
    LockBranch branch;
    int rlockCnt;
}RWLockInfo, *PRWLockInfo;


typedef struct LocalLockInfo
{
    PRWLockInfo lock;
    int lockMode;
    int lockCnt;
}LocalLockInfo, *PLocalLockInfo;


#define AcquireLock(lock, mode)     AcquireRWLockLocal(lock, mode, __FUNCTION__, __LINE__)
#define TryAcquireLock(lock, mode)  TryAcquireRWLockLocal(lock, mode, __FUNCTION__, __LINE__)
#define ReleaseLock(lock, mode)     ReleaseRWLockLocal(lock, mode, __FUNCTION__, __LINE__)

void ReleaseLocalLock();

int AcquireRWLockLocal(PRWLockInfo lock, RWLockMode mode, const char *fun, int line);

/*
 * 尝试获取锁
 * 返回值：
 * 1 获取锁成功；
 * 0 需要等待；
 */
int TryAcquireRWLockLocal(PRWLockInfo lock, RWLockMode mode, const char *fun, int line);

int ReleaseRWLockLocal(PRWLockInfo lock, RWLockMode mode, const char *fun, int line);

#endif 


