/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>
#include "node.h"
#include "buffer.h"
#include "memStack.h"
#include "logger.h"

/*
 *  create empty node
 */
PNode CreateNode(int size, NodeType type)
{
    PNode node = AllocMem(size);
    if(NULL == node)
    {
        hat_log("list create, not enough memory.");
        exit(1);
    }
    hat_debug("CreateNode node:%p size:%d ", node, size);

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
        hat_log("list create, not enough memory.");
        exit(1);
    }
    cell->next = NULL;
    
    /* initial list */
    if(NULL == list)
    {
        list = (PList)CreateNode(sizeof(List), T_List);
        list->length = 0;
    }

    hat_debug("CreateCell list:%p size:%d, cell:%p size:%d ", list,sizeof(List), cell, sizeof(ListCell));

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
        hat_log("list create, not enough memory.");
        exit(1);
    }
    cell->next = NULL;
    cell->value.pValue = n;

    hat_debug("AppendCell list:%p size:%d, cell:%p size:%d ", list,sizeof(List), cell, sizeof(ListCell));

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
 *  merge to new list, which different from list1 and list2. 
 */
PList MergeList(PList list1, PList list2)
{
    PListCell tmpCell = NULL;
    PList tempList = NULL;
    int listLen = 0;

    if(NULL != list1)
    {
        /* double list are not null here. */
        for(tmpCell = list1->head, listLen = 0; (tmpCell != NULL) && (listLen < list1->length); tmpCell = tmpCell->next, listLen++)
        {
            tempList = AppendCellNode(tempList, tmpCell);
        }
    }

    if(NULL != list2)
    {
        for(tmpCell = list2->head, listLen = 0; (tmpCell != NULL) && (listLen < list2->length); tmpCell = tmpCell->next, listLen++)
        {
            tempList = AppendCellNode(tempList, tmpCell);
        }
    }

    return tempList;
}

/* 获取第几个节点 */
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

/* 
 * Append the n node at the tail of list, 
 * when list is NULL, creating at first.
 */
PList AppendCellNode(PList list, PListCell cell)
{
    if(NULL == list)
    {
        list = (PList)CreateNode(sizeof(List), T_List);
        list->length = 0;
    }
    
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

PList DelListNode(PList list, PNode n)
{
    PList tmphead = list;
    PListCell preCell = NULL, curCell = NULL;

    if((NULL == list) || (NULL == n))
        return NULL;
    
    for(curCell = list->head; curCell != NULL; curCell = curCell->next)
    {
        if(curCell->value.pValue == n)
        {
            break;
        }
        preCell = curCell;
    }

    /* not found */
    if(curCell == NULL)
        return tmphead;

    /* found */
    if(preCell == NULL)
    {
        /* list head node will be removed. */
        if(tmphead->length == 1)
        {
            /* empty */
            tmphead->head = tmphead->tail = NULL;
            tmphead->length = 0;
        }
        else
        {
            /* change head linker */
            tmphead->head = tmphead->head->next;
            tmphead->length -= 1;
        }                
    }

    /* tail node will be removed */
    if(curCell == tmphead->tail)
    {
        tmphead->tail = preCell;
        preCell->next = NULL;
    }
    else
    {
        preCell->next = curCell->next;
    }

    tmphead->length -= 1;

    return tmphead;
}



