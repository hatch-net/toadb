/*
 *	toadb string process 
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

#ifndef HAT_STRING_H_H
#define HAT_STRING_H_H



#include "config_pub.h"

#include <string.h>
#include <strings.h>

/*  ignoring any differences in letter case */
#ifdef STRING_IGNORE_CASE
#define hat_strcmp(str1,str2,n) strncasecmp(str1,str2,n) 
#else 
#define hat_strcmp(str1,str2,n) strncmp(str1,str2,n) 
#endif 



/*
 * TODO: 2024/1/23 当前仅支持ascii的比较，后续增加本地化多语言，多种排序方式；
 */
#define hat_strncmp(str1, str2, len) strncmp(str1,str2,len)
#define hat_strlen(str1) strlen(str1)

#endif 