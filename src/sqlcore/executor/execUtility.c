/*
 *	toadb execUtilty
 * Copyright (c) 2023-2024 senllang
 * 
 * toadb is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/

#include <stdio.h>
#include "execUtility.h"
#include "parserNode.h"
#include "queryNode.h"
#include "exectable.h"
#include "execTransaction.h"


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
            hat_log("exec T_CreateStmt Node: tablename:%s ", createstmt->tableName);
            ret = ExecCreateTable(createstmt, portal);
            snprintf(pbuf, PORT_BUFFER_SIZE, "Create table result %s", ret == 0? "success":"failure");
        }
        break;
        case T_DropStmt:
        {
            PDropStmt dropstmt = (PDropStmt)parserNode;
            hat_log("exec T_DropStmt Node: drop table: %s ", dropstmt->tableName);
            ret = ExecDropTable(dropstmt, portal);
            snprintf(pbuf, PORT_BUFFER_SIZE, "Drop table result %s", ret == 0? "success":"failure");
        }
        break;
        case T_TransactionStmt:
        {
            PTransactionStmt transtmt = (PTransactionStmt)parserNode;
            hat_log("exec T_TransactionStmt transaction command: %d ", transtmt->transactionTag);

            ret = ExecTransaction(transtmt, portal);
            if(ret != 0)
                snprintf(pbuf, PORT_BUFFER_SIZE, "result failure.");
        }
        break;
        default:
            hat_log("exec unknow command %d ", parserNode->type);
            snprintf(pbuf, PORT_BUFFER_SIZE, "exec unknow command %d ", parserNode->type);
        break;
    }

    return ;
}