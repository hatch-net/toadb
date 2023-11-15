/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_NODE_H_H
#define HAT_NODE_H_H

#include "nodeType.h"

#define NewNode(type) (type *)CreateNode(sizeof(type),T_##type)

#define GetCellNodeValue(cellNode) (PNode)((cellNode)->value.pValue)
#define GetFirstCellNode(list) (PNode)(GetCellNodeValue((list)->head))
#define GetSecondCellNode(list) (PNode)(GetCellNodeValue((list)->head->next))


typedef enum CmdType
{
	CMD_UNKNOWN,
	CMD_SELECT,					/* select stmt */
	CMD_UPDATE,					/* update stmt */
	CMD_INSERT,					/* insert stmt */
	CMD_DELETE,					/* delete stmt */
	CMD_UTILITY,				/* cmds like create, drop, etc. */
} CmdType;

/* common type, real size is just by type. */
typedef struct Node
{
    NodeType type;
}Node, *PNode;


typedef union CellValue
{
    void* pValue;   /* node struct pointer */
    int iValue;     
}CellValue;

/* tree list cell */
typedef struct ListCell
{
    CellValue value;
    struct ListCell *next;
}ListCell, *PListCell;

/* tree list node */
typedef struct List
{
	NodeType type;
	int length;         /* number of ListCell struct */
	ListCell *head;
	ListCell *tail;
}List, *PList;


typedef void (*fpNodeShow)(PNode n, char *prompt, int level);

PNode CreateNode(int size, NodeType type);
PList CreateCell(PList list);

/* append node to the list tail */
PList AppendNode(PList list, PNode n);
PList AppendCellNode(PList list, PListCell cell);

/* create empty list, and append the node to the tail if it is not null. */
PList CreateList(PNode n);

PNode GetCellValueByIndex(PList list, int index);

void travelParserTree(PList list, char *prompt);

#endif
