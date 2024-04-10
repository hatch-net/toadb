/*
 *	toadb hash table 
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

#include "hashtab.h"
#include "public.h"
#include "hatstring.h"

#if 0
static int InitializeHashTable(PHashTableInfo hashTableInfo);

PHashTableInfo HashTableCreate(int partNum, int bucketMaxSize, int valueSize, PMemContextNode hashMemContext, char *Name)
{
    PMemContextNode oldContext = NULL;
    PHashTableInfo hashTableInfo = NULL;
    int nameLen = 0;

    if(NULL == hashMemContext)
        return -1;

    if(hashMemContext != MemMangerGetCurrentContext())
    {
       oldContext = MemMangerSwitchContext(hashMemContext);
    }

    nameLen = hat_strlen(Name);
    hashTableInfo = AllocMem(sizeof(HashTableInfo) + nameLen + 1);

    hashTableInfo->bucketMaxSize = bucketMaxSize;
    hashTableInfo->hashSize = sizeof(HashKey);
    hashTableInfo->valueSize = valueSize;
    hashTableInfo->segmentSize = partNum;
    hashTableInfo->memContext = hashMemContext;

    hashTableInfo->hashTblName = (char *)(hashTableInfo + 1);
    memcpy(hashTableInfo->hashTblName, Name, nameLen);

    if(InitializeHashTable(hashTableInfo) < 0)
    {
        hat_error("Initialize hash table %s failure.\n", Name);
        hashTableInfo = NULL;
    }

    if(NULL != oldContext)
    {
        MemMangerSwitchContext(oldContext);
    }

    return hashTableInfo;
}

static int InitializeHashTable(PHashTableInfo hashTableInfo)
{
    return 0;
}

#endif 