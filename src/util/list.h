/*
 *	toadb list 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_LIST_H_H
#define HAT_LIST_H_H

/* will be the first member */
typedef struct dList
{
    struct dList *prev;
    struct dList *next;
}DList, *PDList;

typedef struct dListCell
{
    DList list;
    void *value;
}DLCell, *PDLCell;

#define INIT_DLIST_NODE(list) (list.prev = &list, list.next = &list)
int AddDListTail(PDList *list, PDList cell);
int DelDListNode(PDList *list, PDList cell);
int AddCellToListTail(PDList *head, void *ptr);
#endif