/*
 *	toadb portal
 *
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

#ifndef HAT_PORTAL_H_H
#define HAT_PORTAL_H_H

#include "parserNode.h"
#include "list.h"

#define PORT_BUFFER_SIZE 2048

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
int GetPortalSize();
int InitSelectPortal(PList targetList, PPortal portal);

int FlushPortal(PPortal portal);
int SendToPort(PPlanStateNode rowDataInfo, PTableRowData rowData);
int FinishSend(PPortal portal);

int EndPort(PPortal portal);

int PortalPrint(char *buf);
PScanHeaderRowInfo GetRowInfoNode(PPortal portal, char *colName);

int ClientFormRow(PScan scanHead, PSelectStmt stmt);
#endif