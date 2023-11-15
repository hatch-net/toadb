/*
 *	toadb data types
 * Copyright (C) 2023-2023, senllang
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
    int i = 0;

    for(i = 0; i < VT_MAX; i++)
    {
        if(strcmp(typename, ColumnType[i]) == 0)
            index = i;
    }

    return index;
}