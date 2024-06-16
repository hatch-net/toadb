/*
 *	toadb seqscan
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_SEQSCAN_H_H
#define HAT_SEQSCAN_H_H


#include "scan.h"
#include "executor.h"

/* 
 * table scan 
 */
//int ScanTable(PTableList tbl, PScanState tblScan);

/* 
 *  record one row , which is query .
 */
// PTableRowData SeqScanRawRow(PTableList tbl, PScanState tblScan);
// int ScanOneTblRows(char *tblName, PScan scanState);

/* PAX mode, search group member pages */
//PTableRowData SeqScanRawRowForPages(PTableList tbl, PScanState tblScan);


PTableRowData ExecSeqscanNode(PExecState eState);
int ExecReSeqScan(PExecState eState);
PTableRowData ExecSeqscanNodeEnd(PExecState eState);

PTableRowData SeqscanNext(PExecState eState);
PTableRowData SeqscanEnd(PExecState eState);

PScanPageInfo InitScanPositionInfo(PExecState eState);

PTableRowData SeqScanNextColumnOpt(PTableList tbl, PScanState tblScan);

#endif