/*
 *	toadb list 
 * Copyright (C) 2023-2023, senllang
*/

#include "list.h"
#include "buffer.h"

#include <stdio.h>

int AddCellToListTail(PDList *head, void *ptr)
{
    PDLCell cell = (PDLCell)AllocMem(sizeof(DLCell));
    cell->value = ptr;

    AddDListTail(head, (PDList)cell);
    return 0;
}

int AddDListTail(PDList *list, PDList cell) 
{
    PDList head = (PDList)(*list);
    PDList node = (PDList)(cell);

    if(NULL == head) 
    { 
        head = node; 
        head->prev = head; 
        head->next = head; 
    }
    else 
    { 
        node->next = head; 
        node->prev = head->prev; 
        head->prev->next = node; 
        head->prev = node; 
    }

    *list = head;
    return 0;
}