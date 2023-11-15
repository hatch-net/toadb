/*
 *	toadb parserNode 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_PARSER_NODE_H_H
#define HAT_PARSER_NODE_H_H

#include "node.h"
#include "dataTypes.h"

typedef union Data
{
	void *pData;
	int  iData;
	char cData;
	double dData;
	float fData;
}Data;

typedef struct CreateStmt
{
    /* tablename, list of map (column name, column type), other... */
	NodeType type;
	char *tableName;
	List *ColList;
}CreateStmt, *PCreateStmt;

typedef struct ColumnDef
{
	NodeType type;
	char *colName;
	char *colType;
	int ctype;				/* TODO: change type to integer , maybe change process all . */
}ColumnDef, *PColumnDef;

typedef struct DropStmt
{
    /* tablename , other optional ... */
	NodeType type;
	char *tableName;
}DropStmt, *PDropStmt;

typedef struct InsertStmt
{
	NodeType type;
	char *tableName;
	List *attrNameList;
	List *valuesList;
}InsertStmt, *PInsertStmt;

typedef struct AttrName
{
	NodeType type;
	char *attrName;
}AttrName, *PAttrName;


typedef struct TableRefName
{
	NodeType type;
	char *tblRefName;
}TableRefName, *PTableRefName;

typedef struct ValuesData
{
	NodeType type;
	PNode valueNode;			/* constvalue node */
	Data value;					/* not used */
}ValuesData, *PValuesData;

/* 
 * select statement structure 
 */
typedef struct SelectStmt
{
	NodeType type;
	int selectAll;
	List *targetlist;
	List *relrange;
	List *fromList;
	List *whereList;
	List *groupList;
	List *sortList;
	List *limitClause;
}SelectStmt, *PSelectStmt;

/* 
 * result target structure 
 */
typedef struct ResTarget
{
	NodeType	type;
	char	   *name;			
	List	   *indirection;	
	Node	   *val;
	int 		all;			
} ResTarget, *PResTarget;


/* 
 * column reference 
 */
typedef struct ColumnRef
{
	NodeType	type;
	char	   *field;
	char	   *tableName;
	valueType   vt;
}ColumnRef, *PColumnRef;

/* const value */
typedef struct ConstValue
{
	NodeType	type;
	Data		val;
	valueType   vt;
	int			isnull;			
} ConstValue, *PConstValue;


/*
 * A_Expr - infix, prefix, and postfix expressions
 */
typedef enum A_Expr_Type
{
	AEXPR_OP,					/* normal operator */
	AEXPR_OP_ANY,				/* scalar op ANY (array) */
	AEXPR_OP_ALL,				/* scalar op ALL (array) */
	AEXPR_DISTINCT,				/* IS DISTINCT FROM - name must be "=" */
	AEXPR_NOT_DISTINCT,			/* IS NOT DISTINCT FROM - name must be "=" */
	AEXPR_NULLIF,				/* NULLIF - name must be "=" */
	AEXPR_IN,					/* [NOT] IN - name must be "=" or "<>" */
	AEXPR_LIKE,					/* [NOT] LIKE - name must be "~~" or "!~~" */
	AEXPR_ILIKE,				/* [NOT] ILIKE - name must be "~~*" or "!~~*" */
	AEXPR_SIMILAR,				/* [NOT] SIMILAR - name must be "~" or "!~" */
	AEXPR_BETWEEN,				/* name must be "BETWEEN" */
	AEXPR_NOT_BETWEEN,			/* name must be "NOT BETWEEN" */
	AEXPR_BETWEEN_SYM,			/* name must be "BETWEEN SYMMETRIC" */
	AEXPR_NOT_BETWEEN_SYM		/* name must be "NOT BETWEEN SYMMETRIC" */
} A_Expr_Type;

typedef enum A_A_Expr_Op_Type
{
	LESS,			/* < */
	LESS_EQUAL,		/* <= */
	EQUAL,			/* = */
	GREATER_EQUAL,		/* >= */
	GREATER,		/* > */
	NOT_EQUAL,			/* <> */
	POSITIVE,		/* + */
	NEGATIVE,		/* - */
	PLUS,			/* + */
	MINUS,			/* - */
	MULTIPLE,		/* * */
	DIVISIION,		/* / */
	MOD 			/* % */
}A_A_Expr_Op_Type;

typedef struct A_Expr
{
	NodeType	type;
	A_Expr_Type exprType;			
	char	   *name;		
	A_A_Expr_Op_Type exprOpType;
	Node	   *lexpr;			
	Node	   *rexpr;			
} A_Expr, *PA_Expr;


typedef enum BoolExprType
{
	AND_EXPR, 
	OR_EXPR, 
	NOT_EXPR
} BoolExprType;

typedef struct BoolExpr
{
	NodeType	type;
	BoolExprType boolop;
	List	   *args;			
} BoolExpr, *PBoolExpr;

/* relation alias name struction */
typedef struct Alias
{
	NodeType	type;
	char	   *aliasname;	
} Alias, *PAlias;

/* from clause relation information struction */
typedef struct RangeVar
{
	NodeType	type;
	char	   *relname;
	Alias	   *alias;
} RangeVar, *PRangeVar;

/* 根据列名获取在targetlist中的序号 */
int GetAtrrIndexByName(char *attrName, PList list);
int GetColumnRefIndexByName(char *attrName, PList list);

/* 根据atrrnode index， 获取对应的values */
PValuesData GetDataByIndex(int index, PList list);

#endif
