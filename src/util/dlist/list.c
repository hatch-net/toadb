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

        *list = head;
    }
    else 
    { 
        head = (PDList)(*list);
        node->next = head; 
        node->prev = head->prev; 
        head->prev->next = node; 
        head->prev = node; 
    }
    
    return 0;
}

int AddDListNext(PDList *list, PDList cell) 
{
    PDList head = NULL;
    PDList node = (PDList)(cell);

    if(NULL == *list) 
    { 
        head = node; 
        head->prev = head; 
        head->next = head; 

        *list = head;
    }
    else 
    { 
        head = (PDList)(*list);
        
        node->next = head->next; 
        node->prev = head; 
        head->next = node; 
        node->next->prev = node; 
    }
    
    return 0;
}

/* stack pop */
PDList PopDListHeadNode(PDList *list)
{
    PDList node = NULL;
    PDList head = NULL;

    if(NULL == list || NULL == *list) 
    { 
        return NULL;
    }

    head = *list;
    node = head;
    
    head = head->next;
    if(head == node)
    {
        /* only one */
        head = NULL;
    }
    else
    {
        head->prev = node->prev;
        node->prev->next = head;
    }

    /* new head */
    *list = head;

    /* pop node */
    node->prev = node->next = node;

    return node;
}

/* queue pop */
PDList PopDListTailNode(PDList *list)
{
    PDList node = NULL;
    PDList head = NULL;

    if(NULL == list || NULL == *list) 
    { 
        return NULL;
    }

    head = *list;
    node = head->prev;
    
    if(head == node)
    {
        /* only one */
        head = NULL;
    }
    else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    /* new head */
    *list = head;

    /* pop node */
    node->prev = node->next = node;

    return node;
}

int DelDListNode(PDList *list, PDList cell) 
{
    PDList node = (PDList)(cell);

    if(NULL == list || NULL == *list) 
    { 
        return 0;
    }
    
    if(node->prev == node)
    {
        /* the last one */
        return 0;
    }

    node->next->prev = node->prev;
    node->prev->next = node->next;

    node->next = node->prev = node;
    
    return 1;
}

void EreaseDListNode(PDList node)
{
    if(NULL != node)
        FreeMem(node);
}