/*
 *	toadb public
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

#ifndef HAT_PUBLIC_H_H
#define HAT_PUBLIC_H_H

#define TOADB_VERSION "V100C001B001SPC001"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define hat_debug1 printf 
#define hat_debug 
#define hat_log printf 
#define hat_error printf 

/* maybe use offset, meanwhile older compiler.  */
#define GetOffsetSize(member, type) (unsigned long)(((type *)(0))->member)

typedef enum HAT_BOOL
{
    HAT_NO = 0,
    HAT_YES = 1,
    HAT_FALSE = 0,
    HAT_TRUE = 1,
    HAT_UNKNOWN
}HAT_BOOL;



#endif