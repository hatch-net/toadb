/*
 *	toadb data type proc
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

#ifndef HAT_DATA_TYPE__PROC_H_H
#define HAT_DATA_TYPE__PROC_H_H

#include "dataTypes.h"
#include "public.h"
#include "parserNode.h"


typedef PExprDataInfo (*Operator)(PExprDataInfo leftvalue, PExprDataInfo rightvalue);
typedef PExprDataInfo (*OperatorEx)(PExprDataInfo leftvalue, PExprDataInfo rightvalue, PExprDataInfo resExprData);
typedef int (*Operator1)(PExprDataInfo value);

#define DATATYPEPROC_VERSION    (0x3A01)
typedef struct DataTypeProcs
{
    int       version;                  /* 兼容性 */
    valueType dataType;
    /* > ， >= , = , <= , < , != , not, and, or */
    OperatorEx opf[Op_Type_MAX];
    Operator1 getSize;
}DataTypeProcs, *PDataTypeProcs;



/* 获取类型操作处理函数组 */
PDataTypeProcs getDataTypeProcs(valueType type);

#endif