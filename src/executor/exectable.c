/*
 *	toadb tables executor
 * Copyright (C) 2023-2023, senllang
*/


#include "exectable.h"
#include "executor.h"
#include "tables.h"
#include "portal.h"
#include "seqscan.h"

/* printf snprintf */
#include <stdio.h>
#include <string.h>

/* file operator */
#include <fcntl.h>           /* Definition of AT_* constants */
#include <unistd.h>

#include <errno.h>

#define log printf
extern char *DataDir;

static int CreateTableFile(char *filename, int mode);
static int DeleteTableFile(char *filename);

/*
 * executor of table create.
 * list is a tree of parser.
*/
int ExecCreateTable(PCreateStmt stmt)
{
    PColumnDef column = NULL;
    PListCell tmpCell = NULL;
    char pagebuffer[PAGE_MAX_SIZE] = {0};
    PPageDataHeader pageheader = (PPageDataHeader)pagebuffer;
    PTableMetaInfo tableinfo = (PTableMetaInfo) (pagebuffer + sizeof(PageDataHeader));
    int i = 0;
    int tablefile = -1;

    if(NULL == stmt)
    {
        log("create table stmt is NULL\n");
        return -1;
    }

    /* initialize table infomastion to scan list */
    pageheader->header.pageVersion = PAGE_VERSION;
    pageheader->header.pageType = PAGE_HEADER;
    pageheader->header.pageNum = 0x01;

    snprintf(tableinfo->tableName, NAME_MAX_LEN, "%s", stmt->tableName);

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

    /* create table file */
    tablefile = CreateTableFile(tableinfo->tableName, 0666);
    if(tablefile < 0)
    {
        log("exec create %s table failure.\n", tableinfo->tableName);
        return -1;
    }

    /* initialize table file */
    write(tablefile, pagebuffer, PAGE_MAX_SIZE);

    /* end of executor, close and release resources */
    close(tablefile);

    return 0;
}

static int CreateTableFile(char *filename, int mode)
{
    int fd;
    char filepath[1024];

    snprintf(filepath, 1024, "%s/%s", DataDir, filename);

    // 检查文件是否存在
    if (access(filepath, F_OK) == 0) 
    {
        log("table file %s already exist. \n", filepath);
        return -1;
    }

    // 以二进制形式打开文件
    fd = open(filepath, O_RDWR | O_CREAT, mode);
    if (fd == -1) 
    {
        log("create file %s error, maybe space not enough\n", filepath);
        return -1;
    }

    return fd;
}

int ExecDropTable(PDropStmt stmt)
{
    int ret = 0;
    if(NULL == stmt)
    {
        log("drop table stmt is NULL\n");
        return -1;
    }

    /* find file */
    /* delete file */
    ret = DeleteTableFile(stmt->tableName);
    if(0 != ret)
    {
        log("exec drop %s table failure.\n", stmt->tableName);
        return -1;
    }
    return ret;
}

static int DeleteTableFile(char *filename)
{
    int ret = 0;
    char filepath[1024];

    snprintf(filepath, 1024, "%s/%s", DataDir, filename);

    // 检查文件是否存在
    if (access(filepath, F_OK) != 0) 
    {
        log("table file %s is not exist. \n", filepath);
        return -1;
    }

    // 以二进制形式打开文件
    ret = unlink(filepath);
    if (ret != 0) 
    {
        log("create file %s ,errno %d \n", filepath, errno);
        return -1;
    }

    return ret;
}

int OpenTableFile(char *filename, int mode)
{
    int fd;
    char filepath[1024];

    snprintf(filepath, 1024, "%s/%s", DataDir, filename);

    // 检查文件是否存在
    if (access(filepath, F_OK) != 0) 
    {
        log("table file %s is not exist. \n", filepath);
        return -1;
    }

    // 以二进制形式打开文件
    fd = open(filepath, O_RDWR, mode);
    if (fd == -1) 
    {
        log("open file %s error, errno[%d]\n", filepath, errno);
        return -1;
    }

    return fd;
}

int ExecInsertStmt(PInsertStmt stmt)
{
    PTableList tblInfo = NULL;
    PTableRowData rowDataInsert = NULL;
    PPageDataHeader pageInsert = NULL;
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

    /* there we find free space. */
    pageInsert = GetSpacePage(tblInfo, rowDataInsert->size, PAGE_NEW);

    /* row data will be writed through to the table file. */
    ret = WriteRowData(tblInfo, pageInsert, rowDataInsert);
    if(ret != 0)
    {
        log("write row to page failure.[%d]\n", ret);
    }

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
                            if(showPhare)
                            {
                                int *tmp = (int *)(rawRow->columnData[attrIndex]->data);
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*d", rowMaxSize[attrIndex], *tmp);  
                                bufOffset += rowMaxSize[attrIndex] + 1;                              
                            }
                            else
                                rowMaxSize[attrIndex] = sizeof(int);
                        }
                        break;
                        case VARCHAR:
                        {
                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*s", rowMaxSize[attrIndex], rawRow->columnData[attrIndex]->data);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                            {
                                if(strlen(rawRow->columnData[attrIndex]->data) > rowMaxSize[attrIndex])
                                    rowMaxSize[attrIndex] = strlen(rawRow->columnData[attrIndex]->data);
                                
                            }
                        }
                        break;
                        case CHAR:
                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*c", rowMaxSize[attrIndex],rawRow->columnData[attrIndex]->data[0]);
                                // fillBack(pbuf+bufOffset+2, ' ', rowMaxSize[attrIndex] - 1);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                                rowMaxSize[attrIndex] = sizeof(char);
                        break;
                        case BOOL:
                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*c", rowMaxSize[attrIndex],rawRow->columnData[attrIndex]->data[0]);
                                // fillBack(pbuf+bufOffset+2, ' ', rowMaxSize[attrIndex] - 1);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                                rowMaxSize[attrIndex] = sizeof(char);
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

        snprintf(pbuf, PORT_BUFFER_SIZE , "return %d rows", showNum);
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
int ExecSelectStmt(PSelectStmt stmt)
{

    PListCell tmpCell = NULL;
    Scan scanState;
    PPortal portal = NULL;
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
    portal = CreatePortal(stmt);
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


   // EndPort(portal);

    return 0;
}
