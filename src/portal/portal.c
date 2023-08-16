/*
 *	toadb portal
 * Copyright (C) 2023-2023, senllang
*/

#include "portal.h"
#include "buffer.h"

#include <stdio.h>
#include <string.h>

#define log printf 

PPortal CreatePortal( )
{
    PPortal portal = NULL;

    portal = (PPortal) AllocMem(sizeof(Portal));
    memset(portal, 0x00, sizeof(Portal));

    return portal;
}

int InitSelectPortal(PSelectStmt stmt, PPortal portal)
{
    PListCell tmpCell = NULL;
    PScanHeaderRowInfo rowInfo = NULL;
    PTableList tblInfo = NULL;
    int columnNum = 0;

    if(NULL != stmt->columnList)
    {
        /* general column list which ordered by stmt. */
        for(tmpCell = stmt->columnList->head; tmpCell != NULL; tmpCell = tmpCell->next)
        {
            PAttrName node = (PAttrName)(tmpCell->value.pValue);

            rowInfo = (PScanHeaderRowInfo)AllocMem(sizeof(ScanHeaderRowInfo));
            rowInfo->colName = strdup(node->attrName);

            AddCellToListTail(&(portal->list), rowInfo);
        }

        return 0;
    }

    /* select all */
    for(tmpCell = stmt->tblList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PTableRefName node = (PTableRefName)(tmpCell->value.pValue);

        /* get table information */
        tblInfo = GetTableInfo(node->tblRefName);
        if (NULL == tblInfo)
        {
            log("select table failure.\n");
            return -1;
        }

        columnNum = tblInfo->tableDef->colNum;
        for(; columnNum > 0; columnNum--)
        {
            rowInfo = (PScanHeaderRowInfo)AllocMem(sizeof(ScanHeaderRowInfo));
            rowInfo->colName = strdup(tblInfo->tableDef->column[columnNum].colName);

            AddCellToListTail(&(portal->list), rowInfo);
        }
    }

    return 0;
}

/*
*/
int PortalPrint(char *buf)
{
    printf("%s\n", buf);
    return 0;
}

/*
 * send one row to the portal
*/
int SendToPortal(PPortal portal)
{

    PortalPrint(portal->buffer);
    memset(portal->buffer, 0x00, sizeof(portal->buffer));

    return 0;
}

int EndPort(PPortal portal)
{
    if(NULL == portal)
        return -1;

    PortalPrint(portal->buffer);

    FreeMem(portal);

    return 0;
}


/*
 * udpate col type, and max size of col. 
 */
PScanHeaderRowInfo GetRowInfoNode(PPortal portal, char *colName)
{
    PScanHeaderRowInfo rowInfo = NULL;
    PDList node = portal->list->prev;

    /* portal colInfo initialize */
    while(node != NULL && node != portal->list)
    {
        rowInfo = (PScanHeaderRowInfo)(((PDLCell)node)->value);
        if(strcmp(rowInfo->colName, colName) == 0)
            break;

        rowInfo = NULL;
        node = node->next;
    }

    return rowInfo;
}