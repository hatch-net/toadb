/*
 *	toadb executor
 * Copyright (C) 2023-2023, senllang
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "executor.h"
#include "exectable.h"


// #define log printf
#define log 

static void ExecutorPlan(PList list)
{
    PListCell tmpCell = NULL;

    if(NULL == list)
    {
        log("NULL tree\n");
        return;
    }

    /* list cell node show */
    for(tmpCell = list->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PNode node = (PNode)(tmpCell->value.pValue);
        switch(node->type)
        {
            case T_List:
                ExecutorPlan((PList)node);
            break;
            case T_CreateStmt:
                {
                    PCreateStmt createstmt = (PCreateStmt)node;
                    log("exec T_CreateStmt Node: tablename:%s \n", createstmt->tableName);
                    ExecCreateTable(createstmt);
                }
            break;
            case T_DropStmt:
                {
                    PDropStmt dropstmt = (PDropStmt)node;
                    log("exec T_DropStmt Node: drop table :%s \n", dropstmt->tableName);
                    ExecDropTable(dropstmt);
                }
            break;
            case T_InsertStmt:
                {
                    PInsertStmt insertstmt = (PInsertStmt)node;
                    log("T_InsertStmt Node: table :%s \n", insertstmt->tableName);
                    ExecInsertStmt(insertstmt);
                }
            break;

            case T_SelectStmt:
                {
                    PSelectStmt stmt = (PSelectStmt)node;
                    log("T_SelectStmt Node: \n");

                    ExecSelectStmt(stmt);
                }
            break;
            
            default:
            break;
        }
    }

    return;
}

void ExecutorMain(PList list)
{
    ExecutorPlan(list);
    return ;
}


char *ColumnType[] = 
{
    "int",
    "integer",
    "varchar",
    "char",
    "bool",
    ""
};



int GetColumnType(char *typename)
{
    int index = -1;

    for(int i = 0; i < sizeof(ColumnType)/sizeof(char*); i++)
    {
        if(strcmp(typename, ColumnType[i]) == 0)
            index = i;
    }

    return index;
}