/*
 *	toadb nodeType 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_NODE_TYPE_H_H
#define HAT_NODE_TYPE_H_H

/* 
 * 解析树，查询树，计划树的节点类型
 * 注意：这里变动后，尤其中间增加类型后，需要更新树的显示接口数组
 * 以及执行计划初始化接口数组；
 */
typedef enum NodeType
{
	T_START,

	/* parser nodes */
	T_List,
	T_CreateStmt,
    T_ColumnDef,
	T_DropStmt,
	T_InsertStmt,
	T_AttrName,
	T_ValuesData,
	T_SelectStmt,
	T_TableRefName,
	T_ResTarget,
	T_ColumnRef,
	T_ConstValue,
	T_A_Expr,
	T_BoolExpr,
	T_RangeVar,
	T_Alias,

	/* query nodes */
	T_RangTblEntry,
	T_Query,
	T_MergerEntry,
	T_JoinEntry,
	T_ExprEntry,
	T_TargetEntry,

	/* plan nodes */
	T_Plan,
	T_NestLoop,
	T_SeqScan,
	T_ValueScan,
	T_ModifyTbl,
	T_ProjectTbl,
	T_QueryTbl,

	/* executor nodes */
	T_PlanState,
	T_NestLoopState,
	T_SeqScanState,
	T_ValueScanState,
	T_ModifyTblState,
	T_ProjectTblState,
	T_QueryTblState,

	/* memory manager */
	T_MemContextNode,
	T_MemNode,
	T_END
}NodeType;

#endif
