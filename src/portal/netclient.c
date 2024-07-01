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
#include "hatstring.h"
#include "tcpsock.h"

static int GetMsgIntValue(char *buf, int *type);

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
        offset = hat_strlen(pbuf);
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
    
    len = WriteData(portal->clientFd, (char *)&portal->msgBody, portal->msgBody.size+MSG_HEADER_LEN);

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


int ParserMsg(char *msg, int len, PClientContext client)
{
    int msgOffset = 0;
    int restOffset = 0;
    int tmpLen = 0;
    int useMsg = 0;

    if(len + client->restLen < MSG_HEADER_LEN)
    {
        /* msg header is not enogh . */
        return 0;
    }

    /* first fill rest with msg header len */
    if(client->restLen < MSG_HEADER_LEN)
    {
        tmpLen = MSG_HEADER_LEN - client->restLen;
        memcpy(client->restMsg + client->restLen, msg, tmpLen);

        client->restLen += tmpLen;
        msgOffset += tmpLen;
    }

    restOffset += GetMsgIntValue(client->restMsg, &client->msg.type);
    restOffset += GetMsgIntValue(client->restMsg + restOffset, &client->msg.size);
    if(client->msg.size >= PORT_BUFFER_SIZE)
    {
        hat_error("restLen:%d len:%d size:%d", client->restLen, len, client->msg.size);
        return -1;
    }

    if(client->msg.size > (client->restLen + len - msgOffset))
    {
        useMsg = 1;
        goto RESTMSG;
    }

    /* rest context is fullfill one entire msg. */
    if((client->restLen - restOffset) < client->msg.size)
    {
        tmpLen = client->msg.size - client->restLen + restOffset;
        if(len - msgOffset >= tmpLen)
        {
            memcpy(client->restMsg + client->restLen, msg + msgOffset, tmpLen);
            client->restLen += tmpLen;
            msgOffset += tmpLen;
        }
        else 
        {
            useMsg = 1;
            goto RESTMSG;
        }
    }

    /* fill entire message */
    memcpy(client->msg.body, client->restMsg + restOffset, client->msg.size);
    restOffset += client->msg.size;
    client->restLen -= restOffset;

RESTMSG:
    if(useMsg)
    {
        /* wait context, rollback */
        restOffset -= MSG_HEADER_LEN;
        client->msg.type = 0;
        client->msg.size = 0;
    }

    /* clean restmsg */
    if((restOffset > 0) && (client->restLen > 0))
        memcpy(client->restMsg, client->restMsg + restOffset, client->restLen);

    /* save the rest msg */
    tmpLen = len - msgOffset;
    if(tmpLen > 0)
    {
        if(tmpLen > (PORT_BUFFER_SIZE - client->restLen))
        {
            client->clientStatus |= CS_BLOCK;            
        }
        else
        {
            client->clientStatus &= ~CS_BLOCK;
        }

        memcpy(client->restMsg + client->restLen, msg + msgOffset, tmpLen);
        client->restLen += tmpLen;
    }
    return client->msg.size;
}

static int GetMsgIntValue(char *buf, int *type)
{
    int typelen = sizeof(int);
    *type = *((int *) buf);
    return typelen;
}