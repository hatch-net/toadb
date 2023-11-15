/*
 *	toadb execUtilty
 * Copyright (C) 2023-2023, senllang
*/

#include <stdio.h>
#include "execUtility.h"
#include "parserNode.h"
#include "queryNode.h"
#include "exectable.h"

#define log printf

void ExecutorUtility(PPlan plan, PPortal portal)
{
    PQuery query = (PQuery)plan->QueryTree;
    PNode parserNode = query->parserTree;
    char *pbuf = NULL;
    int ret = 0;

    pbuf = portal->buffer;

    switch(parserNode->type)
    {
        case T_CreateStmt:
        {
            PCreateStmt createstmt = (PCreateStmt)parserNode;
            log("exec T_CreateStmt Node: tablename:%s \n", createstmt->tableName);
            ret = ExecCreateTable(createstmt, portal);
            snprintf(pbuf, PORT_BUFFER_SIZE, "Create table result %s", ret == 0? "success":"failure");
        }
        break;
        case T_DropStmt:
        {
            PDropStmt dropstmt = (PDropStmt)parserNode;
            log("exec T_DropStmt Node: drop table :%s \n", dropstmt->tableName);
            ret = ExecDropTable(dropstmt, portal);
            snprintf(pbuf, PORT_BUFFER_SIZE, "Drop table result %s", ret == 0? "success":"failure");
        }
        break;
        default:
            log("exec unknow command %d \n", parserNode->type);
            snprintf(pbuf, PORT_BUFFER_SIZE, "exec unknow command %d \n", parserNode->type);
        break;
    }

    return ;
}