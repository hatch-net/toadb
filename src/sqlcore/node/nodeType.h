/*
 *	toadb nodeType 
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
	T_UpdateStmt,
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
	T_SelectResult,
	T_SelectNewValue,
	
	/* executor nodes */
	T_PlanState,
	T_NestLoopState,
	T_SeqScanState,
	T_ValueScanState,
	T_ModifyTblState,
	T_ProjectTblState,
	T_QueryTblState,
	T_SelectState,

	/* memory manager */
	T_MemContextNode,
	T_MemNode,
	T_END
}NodeType;

#endif
