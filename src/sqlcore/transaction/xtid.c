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

#include "xtid.h"
#include "atom.h"

static TransactionIDInfo transactionInfo = {1,0,0};

PTransactionIDInfo GetTransDataInfo()
{
    return &transactionInfo;
}

XTID GetNewTransactionID()
{
    return atomic_fetch_add64(&transactionInfo.nextTransactionId, 1);
}

XTID SetHighLevelTransaction(XTID xid)
{
    XTID highlevelXid = transactionInfo.highLevelXtid;

    if(XTID_ISINVALID(xid))
        return highlevelXid;

    do {
        if(highlevelXid >= xid)
            break;
        
        /* update highlevelXid with current value. */
        if(atomic_compare_exchange64(&transactionInfo.highLevelXtid, &highlevelXid, xid))
            break;
    }while(1);

    return highlevelXid;
}

XTID SetLowLevelTransaction(XTID xid)
{
    XTID lowLevelXtid = transactionInfo.lowLevelXtid;

    if(XTID_ISINVALID(xid))
        return lowLevelXtid;

    do {
        if(lowLevelXtid <= xid)
            break;
        
        /* update highlevelXid with current value. */
        if(atomic_compare_exchange64(&transactionInfo.lowLevelXtid, &lowLevelXtid, xid))
            break;
    }while(1);

    return lowLevelXtid;
}

XTID GetLowLevelTransaction()
{
    return transactionInfo.lowLevelXtid;
}

XTID GetHighLevelTransaction()
{
    return transactionInfo.highLevelXtid;
}

TransactionState GetTransactionState(XTID xtid)
{
    /* TODO: */
    return TRANS_COMMITED;
}