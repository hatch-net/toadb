/*
 *	toadb executor node proc
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

#ifndef HAT_EXEC_NODE_PROC_H_H
#define HAT_EXEC_NODE_PROC_H_H

#include "execNode.h"

PTableRowData ExecNodeProc(PExecState eState);

PTableRowData ExecProcPlan(PExecState eState);

PTableRowData ExecProcNestLoop(PExecState eState);

PTableRowData ExecProcSeqScan(PExecState eState);
PTableRowData ExecProcReSeqScan(PExecState eState);

PTableRowData ExecProcValueScan(PExecState eState);

PTableRowData ExecProcModifyTbl(PExecState eState);

PTableRowData ExecProcProjectTbl(PExecState eState);

PTableRowData ExecSelectResultNode(PExecState eState);

PTableRowData ExecSelectNewValue(PExecState eState);

#endif
