/*
 *	toadb tables executor
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
#ifndef HAT_EXEC_TABLE_H_H
#define HAT_EXEC_TABLE_H_H

#include "node.h"
#include "buffer.h"
#include "list.h"
#include "portal.h"
#include "executor.h"

#define MAX_ROW_DATA 1024



typedef struct ColumnSelectList
{
    DList list;
    ColumnDef columnInfo;
    PTableList tblInfo;
}ColumnSelectList, *PColumnSelectList;




int ExecCreateTable(PCreateStmt stmt, PPortal portal);

int ExecDropTable(PDropStmt stmt, PPortal portal);

//int ExecInsertStmt(PInsertStmt stmt, PPortal portal);

//int ExecSelectStmt(PSelectStmt stmt, PPortal portal);

int OpenTableFile(char *filename, int mode);





int ExecRowDataCompare(PExecState eState);

#endif