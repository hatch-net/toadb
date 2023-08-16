/*
 *	toadb tables executor
 * Copyright (C) 2023-2023, senllang
*/
#ifndef HAT_EXEC_TABLE_H_H
#define HAT_EXEC_TABLE_H_H

#include "node.h"
#include "buffer.h"
#include "list.h"
#include "portal.h"

#define MAX_ROW_DATA 1024



typedef struct ColumnSelectList
{
    DList list;
    ColumnDef columnInfo;
    PTableList tblInfo;
}ColumnSelectList, *PColumnSelectList;



#define GetOffsetSize(member, type) (unsigned long)(((type *)(0))->member)
#define GetScanState(ptbl) ((PScanState)((unsigned long)ptbl - GetOffsetSize(tblInfo, ScanState)))



int ExecCreateTable(PCreateStmt stmt, PPortal portal);

int ExecDropTable(PDropStmt stmt, PPortal portal);

int ExecInsertStmt(PInsertStmt stmt, PPortal portal);

int ExecSelectStmt(PSelectStmt stmt, PPortal portal);

int OpenTableFile(char *filename, int mode);


#endif