/*
 *	toadb snapshot   
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


#include "snapshot.h"
#include "public_types.h"
#include "public.h"
#include "memStack.h"
#include "transactionControl.h"
#include "xtid.h"
#include "tablecom.h"

#include <stdio.h>
#include <string.h>

//#define SHOW_SNAPSHOT 1

#ifdef SHOW_SNAPSHOT
#define hat_debug_snapshot(...) log_report(LOG_DEBUG, __VA_ARGS__) 
#else 
#define hat_debug_snapshot(...) 
#endif 

static int XtidIsInSnapshot(XTID xtid, PSnapShotInfo snapshot);
static void ShowSnapshot(PSnapShotInfo snapshot);

/* 
 * alloc snapshot resource.
 */
PSnapShotInfo CreateSnapshot(int maxClient)
{
    PSnapShotInfo snaphshot = NULL;
    int size = maxClient * sizeof(XTID) + sizeof(SnapShotInfo);

    snaphshot = (PSnapShotInfo)AllocMem(size);
    snaphshot->xidArrMax = maxClient;
    snaphshot->xidNum = 0;
    snaphshot->runningXidArr = (XTID*)(((char*)snaphshot) + sizeof(SnapShotInfo));

    return snaphshot;
}

void DestroySnapShot(PSnapShotInfo snapshot)
{

}

/* 
 * Generate current snapshot
 */
void GetSnapshot(PSnapShotInfo snapshot)
{
    if(NULL == snapshot)
        return ;

    snapshot->xidNum = GetActiveXtids(snapshot->runningXidArr, snapshot->xidArrMax, &snapshot->highLevel, &snapshot->lowLevel);
}


static int XtidIsInSnapshot(XTID xtid, PSnapShotInfo snapshot)
{
    int txidNum = 0;

    for( ; txidNum < snapshot->xidNum; txidNum++)
    {
        if(xtid == snapshot->runningXidArr[txidNum])
            return 1;
    }
    return 0;
}

/* 
 * tuple visibility judge by snapshot.
 * out-> not running
 * in-> maybe running
 */
TupleVisibility TupleVisibililtySnapshot(XTID currxtid, PSnapShotInfo snapshot)
{
    if(currxtid < snapshot->lowLevel)
    {
        return OUT_SNAPSHOT;
    }
    else if(currxtid < snapshot->highLevel)
    {
        if(XtidIsInSnapshot(currxtid, snapshot))
            return IN_SNAPSHOT;
        else 
            return OUT_SNAPSHOT;
    }
    else 
    {
        ; /* >= snapshot->hightLevel */
    }

    return IN_SNAPSHOT;
}

/*
 * visibility of tuple check from snapshot and tupleHeader.
 * return, TupleVisibility
 */
TupleVisibility TupleVisibilitySatisfy(PTupleHeader tupHeader, PSnapShotInfo snapshot)
{
    TupleVisibility snapshotVisible = TUPLE_INVISIBLE;
    TransactionState tstate = TRANS_UNKNOW;
    XTID curr = GetCurrentTransactionID();

#ifdef SHOW_SNAPSHOT
    ShowSnapshot(snapshot);
#endif 
    hat_debug_snapshot("tuple tmin:%u tmax:%u ", tupHeader->tmin, tupHeader->tmax);

    /* 
     * TODO: transaction commited status 
     * tmin:
     * when tmin >= lowLevel and tmin < highLevel, check tmin in snapshot rang. 
     * in the rang, tmin is in progress, this tuple is not visible.
     * other, continue to check.
     * 
     * tmax: 
     * when tmax >= lowLevel and tmax < highLevel, check tmax in snapshot rang.
     * in the rang, tmax is in progress, this tuple is visible.
     * other, not visible.
     * 
     * advance, transaction state will be check within snapshot branch.
     * running/commited/abort/rollback. 
    */
    if(NULL == snapshot)
    {
        /* no snapshot, all is visiblity. */
        return TUPLE_VISIBLE;
    }

    if(XTID_ISINVALID(tupHeader->tmin))
    {
        /* when abort / crash ocurr, tuple is left. */
        return TUPLE_INVISIBLE;
    }

    /* first , visiblity judge by snapshot. */   
    if(tupHeader->tmin != curr) 
    {
        snapshotVisible = TupleVisibililtySnapshot(tupHeader->tmin, snapshot);
        if(IN_SNAPSHOT == snapshotVisible)
        {
            return TUPLE_INVISIBLE;
        }
        else
        {
            tstate = GetTransactionState(tupHeader->tmin);
            if(TRANS_COMMITED != tstate)
            {
                return TUPLE_INVISIBLE;
            }
        }
    }

    if(XTID_ISINVALID(tupHeader->tmax))
    {
        /* tuple is not update or deleted. */
        return TUPLE_VISIBLE;
    }

    if(tupHeader->tmax != curr) 
    {
        snapshotVisible = TupleVisibililtySnapshot(tupHeader->tmax, snapshot);
        if(IN_SNAPSHOT == snapshotVisible)
        {
            return TUPLE_VISIBLE;
        }
        else
        {
            tstate = GetTransactionState(tupHeader->tmin);
            if(TRANS_COMMITED != tstate)
            {
                /*
                * abort -> visibility
                * running -> crash left
                * commited -> invisibility
                */
                return TUPLE_VISIBLE;
            }
        }
    }

    return TUPLE_INVISIBLE;
}

/*
 * input latest version of tuple.
 * check update/delete is satisfied, result is type of TupleUpdateResult.
 */
TupleUpdateResult TupleUpdateSatisfy(PTupleHeader tupHeader)
{
    TransactionState tstate = TRANS_UNKNOW;
    XTID curr = GetCurrentTransactionID();

    if(XTID_ISINVALID(tupHeader->tmin))
    {
        return TU_Invsible;
    }

    tstate = GetTransactionState(tupHeader->tmin);
    if(TRANS_COMMITED != tstate)
    {
        if(curr == tupHeader->tmin)
        {
            if(XTID_ISINVALID(tupHeader->tmax))
            {
                return TU_OK;
            }

            if(curr != tupHeader->tmax)
            {
                /* error, abort/crash is left. */
                return TU_OK;
            }
        }

        /* tmin is running. */
        return TU_Invsible;
    }

    if(XTID_ISINVALID(tupHeader->tmax))
    {
        return TU_OK;
    }
 #if 0 // TODO:   
    tstate = GetTransactionState(tupHeader->tmax);
    if(TRANS_COMMITED != tstate)
    {
        if(curr == tupHeader->tmax)
        {
            /* tuple is already deleted. */
            return TU_Deleted;
        }

        return TU_BegingModify;
    }
    else
    {
        return TU_Deleted;
    }
#endif 
    return TU_Invsible;
}

#define TEMP_BUFFER_LEN 1024
static void ShowSnapshot(PSnapShotInfo snapshot)
{
    char buffer[TEMP_BUFFER_LEN] = {0};
    char *p = buffer;
    int offset = 0;
    int i = 0;

    if(NULL == snapshot)
        return ;

    for( ; i < snapshot->xidNum; i++)
    {
        snprintf(p, TEMP_BUFFER_LEN - offset - 1, "%u ",snapshot->runningXidArr[i]);
        offset = strlen(buffer);
        p = buffer;
        p += offset;
    }

    hat_log("snapshot infomation: highLevel:%u  lowLevel:%u xidNum:%d xidArrMax%d \n %s", 
            snapshot->highLevel,
            snapshot->lowLevel,
            snapshot->xidNum,
            snapshot->xidArrMax,
            buffer);
}
