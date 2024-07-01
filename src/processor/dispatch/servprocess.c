/*
 *	toadb server process 
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

#include "servprocess.h"
#include "public.h"
#include "tcpsock.h"
#include "threadPool.h"
#include "toadmain.h"
#include "hatstring.h"
#include "portal.h"
#include "netclient.h"

#include "public_types.h"

int servPort = 6389;            /* default value */
char servIP[IPV4_ADDRESS_LEN];
int backlog = 10;

static ThreadLocal ServerContextInfo servContext;
static ThreadLocal ClientContext cliContext;

static int ProcessQuery(PThreadTaskInfo taskInfo);

int InitializeServer()
{
    int servfd = -1;
    int ret = 0;
    
    servfd = CreateTcpSocket();
    if(servfd < 0)
        return -1;

    ret = BuildTcpServ(servfd, 0, servPort, backlog);
    if(ret < 0)
    {
        return -1;
    }

    servContext.servfd = servfd;
    servContext.status = 1;

    hat_log("server build sucess.");
    return ret;
}

void StopServer()
{
    servContext.status = 0;
}

int ServerLoop()
{
    int clientfd = -1;
    int ret = 0;

    /* TODO: dispatch task */
    while(servContext.status)
    {
        clientfd = ServiceRunTcpServ(servContext.servfd);
        if(clientfd < 0)
            break;

        hat_log("new cliend fd[%d]", clientfd);

        ret = PushTask(clientfd, ProcessQuery);
        if(ret < 0)
        {
            hat_error("client is over limited,  client %d maybe rejected.", clientfd);
            CloseSocket(clientfd);
        }
    }

    return 0;
}

int DestoryServer()
{
    CloseSocket(servContext.servfd);
    servContext.servfd = 0;
}

int FinishTask(PThreadTaskInfo taskInfo)
{
    DestroyPortal(servContext.portal);
    
    if(taskInfo->clientfd > 0)
    {
        CloseSocket(taskInfo->clientfd);
        taskInfo->clientfd = -1;
    }

    taskInfo->taskId = -1;

    return 0;
}

int BeginTask(PThreadTaskInfo taskInfo)
{
    servContext.taskId = taskInfo->taskId;
    servContext.servfd = taskInfo->clientfd;
    hat_log("task %d-%d starting...", servContext.taskId , servContext.servfd);

    servContext.portal = CreatePortal();
    return 0;
}

/* 
 *
 */
static int ReadMsg(PClientContext client)
{
    int len = 0;
    char command[MAX_COMMAND_LENGTH] = {0};

    memset((char *)&client->msg, 0x00, sizeof(MsgHeader));
    do {
        len = ReadData(servContext.servfd, command, MAX_COMMAND_LENGTH);
        if(len <= 0)
        {
            hat_error("client interrupt connecting.");
            break;
        }

        len = ParserMsg(command, len, client);
    } while(len <= 0);

    /* received one command. */
    return len;
}

static int ProcessQuery(PThreadTaskInfo taskInfo)
{
    char *command = cliContext.msg.body;
    int ret = 0;

    if(NULL == taskInfo)
        return -1;    
    
    BeginTask(taskInfo);

    while(1) 
    {
        InitPortal(GetServPortal());

        ret = ReadMsg(&cliContext);
        if(ret <= 0)
        {
            hat_error("client interrupt connecting.");
            break;
        }

        hat_log("task[%d-%d] receive command %s", servContext.taskId, servContext.servfd, command);

        if(hat_strncmp(command, "quit", 4) == 0)
        {
            hat_log("task[%d-%d] quiting.", servContext.taskId, servContext.servfd);
            break;
        }
             
        ret = ToadMainEntry(command);
        if(ret < 0)
        {
            hat_error("excutor sql failure.");
        }

        EndPort(GetServPortal());
    }

    hat_log("task %d-%d finish.", servContext.taskId, servContext.servfd);
    FinishTask(taskInfo);
    return ret;
}

int GetTaskId()
{
    return servContext.taskId;
}

int GetServFd()
{
    return servContext.servfd;
}

PPortal GetServPortal()
{
    return servContext.portal;
}