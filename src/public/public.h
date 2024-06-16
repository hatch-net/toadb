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
#include "logger.h"
#include <stddef.h>




/* maybe use offset, meanwhile older compiler.  */
#define GetOffsetSize(type, member) (unsigned long)(&(((type *)(0))->member))
/* 通过成员的地址减去偏移量来获取结构体的地址  */
//#define GetAddrByMember(memberaddr, member, type) (type *)(((char*)(memberaddr)) - offsetof(type,member))

#define GetAddrByMember(memberaddr, member, type) (type *)(((char*)(memberaddr)) - GetOffsetSize(type,member))


#define FLEXIBLE_SIZE 

#define COMMENT_NAME_MAX_LEN (32)


#define COMMAND_FINISH "finish"
#define COMMAND_FiNISH_LEN 6

#endif