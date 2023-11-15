/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>
#include "node.h"
#include "buffer.h"

#define log printf
#define debug 

/*
 *  create empty node
 */
PNode CreateNode(int size, NodeType type)
{
    PNode node = AllocMem(size);
    if(NULL == node)
    {
        log("list create, not enough memory.\n");
        exit(1);
    }
    debug("CreateNode node:%p size:%d \n", node, size);

    node->type = type;
}

/* 
 * Add empty cell node to the list at the tail.
 * when list is NULL, create list at first.
 */
PList CreateCell(PList list)
{
    /* new cell */
    PListCell cell = AllocMem(sizeof(ListCell));
    if(NULL == cell)
    {
        log("list create, not enough memory.\n");
        exit(1);
    }
    cell->next = NULL;
    
    /* initial list */
    if(NULL == list)
    {
        list = (PList)CreateNode(sizeof(List), T_List);
        list->length = 0;
    }

    debug("CreateCell list:%p size:%d, cell:%p size:%d \n", list,sizeof(List), cell, sizeof(ListCell));

    /* add cell to this list */
    if(list->length == 0)
    {
        list->head = cell;
        list->tail = cell;
    }
    else
    {
        list->tail->next = cell;
        list->tail = cell;
    }
    list->length += 1;

    return list;
}

/* 
 * create list, and append node at the tail. 
 */
PList CreateList(PNode n)
{
    PList list = CreateCell(NULL);

    if(NULL != n)
    {
        list->tail->value.pValue = n;
    }

    return list;
}

/* 
 * Append the n node at the tail of list, 
 * when list is NULL, creating at first.
 */
PList AppendNode(PList list, PNode n)
{
    if(NULL == list)
    {
        list = CreateList(n);
        return list;
    }

    /* new cell */
    PListCell cell = AllocMem(sizeof(ListCell));
    if(NULL == cell)
    {
        log("list create, not enough memory.\n");
        exit(1);
    }
    cell->next = NULL;
    cell->value.pValue = n;

    debug("AppendCell list:%p size:%d, cell:%p size:%d \n", list,sizeof(List), cell, sizeof(ListCell));

    /* add cell to this list */
    if(list->length == 0)
    {
        list->head = cell;
        list->tail = cell;
    }
    else
    {
        list->tail->next = cell;
        list->tail = cell;
    }
    list->length += 1;

    return list;
}

PNode GetCellValueByIndex(PList list, int index)
{
    PListCell tmpCell = NULL;
    int count = 0;

    if((NULL == list) || (list->length < index))
        return NULL;
    
    for(tmpCell = list->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        count ++;

        if(count == index)
            break; 
    }

    if(tmpCell == NULL)
        return NULL;
    return GetCellNodeValue(tmpCell);
}



