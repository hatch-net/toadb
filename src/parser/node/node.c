/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

#define log printf
#define debug 

PNode CreateNode(int size, NodeType type)
{
    PNode node = malloc(size);
    if(NULL == node)
    {
        log("list create, not enough memory.\n");
        exit(1);
    }
    debug("CreateNode node:%p size:%d \n", node, size);

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
                log("T_List Node: len:%d \n", ((PList)node)->length);
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
            case T_DropStmt:
                {
                    PDropStmt dropstmt = (PDropStmt)node;
                    log("T_DropStmt Node: drop table :%s \n", dropstmt->tableName);
                }
            break;
            case T_InsertStmt:
                {
                    PInsertStmt insertstmt = (PInsertStmt)node;
                    log("T_InsertStmt Node: table :%s \n", insertstmt->tableName);
                    TravelListCell(insertstmt->attrNameList);
                    TravelListCell(insertstmt->valuesList);
                }
                break;
            case T_AttrName:
                {
                    PAttrName stmt = (PAttrName)node;
                    log("T_AttrName Node: attrname :%s \n", stmt->attrName);
                }
                break;
            case T_ValuesData:
                {
                    PValuesData stmt = (PValuesData)node;
                    log("T_ValuesData Node:  \n");
                }
                break;
            case T_SelectStmt:
                {
                    PSelectStmt stmt = (PSelectStmt)node;
                    log("T_SelectStmt Node: selectAll:%d \n", stmt->selectAll);

                    TravelListCell(stmt->columnList);
                    TravelListCell(stmt->tblList);
                }
                break;
            case T_TableRefName:
                {
                    PTableRefName stmt = (PTableRefName)node;
                    log("T_TableRefName Node: table ref name:%s \n", stmt->tblRefName);
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

/* 
 * 根据表名获取取 attrnode 
 * list , List *attrNameList, cell is AttrName
 */
int GetAtrrIndexByName(char *attrName, PList list)
{
    int index = 0;
    PAttrName attr = NULL;
    PListCell cell = NULL;

    if(NULL == list || NULL == attrName)
        return -1;
    
    cell = list->head;
    for(index = 1; cell != NULL; cell = cell->next, index++)
    {
        attr = (PAttrName)(cell->value.pValue);
        if(strcmp(attrName, attr->attrName) == 0)
        {
            break;
        }
    }

    return index;
}

/* 
 * 根据atrrnode index， 获取对应的values 
 * list , cell is ValuesData type.
 */
PValuesData GetDataByIndex(int index, PList list)
{
    PValuesData data = NULL;
    PListCell cell = NULL;
    int count = 0;

    if(NULL == list)
        return data;
    
    cell = list->head;
    for(count = 1; cell != NULL; cell = cell->next, count++)
    {
        data = (PValuesData)(cell->value.pValue);
        if(count == index)
        {
            break;
        }
    }

    return data;
}