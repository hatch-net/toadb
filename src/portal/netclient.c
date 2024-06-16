/*
 *	toadb netclient
 *
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

#include "netclient.h"
#include "queryNode.h"


/* format head line */
int GeneralVirtualTableHeadRaw(PPortal portal)
{
    PListCell tmpCell = NULL;
    char *pbuf = portal->buffer;
    int offset = 0;

    PTargetEntry targetEntry = NULL;
    PResTarget resTarget = NULL;
    PColumnRef columnRefNode = NULL;

    char *colName = NULL;

    if((NULL == portal) || (NULL == portal->targetList))
        return -1;
        
    for(tmpCell = portal->targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        targetEntry = (PTargetEntry)GetCellNodeValue(tmpCell);
        resTarget = (PResTarget)targetEntry->colRef;
        columnRefNode = (PColumnRef)resTarget->val;

        if(resTarget->name)
            colName = resTarget->name;
        else 
            colName = columnRefNode->field;

        snprintf(pbuf + offset, PORT_BUFFER_SIZE - offset, "|%s", colName);
        offset = strlen(pbuf);
    }

    snprintf(pbuf + offset, PORT_BUFFER_SIZE - offset, "|");

    return offset+2;
}

/*
 * send message of the portal
*/
int WritePortalMessage(PPortal portal)
{
    int len = 0;

    if(NULL == portal)
        return -1;
    
    if(portal->clientFd <= 0)
        return -2;
    
    len = WriteData(portal->clientFd, &portal->msgBody, portal->msgBody.size+MSG_HEADER_LEN);

    return len;
}


/* format rows data */
int ClientRowDataSend(PPortal portal)
{
    PTableRowData rawRow = NULL;

    char *pbuf = portal->buffer;
    int index = 0;
    int bufOffset = 0;
    int width = 0;
    
    /* collect max size per column data */
    rawRow = portal->rowData;
    if(rawRow == NULL)
        return 0;

    for(index = 0; index < rawRow->num; index ++)
    {
        width = GetValueInfo(portal->targetValTypeArr[index], rawRow->columnData[index], pbuf+bufOffset, 0);
        bufOffset += width + 1;
    }

    snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|");

    return bufOffset + 2;
}

int PortalSendRows(PPortal portal)
{
    int ret = 0;

    ret = ClientRowDataSend(portal);
    return ret;
}