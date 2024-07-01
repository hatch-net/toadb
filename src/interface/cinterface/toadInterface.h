/*
 *	toadb c language interface
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
#ifndef HAT_CINTERFACE_H_H
#define HAT_CINTERFACE_H_H

#include "netclient.h"
#include "tcpsock.h"

#define MAX_EVENT_SIZE 2

typedef struct ConnectionContext 
{
    TCPContext tcpCxt;
    ClientContext clientCxt;
}ConnectionContext, *PConnectionContext;

int InitToadbInterface(PConnectionContext conn);
int DestoryToadbInterface(PConnectionContext conn);

int ConnectToadbServer(PConnectionContext conn, char *Addr, int port);
int CloseConnectionToadb(PConnectionContext conn);

int ExecQery(PConnectionContext conn, char *sql);

#endif 