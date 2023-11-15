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

/*
 * transform common row to scanState row with position infomation.
 */
PScanTableRowData TransFormScanRowData(PTableRowData rowData, PScanState scanState)
{
    PScanTableRowData scanTblRowData = NULL;
    PTableRowDataPosition tblRowDataPos = NULL;
    PRowDataPosition rowDataPosition = NULL;

    if((NULL == rowData) || (NULL == scanState))
    {
        return NULL;
    }

    rowDataPosition = (PRowDataPosition)AllocMem(sizeof(RowDataPosition));
    rowDataPosition->rowData = rowData;
    rowDataPosition->scanPostionInfo = scanState->scanPostionInfo;

    tblRowDataPos = (PTableRowDataPosition)AllocMem(sizeof(TableRowDataPosition));
    tblRowDataPos->tblInfo = scanState->tblInfo;
    tblRowDataPos->rowDataPosition = rowDataPosition;
    tblRowDataPos->rowNum = 1;

    scanTblRowData = (PScanTableRowData)AllocMem(sizeof(ScanTableRowData));
    scanTblRowData->tableNum = 1;       /* only scan maybe on one table. */
    scanTblRowData->tableRowData = tblRowDataPos;
    

    return scanTblRowData;
}

PTableRowDataPosition GetTblRowDataPosition(PScanTableRowData scanTblRow, PTableList tblInfo)
{
    PTableRowDataPosition *ptblRowPosition = NULL;
    int tblIndex = 0;

    if((NULL == scanTblRow) || (NULL == tblInfo))
    {
        return NULL;
    }

    ptblRowPosition = &(scanTblRow->tableRowData);
    for(tblIndex = 0; tblIndex < scanTblRow->tableNum; tblIndex++)
    {   
        if(NULL == *ptblRowPosition)
            break;
                 
        if(tblInfo == (*ptblRowPosition)->tblInfo)
        {
            break;
        }

        ptblRowPosition++;
    }

    if(tblIndex >= scanTblRow->tableNum)
        *ptblRowPosition = NULL;
    return *ptblRowPosition;
}
