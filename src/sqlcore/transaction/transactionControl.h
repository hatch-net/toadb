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


#ifndef HAT_TRANSACTION_CONTROL_H_H
#define HAT_TRANSACTION_CONTROL_H_H

#include "public_types.h"

typedef enum TransactionIsolationLevel
{
    til_Read_Commited,
    til_Read_Repeatable
}TransactionIsolationLevel;

typedef enum TransactionFSMState
{
    TS_IDLE,
    TS_START,
    TS_BEGIN,
    TS_INPROCESS,
    TS_COMMIT,
    TS_ABORT,
    TS_UNKNOW
}TransactionFSMState;

typedef struct TransactionContext
{
    volatile TransactionFSMState ts;
    XTID transactionID;
    int snapshot;
}TransactionContext, *PTransactionContext;


typedef struct ProcTransactionInfo
{
    int maxTransactonNum;
    TransactionContext transactionContextArray[];
}ProcTransactionInfo, *PProcTransactionInfo;

int InitTransactContext(int maxNum);

void StartTransaction();
void BeginTransaction();
void EndTransaction();
void AbortTransaction();
void FinishTransaction();
void IncCommandCount();

XTID GetCurrentTransactionID();
TransactionFSMState GetCurrentTransactionState();

typedef struct TupleHeader *PTupleHeader;
void ComputeXminTupleHeader(PTupleHeader tupHeader);
void ComputeXmaxTupleHeader(PTupleHeader tupHeader, PTupleHeader oldHeader);

int GetActiveXtids(XTID *xtids, int maxNum, XTID *maxXtid, XTID *minXtid);
void GenerateSnapshot();

#endif 