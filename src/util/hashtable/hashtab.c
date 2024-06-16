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
#include "spooky-c.h"


//#define debug_hash(...) log_report(LOG_INFO, __VA_ARGS__) 
#define debug_hash(...) 

static HASHKEY getHashKey(char *value, int valueSize);
static int compareHashElement(char *value1, char *value2, int valueSize);
static int valueCopy(PHashElement element, char *value, int valueSize);

static int InitHashTable(PHashTableInfo hashTableInfo);

static PHashElement GetFromFreeList(PHashTableInfo hashTableInfo);
static void PutFreeList(PHashTableInfo hashTableInfo, PHashElement entry);

HASHKEY getAlignNum(unsigned int num)
{
    HASHKEY mask = num - 1;

    mask |= mask >> 1;
    mask |= mask >> 2;
    mask |= mask >> 4;
    mask |= mask >> 8;
    mask |= mask >> 16; /* 32 bit */

    return mask+1;
}

int getShiftNum(HASHKEY key)
{
    int shift = 0;
    while(key = key >> 1)
    {
        shift ++;
    }
    return shift;
}

int GetHashTableSize(int partNum, int maxNum, int valueSize)
{
    /* elements size */
    int len = getAlignNum(maxNum) * (valueSize + HASH_ELEMENT_SIZE);

    /* partition head size, alloc max elements per segment */
    len += getAlignNum(partNum) * (sizeof(HashSegment) + len);

    len += sizeof(HashTableInfo);

    return len;
}

PHashTableInfo HashTableCreate(int partNum, int maxSize, int initSize, int valueSize, int keyValueSize, PMemContextNode hashMemContext, char *Name)
{
    PMemContextNode oldContext = NULL;
    PHashTableInfo hashTableInfo = NULL;
    int hashtableLen = 0;

    if(NULL == hashMemContext)
        return NULL;

    if(hashMemContext != MemMangerGetCurrentContext())
    {
       oldContext = MemMangerSwitchContext(hashMemContext);
    }

    hashtableLen = GetHashTableSize(partNum, maxSize, valueSize);
    hashtableLen += hat_strlen(Name);    
    hashTableInfo = AllocMem(hashtableLen);
    
    hat_strncpy(hashTableInfo->name, Name, COMMENT_NAME_MAX_LEN);

    hashTableInfo->maxNum = maxSize;
    hashTableInfo->initNum = initSize;
    hashTableInfo->partitionNum = partNum;
    hashTableInfo->valueSize = valueSize;
    hashTableInfo->keyValueSize = keyValueSize;
    hashTableInfo->hashMemContext = hashMemContext;
    
    hashTableInfo->bucketMask = getAlignNum(maxSize);
    hashTableInfo->bucketShift = getShiftNum(hashTableInfo->bucketMask);
    hashTableInfo->bucketMask = ~(hashTableInfo->bucketMask-1);
    
    hashTableInfo->partitionMask = getAlignNum(partNum);
    hashTableInfo->partitionShift = HASH_BITS - getShiftNum(hashTableInfo->partitionMask);
    hashTableInfo->partitionMask = ~(((HASHKEY)1 << hashTableInfo->partitionShift) - 1);

    hashTableInfo->hashSegmentSize = HASH_SEGMENT_SIZE;
    hashTableInfo->hashCompare = compareHashElement;
    hashTableInfo->getHashKey = getHashKey;
    hashTableInfo->hashValueCopy = valueCopy;

    if(InitHashTable(hashTableInfo) < 0)
    {
        hat_error("Initialize hash table %s failure.", Name);
        hashTableInfo = NULL;
    }

    if(NULL != oldContext)
    {
        MemMangerSwitchContext(oldContext);
    }

    return hashTableInfo;
}

static int InitHashTable(PHashTableInfo hashTableInfo)
{
    int offset = sizeof(HashTableInfo);
    PHashSegment segment = NULL;
    PHashElement element = NULL;
    int index = 0;
    int segmentIndex = 0;

    if(NULL == hashTableInfo)
        return -1;

    /* bucket */
    hashTableInfo->buckets = (PHashElement) ((char*)hashTableInfo + offset);
    offset += hashTableInfo->maxNum * (hashTableInfo->valueSize + HASH_ELEMENT_SIZE);

    /* freelist */
    hashTableInfo->freeHashElementList = &hashTableInfo->buckets[0];
    element = hashTableInfo->freeHashElementList;

    for(index = 1; index < hashTableInfo->maxNum; index++)
    {
        element->link = (PHashElement)((char *)element + hashTableInfo->valueSize + HASH_ELEMENT_SIZE);
        element = element->link;        
    }
    element->link = NULL;
    SpinLockInit(&hashTableInfo->freeListLock);

    /* segment */
    hashTableInfo->segmentArray = (PHashSegment)((char*)hashTableInfo + offset);
    offset += hashTableInfo->partitionNum * sizeof(HashSegment);

    segment = hashTableInfo->segmentArray;
    for(; segmentIndex < hashTableInfo->partitionNum; segmentIndex++)
    {
        /* buckets */
        segment->segBuckets = (PHashElement*) ((char*)hashTableInfo + offset);
        offset += hashTableInfo->hashSegmentSize * sizeof(PHashElement);

        /* locks */
        SpinLockInit(&segment->segLock);

        /* next segment */
        segment++;
    }
    return 0;
}


int DestroyHashTable(PHashTableInfo hashTableInfo)
{
    PHashSegment segment = NULL;
    int segmentIndex = 0;

    segment = hashTableInfo->segmentArray;
    for(; segmentIndex < hashTableInfo->partitionNum; segmentIndex++)
    {
        SpinLockFree(&segment->segLock);
        segment++;
    }
    SpinLockFree(&hashTableInfo->freeListLock);
    return 0;
}

void HashLockPartition(PHashTableInfo hashTableInfo, int partition)
{
    SpinLockAquire(&hashTableInfo->segmentArray[partition].segLock);
}

void HashLockPartitionRelease(PHashTableInfo hashTableInfo, int partition)
{
    SpinLockRelease(&hashTableInfo->segmentArray[partition].segLock);
}

void HashLockFreeList(PHashTableInfo hashTableInfo)
{
    SpinLockAquire(&hashTableInfo->freeListLock);
}

void HashLockReleaseFreeList(PHashTableInfo hashTableInfo)
{
    SpinLockRelease(&hashTableInfo->freeListLock);
}

int HashDeleteEntry(PHashTableInfo hashTableInfo, HASHKEY key, char *value)
{
    PHashElement entry = NULL;
    int partIndex = 0;
    int bucket = 0;

    partIndex = GetPartitionIndex(key, hashTableInfo);
    HashLockPartition(hashTableInfo, partIndex);

    bucket = GetBucketIndex(key, hashTableInfo);
    entry = DeleteHashEntryFromBucket(hashTableInfo, &(hashTableInfo->segmentArray[partIndex].segBuckets[bucket]), key, value);

    HashLockPartitionRelease(hashTableInfo, partIndex);

    debug_hash("del part:%d bucket:%d key:%0x entry:%p", partIndex, bucket, key, entry);
    if(NULL != entry)
        PutFreeList(hashTableInfo, entry);

    return 0;
}

PHashElement HashFindEntry(PHashTableInfo hashTableInfo, HASHKEY key, char *value, int partition)
{
    PHashElement entry = NULL;
    int bucket = 0;

    bucket = GetBucketIndex(key, hashTableInfo);
    entry = GetHashEntryFromBucket(hashTableInfo, hashTableInfo->segmentArray[partition].segBuckets[bucket], key, value);

    return entry;
}

PHashElement HashGetFreeEntry(PHashTableInfo hashTableInfo, HASHKEY key, char *value)
{
    PHashElement entry = NULL;
    int partIndex = 0;
    int bucket = 0;

    /* get free elemnet */
    entry = GetFromFreeList(hashTableInfo);
    if(NULL == entry)
    {
        hat_error("hash table %s is not enogh element rest.", hashTableInfo->name);
        return NULL;
    }

    partIndex = GetPartitionIndex(key, hashTableInfo);
    HashLockPartition(hashTableInfo, partIndex);

    bucket = GetBucketIndex(key, hashTableInfo);
    
    /* insert to segment */
    entry->link = hashTableInfo->segmentArray[partIndex].segBuckets[bucket];
    hashTableInfo->segmentArray[partIndex].segBuckets[bucket] = entry;

    HashLockPartitionRelease(hashTableInfo, partIndex);

    debug_hash("insert part:%d bucket:%d key:%0x entry:%p", partIndex, bucket, key, entry);
    entry->hashKey = key;
    hashTableInfo->hashValueCopy(entry, value, hashTableInfo->valueSize);
    return entry;
}

char * GetEntryValue(PHashElement element)
{
    char *pvalue = GetValue(element);

    return pvalue;
}

int HashEntryProcess(PHashTableInfo hashTableInfo, HASHKEY key, char *value, int valueSize, HashOp op)
{
    return 0;
}


static PHashElement GetFromFreeList(PHashTableInfo hashTableInfo)
{
    PHashElement entry = NULL;
    
    HashLockFreeList(hashTableInfo);
    
    if(NULL != hashTableInfo->freeHashElementList) 
    {
        entry = hashTableInfo->freeHashElementList;
        hashTableInfo->freeHashElementList = entry->link;
        entry->link = NULL;
    }
    
    HashLockReleaseFreeList(hashTableInfo);

    return entry;
}

static void PutFreeList(PHashTableInfo hashTableInfo, PHashElement entry)
{
    if(NULL == entry)
        return ;
    
    HashLockFreeList(hashTableInfo);
    
    entry->link = hashTableInfo->freeHashElementList;
    hashTableInfo->freeHashElementList = entry;
    
    HashLockReleaseFreeList(hashTableInfo);

    return ;
}

int GetPartitionIndex(HASHKEY key, PHashTableInfo hashTableInfo)
{
    int partition = key & hashTableInfo->partitionMask >> hashTableInfo->partitionShift;

    return partition;
}

int GetBucketIndex(HASHKEY key, PHashTableInfo hashTableInfo)
{
    int bucket = key & (~hashTableInfo->partitionMask) % hashTableInfo->hashSegmentSize;

    return bucket;
}

PHashElement GetHashEntryFromBucket(PHashTableInfo hashTableInfo, PHashElement bucket, HASHKEY key, char *value)
{
    PHashElement element = bucket;

    while(element != NULL)
    {
        if((element->hashKey == key) 
            && 0 == hashTableInfo->hashCompare(GetValue(element), value, hashTableInfo->keyValueSize))
        {
            return element;
        }

        element = element->link;
    }

    return NULL;
}

PHashElement DeleteHashEntryFromBucket(PHashTableInfo hashTableInfo, PHashElement *bucket, HASHKEY key, char *value)
{
    PHashElement element = *bucket;
    PHashElement preelement = element;

    while(element != NULL)
    {
        //debug_hash("del key:%0x entryKey:%0x entry:%p pre:%p", key, element->hashKey, element, preelement);
        if((element->hashKey == key) 
            && 0 == hashTableInfo->hashCompare(GetValue(element), value, hashTableInfo->keyValueSize))
        {
            if(preelement == element)
            {
                *bucket = element->link;
            }

            preelement->link = element->link;
            break;
        }

        preelement = element;
        element = element->link;
    }

    return element;
}

static HASHKEY getHashKey(char *value, int valueSize)
{
    return spooky_hash64(value, valueSize, 0);
}

static int compareHashElement(char *value1, char *value2, int valueSize)
{
    return memcmp(value1,value2, valueSize);
}

static int valueCopy(PHashElement element, char *value, int valueSize)
{
    char *dest = GetValue(element);
    memcpy(dest, value, valueSize);
    return 0;
}