/*
 *	toadb data types
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

#ifndef HAT_DATA_TYPES_H_H
#define HAT_DATA_TYPES_H_H

/* 当为4 Bytes时, 1bit 符号位， 23bit小数位，小数点后7位，这里为小数6位 */
#define FLOAT_EPSILON (1e-6)


typedef enum HAT_BOOL
{
    HAT_NO = 0,
    HAT_YES = 1,
    HAT_FALSE = 0,
    HAT_TRUE = 1,
    HAT_UNKNOWN
}HAT_BOOL;

typedef enum valueType
{
	VT_UNKNOWN,
	VT_POINTER,
	VT_INT,
    VT_INTEGER,
    VT_VARCHAR,
	VT_STRING,
	VT_CHAR,
    VT_DOUBLE,
	VT_FLOAT,	
	VT_BOOL,
	VT_MAX
}valueType;

/* 数据在逻辑处理中的存储形式 */
typedef union Data
{
	void *pData;
	int  iData;
	char cData;
	double dData;
	float fData;
}Data;

typedef struct ExprDataInfo 
{
    int type;
    int size;       /* 当size=-1时，根据类型得到实际size；如果为null时，size为0 */
    Data *data;
}ExprDataInfo, *PExprDataInfo;


#define GetIntValue(pExprData) ((NULL == (pExprData))? 0: ((pExprData)->data->iData))
#define GetDataPointer(pExprData) (Data*)(((char*)(pExprData)) + sizeof(ExprDataInfo))

int GetColumnType(char *typename);
int getDataTypeSize(void *pval, valueType type);
PExprDataInfo getDataInfo(void *pval, valueType type, PExprDataInfo resExprData);
HAT_BOOL getDataBool(PExprDataInfo dataInfo);

#endif