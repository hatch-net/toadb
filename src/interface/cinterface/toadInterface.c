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

#include "toadInterface.h"
#include "toadmain.h"
#include "tcpsock.h"
#include "public.h"
#include "public_types.h"
#include "logger.h"
#include "hatstring.h"
#include "portal.h"
#include "netclient.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>


static int ReadMessage(PEventProcInfo info, MessageProc msgProc);

static int InitClientContext(PConnectionContext conn);
static int ConnectServer(PConnectionContext conn, int port, char *addr);
static int ProcessClient(PConnectionContext conn, PMsgHeader msg);

int InitToadbInterface(PConnectionContext conn)
{
    int ret = 0;

    ret = InitClientContext(conn);
    
    return ret;
}

int DestoryToadbInterface(PConnectionContext conn)
{
    return 0;
}

int ConnectToadbServer(PConnectionContext conn, char *Addr, int port)
{
    int ret = 0;
    ret = ConnectServer(conn, port, Addr);
    if(ret < 0)
    {
        hat_error("connect to server failure.");
        return -1;
    }

    conn->tcpCxt.epoll_fd = CreateEpoll();
    if(conn->tcpCxt.epoll_fd < 0)
    {
        hat_error("CreateEpoll epoll failure.");
        return -1;
    }

    ret = AddEpollREvent(conn->tcpCxt.epoll_fd, conn->tcpCxt.sock_fd);
    return ret;
}

int CloseConnectionToadb(PConnectionContext conn)
{
    if(conn->tcpCxt.sock_fd > 0)
        CloseSocket(conn->tcpCxt.sock_fd);

    if(conn->tcpCxt.epoll_fd > 0)
        CloseSocket(conn->tcpCxt.epoll_fd);
    return 0;
}

int ExecQery(PConnectionContext conn, char *sql)
{
    int ret = 0;
    MsgHeader msg;
    
    hat_strncpy(msg.body, sql, PORT_BUFFER_SIZE);
    msg.size = hat_strlen(msg.body);
    msg.type = PORT_MSG_REQUEST;

    ret = ProcessClient(conn, &msg);
    return 0;
}


static int ProcessClient(PConnectionContext conn, PMsgHeader msg)
{
    int ret = 0;
    struct epoll_event events[MAX_EVENT_SIZE];
    EventProcInfo info = {0};

    conn->clientCxt.clientStatus |= CS_SENDCOMMD;
    ret = WriteData(conn->tcpCxt.sock_fd, (char*)msg, msg->size + MSG_HEADER_LEN);
    if(ret < 0)
    {
        hat_error("send command failure.");
        return ret;
    }

    info.otherData = (void *)(&conn->clientCxt);
    conn->tcpCxt.info = &info;
    do {
        /* epoll wait write/read. */
        ret = EpollProcess(&conn->tcpCxt, events, MAX_EVENT_SIZE, -1);
    }while((ret >= 0) && ((conn->clientCxt.clientStatus & CS_CLIENT_STATE_MASK) != CS_FINISH));

    /* recevied 'transaction command end' */
    conn->clientCxt.clientStatus = CS_IDLE;
    return ret;
}

static int InitClientContext(PConnectionContext conn)
{
    conn->tcpCxt.readProc = ReadMessage;
    conn->tcpCxt.writeProc = NULL;
    conn->tcpCxt.msgProc = NULL;

    conn->tcpCxt.maxEvent = MAX_EVENT_SIZE;
    return 0;
}

static int ConnectServer(PConnectionContext conn, int port, char *addr)
{
    int ret = 0;

    conn->tcpCxt.sock_fd = CreateTcpSocket();
    if(conn->tcpCxt.sock_fd < 0)
        return -1;

    ret = ConnectTcpServer(conn->tcpCxt.sock_fd, port, addr);
    if(ret < 0)
        CloseSocket(conn->tcpCxt.sock_fd);
    return ret;
}

static int ReadMessage(PEventProcInfo info, MessageProc msgProc)
{
    int len = 0;
    char *message = NULL;
    int ret = 0;
    PClientContext client = (PClientContext)info->otherData;

    if(0 == (info->message & TS_BLOCK_READ))
    {
        /* process msg */
        len = info->len;
        message = info->buffer;

        if(len <= 0)
        {
            hat_error("connect is close by remote.");
            return -1;
        }
    }

    do {
        ret = ParserMsg(message, len, client);        
        if(client->msg.type == PORT_MSG_FINISH)
        {
            client->clientStatus |= CS_FINISH;
            break;
        }

        if(msgProc != NULL)
            msgProc(client->msg.body, client->msg.size);

        memset(&client->msg, 0x00, sizeof(client->msg));
        message = NULL;
        len = 0;
    }while(ret > 0);    /* until empty */
    
    if(client->clientStatus & CS_BLOCK)
    {
        info->message |= TS_BLOCK_READ;
    }
    else 
    {
        info->message &= ~TS_BLOCK_READ;
    }

    return 0;
}

