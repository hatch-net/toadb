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

int fillBack(char *buf, char op, int size)
{
    if(size <= 0)
        return size;
    memset(buf, op, size);
    return size;
}

typedef enum SHOW_PHARE
{
    SHOW_PHARE_ROW_SIZE,
    SHOW_PHARE_SHOW_DATA,
    SHOW_PHARE_MAX
}SHOW_PHARE;

int ClientFormRow(PScan scanHead, PSelectStmt stmt)
{
    char *pbuf = scanHead->portal->buffer;
    int bufOffset = 0;
    int *rowMaxSize = NULL;
    PScanState scantbl = NULL;
    PDList node = NULL;
    PDList rownode = NULL;
    PTableRowData rawRow = NULL;
    PColumnDefInfo colDef = NULL;

    PValuesData attrData = NULL;
    int size = 0;
    int index = 0;
    int attrIndex = 0;
    int first = 1;
    int showPhare = SHOW_PHARE_ROW_SIZE;
    int showNum = 0;

    memset(pbuf, 0x00, PORT_BUFFER_SIZE);

    /* rows order by select */
    for(node = scanHead->list; (first || node != scanHead->list) && node != NULL; node = node->next)
    {
        scantbl = (PScanState)(((PDLCell)node)->value);
        colDef = scantbl->tblInfo->tableDef->column;
        bufOffset = 0;
        showPhare = SHOW_PHARE_ROW_SIZE;

        do
        {
            first = 1;
            showNum = 0;

            if(scantbl->rows == NULL )
            {
                first = 0;
                /* no rows to show. */
                break;
            }

           /* two show phare , first collect max column size, then show . */
            if(showPhare)
            {
                for(attrIndex = 0; attrIndex < scantbl->tblInfo->tableDef->colNum; attrIndex ++)
                {
                    snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%s", colDef[attrIndex].colName);
                    size = strlen(colDef[attrIndex].colName);
                    bufOffset += size + 1;

                    /* value len is smaller than title name len. */
                    rowMaxSize[attrIndex] = rowMaxSize[attrIndex] > size ? rowMaxSize[attrIndex] : size;

                    fillBack(pbuf+bufOffset, '-', rowMaxSize[attrIndex] - size);
                    bufOffset += rowMaxSize[attrIndex] - size;
                }
                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|");
                SendToPortal(scanHead->portal); 
                bufOffset = 0;
            }

            for(rownode = scantbl->rows; (first ||rownode != scantbl->rows) && rownode != NULL; rownode = rownode->next)
            {
                first = 0;
                rawRow = (PTableRowData)(((PDLCell)rownode)->value);

                if(rowMaxSize == NULL)
                {
                    rowMaxSize = (int*) AllocMem(rawRow->num * sizeof(int));
                    memset(rowMaxSize, 0x00, rawRow->num * sizeof(int));
                }                

                for(attrIndex = 0; attrIndex < rawRow->num; attrIndex ++)
                {
                    switch(colDef[attrIndex].type)
                    {
                        case INT:
                        case INTEGER:
                        {
                            int *tmp = NULL;
                            char digit[128];
                            int size = 0;
                            if(rawRow->columnData[attrIndex] != NULL)
                            {
                                tmp = (int *)(rawRow->columnData[attrIndex]->data);
                                snprintf(digit, 128, "%d", *tmp);
                                size = strlen(digit);
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*d", rowMaxSize[attrIndex], *tmp);  
                                bufOffset += rowMaxSize[attrIndex] + 1;                              
                            }
                            else
                            {
                                if(size > rowMaxSize[attrIndex])
                                    rowMaxSize[attrIndex] = size + 1;
                            }
                        }
                        break;
                        case VARCHAR:
                        {
                            int size = 0;
                            char *data = NULL;
                            if (rawRow->columnData[attrIndex] != NULL)
                            {
                                data = rawRow->columnData[attrIndex]->data;
                                size = rawRow->columnData[attrIndex]->size;
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*s", rowMaxSize[attrIndex], data);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                            {
                                if(size > rowMaxSize[attrIndex])
                                    rowMaxSize[attrIndex] = size;
                                
                            }
                        }
                        break;
                        case CHAR:
                        {
                            char data = '\0';
                            if (rawRow->columnData[attrIndex] != NULL)
                            {
                                data = rawRow->columnData[attrIndex]->data[0];
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*c", rowMaxSize[attrIndex],data);
                                // fillBack(pbuf+bufOffset+2, ' ', rowMaxSize[attrIndex] - 1);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                                rowMaxSize[attrIndex] = sizeof(char);
                        }
                        break;
                        case BOOL:
                        {
                            char data = '\0';
                            if (rawRow->columnData[attrIndex] != NULL)
                            {
                                data = rawRow->columnData[attrIndex]->data[0];
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*c", rowMaxSize[attrIndex],data);
                                // fillBack(pbuf+bufOffset+2, ' ', rowMaxSize[attrIndex] - 1);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                                rowMaxSize[attrIndex] = sizeof(char);
                        }
                        break;
                        default:
                        break;
                    }                    
                }
                if(showPhare)
                {
                    snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|");
                    SendToPortal(scanHead->portal);
                    bufOffset = 0;
                    showNum ++;
                }
            }

            showPhare++;
        }while(showPhare < SHOW_PHARE_MAX);

        snprintf(pbuf, PORT_BUFFER_SIZE , "total %d rows", showNum);
        SendToPortal(scanHead->portal);

        if(NULL != rowMaxSize)
        {
            FreeMem(rowMaxSize);
            rowMaxSize = NULL;
        }
    }
 
    return 0;
}

/*
 * select 执行入口
 */
int ExecSelectStmt(PSelectStmt stmt, PPortal portal)
{
    PListCell tmpCell = NULL;
    Scan scanState;
    int num = 0;

    if(NULL == stmt)
    {
        log("select table stmt is NULL\n");
        return -1;
    }

    if(stmt->tblList == NULL)
    {
        log("table reference is null. \n");
        return 0;
    }

    memset(&scanState, 0x00, sizeof(Scan));

    /* create portal, which will store all rows. */
    InitSelectPortal(stmt, portal);
    scanState.portal = portal;

    /* we will scan all tables, from table head to table tail. */
    for(tmpCell = stmt->tblList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PTableRefName node = (PTableRefName)(tmpCell->value.pValue);
        num = ScanOneTblRows(node->tblRefName, &scanState);
        portal->num = num;
    }

    /* form temp row data , which will be shown on client. */
    ClientFormRow(&scanState, stmt);

    return 0;
}
