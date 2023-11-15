/*
 *	toadb executor
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_EXECUTOR_H_H
#define HAT_EXECUTOR_H_H

#include "node.h"
#include "portal.h"
#include "tables.h"

/* ExecRetCode > 0  is number of rows. */
#define ExecRetCode_ERR (-1)
#define ExecRetCode_SUC (0)

typedef struct ExecState
{
    CmdType     commandType;
    PNode       plan;
    PPortal     portal;
    PNode       planState;
    PNode       subPlanNode;
    PNode       subPlanStateNode;
    int         isTop;
    int         retCode;
}ExecState, *PExecState;

void ExecutorMain(PList list);

void ExecutorStart(PNode subPlan, PExecState *eState);

void ExecutorPlan(PExecState eState);

void ExecutorEnd(PExecState eState);

void InitExecState(PExecState eState);

void EndExecState(PExecState eState);

void EndExecPlan(PExecState eState);

PTableRowData ExecNodeProc(PExecState eState);

PTableRowData ExecNodeReScan(PExecState eState);

#endif