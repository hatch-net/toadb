/*
 *	toadb tsqlmain  
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


#ifndef HAT_TSQLMAIN_H_H
#define HAT_TSQLMAIN_H_H



typedef enum enRunMode 
{
    TSQL_RUN_COMMAND,
    TSQL_RUN_SINGLE_INPUT,
    TSQL_RUN_ONLY_CLIENT,
    TSQL_RUN_ONLY_SERVER,
    TSQL_RUN_INVALID
}enRunMode;



int OnlyClientRun(char *command);

#endif