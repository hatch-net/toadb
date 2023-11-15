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
    PlanStateNode stateNode;

    PNode       left;
    PNode       right;    
    PScanState scanState;
}PlanState, *PPlanState;

typedef struct NestLoopState
{
    PlanStateNode stateNode;

    PNode       left;
    PNode       right;
    PNode       outResultRow;
    int         outerNeedNew;
    int         outerIsEnd;
    int         innerNeedNew;
    int         innerIsEnd;

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
}ProjectTblState, *PProjectTblState;


typedef struct QueryTblState
{
    PlanStateNode stateNode;

    PNode       subplanState;

    PScanState scanState;
}QueryTblState, *PQueryTblState;

#endif
