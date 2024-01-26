/*
 *	toadb expreCompute
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


#include "expreCompute.h"
#include "dataTypeProc.h"
#include "public.h"
#include "parserNode.h"

/* 
 * 计算布尔运算
 */
int ComputeBoolExpr(PExprDataInfo left, PExprDataInfo right, int op)
{
    PDataTypeProcs typeprocs = NULL;

    if((NULL == left) || (NULL == right))    
        return 0;
    
    do 
    {
        typeprocs = getDataTypeProcs(left->type);
        if(NULL == typeprocs)
        {
            break;
        }

        if(typeprocs->version != DATATYPEPROC_VERSION)
        {
            hat_error("type (%d) procs version (%d) incompate with version(%d)\n", 
                        typeprocs->dataType, typeprocs->version, DATATYPEPROC_VERSION);
            break;
        }

        if((op >= LESS) && (op < Op_Type_MAX))
        {
            if(typeprocs->opf[op] != NULL)
                return typeprocs->opf[op](left, right);
        }
    }while(0);
    
    return 0;
}