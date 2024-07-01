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


#include "portal.h"
#include "buffer.h"
#include "seqscan.h"
#include "execNode.h"
#include "queryNode.h"
#include "toadmain.h"
#include "servprocess.h"
#include "netclient.h"
#include "memStack.h"
#include "public.h"
#include "hatstring.h"

#include <stdio.h>
#include <string.h>


#define hat_debug_portal(...) log_report(LOG_DEBUG, __VA_ARGS__) 

#define NULL_VALUE " "

extern int runMode;

static int fillBack(char *buf, char op, int size);

static int InitTargetAttrType(PPortal portal);

static int SendToNetPort(PPortal portal);

static int GeneralVirtualTableHead(PPortal portal);

static int ClientRowDataShow(PPortal portal);


int GetPortalSize()
{
    int size = sizeof(Portal) ;
    return size;
}

/*
 * initialize portal memory. 
 * 
 * portal is a describe of client, 
 * which is managed by server.
 *
 */
PPortal CreatePortal()
{
    PPortal portal = NULL;
    int portalSize = GetPortalSize();

    portal = AllocMem(portalSize);

    portal->clientFd = GetServFd();
    portal->buffer = portal->msgBody.body;

    InitPortal(portal);
    return portal;
}

void DestroyPortal(PPortal portal)
{
    if(NULL == portal)
        return ;
    
    FreeMem(portal);
}

int InitPortal(PPortal portal)
{
    portal->msgBody.type = PORT_MSG_START;
    portal->flag = PORT_NOTHING;

    return 0;
}

/* 
 * Add column define to portal, rows match the targetList. 
 */
int InitSelectPortal(PList targetList, PPortal portal)
{
    int ret = 0;

    if(NULL == portal->targetList)
        portal->targetList = targetList;
    
    portal->flag = PORT_ROW_HEADER;

    ret = InitTargetAttrType(portal);

    return ret;
}

/*
*/
int PortalPrint(char *buf)
{
    if(TOADSERV_RUN_ONLY_SERVER == runMode)
    {
        /* TODO: send to network */
        //SendServerResult(buf);
        return 0;
    }

    printf("%s", buf);
    return 0;
}

/*
 * flush the portal
*/
int FlushPortal(PPortal portal)
{
    int ret = 0;
    if(TOADSERV_CS_MODE_SERVER == runMode)
    {
        if(portal->msgBody.size <= 0)
            portal->msgBody.size = hat_strlen(portal->msgBody.body) + 1;
        ret = WritePortalMessage(portal);
    }
    else 
    {
        PortalPrint(portal->buffer);
    }
    portal->msgBody.size = -1;
    memset(portal->msgBody.body, 0x00, PORT_BUFFER_SIZE);

    return ret;
}

int SendToPortStr(PPortal portal, char *str)
{
    int ret = 0;

    if((NULL == portal) || (NULL == str) || '\0' == str[0])
        return -1;
    
    snprintf(portal->buffer, PORT_BUFFER_SIZE, "%s", str);

    portal->flag = PORT_ROW_STRING;
    ret = SendToNetPort(portal);
    return ret;
}

/*
 * After the excutor finds a line of data, it sends it to the client immediately.  
*/
int SendToPort(PPlanStateNode rowDataInfo, PTableRowData rowData)
{
    PPortal portal = rowDataInfo->portal;
    PDList rows = NULL;
    PTableRowData localRowData = NULL;
    int dataSize = 0;
    int ret = 0;

    if((NULL == portal) || (NULL == rowData))
        return -1;
    
    if(TOADSERV_CS_MODE_SERVER == runMode)
    {
        portal->rowData = rowData;
        ret = SendToNetPort(portal);
        return ret;
    }

    /* copy rowData locally, rowColumnData not. */
    dataSize = sizeof(TableRowData) + sizeof(PRowColumnData) * rowData->num;
    localRowData = (PTableRowData)AllocMem(dataSize);

    memcpy(localRowData, rowData, dataSize);

    rows = portal->rows;
    if(AddCellToListTail(&rows, localRowData) < 0)
    {
        return -1;
    }

    /* reassign rows pointer, maybe rows address is change. */
    portal->rows = rows;    
    portal->num += 1;

    return ret;
}

static int SendToNetPort(PPortal portal)
{
    int ret = 0;
    
    /* state machine */
    switch(portal->msgBody.type)
    {
        case PORT_MSG_START:
            portal->msgBody.type = PORT_MSG_BEGIN;
            break;
        case PORT_MSG_BEGIN:
            portal->msgBody.type = PORT_MSG_CONTINUE;
            break;
        default:
            ;
    }

    switch(portal->flag)
    {
    case PORT_ROW_HEADER:
        ret = GeneralVirtualTableHeadRaw(portal);
        portal->flag = PORT_NOTHING;
        if(ret <= 0)
        {
            goto RET;
        }

        portal->msgBody.size = ret;
        FlushPortal(portal);

        ret = PortalSendRows(portal);
        if(ret <= 0)
        {
            goto RET;
        }
    break;

    case PORT_ROW_STRING:
        portal->flag = PORT_NOTHING;
        ret = hat_strlen(portal->buffer);
    break;

    default:
        ret = PortalSendRows(portal);
        if(ret <= 0)
        {
            goto RET;
        }
    break;
    }

    portal->msgBody.size = ret;
    FlushPortal(portal);
RET:
    return ret;
}

int EndPort(PPortal portal)
{
    if(NULL == portal)
        return -1;

    FlushPortal(portal);
    portal->msgBody.type = PORT_MSG_FINISH;
    FlushPortal(portal);

    if(NULL != portal->targetValTypeArr)
    {
        FreeMem(portal->targetValTypeArr);
        portal->targetValTypeArr = NULL;
    }

    if(NULL != portal->attrWidth)
    {
        FreeMem(portal->attrWidth);
        portal->attrWidth = NULL;
    }

    portal->targetList = NULL;
    portal->rowData = NULL;
    return 0;
}


/*
 * udpate col type, and max size of col. 
 */
PScanHeaderRowInfo GetRowInfoNode(PPortal portal, char *colName)
{
    PScanHeaderRowInfo rowInfo = NULL;
    PDList node = portal->list->prev;

    /* portal colInfo initialize */
    while(node != NULL && node != portal->list)
    {
        rowInfo = (PScanHeaderRowInfo)(((PDLCell)node)->value);
        if(strcmp(rowInfo->colName, colName) == 0)
            break;

        rowInfo = NULL;
        node = node->next;
    }

    return rowInfo;
}

static int fillBack(char *buf, char op, int size)
{
    if(size <= 0)
        return size;
    memset(buf, op, size);
    return size;
}

static int InitTargetAttrType(PPortal portal)
{
    valueType *vtArr = NULL;
    PListCell tmpCell = NULL;

    PTargetEntry targetEntry = NULL;
    PResTarget resTarget = NULL;
    PColumnRef columnRefNode = NULL;

    int index = 0;

    if((NULL == portal) || (NULL == portal->targetList) || (portal->targetList->length <= 0))
        return -1;

     /* collect column data type */
    vtArr = (valueType *)AllocMem(portal->targetList->length * sizeof(valueType));
    for(tmpCell = portal->targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        targetEntry = (PTargetEntry)GetCellNodeValue(tmpCell);
        resTarget = (PResTarget)targetEntry->colRef;
        columnRefNode = (PColumnRef)resTarget->val;

        vtArr[index] = columnRefNode->vt;
        index++;
    }

    if(NULL != portal->targetValTypeArr)
        FreeMem(portal->targetValTypeArr);

    portal->targetValTypeArr = vtArr;
    portal->attrWidth = (int *)AllocMem(sizeof(int) * portal->targetList->length);

    return 0;
}

/*
 * collect Max attr values width, traval all rows attr one by one.
 */
int CalculatorColumnWidth(PPortal portal, int *rowMaxSize)
{
    PDList rownode = NULL;
    valueType *vtArr = NULL;

    PTableRowData rawRow = NULL;
    int index = 0;
    int rowWidth = 0;
    
    if((NULL == rowMaxSize) || (NULL == portal) || (NULL == portal->targetList))
        return -1;

    vtArr = portal->targetValTypeArr;

    /* collect max size per column data */
    rownode = portal->rows;
    do {
        if(NULL == rownode)
            break;
        
        rawRow = (PTableRowData)(((PDLCell)rownode)->value);

        for(index = 0; index < rawRow->num; index ++)
        {
            rowWidth = GetValueInfo(vtArr[index], rawRow->columnData[index], NULL, 0);
            if(rowMaxSize[index] < rowWidth)
                rowMaxSize[index] = rowWidth;
        }

        rownode = rownode->next;
    }while(rownode != portal->rows);

    return 0;
}

int FinishSend(PPortal portal)
{
    int ret = 0;

    if((NULL == portal) ||(NULL == portal->targetList))
        return -1;

    if(NULL != portal->attrWidth)
        FreeMem(portal->attrWidth);

    if(TOADSERV_CS_MODE_SERVER == runMode)
    {
        return 0;
    }

    /* first calculator column size */
    ret = CalculatorColumnWidth(portal, portal->attrWidth);
    if(ret < 0)
    {
        hat_log("[FinishSend] calculator column width falure.ret[%d]", ret);
        return ret;
    }

    /* format head line */
    ret = GeneralVirtualTableHead(portal);
    if(ret < 0)
    {
        hat_log("[FinishSend] generate virtual table headline falure.ret[%d]", ret);
        return ret;
    }

    /* format rows data */
    ClientRowDataShow(portal);

    hat_log("excutor rows %d sucess.", portal->num);
    return 0;
}

/* format head line */
int GeneralVirtualTableHead(PPortal portal)
{
    PListCell tmpCell = NULL;
    char *pbuf = portal->buffer;

    PTargetEntry targetEntry = NULL;
    PResTarget resTarget = NULL;
    PColumnRef columnRefNode = NULL;

    char *colName = NULL;
    int index = 0;
    int size = 0;
    int bufOffset = 0;

    if((NULL == portal) || (NULL == portal->targetList))
        return -1;
        
    for(tmpCell = portal->targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        targetEntry = (PTargetEntry)GetCellNodeValue(tmpCell);
        resTarget = (PResTarget)targetEntry->colRef;
        columnRefNode = (PColumnRef)resTarget->val;

        if(resTarget->name)
            colName = resTarget->name;
        else 
            colName = columnRefNode->field;

        snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%s", colName);
        size = strlen(colName);
        bufOffset += size + 1;

        /* value len is smaller than title name len. */
        if(size > portal->attrWidth[index])
        {
            portal->attrWidth[index] = size;
        }

        fillBack(pbuf+bufOffset, '-', portal->attrWidth[index] - size);
        bufOffset += portal->attrWidth[index] - size;

        index++;
    }

    snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|");
    FlushPortal(portal); 

    return 0;
}

/* format rows data */
static int ClientRowDataShow(PPortal portal)
{
    PDList rownode = NULL;
    PTableRowData rawRow = NULL;

    char *pbuf = portal->buffer;
    int index = 0;
    int bufOffset = 0;
    
    /* collect max size per column data */
    rownode = portal->rows;
    do 
    {
        if(rownode == NULL)
            break;
        
        rawRow = (PTableRowData)(((PDLCell)rownode)->value);
        bufOffset = 0;

        for(index = 0; index < rawRow->num; index ++)
        {
            GetValueInfo(portal->targetValTypeArr[index], rawRow->columnData[index], pbuf+bufOffset, portal->attrWidth[index]);
            bufOffset += portal->attrWidth[index] + 1;
        }

        snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|");
        FlushPortal(portal);

        rownode = rownode->next;
    }while(rownode != portal->rows);
   
    return 0;
}

/* 
 * value data info : return values width of string format.
 * if buffer is not NULL, values print to buffer as string format as maxwidth.
 */
int GetValueInfo(valueType vt, PRowColumnData colmnData, char *buffer, int Maxwidth)
{
    int size = 0;

    if(NULL == colmnData)
    {
        return -1;
    }

    if(colmnData->size == MIN_DATA_SIZE)
    {
        if(buffer)
        {
            snprintf(buffer, PORT_BUFFER_SIZE, "|%*s", Maxwidth, NULL_VALUE);  
        }

        return strlen(NULL_VALUE)+1;
    }

    switch(vt)
    {
        case VT_INT:
        case VT_INTEGER:
        {
            int *tmp = NULL;
            char digit[128];

            tmp = (int *)(colmnData->data);
            snprintf(digit, 128, "%d", *tmp);
            size = strlen(digit);

            if(size > Maxwidth)
                Maxwidth = size;

            if(buffer)
            {
                snprintf(buffer, PORT_BUFFER_SIZE, "|%*d", Maxwidth, *tmp);                     
            }
        }
        break;
        case VT_VARCHAR:
        case VT_STRING:
        {
            char *data = colmnData->data;            
            size = colmnData->size;
            
            if(size > Maxwidth)
                Maxwidth = size;

            if(buffer)
            {
                snprintf(buffer, PORT_BUFFER_SIZE, "|%*s", Maxwidth, data);
            }
        }
        break;
        case VT_CHAR:
        {
            char data = colmnData->data[0];
            size = sizeof(char);

            if(size > Maxwidth)
                Maxwidth = size;            

            if(buffer)
            {
                snprintf(buffer, PORT_BUFFER_SIZE, "|%*c", Maxwidth, data);
            }
        }
        break;
        case VT_DOUBLE:
        case VT_FLOAT:
        {
            float *tmp = NULL;
            char digit[128];

            tmp = (float *)(colmnData->data);
            snprintf(digit, 128, "%f", *tmp);
            size = strlen(digit);

            if(size > Maxwidth)
                Maxwidth = size;

            if(buffer)
            {
                snprintf(buffer, PORT_BUFFER_SIZE, "|%*f", Maxwidth, *tmp);                        
            }
        }
        break;
        case VT_BOOL:
        {
            char data = colmnData->data[0];
            size = sizeof(char);

            if(size > Maxwidth)
                Maxwidth = size; 

            if(buffer)
            {
                snprintf(buffer, PORT_BUFFER_SIZE, "|%*c", Maxwidth, data);
            }
        }
        break;
        default:
        break;
    }                    

    return Maxwidth;
}
