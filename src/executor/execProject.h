/*
 *	toadb project node proc
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

#ifndef HAT_EXEC_PROJECT_H_H
#define HAT_EXEC_PROJECT_H_H

#include "executor.h"
#include "tables.h"


typedef struct ProjectStateData
{
    PRowData       rowData;         /* finally, the result is wrapped all the columns. */
    
    PRowColumnData *rawcolrow;       /* 
                                     * project operator is used, it is a array, 
                                     * size the same as top targetlist lenght. 
                                     */
}ProjectStateData, *PProjectStateData;

PProjectStateData InitProjectData(int columnNum);

PTableRowData ExecTableProject(PExecState eState);

// PTableRowData ExecTableUpdateProject(PExecState eState);

PTableRowData ExecTableQuery(PExecState eState);


#endif
