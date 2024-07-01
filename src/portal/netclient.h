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

#ifndef HAT_NET_CLIENT_H_H
#define HAT_NET_CLIENT_H_H

#include "portal.h"

typedef enum ClientStatus
{
    CS_IDLE,
    CS_SENDCOMMD,
    CS_READ,
    CS_FINISH
}ClientStatus;

#define CS_CLIENT_STATE_MASK    (0x0000FFFF)
#define CS_BLOCK                (0x01000000)

typedef struct ClientContext 
{
    ClientStatus clientStatus;          /* low 2 Bytes store ClientStatus
                                         * High 2 Bytes store msgstatus(CS_BLOCK)
                                        */

    int restLen;
    char restMsg[PORT_BUFFER_SIZE+PORT_BUFFER_SIZE];     /* rest of recv msg. double recv buffer */

    MsgHeader msg;
}ClientContext, *PClientContext;

int GeneralVirtualTableHeadRaw(PPortal portal);

int WritePortalMessage(PPortal portal);

int ClientRowDataSend(PPortal portal);

int PortalSendRows(PPortal portal);

int ParserMsg(char *msg, int len, PClientContext client);
#endif 
