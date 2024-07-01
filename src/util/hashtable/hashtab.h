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

#include "public.h"
#include "memStack.h"
#include "spinlock.h"

typedef unsigned long long HASHKEY; 
#define HASH_BITS       (sizeof(HASHKEY)*8)

typedef enum HashOp 
{
    HASH_FIND,
    HASH_INSERT,
    HASH_DELETE,
    HASH_NULL
}HashOp;

#define HASH_ELEMENT_SIZE (sizeof(HashElement))
#define GetValue(element) ((char*)element + HASH_ELEMENT_SIZE)
typedef struct HashElement
{
    struct HashElement *link;
    HASHKEY hashKey;
}HashElement, *PHashElement;


#define HASH_SEGMENT_SIZE (32)
typedef struct HashSegment
{
    SPINLOCK        segLock;
    PHashElement    *segBuckets;
}HashSegment, *PHashSegment;

typedef HASHKEY (*HashKeyFun)(char *value, int valueSize);
typedef int (*HashCompareFun)(char *value1, char *value2, int valueSize);
typedef int (*ValueCopyFun)(PHashElement element, char *value, int valueSize);

typedef struct HashTableInfo 
{
    char name[COMMENT_NAME_MAX_LEN];
    int initNum;
    int maxNum;
    int partitionNum;
    int valueSize;
    int keyValueSize;

    PHashElement    buckets;
    PHashElement    freeHashElementList;    
    SPINLOCK        freeListLock;
    int             freeNum;

    HASHKEY partitionMask;
    HASHKEY bucketMask;
    int partitionShift;
    int bucketShift;
    int hashSegmentSize;

    PHashSegment        segmentArray;
    PMemContextNode     hashMemContext;
    int                 usedNum;

    HashKeyFun          getHashKey;
    HashCompareFun      hashCompare;
    ValueCopyFun        hashValueCopy;
}HashTableInfo, *PHashTableInfo;


PHashTableInfo HashTableCreate(int partNum, int maxSize, int initSize, int valueSize, int keyValueSize, PMemContextNode hashMemContext, char *Name);
int DestroyHashTable(PHashTableInfo hashTableInfo);

char * GetEntryValue(PHashElement element);
int GetPartitionIndex(HASHKEY key, PHashTableInfo hashTableInfo);
int GetBucketIndex(HASHKEY key, PHashTableInfo hashTableInfo);

void HashLockPartition(PHashTableInfo hashTableInfo, int partition);
void HashLockPartitionRelease(PHashTableInfo hashTableInfo, int partition);

PHashElement DeleteHashEntryFromBucket(PHashTableInfo hashTableInfo, PHashElement *bucket, HASHKEY key, char *value);
PHashElement GetHashEntryFromBucket(PHashTableInfo hashTableInfo, PHashElement bucket, HASHKEY key, char *value);

PHashElement HashFindEntry(PHashTableInfo hashTableInfo, HASHKEY key, char *value, int partition);
PHashElement HashGetFreeEntry(PHashTableInfo hashTableInfo, HASHKEY key, char *value, int *flag);
int HashDeleteEntry(PHashTableInfo hashTableInfo, HASHKEY key, char *value);

typedef struct BufferPoolContext *PBufferPoolContext;
void ShowHashTableValues(PHashTableInfo hashTableInfo, PBufferPoolContext bufferPool);
#endif 
