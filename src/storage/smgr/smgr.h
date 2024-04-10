/*
 *	storage manager 
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

#ifndef HAT_SMGR_H_H
#define HAT_SMGR_H_H

#include "list.h"
#include "tables.h"


/* 
 * filename 
 * table name : table file
 *            : grp_tablename
 */

#include <stdio.h>

#define GROUP_FILE_PRE  "grp_"


#define SMGR_VERSION (0x0678)

#define FILE_MAX_OPENED_PER_PROCESS (1000)

typedef union FileHandle
{
    int fd;
    FILE *fp;
}FileHandle, *PFileHandle;

typedef struct VFVec
{
    DList list;
    int forkNum;
    PFileHandle pfh;
}VFVec, *PVFVec;

typedef struct StorageManagerInfo
{
    int version; 
    int storageType;
    PVFVec vfhead;
    PVFVec vfend;
}sgmrInfo, *PsgmrInfo;

typedef struct StorageMangeContext
{
    PVFVec vfhead;
    int fileMaxNum;
    int fileOpenedNum;
    int vFileNum;
}StorageMangeContext, *PStorageMangeContext;


PVFVec SearchVF(PVFVec head, ForkType forknum);

/* raw operations interface */
PVFVec smgr_create(PsgmrInfo smgrInfo, char *fileName, ForkType forkNum);
PVFVec smgr_open(PsgmrInfo smgrInfo, char *fileName, ForkType forkNum);
int smgr_close(PVFVec vfInfo);

int smgr_read(PVFVec vfInfo, PPageOffset pageOffset, char *page);
int smgr_write(PVFVec vfInfo, PPageOffset pageOffset, PPageHeader page);

int smgrRelease(PsgmrInfo sgmrInfo);




#endif 