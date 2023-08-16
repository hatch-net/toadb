/*
 *	toadb tables common defines  
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_TABLE_COM_H_H
#define HAT_TABLE_COM_H_H

#include "tfile.h"


#define NAME_MAX_LEN  64

#pragma push(pack(1))



typedef struct ColumnDefInfo
{
    char colName[NAME_MAX_LEN];
    int type;
    int options;
}ColumnDefInfo, *PColumnDefInfo;

typedef struct TableMetaInfo
{
    int tableId;
    int tableType;
    char tableName[NAME_MAX_LEN];
    int colNum;
    ColumnDefInfo column[FLEXIBLE_SIZE];
}TableMetaInfo, *PTableMetaInfo;


typedef struct RowColumnData
{
    int size;
    int attrindex;              /* start from 0 */
    char data[FLEXIBLE_SIZE];
}RowColumnData, *PRowColumnData;

/* exec form rowdata */
typedef struct TableRowData
{
    int size;           /* row data size */
    int num;            /* column num of row data */
    PRowColumnData columnData[FLEXIBLE_SIZE];
}TableRowData, *PTableRowData;

#define MIN_ROW_SIZE (sizeof(TableRowData)+sizeof(RowColumnData))


#pragma pop(pack(1))

#endif