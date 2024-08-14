/*
 *	toadb public types
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

#ifndef HAT_PUBLIC_TYPES_H_H
#define HAT_PUBLIC_TYPES_H_H

typedef unsigned long long UINT64;
typedef long long           INT64;

typedef unsigned int       UINT32;
typedef int                 INT32;

typedef volatile unsigned int  atom_uint;
typedef volatile unsigned long long  atom_uint64;
typedef unsigned int  uint32;

typedef unsigned char       BYTE;

typedef UINT64              XTID;

#define ThreadLocal         __thread 


#endif 