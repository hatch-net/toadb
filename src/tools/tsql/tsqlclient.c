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

#include <stdio.h>
#include <stdlib.h>

extern char *serverAddr ;
extern int serverPort ;

static ThreadLocal TCPContext tsqlClient_context = {0};
static ClientContext client = {0};

static int WriteMessage(PEventProcInfo info);
static int ReadMessage(PEventProcInfo info);

static int InitClient();
static int InitClientContext();
static int ConnectServer(int port, char *addr);
static int ProcessClient(char *command, int len);

static int ShowResult(char *message, int len);

static int GetMsgIntValue(char *buf, int *type);
static int ParserMsg(char *msg, int len);


int CSClient_main(int argc, char *argv[])
{
    char command[MAX_COMMAND_LENGTH] = {0};
    int len = 0;
    int ret = 0;

    ret = InitClient();
    if(ret < 0)
        return ret;

    do 
    {
        if((len = ReadClientCommand(command)) < 0)
            break;
        
        ret = ProcessClient(command, len);
        if(ret < 0)
        {
            hat_error("server is disconnected, we will exit.");
            break;
        }
    }while(1);

    CloseSocket(tsqlClient_context.sock_fd);
    close(tsqlClient_context.epoll_fd);

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

static int ProcessClient(char *command, int len)
{
    int ret = 0;
    struct epoll_event events[MAX_EVENT_SIZE];
    EventProcInfo info = {0};

    client.clientStatus = CS_SENDCOMMD;
    ret = WriteData(tsqlClient_context.sock_fd, command, len);
    if(ret < 0)
    {
        hat_error("send command failure.");
        return ret;
    }

    tsqlClient_context.info = &info;
    do {
        info.message += 1;
        /* epoll wait write/read. */
        ret = EpollProcess(&tsqlClient_context, events, MAX_EVENT_SIZE, -1);
    }while((ret >= 0) && (client.clientStatus != CS_FINISH));

    /* recevied 'transaction command end' */
    client.clientStatus = CS_IDLE;
    return ret;
}

static int InitClientContext()
{
    tsqlClient_context.readProc = ReadMessage;
    tsqlClient_context.writeProc = WriteMessage;

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

static int ReadMessage(PEventProcInfo info)
{
    int len = info->len;
    char *message = info->buffer;
    int ret = 0;

    if(len <= 0)
    {
        hat_error("connect is close by remote.");
        return -1;
    }

    do {
        ret = ParserMsg(message, len);        
        if(client.msg.type == PORT_MSG_FINISH)
        {
            client.clientStatus = CS_FINISH;
            break;
        }

        ShowResult(client.msg.body, client.msg.size);
        memset(&client.msg, 0x00, sizeof(client.msg));
        message = NULL;
        len = 0;
    }while(ret > 0);    /* until empty */
    
    return 0;
}

static int WriteMessage(PEventProcInfo info)
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

static int ParserMsg(char *msg, int len)
{
    int msgOffset = 0;
    int restOffset = 0;
    int tmpLen = 0;
    int useMsg = 0;

    if(len + client.restLen < MSG_HEADER_LEN)
    {
        /* msg header is not enogh . */
        return 0;
    }

    /* first fill rest with msg header len */
    if(client.restLen < MSG_HEADER_LEN)
    {
        tmpLen = MSG_HEADER_LEN - client.restLen;
        memcpy(client.restMsg + client.restLen, msg, tmpLen);
        client.restLen += tmpLen;
        msgOffset += tmpLen;
    }

    restOffset += GetMsgIntValue(client.restMsg, &client.msg.type);
    restOffset += GetMsgIntValue(client.restMsg + restOffset, &client.msg.size);

    if(client.msg.size > (client.restLen + len - msgOffset))
    {
        useMsg = 1;
        goto RESTMSG;
    }

    /* rest context is fullfill one entire msg. */
    if((client.restLen - restOffset) < client.msg.size)
    {
        tmpLen = client.msg.size - client.restLen + restOffset;
        if(len - msgOffset >= tmpLen)
        {
            memcpy(client.restMsg + client.restLen, msg + msgOffset, tmpLen);
            client.restLen += tmpLen;
            msgOffset += tmpLen;
        }
        else 
        {
            useMsg = 1;
            goto RESTMSG;
        }
    }

    /* fill entire message */
    memcpy(client.msg.body, client.restMsg + restOffset, client.msg.size);
    restOffset += client.msg.size;
    client.restLen -= restOffset;

RESTMSG:
    if(useMsg)
    {
        /* wait context, rollback */
        restOffset -= MSG_HEADER_LEN;
        client.msg.type = 0;
        client.msg.size = 0;
    }

    /* clean restmsg */
    if((restOffset > 0) && (client.restLen > 0))
        memcpy(client.restMsg, client.restMsg + restOffset, client.restLen);

    /* save the rest msg */
    tmpLen = len - msgOffset;
    if(tmpLen > 0)
    {
        memcpy(client.restMsg + client.restLen, msg + msgOffset, tmpLen);
        client.restLen += tmpLen;
    }
    return client.msg.size;
}

static int GetMsgIntValue(char *buf, int *type)
{
    int typelen = sizeof(int);
    *type = *((int *) buf);
    return typelen;
}