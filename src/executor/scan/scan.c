/*
 *	toadb scan
 * Copyright (C) 2023-2023, senllang
*/

#include "scan.h"
#include <stddef.h>
#include <string.h>

/* 
 * Initialize scanState ,which is alive untile scan end. 
 * add this table to the scan table list. 
 */
PScanState InitScanState(PTableList tblInfo, PScan scan)
{
    PScanState scanState = NULL;

    scanState = (PScanState)AllocMem(sizeof(ScanState));
    memset(scanState, 0x00, sizeof(ScanState));
    
    if(scan != NULL)
    {
        AddCellToListTail(((PDList*)&(scan->list)), (void *)scanState);
    }

    if(tblInfo != NULL)
    {
        scanState->columnNum = tblInfo->tableDef->colNum;
    }

    scanState->tblInfo = tblInfo;
    scanState->scanInfo = scan;

    return scanState;
}

PScanState GetScanState(PTableList tblInfo, PScanState scanStateHead)
{
    PScanState scanState = NULL;

    return scanState;
}

PScanTableRowData TransFormScanRowData(PTableRowData rowData, PScanState scanState)
{
    
    return NULL;
}

PTableRowDataPosition GetTblRowDataPosition(PScanTableRowData scanTblRow, PTableList tblInfo)
{
    PTableRowDataPosition tblRowPosition = NULL;
    int tblIndex = 0;

    if((NULL == scanTblRow) || (NULL == tblInfo))
    {
        return NULL;
    }

    tblRowPosition = scanTblRow->tableRowData;
    for(tblIndex = 0; tblIndex < scanTblRow->tableNum; tblIndex++)
    {        
        if(tblInfo == tblRowPosition->tblInfo)
        {
            break;
        }

        tblRowPosition++;
    }

    if(tblIndex >= scanTblRow->tableNum)
        tblRowPosition = NULL;
    return tblRowPosition;
}
