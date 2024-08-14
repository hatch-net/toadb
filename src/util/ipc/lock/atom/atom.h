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

#ifndef HAT_ATOM_H_H
#define HAT_ATOM_H_H

#include "public_types.h"


int atomic_compare_exchange(atom_uint *ptr,
									unsigned int *expected, unsigned int newval);

atom_uint atomic_fetch_add(atom_uint *ptr, unsigned int add_);

atom_uint atomic_fetch_sub(atom_uint *ptr, unsigned int sub_);

void atomic_clear_flag(atom_uint *ptr);



atom_uint64 atomic_fetch_add64(atom_uint64 *ptr, UINT64 add_);

atom_uint64 atomic_fetch_sub64(atom_uint64 *ptr, UINT64 sub_);				

int atomic_compare_exchange64(atom_uint64 *ptr,
									UINT64 *expected, UINT64 newval);
#endif 