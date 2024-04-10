/*
 *	toadb ipc  
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
#ifndef HAT_TOADB_IPC_H_H
#define HAT_TOADB_IPC_H_H

#define TOADB_SHARED_ID 0x1001
#define TOADB_SHARED_SEM_READ_ID "client_share_read_sem"
#define TOADB_SHARED_SEM_WRITE_ID "client_share_write_sem"
#define TOADB_SHARED_SEM_CLIENT_ID "client_share_operating_sem"

#define CLIENT_SHARED_BUFFER_SIZE (8192)

typedef struct ClientSharedInfo
{
    int command;
    int dataLen;
    char data[];
}ClientSharedInfo, *PClientSharedInfo;


int InitSharedEvn();

int InitServerSharedEnv(char **ptr);
int DestorySeverSharedEnv(char **ptr);

int InitClientSharedEnv(char **ptr);
int DestoryClientSharedEnv(char **ptr);

int WaitClientDataArrive();
int NotifyClientReadData();

int NotifyServerReadData();
int WaitServerDataArrive();

int WaitClientControlLock();
int ReleaseClientControlLock();

#endif 