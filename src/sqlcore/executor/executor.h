/*
 *	toadb executor
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
*/

#ifndef HAT_EXECUTOR_H_H
#define HAT_EXECUTOR_H_H

#include "node.h"
#include "portal.h"
#include "tables.h"
#include "snapshot.h"

/* ExecRetCode > 0  is number of rows. */
#define ExecRetCode_ERR (-1)
#define ExecRetCode_SUC (0)

typedef struct ExecState
{
    CmdType     commandType;
    PNode       plan;                   /* 原始计划树根节点 */
    PPortal     portal;
    PNode       planState;              /* 原始计划树状态树根节点 */
    PNode       subPlanNode;            /* 当前正在处理的计划树节点 */
    PNode       parentPlanNode;
    PNode       subPlanStateNode;       /* 当前正在处理的计划树状态节点 */
    PNode       parentPlanStateNode;
    PTableRowData scanRowDataLeft;      /* 当前得到的数据行 */
    PTableRowData scanRowDataRight;     /* 当前得到的数据行 */
    PSnapShotInfo snapshot;
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