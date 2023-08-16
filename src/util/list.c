/*
 *	toadb list 
 * Copyright (C) 2023-2023, senllang
*/

#include "list.h"
#include "buffer.h"

#include <stdio.h>

int AddCellToListTail(PDList *head, void *ptr)
{
    PDLCell cell = NULL;

    if(head == NULL)
        return -1;

    cell = (PDLCell)AllocMem(sizeof(DLCell));
    cell->value = ptr;
    
    AddDListTail(head, (PDList)cell);
    return 0;
}

int AddDListTail(PDList *list, PDList cell) 
{
    PDList head = NULL;
    PDList node = (PDList)(cell);

    if(NULL == *list) 
    { 
        head = node; 
        head->prev = head; 
        head->next = head; 
    }
    else 
    { 
        head = (PDList)(*list);
        node->next = head; 
        node->prev = head->prev; 
        head->prev->next = node; 
        head->prev = node; 
    }

    *list = head;
    return 0;
}

int DelDListNode(PDList *list, PDList cell) 
{
    PDList node = (PDList)(cell);

    if(NULL == list || NULL == *list) 
    { 
        return 0;
    }
    
    node->next->prev = node->prev;
    node->prev->next = node->next;
    
    return 0;
}