/*
 *	toadb portal
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_PORTAL_H_H
#define HAT_PORTAL_H_H

#include "node.h"
#include "list.h"

#define PORT_BUFFER_SIZE 1024

typedef struct Portal
{
    int opType;
    int num;
    char buffer[PORT_BUFFER_SIZE];
    PDList list;
}Portal, *PPortal;


typedef struct ScanHeaderRowInfo
{
    char *colName;
    int colType;
    int colMaxSize;    
}ScanHeaderRowInfo, *PScanHeaderRowInfo;

PPortal CreatePortal(PSelectStmt stmt);
int SendToPortal(PPortal portal);
int EndPort(PPortal portal);

int PortalPrint(char *buf);
PScanHeaderRowInfo GetRowInfoNode(PPortal portal, char *colName);

#endif