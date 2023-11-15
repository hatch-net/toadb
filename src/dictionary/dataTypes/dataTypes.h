/*
 *	toadb data types
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_DATA_TYPES_H_H
#define HAT_DATA_TYPES_H_H

typedef enum valueType
{
	VT_UNKNOWN,
	VT_POINTER,
	VT_INT,
    VT_INTEGER,
    VT_VARCHAR,
	VT_STRING,
	VT_CHAR,
    VT_DOUBLE,
	VT_FLOAT,	
	VT_BOOL,
	VT_MAX
}valueType;

int GetColumnType(char *typename);

#endif