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
	T_END
}NodeType;

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

PNode CreateNode(int size, NodeType type);
PList CreateCell(PList list);

void travelParserTree(PList list);
#endif
