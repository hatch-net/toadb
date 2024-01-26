/*
 *	toadb nestloop executor 
 * Copyright (C) 2023-2023, senllang
*/
#ifndef HAT_EXEC_NEST_LOOP_H_H
#define HAT_EXEC_NEST_LOOP_H_H

#include "executor.h"
#include "tables.h"
#include "node.h"

PNode ExecInitNestLoopNode(PExecState eState);

PTableRowData ExecNestLoopNode(PExecState eState);

PTableRowData ExecNestLoopScan(PExecState eState);

PTableRowData ReScanNestLoopNode(PExecState eState);

PTableRowData ExecTableQueryNode(PExecState eState);
#endif
