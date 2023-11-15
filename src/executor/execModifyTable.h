/*
 *	toadb table modify executor 
 * Copyright (C) 2023-2023, senllang
*/
#ifndef HAT_EXEC_MODIFY_TABLE_H_H
#define HAT_EXEC_MODIFY_TABLE_H_H

#include "node.h"
#include "buffer.h"
#include "executor.h"

/* routing Modify table operator,  excutor will call here. */
PTableRowData ExecTableModifyTbl(PExecState eState);

/* routing storage type operator */
int ExecModifyTable(PTableList tblInfo, PTableRowData insertdata, NodeType type); 

/* NSM storage operator */
int nsm_ExecModifyTable(PTableList tblInfo, PTableRowData insertdata, NodeType type);
int nsm_ExecInsert(PTableList tblInfo, PTableRowData insertdata);

/* PAX storage operators */
int pax_ExecModifyTable(PTableList tblInfo, PTableRowData insertdata, NodeType type);
int pax_ExecInsert(PTableList tblInfo, PTableRowData insertdata);



#endif
