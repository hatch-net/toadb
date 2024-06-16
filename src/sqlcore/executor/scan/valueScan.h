/*
 *	toadb valueScan 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_VALUE_SCAN_H_H
#define HAT_VALUE_SCAN_H_H

#include "tables.h"
#include "executor.h"


PTableRowData ExecScanValuesNode(PExecState eState);

PTableRowData ExecScanValuesNodeEnd(PExecState eState);

int TransformValues(PExecState eState);

#endif 

