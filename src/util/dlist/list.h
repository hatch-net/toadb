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

/* used as follow, it is the default DList node. */
typedef struct dListCell
{
    DList list;
    void *value;
}DLCell, *PDLCell;


#define INIT_DLIST_NODE(list) (list.prev = &list, list.next = &list)
#define DList_Node_Value(node) (((PDLCell)(node))->value)

/* only head = NULL */
#define DList_IS_NULL(list) ((list) == NULL)

int AddDListTail(PDList *list, PDList cell);
int DelDListNode(PDList *list, PDList cell);

int AddCellToListTail(PDList *head, void *ptr);
int AddDListNext(PDList *list, PDList cell);

PDList PopDListHeadNode(PDList *list);
PDList PopDListTailNode(PDList *list);

void EreaseDListNode(PDList node);

#endif