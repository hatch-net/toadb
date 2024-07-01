/*
 *	toadb client  
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

#include "tsqlclient.h"
#include "tsqlmain.h"
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

extern char *serverAddr ;
extern int serverPort ;
MsgHeader msg = {0};

static ThreadLocal TCPContext tsqlClient_context = {0};
static ClientContext client = {0};

static int WriteMessage(PEventProcInfo info, MessageProc msgProc);
static int ReadMessage(PEventProcInfo info, MessageProc msgProc);
static int ShowResult(char *message, int len);

static int InitClient();
static int InitClientContext();
static int ConnectServer(int port, char *addr);
static int ProcessClient(PMsgHeader msg);



int CSClient_main(int argc, char *argv[])
{
    char *command = msg.body;
    int len = 0;
    int ret = 0;

    ret = InitClient();
    if(ret < 0)
        return ret;

    do 
    {
        if((len = ReadClientCommand(command)) < 0)
            break;
        
        msg.size = len;
        msg.type = PORT_MSG_REQUEST;
        
        ret = ProcessClient(&msg);
        if(ret < 0)
        {
            hat_error("server is disconnected, we will exit.");
            break;
        }
    }while(1);

    CloseSocket(tsqlClient_context.sock_fd);
    CloseSocket(tsqlClient_context.epoll_fd);

    return ret;
}

int CSClient_Once(PMsgHeader msg)
{
    int ret = 0;
    
    ret = InitClient();
    if(ret < 0)
        return ret;

    ret = ProcessClient(msg);
    if(ret < 0)
    {
        hat_error("server is disconnected, we will exit.");
    }

    CloseSocket(tsqlClient_context.sock_fd);
    CloseSocket(tsqlClient_context.epoll_fd);

    return ret;
}

static int InitClient()
{
    int ret = 0;

    InitClientContext();
    ret = ConnectServer(serverPort, serverAddr);
    if(ret < 0)
    {
        hat_error("connect to server failure.");
        return -1;
    }

    tsqlClient_context.epoll_fd = CreateEpoll();
    if(tsqlClient_context.epoll_fd < 0)
    {
        hat_error("CreateEpoll epoll failure.");
        return -1;
    }

    ret = AddEpollREvent(tsqlClient_context.epoll_fd, tsqlClient_context.sock_fd);
    return ret;
}

static int ProcessClient(PMsgHeader msg)
{
    int ret = 0;
    struct epoll_event events[MAX_EVENT_SIZE];
    EventProcInfo info = {0};

    client.clientStatus |= CS_SENDCOMMD;
    ret = WriteData(tsqlClient_context.sock_fd, (char*)msg, msg->size + MSG_HEADER_LEN);
    if(ret < 0)
    {
        hat_error("send command failure.");
        return ret;
    }

    tsqlClient_context.info = &info;
    do {
        /* epoll wait write/read. */
        ret = EpollProcess(&tsqlClient_context, events, MAX_EVENT_SIZE, -1);
    }while((ret >= 0) && ((client.clientStatus & CS_CLIENT_STATE_MASK) != CS_FINISH));

    /* recevied 'transaction command end' */
    client.clientStatus = CS_IDLE;
    return ret;
}

static int InitClientContext()
{
    tsqlClient_context.readProc = ReadMessage;
    tsqlClient_context.writeProc = WriteMessage;
    tsqlClient_context.msgProc = ShowResult;

    tsqlClient_context.maxEvent = MAX_EVENT_SIZE;
    return 0;
}

static int ConnectServer(int port, char *addr)
{
    int ret = 0;

    tsqlClient_context.sock_fd = CreateTcpSocket();
    if(tsqlClient_context.sock_fd < 0)
        return -1;

    ret = ConnectTcpServer(tsqlClient_context.sock_fd, port, addr);
    if(ret < 0)
        CloseSocket(tsqlClient_context.sock_fd);
    return ret;
}

static int ReadMessage(PEventProcInfo info, MessageProc msgProc)
{
    int len = 0;
    char *message = NULL;
    int ret = 0;

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
        ret = ParserMsg(message, len, &client);        
        if(client.msg.type == PORT_MSG_FINISH)
        {
            client.clientStatus |= CS_FINISH;
            break;
        }

        if(msgProc != NULL)
            msgProc(client.msg.body, client.msg.size);

        memset(&client.msg, 0x00, sizeof(client.msg));
        message = NULL;
        len = 0;
    }while(ret > 0);    /* until empty */
    
    if(client.clientStatus & CS_BLOCK)
    {
        info->message |= TS_BLOCK_READ;
    }
    else 
    {
        info->message &= ~TS_BLOCK_READ;
    }

    return 0;
}

static int WriteMessage(PEventProcInfo info, MessageProc msgProc)
{
    /* TODO: nothing */
    return 0;
}

static int ShowResult(char *message, int len)
{
    if(len <= 0)
        return 0;

    //snprinf(buffer, len, "%s", message);
    printf("%s\n", message);
    return 0;
}
