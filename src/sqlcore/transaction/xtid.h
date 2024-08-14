/*
 *	toadb transaction id   
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


#ifndef HAT_XTID_H_H
#define HAT_XTID_H_H

#include "public_types.h"

#define INVALID_XTID    ((XTID)(-1))
#define CLEAN_XTID      ((XTID)(0))
#define XTID_ISINVALID(xid) ((xid) == INVALID_XTID)

/* 
 * transaction result state
 * abort include: rollback, other restore status.
*/
typedef enum TransactionState
{
    TRANS_RUNNING,
    TRANS_COMMITED,
    TRANS_ABORT,
    TRANS_UNKNOW
}TransactionState;

#pragma pack(push, 1)

typedef struct TransactionIDInfo
{
    volatile XTID nextTransactionId;
    volatile XTID lowLevelXtid;             /* minimal xtid of running transactions */
    volatile XTID highLevelXtid;            /* maxmum xtid of finish transactions */
}TransactionIDInfo, *PTransactionIDInfo;
#define TRANSACTION_INFO_SIZE (sizeof(TransactionIDInfo))

#pragma pack(pop)

PTransactionIDInfo GetTransDataInfo();

XTID GetNewTransactionID();
XTID SetLowLevelTransaction(XTID xid);
XTID SetHighLevelTransaction(XTID xid);

XTID GetLowLevelTransaction();
XTID GetHighLevelTransaction();

TransactionState GetTransactionState(XTID xtid);

#endif 