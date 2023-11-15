/*
 *	toadb portal
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_PORTAL_H_H
#define HAT_PORTAL_H_H

#include "parserNode.h"
#include "list.h"

#define PORT_BUFFER_SIZE 8192

typedef struct Scan *PScan;
typedef struct PlanStateNode *PPlanStateNode;
typedef struct TableRowData *PTableRowData;

typedef struct Portal
{
    int opType;
    char buffer[PORT_BUFFER_SIZE];

    /* All rows will be push in this list. 
     * first is the title row, which type is ScanHeaderRowInfo. 
     * other row type is PTableRowData. 
     */
    PDList list;   

    /* storage rowdata which searched. */
    PDList rows;                 
    int num;
    PList targetList;

    /* temp */
    valueType *targetValTypeArr;
    int *attrWidth; 
}Portal, *PPortal;


typedef struct ScanHeaderRowInfo
{
    char *colName;
    int colIndex;
    int colType;
    int colMaxSize;    
}ScanHeaderRowInfo, *PScanHeaderRowInfo;

typedef enum SHOW_PHARE
{
    SHOW_PHARE_ROW_SIZE,
    SHOW_PHARE_SHOW_DATA,
    SHOW_PHARE_MAX
}SHOW_PHARE;

PPortal CreatePortal();
int InitSelectPortal(PList targetList, PPortal portal);

int FlushPortal(PPortal portal);
int SendToPort(PPlanStateNode rowDataInfo, PTableRowData rowData);
int FinishSend(PPortal portal);

int EndPort(PPortal portal);

int PortalPrint(char *buf);
PScanHeaderRowInfo GetRowInfoNode(PPortal portal, char *colName);

int ClientFormRow(PScan scanHead, PSelectStmt stmt);
#endif