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
#include "tables.h"
#include "list.h"

#define PORT_BUFFER_SIZE 2048

typedef struct Scan *PScan;
typedef struct PlanStateNode *PPlanStateNode;
typedef struct TableRowData *PTableRowData;

typedef enum msgType
{
    PORT_MSG_START,
    PORT_MSG_BEGIN,
    PORT_MSG_CONTINUE,
    PORT_MSG_FINISH,
    PORT_MSG_END
}msgType;

typedef struct MsgHeader
{
    int type;
    int size;
    char body[PORT_BUFFER_SIZE];
}MsgHeader, *PMsgHeader;
#define MSG_HEADER_LEN (sizeof(int)+sizeof(int))

typedef enum PortFlag
{
    PORT_ROW_HEADER,
    PORT_ROW_STRING,
    PORT_NOTHING
}PortFlag;

typedef struct Portal
{
    int opType;
    char *buffer;
    int bufOffset;
    int clientFd;
    int flag;       /* table header is sended ? 1 ,else 0 */

    MsgHeader msgBody;

    /* All rows will be push in this list. 
     * first is the title row, which type is ScanHeaderRowInfo. 
     * other row type is PTableRowData. 
     */
    PDList list;   
    PTableRowData rowData;

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
void DestroyPortal(PPortal portal);

int GetPortalSize();
int InitPortal(PPortal portal);
int InitSelectPortal(PList targetList, PPortal portal);

int FlushPortal(PPortal portal);
int SendToPortStr(PPortal portal, char *str);
int SendToPort(PPlanStateNode rowDataInfo, PTableRowData rowData);
int FinishSend(PPortal portal);

int EndPort(PPortal portal);

int PortalPrint(char *buf);
PScanHeaderRowInfo GetRowInfoNode(PPortal portal, char *colName);

int ClientFormRow(PScan scanHead, PSelectStmt stmt);
int GetValueInfo(valueType vt, PRowColumnData colmnData, char *buffer, int Maxwidth);
#endif