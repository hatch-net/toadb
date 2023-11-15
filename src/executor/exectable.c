/*
 *	toadb tables executor
 * Copyright (C) 2023-2023, senllang
*/


#include "exectable.h"
#include "executor.h"
#include "tables.h"

#include "seqscan.h"
#include "tfile.h"
#include "execModifyTable.h"

/* printf snprintf */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>

#define log printf
extern char *DataDir;



/*
 * executor of table create.
 * list is a tree of parser.
*/
int ExecCreateTable(PCreateStmt stmt, PPortal portal)
{
    PColumnDef column = NULL;
    PListCell tmpCell = NULL;
    char pagebuffer[PAGE_MAX_SIZE] = {0};
    PPageDataHeader pageheader = (PPageDataHeader)pagebuffer;
    PTableMetaInfo tableinfo = (PTableMetaInfo) (pagebuffer + PAGE_DATA_HEADER_SIZE);
    int i = 0;
    int tablefile = -1;
    TableList tbl_temp = {0};
    int ret = 0;
    int tableType = 0;
    
    if(NULL == stmt)
    {
        log("create table stmt is NULL\n");
        return -1;
    }

#ifdef STORAGE_FORMAT_NSM
    tableType = ST_NSM << STORAGE_TYPE_SHIFT;
#endif
#ifdef STORAGE_FORMAT_PAX
    tableType = ST_PAX << STORAGE_TYPE_SHIFT;
#endif

    /* initialize table infomastion to scan list */
    pageheader->header.pageVersion = PAGE_VERSION;
    pageheader->header.pageType = PAGE_HEADER | tableType;
    pageheader->header.pageNum = PAGE_HEAD_PAGE_NUM;
    pageheader->pageCnt = PAGE_HEAD_PAGE_NUM;

    snprintf(tableinfo->tableName, NAME_MAX_LEN, "%s", stmt->tableName);
    tableinfo->tableType = tableType;
    tableinfo->tableId = GetObjectId();
    
    /* column info */
    for(tmpCell = stmt->ColList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        column = (PColumnDef)(tmpCell->value.pValue);
        if(NULL == column)
        {
            log("exec create %s table, column info invalid\n", tableinfo->tableName);
            return -1;
        }

        snprintf(tableinfo->column[i].colName, NAME_MAX_LEN, "%s", column->colName);
        tableinfo->column[i].type = GetColumnType(column->colType);
        if(tableinfo->column[i].type < 0)
        {
            log("exec create %s table, column type %s invalid\n", tableinfo->tableName, column->colType);
            return -1;
        }

        i++;
    }
    tableinfo->colNum = i;
    pageheader->dataOffset = PAGE_DATA_HEADER_SIZE;
    pageheader->dataEndOffset = sizeof(TableMetaInfo) + sizeof(ColumnDefInfo) * i;

    /* create table file */
    tablefile = CreateTableFile(tableinfo->tableName, 0666);
    if(tablefile < 0)
    {
        log("exec create %s table failure.\n", tableinfo->tableName);
        return -1;
    }

    /* initialize table file */
    tbl_temp.tbl_fd = tablefile;
    ret = FlushBuffer(&tbl_temp, pagebuffer);
    if(ret < 0)
    {
        log("exec create %s table failure,errno[%d].\n", tableinfo->tableName, ret);
        return -1;
    }
    
    smgrClose(tbl_temp.tbl_fd);
    
    /* init group file and open */
    ret = TableCreate(tableinfo->tableName, GROUP_FORK);
    if (ret < 0)
    {
        log("create %s group file failure.\n", tableinfo->tableName);
    }

    return ret;
}


int ExecDropTable(PDropStmt stmt, PPortal portal)
{
    int ret = 0;
    PTableList tblInfo = NULL;

    if(NULL == stmt)
    {
        log("drop table stmt is NULL\n");
        return -1;
    }

    /* find file */
    tblInfo = GetTableInfo(stmt->tableName);
    if(NULL == tblInfo)
    {
        log("drop table %s failure, It's not exist.\n",stmt->tableName);
        return -1;
    }

    /* delete file */
    ret = TableDrop(tblInfo);
    
    ReleaseTblInfo(tblInfo);
    return ret;
}

int ExecInsertStmt(PInsertStmt stmt, PPortal portal)
{
    PTableList tblInfo = NULL;
    PTableRowData rowDataInsert = NULL;
    int ret = 0;

    if(NULL == stmt)
    {
        log("insert table stmt is NULL\n");
        return -1;
    }

    /* initial execute infomation about table. */
    tblInfo = GetTableInfo(stmt->tableName);
    if(NULL == tblInfo)
    {
        log("insert table failure.\n");
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
        log("select table stmt is NULL\n");
        return -1;
    }

    if(stmt->relrange == NULL)
    {
        log("table reference is null. \n");
        return 0;
    }

    memset(&scan, 0x00, sizeof(Scan));

    /* create portal, which will store all rows. */
    if(InitSelectPortal(stmt->targetlist, portal) < 0)
    {
        log("select excutor failure.\n");
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


/* 
 * Merge two row into struct PScanTableRowData. 
 */
PTableRowData ExecMergeRowData(PExecState eState)
{
    PScanTableRowData scanTblRowInfo = NULL, rightRow = NULL, leftRow = NULL;;
    PTableRowDataPosition *tblRow = NULL;
    int tableNum = 0;
    int index = 0;

    leftRow = (PScanTableRowData)eState->scanRowDataLeft;
    rightRow = (PScanTableRowData)eState->scanRowDataRight;
    
    if(NULL != leftRow)
    {
        tableNum = leftRow->tableNum;
    }

    if(NULL != rightRow)
    {
        tableNum += rightRow->tableNum;
    }

    if(tableNum < 1)
    {
        return NULL;
    }

    scanTblRowInfo = (PScanTableRowData)AllocMem(sizeof(ScanTableRowData) + (tableNum -1) *sizeof(PTableRowDataPosition));
    tblRow = &(scanTblRowInfo->tableRowData);

    if(NULL != leftRow)
    {
        tblRow[index] = leftRow->tableRowData;
        index ++;
    }

    if(NULL != rightRow)
    {
        tblRow[index] = rightRow->tableRowData;
    }

    scanTblRowInfo->tableNum = tableNum;
    return (PTableRowData)scanTblRowInfo;
}

int ExecRowDataCompare(PExecState eState)
{
    return 0;
}
