/*
 *	toadb tcp socket 
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

#ifndef HAT_TCP_SOCKET_H_H
#define HAT_TCP_SOCKET_H_H

#include <sys/epoll.h>

#define IPV4_ADDRESS_LEN 16
#define SOCKET_BUFFER_LEN 2048 

typedef enum TCPState
{
    TS_NONE         = 0x00,
    TS_BLOCK_READ   = 0x01,
    TS_BLOCK_WRITE  = 0x02,
    TS_MAX
}TCPState;

typedef struct EventProcInfo
{
    int message;                /* state bit -> TCPState */
    int len; 
    char *buffer;
    void *otherData;
}EventProcInfo, *PEventProcInfo;

typedef int (*MessageProc)(char *message, int len);
typedef int (*EventProc)(PEventProcInfo info, MessageProc msgProc);

typedef struct TCPContext
{
    int sock_fd;

    int epoll_fd;
    int maxEvent;
       
    PEventProcInfo info;
    EventProc readProc;
    EventProc writeProc;
    MessageProc msgProc;
}TCPContext, *PTCPContext;

int CreateTcpSocket();

int ConnectTcpServer(int fd, int port, char *addr);
int BuildTcpServ(int sockfd, unsigned int servIP, int  servPort, int queueNum);

int ServiceRunTcpServ(int sockfd);

int ReadData(int sockfd, char *buf, int bufLen);
int WriteData(int sockfd, char *buf, int bufLen);

int CloseSocket(int sockfd);

int CreateEpoll();
int EpollProcess(PTCPContext context, struct epoll_event *events, int maxEvent, int timeout);
int AddEpollEvent(int epoll, int fd, int event);
int DelEpollEvent(int epoll, int fd, int event);
int AddEpollREvent(int epoll, int fd);
#endif 