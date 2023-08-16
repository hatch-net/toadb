/*
 *	toadb table modify executor 
 * Copyright (C) 2023-2023, senllang
*/
#include "execModifyTable.h"
#include "tfile.h"
#include "tables.h"
#include "node.h"
#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define log printf

/* 
 * routine of modify table operators 
 */
int ExecModifyTable(PTableList tblInfo, PTableRowData insertdata, NodeType type)
{
    int ret = 0;
    int tabletype = GET_STORAGE_TYPE_SHIFT(tblInfo->tableInfo->header.pageType);

    switch(tabletype)
    {
        case ST_NSM:
            ret = nsm_ExecModifyTable(tblInfo, insertdata, type);
            break;
        case ST_PAX:
            ret = pax_ExecModifyTable(tblInfo, insertdata, type);
            break;
        default:
            log("unknow table storage type %d. \n", tabletype);
            ret = -1;
            break;
    }

    return ret;
}

/* 
 * routine of modify table operators , under NSM storage 
 */
int nsm_ExecModifyTable(PTableList tblInfo, PTableRowData insertdata, NodeType type)
{
    int ret = 0;

    switch(type)
    {
        case T_InsertStmt:
            ret = nsm_ExecInsert(tblInfo, insertdata);
        break;
        default:
            log("unsuport node type %d. \n", type);
            ret = -1;
        break;
    }

    return ret;
}

int nsm_ExecInsert(PTableList tblInfo, PTableRowData insertdata)
{
    int ret = 0;
    PPageDataHeader pageInsert = NULL;

    /* there we find free space. */
    pageInsert = GetSpacePage(tblInfo, insertdata->size, PAGE_NEW, MAIN_FORK);

    /* row data will be writed through to the table file. */
    ret = WriteRowData(tblInfo, pageInsert, insertdata);
    if(ret != 0)
    {
        log("write row to page failure.[%d]\n", ret);
    }
    return ret;
}


/* 
 * routine of modify table operators , under PAX storage 
 */
int pax_ExecModifyTable(PTableList tblInfo, PTableRowData insertdata, NodeType type)
{
    int ret = 0;

    switch(type)
    {
        case T_InsertStmt:
            ret = pax_ExecInsert(tblInfo, insertdata);
        break;
        default:
            log("unsuport node type %d of PAX storage . \n", type);
            ret = -1;
        break;
    }

    return ret;
}

int pax_ExecInsert(PTableList tblInfo, PTableRowData insertdata)
{
    int ret = 0;
    PPageDataHeader *pageList = NULL;
    PTableRowData colRows = NULL; 
    PTableRowData currRows = NULL;
    PPageDataHeader page = NULL;
    int ColNum = 0;
    int itemsize = sizeof(TableRowData) + sizeof(PRowColumnData);

    /* 
     * find free space
     * several pages will insert columns, and these are in one group. 
    */
    pageList = (PPageDataHeader *)AllocMem(sizeof(PPageDataHeader)*insertdata->num);

    /* find enough free space group page */
    do
    {
        ret = GetSpaceGroupPage(tblInfo, insertdata, PAGE_NEW, pageList);
        if(ret < 0)
        {
            /* current groups has not enough free space, we extension a new group. */
            page = ExtensionTbl(tblInfo, insertdata->num, MAIN_FORK);

            /* update group file, insert one group data */
            InsertGroupItem(tblInfo, page, insertdata->num);

            FreeMem(page);
            page = NULL;
        }
    } while (ret < 0);

    /* Now, to form rows column by column. */
    colRows = FormColRowsData(insertdata);

    /* insert into pages */
    for(ColNum = 0; ColNum < insertdata->num; ColNum++)
    {
        currRows = (PTableRowData)((char *) colRows + ColNum * itemsize);
        ret = WriteRowItemData(tblInfo, pageList[ColNum], currRows);
    }

    /* resource release */
    ReleasePageList(pageList, insertdata->num);
    
    return 1;
}


