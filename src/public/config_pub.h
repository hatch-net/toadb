/*
 *	toadb public configures
 *
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

#ifndef HAT_CONFIGURE_PUBLIC_H_H
#define HAT_CONFIGURE_PUBLIC_H_H

/* Notes: Switches here, which we will change this file, or add to makefile. */


/* 
 * 内存上下文管理相关开关 
 */

#define MEM_MANAGER 1

/* under MEM_MANAGER=1 */ 
//#define MEM_MANAGER_VALID_CHECK 1
#ifdef MEM_MANAGER_VALID_CHECK
#define MEM_DEBUG 1
#endif 

// #define MEM_MANAGER_SHOW 1


/* 
 * 解析器相关开关 
 */

/* 当开关打开时，打印语法树，解析树，计划树 */
//#define PARSER_TREE_PRINT 1

/* 
 * 数据库标识符是否区分大小写 
 * 定义后，则不区分 tableName = tablename
 */
//#define STRING_IGNORE_CASE 1

/* 
 * 顺序扫描相关开关 
 */
/* 当开关打开时，打印扫描到的item位置 */
// #define SEQSCAN_STEP_ITEM_POSITION 1

/* 是否需要同步刷盘 */
extern int config_fsync;

extern int pageNum;
#endif 