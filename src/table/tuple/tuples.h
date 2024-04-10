/*
 *	toadb tuples 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_TUPLES_H_H
#define HAT_TUPLES_H_H

#include "tables.h"
#include "buffer.h"
#include "node.h"

PTableRowData ValuesFormRowData(PTableMetaInfo tblMeta, PNode targetList, PNode valueList);

PRowColumnData TransformConstValueRowData(PConstValue constValue, PColumnDefInfo colDef, int attrIndex);

PExprDataInfo TransformConstExprValue(PConstValue constValue);

PRowColumnData transFormExpr2RowColumnData(PExprDataInfo exprData, int attrIndex);

#endif
