/*
 *	toadb tables executor
 * Copyright (C) 2023-2023, senllang
*/


#include "exectable.h"
#include "executor.h"
#include "tables.h"
#include "memStack.h"

#include "seqscan.h"
#include "tfile.h"
#include "execModifyTable.h"

/* printf snprintf */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>

#define hat_log printf
extern char *DataDir;



/*
 * executor of table create.
 * list is a tree of parser.
*/
int ExecCreateTable(PCreateStmt stmt, PPortal portal)
{
    PColumnDef column = NULL;
    PListCell tmpCell = NULL;
    char *pagebuffer = NULL;
    PTableMetaInfo tblDefInfo = NULL;
    PTableList tblInfo = NULL;
    int tableType = 0;
    int i = 0;
    
    if(NULL == stmt)
    {
        hat_log("create table stmt is NULL");
        return -1;
    }

    pagebuffer = (char *)AllocMem(PAGE_MAX_SIZE);
    tblDefInfo = (PTableMetaInfo) (pagebuffer + PAGE_DATA_HEADER_SIZE);

#ifdef STORAGE_FORMAT_NSM
    tableType = ST_NSM << STORAGE_TYPE_SHIFT;
#endif
#ifdef STORAGE_FORMAT_PAX
    tableType = ST_PAX << STORAGE_TYPE_SHIFT;
#endif

    /* 1. parser table metadata */
    snprintf(tblDefInfo->tableName, NAME_MAX_LEN, "%s", stmt->tableName);
    tblDefInfo->tableType = tableType;
    
    /* column info */
    for(tmpCell = stmt->ColList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        column = (PColumnDef)(tmpCell->value.pValue);
        if(NULL == column)
        {
            hat_log("exec create %s table, column info invalid", tblDefInfo->tableName);
            return -1;
        }

        snprintf(tblDefInfo->column[i].colName, NAME_MAX_LEN, "%s", column->colName);
        tblDefInfo->column[i].type = GetColumnType(column->colType);
        if(tblDefInfo->column[i].type < 0)
        {
            hat_log("exec create %s table, column type %s invalid", tblDefInfo->tableName, column->colType);
            return -1;
        }

        tblDefInfo->column[i].attrIndex = i;

        i++;
    }
    tblDefInfo->colNum = i;

    /* 2. create table file */
    tblInfo = CreateTblInfo(tblDefInfo);
    if(NULL == tblInfo)
    {
        hat_log("exec create %s table failure.", tblDefInfo->tableName);
        return -1;
    }

    return 0;
}

int ExecDropTable(PDropStmt stmt, PPortal portal)
{
    int ret = 0;
    PTableList tblInfo = NULL;

    if(NULL == stmt)
    {
        hat_log("drop table stmt is NULL");
        return -1;
    }

    /* find file */
    tblInfo = GetTableInfo(stmt->tableName);
    if(NULL == tblInfo)
    {
        hat_log("drop table %s failure, It's not exist.",stmt->tableName);
        return -1;
    }

    /* delete file */
    ret = TableDrop(tblInfo);
    
    ReleaseTblInfo(tblInfo);
    return ret;
}

#if 0
int ExecInsertStmt(PInsertStmt stmt, PPortal portal)
{
    PTableList tblInfo = NULL;
    PTableRowData rowDataInsert = NULL;
    int ret = 0;

    if(NULL == stmt)
    {
        hat_log("insert table stmt is NULL");
        return -1;
    }

    /* initial execute infomation about table. */
    tblInfo = GetTableInfo(stmt->tableName);
    if(NULL == tblInfo)
    {
        hat_log("insert table failure.");
        return -1;
    }
    
    /* TODO check attrlist  and values number is equal. */

    /* form row data */
    rowDataInsert = FormRowData(tblInfo->tableDef, stmt);

    ret = ExecModifyTable(tblInfo, rowDataInsert, T_InsertStmt);

    FreeMem(rowDataInsert);
    return ret;
}


/*
 * select 执行入口
 */
int ExecSelectStmt(PSelectStmt stmt, PPortal portal)
{
    PListCell tmpCell = NULL;
    Scan scan;
    int num = 0;

    if(NULL == stmt)
    {
        hat_log("select table stmt is NULL");
        return -1;
    }

    if(stmt->relrange == NULL)
    {
        hat_log("table reference is null. ");
        return 0;
    }

    memset(&scan, 0x00, sizeof(Scan));

    /* create portal, which will store all rows. */
    if(InitSelectPortal(stmt->targetlist, portal) < 0)
    {
        hat_log("select excutor failure.");
        return -1;
    }
    scan.portal = portal;

    /* we will scan all tables, from table head to table tail. */
    for(tmpCell = stmt->relrange->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PTableRefName node = (PTableRefName)(tmpCell->value.pValue);
        num = ScanOneTblRows(node->tblRefName, &scan);
        portal->num = num;
    }

    /* form temp row data , which will be shown on client. */
    ClientFormRow(&scan, stmt);

    return 0;
}
#endif 


int ExecRowDataCompare(PExecState eState)
{
    return 0;
}
