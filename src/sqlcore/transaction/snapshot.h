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


#ifndef HAT_SNAPSHOT_H_H
#define HAT_SNAPSHOT_H_H

#include "public_types.h"

typedef enum TupleVisibility 
{
    TUPLE_INVISIBLE,
    TUPLE_VISIBLE,
    
    IN_SNAPSHOT,
    OUT_SNAPSHOT,
    TUPLE_UNKNOWN
}TupleVisibility;


typedef enum TupleUpdateResult
{
    TU_OK,
    TU_Invsible,
    TU_BegingModify,
    TU_Upated,
    TU_Deleted,
    TU_SelfModifed,
    TU_UNKNOW
}TupleUpdateResult;

typedef struct TupleHeader *PTupleHeader;
typedef struct SnapShotInfo
{
    XTID highLevel;
    XTID lowLevel;
    
    int xidNum;
    int xidArrMax;
    XTID *runningXidArr;
}SnapShotInfo, *PSnapShotInfo;


PSnapShotInfo CreateSnapshot(int maxClient);
void DestroySnapShot(PSnapShotInfo snapshot);
void GetSnapshot(PSnapShotInfo snapshot);


TupleVisibility TupleVisibilitySatisfy(PTupleHeader tupHeader, PSnapShotInfo snapshot);
TupleUpdateResult TupleUpdateSatisfy(PTupleHeader tupHeader);

#endif 
