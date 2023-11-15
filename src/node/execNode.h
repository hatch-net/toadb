/*
 *	toadb execNode 
 * Copyright (C) 2023-2023, senllang
*/


#ifndef HAT_EXEC_NODE_H_H
#define HAT_EXEC_NODE_H_H

#include "node.h"
#include "portal.h"
#include "planNode.h"
#include "tables.h"
#include "seqscan.h"

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
}PlanStateNode, *PPlanStateNode;

typedef struct PlanState
{
    PPlanStateNode stateNode;

    PNode       left;
    PNode       right;    
    PScanState scanState;
}PlanState, *PPlanState;

typedef struct NestLoopState
{
    PPlanStateNode stateNode;

    PNode       left;
    PNode       right;
    int         outerNeedNew;
    int         outerIsEnd;
    int         innerNeedNew;
    int         innerIsEnd;
    PTableRowData scanRowDataLeft;
    PTableRowData scanRowDataRight;
}NestLoopState, *PNestLoopState;

typedef struct SeqScanState
{
    PPlanStateNode stateNode;
    PScanState scanState;
}SeqScanState, *PSeqScanState;

typedef struct ValueScanState
{
    PPlanStateNode stateNode;
    PScanState scanState;
}ValueScanState, *PValueScanState;

typedef struct ModifyTblState
{
    PPlanStateNode stateNode;

    PNode       left;
    PNode       right;

    PScanState scanState;
}ModifyTblState, *PModifyTblState;

typedef struct ProjectTblState
{
    PPlanStateNode stateNode;

    PNode       subplanState;

    PScanState scanState;
}ProjectTblState, *PProjectTblState;


typedef struct QueryTblState
{
    PPlanStateNode stateNode;

    PNode       subplanState;

    PScanState scanState;
}QueryTblState, *PQueryTblState;

#endif
