/*
 *	toadb toadmain 
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

#ifndef HAT_TOADMAIN_H_H
#define HAT_TOADMAIN_H_H

#define MAX_COMMAND_LENGTH 1024

int toadbMain(int argc, char *argv[]) ;

int ToadMainEntry(char *query);

int InitToad();
int ExitToad();

int args_opt(int argc, char *argv[]);

int checkDataDir();
void ShowToadbInfo();
#endif