/*
 *	toadb portal
 * Copyright (C) 2023-2023, senllang
*/

#include "portal.h"
#include "buffer.h"

#include <stdio.h>
#include <string.h>


PPortal CreatePortal(PSelectStmt stmt)
{
    PPortal portal = NULL;
    PListCell tmpCell = NULL;
    PScanHeaderRowInfo rowInfo = NULL;

    portal = (PPortal) AllocMem(sizeof(Portal));
    memset(portal, 0x00, sizeof(Portal));

    /* general column list which ordered by stmt. */
    for(tmpCell = stmt->columnList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PAttrName node = (PAttrName)(tmpCell->value.pValue);

        rowInfo = (PScanHeaderRowInfo)AllocMem(sizeof(ScanHeaderRowInfo));
        rowInfo->colName = strdup(node->attrName);

        AddCellToListTail(&(portal->list), rowInfo);
    }

    return portal;
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
    PortalPrint(portal->buffer);

    FreeMem(portal);
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