/*
 *	toadb public
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_PUBLIC_H_H
#define HAT_PUBLIC_H_H

/* maybe use offset, meanwhile older compiler.  */
#define GetOffsetSize(member, type) (unsigned long)(((type *)(0))->member)

typedef enum HAT_BOOL
{
    HAT_NO,
    HAT_YES,
    HAT_UNKNOWN
}HAT_BOOL;

#endif