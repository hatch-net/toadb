/*
 *	toadb resource manager  
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
#ifndef HAT_RESOURCE_MANAGER_H_H
#define HAT_RESOURCE_MANAGER_H_H

#include "list.h"
#include "tablecom.h"

typedef struct ResourceOwnerNode
{
    DList list;
    int type;
}ResourceOwnerNode, *PResourceOwnerNode;

typedef struct BufferResource 
{
    ResourceOwnerNode header;
    PPageDataHeader buffer;
}BufferResource, *PBufferResource; 

typedef struct ResourceOnwerPool
{
    DList freeList;
    int freeNodeNum;
    DList header;
    int onwerResourceNum;
}ResourceOnwerPool, *PResourceOnwerPool;

int CreateResourceOwnerPool();

int PushResourceOwner(PPageDataHeader buffer, int type);
int ReleaseResourceOwner(PPageDataHeader buffer, int type);

int PopResourceOwner(PPageDataHeader *buffer);
#endif 
