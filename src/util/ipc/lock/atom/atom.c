/*
 *	toadb atom   
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

#include "atom.h"

int atomic_compare_exchange(volatile atom_uint *ptr,
									unsigned int *expected, unsigned int newval)
{
	int	ret;
	atom_uint	current;
	current = __sync_val_compare_and_swap(ptr, *expected, newval);
	ret = current == *expected;
	*expected = current;
	return ret;
}

atom_uint atomic_fetch_add(volatile atom_uint *ptr, unsigned int add_)
{
	return __sync_fetch_and_add(ptr, add_);
}

atom_uint ux_atomic_fetch_sub(volatile atom_uint *ptr, unsigned int sub_)
{
	return __sync_fetch_and_sub(ptr, sub_);
}


void atomic_clear_flag(volatile atom_uint *ptr)
{
	__sync_lock_release(ptr);
}


int ux_atomic_compare_exchange_impl(volatile atom_uint *ptr,
									uint32 *expected, uint32 newval)
{
	int	ret;
	uint32	current;
	current = __sync_val_compare_and_swap(ptr, *expected, newval);
	ret = current == *expected;
	*expected = current;
	return ret;
}

int atomic_tas(volatile atom_uint *ptr)
{
	return __sync_lock_test_and_set(ptr, 1);
}