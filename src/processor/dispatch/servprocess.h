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

#ifndef HAT_SERVER_SOCKET_H_H
#define HAT_SERVER_SOCKET_H_H

#include "snapshot.h"

typedef struct Portal *PPortal;
typedef struct ServerContextInfo 
{
    int servfd ;
    int taskId;
    volatile int status;
    PPortal portal;

    /* one client initial once . */
    PSnapShotInfo snapshotPortal;
}ServerContextInfo, *PServerContextInfo;


int InitializeServer();

int ServerLoop();

void StopServer();

void DestoryServer();

int GetTaskId();
int GetServFd();
PPortal GetServPortal();
PSnapShotInfo GetServSnapShot();

#endif 