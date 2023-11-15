/*
 *	toadb project node proc
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_EXEC_PROJECT_H_H
#define HAT_EXEC_PROJECT_H_H

#include "executor.h"
#include "tables.h"

PTableRowData ExecTableProject(PExecState eState);

PTableRowData ExecTableQuery(PExecState eState);



#endif
