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

#include "dataTypeProc.h"
#include "public.h"
#include "server_pub.h"
#include "charType.h"
#include "floatType.h"

extern DataTypeProcs integerDataTypeProcEntry;
extern DataTypeProcs intDataTypeProcEntry;

extern DataTypeProcs stringDataTypeProcEntry;
extern DataTypeProcs varcharDataTypeProcEntry;

extern DataTypeProcs boolDataTypeProcEntry;

/* 每种类型有一组处理函数，数组下标对应类型 */
PDataTypeProcs dataProcsEntry[VT_MAX] = 
{
    NULL,   // unknow
    NULL,   // pointer
    &intDataTypeProcEntry,      // int
    &integerDataTypeProcEntry,  // integer
    &varcharDataTypeProcEntry,  // VT_VARCHAR
    &stringDataTypeProcEntry,   // VT_STRING
    &charDataTypeProcEntry,     // VT_CHAR
    &floatDataTypeProcEntry,    // VT_DOUBLE
    &floatDataTypeProcEntry,    // VT_FLOAT
    &boolDataTypeProcEntry      // VT_BOOL
                                // VT_MAX
};

PDataTypeProcs getDataTypeProcs(valueType type)
{
    if((type < VT_MAX) && (type > VT_UNKNOWN))
        return dataProcsEntry[type];
    
    return NULL;
}