/*
 *	toadb table modify executor 
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
#ifndef HAT_EXEC_MODIFY_TABLE_H_H
#define HAT_EXEC_MODIFY_TABLE_H_H

#include "node.h"
#include "buffer.h"
#include "executor.h"

/* routing Modify table operator,  excutor will call here. */
PTableRowData ExecTableModifyTbl(PExecState eState);

/* routing storage type operator */
int ExecModifyTable(PExecState eState, PTableList tblInfo, PTableRowData insertdata, NodeType type); 

/* NSM storage operator */
int nsm_ExecModifyTable(PTableList tblInfo, PTableRowData insertdata, NodeType type);
int nsm_ExecInsert(PTableList tblInfo, PTableRowData insertdata);

/* PAX storage operators */
int pax_ExecModifyTable(PExecState eState, PTableList tblInfo, PTableRowData rowsdata, NodeType type);
int pax_ExecInsert(PExecState eState, PTableList tblInfo, PTableRowData insertdata);
int pax_ExecUpdate(PExecState eState, PTableList tblInfo, PTableRowData updateDataPos);
int pax_ExecDelete(PExecState eState, PTableList tblInfo, PTableRowData delDataPos);

#endif
