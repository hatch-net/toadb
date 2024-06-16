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


#ifndef HAT_CLIENT_H_H
#define HAT_CLIENT_H_H

#include "portal.h"

#define MAX_EVENT_SIZE 2

typedef enum ClientStatus
{
    CS_IDLE,
    CS_SENDCOMMD,
    CS_READ,
    CS_FINISH
}ClientStatus;

typedef struct ClientContext 
{
    int clientStatus;

    int restLen;
    char restMsg[PORT_BUFFER_SIZE];     /* rest of recv msg. */

    MsgHeader msg;
}ClientContext, *PClientContext;

int CSClient_main(int argc, char *argv[]);



#endif 
