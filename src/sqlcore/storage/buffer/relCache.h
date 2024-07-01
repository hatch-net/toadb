/*
 *	toadb relation cache 
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
 * 
*/

#ifndef HAT_REL_CACHE_H_H
#define HAT_REL_CACHE_H_H

#include "list.h"
#include "relation.h"
#include "tables.h"
#include "smgr.h"
#include "rwlock.h"

typedef struct TableList
{
    DList list;
    PRelation rel;
    PPageDataHeader tableInfo;  /* first data page */
    PTableMetaInfo tableDef;
    PGroupPageHeader groupInfo;  /* first group page */
    PsgmrInfo sgmr;
    RWLockInfo tblLock;          /* table lock level */
    RWLockInfo extLock;          /* table extension */
}TableList, *PTableList;

typedef struct DictionaryContext
{
    DList *tblList;
    RWLockInfo rwLock;
}DictionaryContext, *PDictionaryContext;


int InitTblInfo();
int ReleaseTblInfo(PTableList tblInfo);
int ReleaseAllTblInfoResource();

/* search table metadata infomation, if NULL, load from file. */
PTableList GetTableInfo(char *filename);
PTableList GetTableInfoByRel(PRelation rel);

PTableList CreateTblInfo(PTableMetaInfo tblDef);

/* search table metadata, return index of attribute Name position. if not found, return -1 . */
int GetAttrIndex(PTableList tblInfo, char *attrName);

/* search table metadata, if not found, return NULL . */
PColumnDefInfo GetAttrDef(PTableList tblInfo, char *attrName);

char * CreateDictionaryItem(int size);

#define StartExtensionTbl(tbl) StartExtensionLock(tbl, __FUNCTION__, __LINE__)
#define EndExtensionTbl(tbl) EndExtensionLock(tbl, __FUNCTION__, __LINE__)

void StartExtensionLock(PTableList tblInfo, char *fun, int line);
void EndExtensionLock(PTableList tblInfo, char *fun, int line);

#endif 