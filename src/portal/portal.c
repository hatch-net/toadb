/*
 *	toadb portal
 * Copyright (C) 2023-2023, senllang
*/



#include "portal.h"
#include "buffer.h"
#include "seqscan.h"
#include "execNode.h"
#include "queryNode.h"

#include <stdio.h>
#include <string.h>

#define hat_log printf 

#define NULL_VALUE " "

static int fillBack(char *buf, char op, int size);

static int GetValueInfo(valueType vt, PRowColumnData data, char *buffer, int Maxwidth);

int GeneralVirtualTableHead(PPortal portal);

int ClientRowDataShow(PPortal portal);


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

    portal = (PPortal) AllocMem(sizeof(Portal));
    memset(portal, 0x00, sizeof(Portal));

    return portal;
}

/* 
 * Add column define to portal, rows match the targetList. 
 */
int InitSelectPortal(PList targetList, PPortal portal)
{
    PListCell tmpCell = NULL;
    PScanHeaderRowInfo rowInfo = NULL;
    PTableList tblInfo = NULL;
    int columnNum = 0;

    if(NULL == portal->targetList)
        portal->targetList = targetList;

#if 0
    /* Initialized already */
    if(!DList_IS_NULL(portal->list))
        return -1;

    if(NULL != targetList)
    {
        /* general column list which ordered by stmt. */
        for(tmpCell = targetList->head; tmpCell != NULL; tmpCell = tmpCell->next)
        {
            PAttrName node = (PAttrName)(tmpCell->value.pValue);

            rowInfo = (PScanHeaderRowInfo)AllocMem(sizeof(ScanHeaderRowInfo));
            rowInfo->colName = strdup(node->attrName);

            AddCellToListTail(&(portal->list), rowInfo);
        }
    }


    /* select all */
    for(tmpCell = stmt->relrange->head; tmpCell != NULL; tmpCell = tmpCell->next)
    {
        PTableRefName node = (PTableRefName)(tmpCell->value.pValue);
        
        /* get table information */
        tblInfo = GetTableInfo(node->tblRefName);
        if (NULL == tblInfo)
        {
            hat_log("select table failure.\n");
            return -1;
        }

        columnNum = tblInfo->tableDef->colNum;
        for(; columnNum > 0; columnNum--)
        {
            rowInfo = (PScanHeaderRowInfo)AllocMem(sizeof(ScanHeaderRowInfo));
            rowInfo->colName = strdup(tblInfo->tableDef->column[columnNum].colName);
            rowInfo->colIndex = columnNum;
            AddCellToListTail(&(portal->list), rowInfo);
        }
    }
#endif
    return 0;
}

/*
*/
int PortalPrint(char *buf)
{
    printf("%s\n", buf);
    return 0;
}

/*
 * flush the portal
*/
int FlushPortal(PPortal portal)
{
    PortalPrint(portal->buffer);
    memset(portal->buffer, 0x00, sizeof(portal->buffer));

    return 0;
}

/*
 * After the excutor finds a line of data, it sends it to the client immediately.  
*/
int SendToPort(PPlanStateNode rowDataInfo, PTableRowData rowData)
{
    PPortal portal = rowDataInfo->portal;
    PDList rows = NULL;

    if((NULL == portal) || (NULL == rowData))
        return -1;

    rows = portal->rows;

    if(AddCellToListTail(&rows, rowData) < 0)
    {
        return -1;
    }

    /* reassign rows pointer, maybe rows address is change. */
    portal->rows = rows;
    portal->num += 1;

    return 0;
}

int EndPort(PPortal portal)
{
    if(NULL == portal)
        return -1;

    FlushPortal(portal);

    if(NULL != portal->targetValTypeArr)
        FreeMem(portal->targetValTypeArr);

    if(NULL != portal->attrWidth)
        FreeMem(portal->attrWidth);

    FreeMem(portal);

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

/*
 * Client show result rows. 
 */
int ClientFormRow(PScan scanHead, PSelectStmt stmt)
{
    char *pbuf = scanHead->portal->buffer;
    int bufOffset = 0;
    int *rowMaxSize = NULL;
    PScanState scantbl = NULL;
    PDList node = NULL;
    PDList rownode = NULL;
    PTableRowData rawRow = NULL;
    PColumnDefInfo colDef = NULL;

    PValuesData attrData = NULL;
    int size = 0;
    int index = 0;
    int attrIndex = 0;
    int first = 1;                      /* Only ignore dlink first node , node == head->list , the end of list when just is true once . */
    int showPhare = SHOW_PHARE_ROW_SIZE;
    int showNum = 0;

    memset(pbuf, 0x00, PORT_BUFFER_SIZE);

    /* rows order by select */
    for(node = scanHead->list; (first || node != scanHead->list) && node != NULL; node = node->next)
    {
        scantbl = (PScanState)(((PDLCell)node)->value);
        colDef = scantbl->tblInfo->tableDef->column;
        bufOffset = 0;
        showPhare = SHOW_PHARE_ROW_SIZE;

        do
        {
            first = 1;
            showNum = 0;

            if(scantbl->rows == NULL )
            {
                first = 0;
                /* no rows to show. */
                break;
            }

           /* two show phare , first collect max column size, then show . */
            if(showPhare)
            {
                for(attrIndex = 0; attrIndex < scantbl->tblInfo->tableDef->colNum; attrIndex ++)
                {
                    snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%s", colDef[attrIndex].colName);
                    size = strlen(colDef[attrIndex].colName);
                    bufOffset += size + 1;

                    /* value len is smaller than title name len. */
                    rowMaxSize[attrIndex] = rowMaxSize[attrIndex] > size ? rowMaxSize[attrIndex] : size;

                    fillBack(pbuf+bufOffset, '-', rowMaxSize[attrIndex] - size);
                    bufOffset += rowMaxSize[attrIndex] - size;
                }
                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|");
                FlushPortal(scanHead->portal); 
                bufOffset = 0;
            }

            for(rownode = scantbl->rows; (first ||rownode != scantbl->rows) && rownode != NULL; rownode = rownode->next)
            {
                first = 0;
                rawRow = (PTableRowData)(((PDLCell)rownode)->value);

                if(rowMaxSize == NULL)
                {
                    rowMaxSize = (int*) AllocMem(rawRow->num * sizeof(int));
                    memset(rowMaxSize, 0x00, rawRow->num * sizeof(int));
                }                

                for(attrIndex = 0; attrIndex < rawRow->num; attrIndex ++)
                {
                    switch(colDef[attrIndex].type)
                    {
                        case VT_INT:
                        case VT_INTEGER:
                        {
                            int *tmp = NULL;
                            char digit[128];
                            int size = 0;
                            if(rawRow->columnData[attrIndex] != NULL)
                            {
                                tmp = (int *)(rawRow->columnData[attrIndex]->data);
                                snprintf(digit, 128, "%d", *tmp);
                                size = strlen(digit);
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*d", rowMaxSize[attrIndex], *tmp);  
                                bufOffset += rowMaxSize[attrIndex] + 1;                              
                            }
                            else
                            {
                                if(size > rowMaxSize[attrIndex])
                                    rowMaxSize[attrIndex] = size + 1;
                            }
                        }
                        break;
                        case VT_VARCHAR:
                        case VT_STRING:
                        {
                            int size = 0;
                            char *data = NULL;
                            if (rawRow->columnData[attrIndex] != NULL)
                            {
                                data = rawRow->columnData[attrIndex]->data;
                                size = rawRow->columnData[attrIndex]->size;
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*s", rowMaxSize[attrIndex], data);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                            {
                                if(size > rowMaxSize[attrIndex])
                                    rowMaxSize[attrIndex] = size;
                                
                            }
                        }
                        break;
                        case VT_CHAR:
                        {
                            char data = '\0';
                            if (rawRow->columnData[attrIndex] != NULL)
                            {
                                data = rawRow->columnData[attrIndex]->data[0];
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*c", rowMaxSize[attrIndex],data);
                                // fillBack(pbuf+bufOffset+2, ' ', rowMaxSize[attrIndex] - 1);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                                rowMaxSize[attrIndex] = sizeof(char);
                        }
                        break;
                        case VT_DOUBLE:
                        case VT_FLOAT:
                        {
                            float *tmp = NULL;
                            char digit[128];
                            int size = 0;
                            if(rawRow->columnData[attrIndex] != NULL)
                            {
                                tmp = (float *)(rawRow->columnData[attrIndex]->data);
                                snprintf(digit, 128, "%f", *tmp);
                                size = strlen(digit);
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*f", rowMaxSize[attrIndex], *tmp);  
                                bufOffset += rowMaxSize[attrIndex] + 1;                              
                            }
                            else
                            {
                                if(size > rowMaxSize[attrIndex])
                                    rowMaxSize[attrIndex] = size + 1;
                            }
                        }
                        break;
                        case VT_BOOL:
                        {
                            char data = '\0';
                            if (rawRow->columnData[attrIndex] != NULL)
                            {
                                data = rawRow->columnData[attrIndex]->data[0];
                            }

                            if(showPhare)
                            {
                                snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|%*c", rowMaxSize[attrIndex],data);
                                // fillBack(pbuf+bufOffset+2, ' ', rowMaxSize[attrIndex] - 1);
                                bufOffset += rowMaxSize[attrIndex] + 1; 
                            }
                            else
                                rowMaxSize[attrIndex] = sizeof(char);
                        }
                        break;
                        default:
                        break;
                    }                    
                }
                if(showPhare)
                {
                    snprintf(pbuf + bufOffset, PORT_BUFFER_SIZE - bufOffset, "|");
                    FlushPortal(scanHead->portal);
                    bufOffset = 0;
                    showNum ++;
                }
            }

            showPhare++;
        }while(showPhare < SHOW_PHARE_MAX);

        snprintf(pbuf, PORT_BUFFER_SIZE , "total %d rows", showNum);
        FlushPortal(scanHead->portal);

        if(NULL != rowMaxSize)
        {
            FreeMem(rowMaxSize);
            rowMaxSize = NULL;
        }
    }
 
    return 0;
}

/*
 * collect Max attr values width, traval all rows attr one by one.
 */
int CalculatorColumnWidth(PPortal portal, int *rowMaxSize)
{
    PDList rownode = NULL;
    valueType *vtArr = NULL;
    PListCell tmpCell = NULL;

    PTargetEntry targetEntry = NULL;
    PResTarget resTarget = NULL;
    PColumnRef columnRefNode = NULL;

    PTableRowData rawRow = NULL;
    int index = 0;
    int rowWidth = 0;
    
    if((NULL == rowMaxSize) || (NULL == portal) || (NULL == portal->targetList))
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

    if(NULL != portal->targetValTypeArr)
        FreeMem(portal->targetValTypeArr);

    portal->targetValTypeArr = vtArr;
    return 0;
}

int FinishSend(PPortal portal)
{
    int ret = 0;

    if((NULL == portal) ||(NULL == portal->targetList))
        return -1;

    if(NULL != portal->attrWidth)
        FreeMem(portal->attrWidth);

    portal->attrWidth = (int *)AllocMem(sizeof(int) * portal->targetList->length);

    /* first calculator column size */
    ret = CalculatorColumnWidth(portal, portal->attrWidth);
    if(ret < 0)
    {
        hat_log("[FinishSend] calculator column width falure.ret[%d]\n", ret);
        return ret;
    }

    /* format head line */
    ret = GeneralVirtualTableHead(portal);
    if(ret < 0)
    {
        hat_log("[FinishSend] generate virtual table headline falure.ret[%d]\n", ret);
        return ret;
    }

    /* format rows data */
    ClientRowDataShow(portal);
    
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
int ClientRowDataShow(PPortal portal)
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
static int GetValueInfo(valueType vt, PRowColumnData colmnData, char *buffer, int Maxwidth)
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
