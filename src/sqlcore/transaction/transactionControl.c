/*
 *	toadb transaction control   
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
 * 
*/

#include "transactionControl.h"
#include "shareMem.h"
#include "threadPool.h"
#include "logger.h"
#include "xtid.h"
#include "tablecom.h"
#include "servprocess.h"
#include "snapshot.h"

#include <string.h>

static PProcTransactionInfo procTransactionInfo;
ThreadLocal PTransactionContext myTransaction = NULL;

int config_transIsolationLevel = til_Read_Repeatable;

int InitTransactContext(int maxNum)
{
    int memSize = sizeof(ProcTransactionInfo);

    if(maxNum <= 0)
        return -1;
    
    memSize += maxNum * sizeof(TransactionContext);
    procTransactionInfo = (PProcTransactionInfo)SMemAlloc(memSize);
    memset(procTransactionInfo, 0x00, memSize);

    procTransactionInfo->maxTransactonNum = maxNum;

    return 0;
}


/* 
 * transaction is starting.
 */
void StartTransaction()
{
    int myIndex = 0;
    
    if(NULL == myTransaction)
    {
        myIndex = GetCurrentWorkerIndex();
        if(myIndex > procTransactionInfo->maxTransactonNum)
        {
            hat_error("myIndex %d is overhead maxmum %d transaction array.", myIndex, procTransactionInfo->maxTransactonNum);
            return ;
        }

        myTransaction = &(procTransactionInfo->transactionContextArray[myIndex-1]);
        myTransaction->snapshot = 0;
    }
    else
    {
        /* transaction is already started. */
        return ;
    }

    if(TS_IDLE == myTransaction->ts)
    {
        myTransaction->ts = TS_START;
        myTransaction->transactionID = GetNewTransactionID();
    }
    else 
    {
        hat_error("transaction state is %d, starting error.", myTransaction->ts);
    }

    hat_log("transaction %u state is %d.", myTransaction->transactionID, myTransaction->ts);
}

/* 
 * transaction is begin.
 */
void BeginTransaction()
{
    int myIndex = 0;
    
    if(NULL == myTransaction)
    {
        myIndex = GetCurrentWorkerIndex();
        if(myIndex > procTransactionInfo->maxTransactonNum)
        {
            hat_error("myIndex %d is overhead maxmum %d transaction array.", myIndex, procTransactionInfo->maxTransactonNum);
            return ;
        }

        myTransaction = &(procTransactionInfo->transactionContextArray[myIndex-1]);
        myTransaction->snapshot = 0;
    }

    switch(myTransaction->ts)
    {
    case TS_IDLE:
        myTransaction->transactionID = GetNewTransactionID();
    case TS_START:
        myTransaction->ts = TS_BEGIN;
    break;
    case TS_BEGIN:
    break;
    case TS_INPROCESS:
    break;
    case TS_COMMIT:
    case TS_ABORT:
    break;
    default:
    break;
    }

    hat_log("transaction %u state is %d.", myTransaction->transactionID, myTransaction->ts);
}

/* 
 * transaction is end with commit.
 */
void EndTransaction()
{
    int myIndex = 0;
    
    if(NULL == myTransaction)
    {
        return ;
    }

    /* transaction is already started. */
    if(TS_IDLE == myTransaction->ts)
    {
        hat_error("transaction state is %d, end error.", myTransaction->ts);
        return ;
    }

    myTransaction->ts = TS_COMMIT;
    hat_log("transaction %u state is %d.", myTransaction->transactionID, myTransaction->ts);
}

/* 
 * transaction is end with commit.
 */
void AbortTransaction()
{
    int myIndex = 0;
    
    if(NULL == myTransaction)
    {
        return ;
    }

    /* transaction is already started. */
    if(TS_IDLE == myTransaction->ts)
    {
        hat_error("transaction state is %d, abort error.", myTransaction->ts);
        return ;
    }

    myTransaction->ts = TS_ABORT;
    hat_log("transaction %u state is %d.", myTransaction->transactionID, myTransaction->ts);
}

/*
 * transaction is finishing, 
 * which will be commited, or be aborted.
 */
void FinishTransaction()
{
    /* transaction is not started. */
    if(NULL == myTransaction)
        return ;
   
    switch(myTransaction->ts)
    {
    case TS_IDLE:
    break;
    case TS_START:
        myTransaction->ts = TS_IDLE;
        
        /* max finish transaction id */
        SetHighLevelTransaction(myTransaction->transactionID);

        myTransaction = NULL;
    break;
    case TS_BEGIN:
    break;
    case TS_INPROCESS:
    break;
    case TS_COMMIT:
    case TS_ABORT:
        myTransaction->ts = TS_IDLE;
        
        /* max finish transaction id */
        SetHighLevelTransaction(myTransaction->transactionID);

        myTransaction = NULL;
    break;
    default:
    break;
    }

    if(NULL != myTransaction)
        hat_log("transaction %u state is %d.", myTransaction->transactionID, myTransaction->ts);
}


TransactionFSMState GetCurrentTransactionState()
{
    if(NULL == myTransaction)
        return TS_IDLE;
    
    return myTransaction->ts;
}

XTID GetCurrentTransactionID()
{
    if(NULL == myTransaction)
        return INVALID_XTID;

    return myTransaction->transactionID;
}

void IncCommandCount()
{

}

void ComputeXminTupleHeader(PTupleHeader tupHeader)
{
    if(NULL == tupHeader)
        return ;

    tupHeader->cid = 0;
    tupHeader->tmin = GetCurrentTransactionID();
    tupHeader->tmax = INVALID_XTID;
    tupHeader->tinfo = CLEAN_XTID;
}

void ComputeXmaxTupleHeader(PTupleHeader tupHeader, PTupleHeader oldHeader)
{
    if((NULL == tupHeader) || (NULL == oldHeader))
        return ;

    tupHeader->cid = 0;
    tupHeader->tmin = GetCurrentTransactionID();
    tupHeader->tmax = INVALID_XTID;
    tupHeader->tinfo = CLEAN_XTID;

    oldHeader->cid = 0;
    oldHeader->tmax = tupHeader->tmin;
}

int GetActiveXtids(XTID *xtids, int maxNum, XTID *maxXtid, XTID *minXtid)
{
    int count = 0;
    int i = 0;
    XTID max = GetHighLevelTransaction() + 1; 
    XTID min = max;
    
    if(NULL == xtids)
        return 0;

    /* low limit include current transaction. */
    if((NULL != myTransaction) && (myTransaction->transactionID < min))
        min = myTransaction->transactionID;

    /* TODO: lock */
    for(i = 0; i < procTransactionInfo->maxTransactonNum; i ++)
    {
        if((TS_IDLE == procTransactionInfo->transactionContextArray[i].ts) 
            || ((NULL != myTransaction) && (procTransactionInfo->transactionContextArray[i].transactionID == myTransaction->transactionID)))
            continue;

        /* xtid overhead max is treated as running . */
        if(xtids[count] > max)
            continue;
        
        xtids[count] = procTransactionInfo->transactionContextArray[i].transactionID;

        if(xtids[count] < min)
            min = xtids[count];
           
        if(++count >= maxNum)
            break;
    }

    *maxXtid = max;
    *minXtid = min;

    return count;
}

void GenerateSnapshot()
{
    int isNewSnapShot = 0;

    if(NULL == myTransaction)
        return ;

    switch(config_transIsolationLevel)
    {
        case til_Read_Commited:
            /* snapshot is generated of entering. */
            isNewSnapShot = 1;
        break;
        case til_Read_Repeatable:
            /* snapshot is generated once. */
            isNewSnapShot = ! myTransaction->snapshot;
        break;
        default:
        break;
    }

    if(isNewSnapShot)
    {
        GetSnapshot(GetServSnapShot());
        myTransaction->snapshot = 1;
    }
}
