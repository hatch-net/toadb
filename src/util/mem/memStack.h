/*
 *	toadb memory stack 
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

#ifndef HAT_MEM_STACK_H_H
#define HAT_MEM_STACK_H_H

#include "nodeType.h"

typedef struct List *PList;


#define MEM_CONTEXT_NAME_LEN (64)
typedef struct MemContextNode
{
    NodeType type;
    char contextName[MEM_CONTEXT_NAME_LEN];
    PList memList;
}MemContextNode, *PMemContextNode;

typedef struct MemNode
{
    NodeType type;
    int memSize;
    void *ptr;
}MemNode, *PMemNode;

typedef struct MemValidCheckData
{
    int pos;
    PList parent;
    PList preCell;
    PList currentCell;
    PList currentContext;
}MemValidCheckData, *PMemValidCheckData;

void MemMangerInit();
void MemMangerDestroy();

void *AllocMem(unsigned int size);
int FreeMem(void *pMem);

PMemContextNode MemMangerSwitchContext(PMemContextNode oldContext);
PMemContextNode MemMangerNewContext(char *contextName);
void MemMangerDeleteContext(PMemContextNode preContext, PMemContextNode delContext);

#endif 