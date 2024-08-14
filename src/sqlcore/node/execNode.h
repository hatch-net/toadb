/*
 *	toadb execNode 
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


#ifndef HAT_EXEC_NODE_H_H
#define HAT_EXEC_NODE_H_H

#include "node.h"
#include "portal.h"
#include "planNode.h"
#include "tables.h"
#include "seqscan.h"
#include "execProject.h"
#include "execSelect.h"
#include "execNestLoop.h"

typedef struct ExecState *PExecState;
typedef PTableRowData (*execProcNodeFunc)(PExecState eStat);

typedef struct PlanStateNode
{
    NodeType    type;
    CmdType     commandType;
    PNode       plan;
    PPortal     portal;
    execProcNodeFunc execProcNode;
    execProcNodeFunc execReScanNode;
    execProcNodeFunc execEndProcNode;
}PlanStateNode, *PPlanStateNode;

typedef struct PlanState
{
    PlanStateNode stateNode;

    PNode       left;
    PNode       right;    

}PlanState, *PPlanState;

typedef struct NestLoopState
{
    PlanStateNode stateNode;

    PNode       left;
    PNode       right;
    PNode       outResultRow;       /* left查询结果，实际为rowdata指针，这里为了不引用，转为PNode类型 */
    int         outerNeedNew;
    int         outerIsEnd;
    int         innerNeedNew;
    int         innerIsEnd;
    PNestLoopData nestloopData;
}NestLoopState, *PNestLoopState;

typedef struct SeqScanState
{
    PlanStateNode stateNode;
    PScanState scanState;
}SeqScanState, *PSeqScanState;

typedef struct ValueScanState
{
    PlanStateNode stateNode;
    PScanState scanState;
}ValueScanState, *PValueScanState;

typedef struct ModifyTblState
{
    PlanStateNode stateNode;

    PNode       left;
    PNode       right;

    PScanState scanState;
}ModifyTblState, *PModifyTblState;

typedef struct ProjectTblState
{
    PlanStateNode stateNode;
    PNode       subplanState;
    PScanState scanState;
    PNode       resultRow;       /* left查询结果，实际为rowdata/PTableRowDataWithPos指针，这里为了不引用，转为PNode类型 */
    PProjectStateData prjData;
}ProjectTblState, *PProjectTblState;


typedef struct QueryTblState
{
    PlanStateNode stateNode;

    PNode       subplanState;
}QueryTblState, *PQueryTblState;

typedef struct SelectState
{
    PlanStateNode stateNode;

    PNode       subplanState;

    PScanState scanState;
    PNode       resultRow;       /* left查询结果，实际为rowdata指针，这里为了不引用，转为PNode类型 */
    PList       rtable;

    PSelectExpreData selectExpreData; 
}SelectState, *PSelectState;

#endif
