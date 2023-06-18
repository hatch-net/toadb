/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_NODE_H_H
#define HAT_NODE_H_H

#define NewNode(type) (type *)CreateNode(sizeof(type),T_##type)

typedef enum NodeType
{
	T_START,
	T_List,
	T_CreateStmt,
    T_ColumnDef,
	T_DropStmt,
	T_InsertStmt,
	T_AttrName,
	T_ValuesData,
	T_SelectStmt,
	T_TableRefName,
	T_END
}NodeType;

typedef union Data
{
	void *pData;
	int  iData;
	char cData;
	double dData;
}Data;

typedef union CellValue
{
    void* pValue;   /* node struct pointer */
    int iValue;     
}CellValue;

typedef struct ListCell
{
    CellValue value;
    struct ListCell *next;
}ListCell, *PListCell;

typedef struct List
{
	NodeType type;
	int length;         /* number of ListCell struct */
	ListCell *head;
	ListCell *tail;
}List, *PList;

/* common type, real size is just by type. */
typedef struct Node
{
    NodeType type;
}Node, *PNode;

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
	Data value;
}ValuesData, *PValuesData;


typedef struct SelectStmt
{
	NodeType type;
	int selectAll;
	List *columnList;
	List *tblList;
}SelectStmt, *PSelectStmt;


PNode CreateNode(int size, NodeType type);
PList CreateCell(PList list);

void travelParserTree(PList list);

/* 根据表名获取取 attrnode */
int GetAtrrIndexByName(char *attrName, PList list);
/* 根据atrrnode index， 获取对应的values */
PValuesData GetDataByIndex(int index, PList list);


#endif
