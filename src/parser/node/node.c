/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>
#include "node.h"

#define log printf


PNode CreateNode(int size, NodeType type)
{
    PNode node = malloc(sizeof(size));
    if(NULL == node)
    {
        log("list create, not enough memory.\n");
        exit(1);
    }

    node->type = type;
}

PList CreateCell(PList list)
{
    /* new cell */
    PListCell cell = malloc(sizeof(ListCell));
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


static void TravelListCell(PList list)
{
    PListCell tmpCell = NULL;

    if(NULL == list)
    {
        log("NULL tree\n");
        return;
    }

    /* list node show */
    log("T_List Node: length:%d \n", list->length);

    /* list cell node show */
    for(tmpCell = list->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)(tmpCell->value.pValue);
        switch(node->type)
        {
            case T_List:
                TravelListCell((PList)node);
            break;
            case T_CreateStmt:
                {
                    PCreateStmt createstmt = (PCreateStmt)node;
                    log("T_CreateStmt Node: tablename:%s \n", createstmt->tableName);
                    TravelListCell(createstmt->ColList);
                }
            break;
            case T_ColumnDef:
                {
                    PColumnDef column = (PColumnDef)node;
                    log("T_ColumnDef Node: columnName:%s type:%s \n", column->colName,column->colType);
                }
            break;
            default:
            break;
        }
    }

    return;
}

void travelParserTree(PList list)
{
    TravelListCell(list);
    return ;
}
