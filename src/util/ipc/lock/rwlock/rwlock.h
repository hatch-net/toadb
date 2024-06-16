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

#ifndef HAT_RWLOCK_H_H
#define HAT_RWLOCK_H_H

#include <pthread.h>  
#include <unistd.h>  

typedef enum RWLockMode 
{
    RWLock_NULL,
    RWLock_READ,
    RWLock_WRITE
}RWLockMode;

typedef struct RWLockInfo 
{
    pthread_rwlock_t rwlock; 
    RWLockMode lockMode;
}RWLockInfo, *PRWLockInfo;

int InitRWLock(PRWLockInfo lock);

int AcquireRWLock(PRWLockInfo lock, RWLockMode mode) ;

int ReleaseRWLock(PRWLockInfo lock, RWLockMode mode);

int DestroyRWLock(PRWLockInfo lock);

#endif 