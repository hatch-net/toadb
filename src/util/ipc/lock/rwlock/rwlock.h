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



#include "locallock.h"




int InitRWLock(PRWLockInfo lock, LockBranch branch);

int TryAcquireRWLockEx(PRWLockInfo lock, RWLockMode mode, const char *fun, int line);
int AcquireRWLockEx(PRWLockInfo lock, RWLockMode mode, const char *fun, int line);
int ReleaseRWLockEx(PRWLockInfo lock, RWLockMode mode, const char *fun, int line);

int DestroyRWLock(PRWLockInfo lock);

#endif 