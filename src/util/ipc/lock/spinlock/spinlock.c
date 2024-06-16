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

#include "spinlock.h"
#include "atom.h"
#include "public.h"

#define TAS_SPIN(lock)    ((lock->stom) ? 1 : spin_tas(lock))

static inline void spin_delay(void);
static inline int spin_tas(volatile PSPINLOCK lock);
static inline void spin_clear(volatile PSPINLOCK lock);

void SpinLockInit(PSPINLOCK lock)
{
    lock->stom = 0;
}

int SpinLockAquire(PSPINLOCK lock)
{
	int delays = 0;
	int cur_delays = 0;

	while (TAS_SPIN(lock))
	{
		spin_delay();

		/* delay count */
		if(++cur_delays > SPIN_DELAY_MAX)
		{
			if(++delays > SPIN_DELAY_MAX)
			{
				/* It's waiting too long, yield to lock. */
				hat_error("spin dely count overhead");
				break;
			}

			cur_delays = 0;
		}
	}

	return delays;
}

void SpinLockRelease(PSPINLOCK lock)
{
    spin_clear(lock);
}

void SpinLockFree(PSPINLOCK lock)
{
	spin_clear(lock);
}

static inline void spin_delay(void)
{
	__asm__ __volatile__(" rep; nop			");
}



static inline int spin_tas(volatile PSPINLOCK lock)
{
	return __sync_lock_test_and_set(&lock->stom, 1);
}

static inline void spin_clear(volatile PSPINLOCK lock)
{
	__sync_lock_release(&lock->stom);
}