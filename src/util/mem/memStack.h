/*
 *	toadb memory stack 
 * Copyright (C) 2023-2023, senllang
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
void MemMangerDeleteContext(PMemContextNode context);

#endif 