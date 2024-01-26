/*
 *	toadb main 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toadmain.h"


/* 
 * toadb programmer main entrypoint. 
 * Do nothing here. 
 */
int main(int argc, char *argv[])
{
	printf("Welcome to Toad Database Manage System.\n")	;
	toadbMain(argc, argv);
	return 0;
}
