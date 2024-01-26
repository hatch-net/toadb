/*
 *	toadb float type 
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

#ifndef HAT_FLOAT_TYPE_H_H
#define HAT_FLOAT_TYPE_H_H
#include "dataTypeProc.h"

/* 当为4 Bytes时, 1bit 符号位， 23bit小数位，小数点后7位，这里为小数6位 */
#define FLOAT_EPSILON (1e-6)


extern DataTypeProcs floatDataTypeProcEntry;


#endif