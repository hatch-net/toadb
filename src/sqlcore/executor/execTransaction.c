/*
 *	toadb execTransaction 
 *
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

#include "execTransaction.h"
#include "transactionControl.h"


int ExecTransaction(PTransactionStmt stmt, PPortal portal)
{
    TransactionFSMState currentTranState = TS_IDLE;
    int ret = -1;

    currentTranState = GetCurrentTransactionState();
    switch(stmt->transactionTag)
    {
    case TF_BEGIN:
        BeginTransaction();
        ret = 0;
    break;
    case TF_END:
    case TF_COMMIT:
        if(TS_IDLE != currentTranState)
        {
            EndTransaction();
            ret = 0;
        }
    break;
	case TF_ROLLBACK:
        if(TS_IDLE != currentTranState)
        {
            AbortTransaction();
            ret = 0;
        }
    break;
	case TF_SAVEPOINT:
        ret = 0;
    break;
    default:
    break;
    }

    return ret;
}