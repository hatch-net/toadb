/*
 *	toadb execSelect 
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

#ifndef HAT_EXEC_SELECT_H_H
#define HAT_EXEC_SELECT_H_H

#include "executor.h"
#include "tables.h"

typedef struct SelectExpreData
{
    PExprDataInfo leftExpreData;
    PExprDataInfo righExpreData;
    PExprDataInfo resultExpreData;

}SelectExpreData, *PSelectExpreData;

PSelectExpreData InitSelectExpreData();
void SwitchToResultExpreData(PExprDataInfo *other, PExprDataInfo *result);

PTableRowData ExecSelect(PExecState eState);

/* select for update set list */
PTableRowData ExecUpdateSelect(PExecState eState);
#endif
