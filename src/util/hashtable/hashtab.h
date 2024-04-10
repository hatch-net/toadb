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
#ifndef HAT_HASH_TABLE_H_H
#define HAT_HASH_TABLE_H_H

#include "list.h"
#include "memStack.h"

typedef struct HashKey
{
    unsigned long hashKey;
}HashKey, *PHashKey;

typedef struct HashEntry 
{
    DList link;
    HashKey key;
    char *value;
}HashEntry, *PHashEntry;

typedef struct HashSegment
{
    PDList bucket;              /* every bucket is a double linke. */
}HashSegment, *PHashSegment;

typedef struct HashTableContext 
{
    PHashSegment segment;            /* partial of hash table */
}HashTableContext, *PHashTableContext;


typedef int (*hashCompare)(PHashEntry hashEntr1, PHashEntry hashEntry2, int keySize, int valueSize);
typedef int (*hashCompute)(char *pValue, int size);

typedef struct HashTableInfo
{
    PHashTableContext hashContext;
    PHashEntry freeList;            /* free entry list. */
    char *hashTblName;
    int segmentSize;
    int bucketMaxSize;
    int valueSize;
    int hashSize;

    /* operator functions */
    hashCompare funCompare;
    hashCompute getHashKey;

    PMemContextNode memContext;
}HashTableInfo, *PHashTableInfo;

#endif 
