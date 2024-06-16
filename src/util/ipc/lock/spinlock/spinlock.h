/*
 *	toadb spinlock  
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

#ifndef HAT_SPIN_LOCK_H_H
#define HAT_SPIN_LOCK_H_H

#define SPIN_DELAY_MAX 		(10000)

/* only X86-64 support . */
typedef struct SPINLOCK
{
    volatile int  stom;   
}SPINLOCK, *PSPINLOCK;

void SpinLockInit(PSPINLOCK lock);

int SpinLockAquire(PSPINLOCK lock);

void SpinLockRelease(PSPINLOCK lock);

void SpinLockFree(PSPINLOCK lock);
#endif 

