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

#include "resourceMgr.h"
#include "memStack.h"

extern __thread PResourceOnwerPool resourceOnwerPool ;
extern __thread PMemContextNode resourceMemContext;

static PResourceOwnerNode GetResourceOwnerNode();
static int AddResourceOwnerNodeToFreeList(PResourceOwnerNode node);

int CreateResourceOwnerPool()
{
    PMemContextNode old = MemMangerNewContext("resourceOwner");

    resourceOnwerPool = AllocMem(sizeof(ResourceOnwerPool));
    INIT_DLIST_NODE(resourceOnwerPool->freeList);
    INIT_DLIST_NODE(resourceOnwerPool->header);

    resourceOnwerPool->freeNodeNum = 0;
    resourceOnwerPool->onwerResourceNum = 0;

    resourceMemContext = MemMangerSwitchContext(old);
    return 0;
}

int PushResourceOwner(PPageDataHeader buffer, int type)
{
    PBufferResource node = NULL;
    PDList header = &(resourceOnwerPool->header);

    node = (PBufferResource)GetResourceOwnerNode();
    node->header.type = type;
    node->buffer = buffer;
    INIT_DLIST_NODE(node->header.list);

    AddDListTail(&header, &(node->header.list));
    resourceOnwerPool->onwerResourceNum += 1;
    return 0;
}

int ReleaseResourceOwner(PPageDataHeader buffer, int type)
{
    PDList header = &(resourceOnwerPool->header);
    PDList cur = header->next;
    PDList next = NULL;
    PBufferResource node = NULL;
    int ret = 0;

    if(resourceOnwerPool->onwerResourceNum == 0)
        return ret;

    while(cur != header)
    {
        next = cur->next;

        node = (PBufferResource)cur;
        if((node->buffer == buffer) && (node->header.type == type))
        {
            DelDListNode(&header, cur); 
            resourceOnwerPool->onwerResourceNum -= 1;

            AddResourceOwnerNodeToFreeList((PResourceOwnerNode)cur);
            ret = 1;
            break;
        }   

        cur = next;    
    }

    return ret;
}

int PopResourceOwner(PPageDataHeader *buffer)
{
    PDList header = &(resourceOnwerPool->header);
    PDList cur = header->next;
    PBufferResource node = NULL;
    int ret = 0;

    if(resourceOnwerPool->onwerResourceNum == 0)
        return ret;

    if(cur != header)
    {
        node = (PBufferResource)cur;
        DelDListNode(&header, cur); 
        resourceOnwerPool->onwerResourceNum -= 1;

        *buffer = node->buffer;
        AddResourceOwnerNodeToFreeList((PResourceOwnerNode)cur);
        ret = 1;
    }

    return ret;
}


static int AddResourceOwnerNodeToFreeList(PResourceOwnerNode node)
{
    PDList header = &(resourceOnwerPool->freeList);
    AddDListTail(&header, &(node->list));

    resourceOnwerPool->freeNodeNum += 1;
}

static PResourceOwnerNode GetResourceOwnerNode()
{
    PResourceOwnerNode node = NULL;
    PDList freelist = &(resourceOnwerPool->freeList);
    PMemContextNode old = NULL;

    if(resourceOnwerPool->freeNodeNum > 0)
    {
        node = (PResourceOwnerNode)PopDListTailNode(&freelist);
        resourceOnwerPool->freeNodeNum -= 1;
    }
    else
    {
        old = MemMangerSwitchContext(resourceMemContext);

        node = (PResourceOwnerNode)AllocMem(sizeof(BufferResource));
        MemMangerSwitchContext(old);
    }
    return node;
}
