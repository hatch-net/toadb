/*
 *	toadb executor node proc
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_EXEC_NODE_PROC_H_H
#define HAT_EXEC_NODE_PROC_H_H

#include "execNode.h"

PTableRowData ExecNodeProc(PExecState eState);

PTableRowData ExecProcPlan(PExecState eState);

PTableRowData ExecProcNestLoop(PExecState eState);

PTableRowData ExecProcSeqScan(PExecState eState);

PTableRowData ExecProcValueScan(PExecState eState);

PTableRowData ExecProcModifyTbl(PExecState eState);

PTableRowData ExecProcProjectTbl(PExecState eState);


#endif
