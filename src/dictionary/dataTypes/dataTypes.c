/*
 *	toadb data types
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

#include "dataTypes.h"
#include "string.h"

char *ColumnType[VT_MAX] = 
{
    "UNKNOWN",
	"pointer",
    "int",
	"integer",
	"varchar",
    "string",
	"char",
    "double",
	"float",
    "bool"
};

int GetColumnType(char *typename)
{
    int index = -1;

    for(int i = 0; i < VT_MAX; i++)
    {
        if(strcmp(typename, ColumnType[i]) == 0)
            index = i;
    }

    return index;
}