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

#include "tcpsock.h"
#include "public.h"

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/tcp.h>    // nodelay
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>     // close
#include <errno.h>

#include <sys/epoll.h> 

int CreateTcpSocket()
{
    int sockfd = -1;
    int opt = 1;
	
	sockfd = socket(AF_INET , SOCK_STREAM , 0);
	if (sockfd < 0)
	{
		hat_error("create tcp socket error[%d]", errno);
	}

    // 设置SO_REUSEADDR选项  
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
    {  
        hat_error("setsockopt(SO_REUSEADDR) failed"); 
        close(sockfd); 
        return -1;  
    }  

    // 启用TCP_NODELAY   
    // 设置socket选项以禁用Nagle算法  
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&opt, sizeof(opt)) < 0) 
    {  
        hat_error("setsockopt(TCP_NODELAY) %d", errno);  
        close(sockfd);
        return -1;
    }
    return sockfd;
}

/* 
 * only support IPv4 
*/
int BuildTcpServ(int sockfd, unsigned int servIP, int  servPort, int queueNum)
{
	struct sockaddr_in serverAddr;
	
    if(sockfd < 0)
    {
        return -1;
    }

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = servIP > 0 ? servIP : INADDR_ANY;
	serverAddr.sin_port = htons(servPort);
	
	if( bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		hat_error("bind tcp socket error[%d]", errno);
        return -2;
	}

	if(listen(sockfd , queueNum) < 0)
    {
        hat_error("listen tcp socket error[%d]", errno);
        return -3;
    }
	return 0;
}

int ServiceRunTcpServ(int sockfd)
{
    int new_socket, clientAddrLen;
	struct sockaddr_in clientAddr;
    int flag = 1; 

    if(sockfd < 0)
    {
        return -1;
    }

	clientAddrLen = sizeof(clientAddr);
	new_socket = accept(sockfd, (struct sockaddr *)&clientAddr, (socklen_t*)&clientAddrLen);
	if (new_socket < 0)
	{
		hat_error("listen tcp socket error[%d]", errno);
        return -1;
	}
	
    // 启用TCP_NODELAY   
    // 设置socket选项以禁用Nagle算法  
    if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)) < 0) 
    {  
        hat_error("setsockopt(TCP_NODELAY) %d", errno);  
        close(new_socket);
        return -1;
    }

    hat_log("listen recv cliend fd[%d]", new_socket);
	return new_socket;
}


int ReadData(int sockfd, char *buf, int bufLen)
{
    int ret = 0;
    if((sockfd < 0) || (NULL == buf))
        return -1;
    
    ret = recv(sockfd, buf, bufLen, 0);
    if(ret <= 0)
    {
        hat_error("send tcp socket error[%d]", errno);
    }
    return ret;
}

int WriteData(int sockfd, char *buf, int bufLen)
{
    int ret = 0;
    if((sockfd < 0) || (NULL == buf))
        return -1;
    
    ret = send(sockfd, buf, bufLen, 0);
    if(ret <= 0)
    {
        hat_error("send tcp socket error[%d]", errno);
    }
    return ret;
}


int CloseSocket(int sockfd)
{
    if(sockfd > 0)
        close(sockfd);
    return 0;
}

int ConnectTcpServer(int fd, int port, char *addr)
{
    struct sockaddr_in serv_addr;  

    bzero((char *) &serv_addr, sizeof(serv_addr));  

    serv_addr.sin_family = AF_INET;  
    serv_addr.sin_addr.s_addr = inet_addr(addr);
    serv_addr.sin_port = htons(port);  
  
    // 连接到服务器  
    if (connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        hat_error("connect to server[%s:%d] failure.[%d]", addr, port, errno);
        return -1;
    }

    return 0;
}

int CreateEpoll()
{
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if(epoll_fd < 0)
    {
        hat_error("create epoll falure. [%d]", errno);
        return -1;
    }

    return epoll_fd;
}

int AddEpollEvent(int epoll, int fd, int event)
{
    struct epoll_event ev;

    ev.events = event ; // | EPOLLET;  // EPOLLIN EPOLLOUT
    ev.data.fd = fd;  

    if (epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &ev) == -1) 
    {  
        hat_error("%d add epoll falure. [%d]", fd, errno);
        return -1; 
    }  
    return 0;
}

int AddEpollREvent(int epoll, int fd)
{
    return AddEpollEvent(epoll, fd, EPOLLIN);
}

int DelEpollEvent(int epoll, int fd, int event)
{
    return 0;
}

int EpollProcess(PTCPContext context, struct epoll_event *events, int maxEvent, int timeout)
{
    int nfds = 0;
    int n = 0;
    int ret = 0;
    char buffer[SOCKET_BUFFER_LEN];
    PEventProcInfo epInfo = context->info;

    nfds = epoll_wait(context->epoll_fd, events, maxEvent, timeout);  
    if (nfds < 0) 
    {  
        if (errno == EINTR) {  
            return 1;  
        }  
    }  
  
    /* 处理所有接收到的事件   */
    for (n = 0; n < nfds; ++n) 
    {  
        if(events[n].events & EPOLLIN) 
        {  
            ret = ReadData(events[n].data.fd, buffer, SOCKET_BUFFER_LEN);
            epInfo->len = ret;
            epInfo->buffer = buffer;

            if(context->readProc != NULL)
                ret = context->readProc(epInfo);
        }  
        
        if(events[n].events & EPOLLOUT) 
        {  
            ret = WriteData(events[n].data.fd, epInfo->buffer, epInfo->len);

            if(context->writeProc != NULL)
                ret = context->writeProc(epInfo);
        }  
    }

    return ret;
}