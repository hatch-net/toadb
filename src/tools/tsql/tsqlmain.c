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
#include <unistd.h>
#include <getopt.h>          
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tsqlmain.h"
#include "toadmain.h"
#include "public.h"

extern char *DataDir; 

int main(int argc, char *argv[])
{
    int	optindex;
	int	c;
    int digit_optind = 0;
    char *command = NULL;
    enRunMode runMode = TSQL_RUN_COMMAND;

    static struct option long_options[] =
    {
        {"help",    no_argument,    NULL, 'h'},
        {"version", no_argument,    NULL, 'v'},
        {NULL,      0,              NULL, 0  },
    };

    /* getopt error */
    opterr = 1;

    while((c =getopt_long(argc, argv, "D:C:-", 
                            long_options, &optindex))!= -1)
    {  
        switch(c)
        {
            case 'D':
                DataDir = strdup(optarg);
            break;
            case 'C':
                command = strdup(optarg);
                runMode = TSQL_RUN_SINGLE_INPUT;
            break;
            case 'v':
                printf("toadb version:%s\n", TOADB_VERSION);
                exit(0);
            break;
            default:
                printf("unknow argument %c\n", c);
            return -1;
        }
    }

    switch(runMode)
    {
    case TSQL_RUN_COMMAND:
        toadbMain(argc, argv);
    break;
    case TSQL_RUN_SINGLE_INPUT:
        InitToad();
        ToadMainEntry(command);
        ExitToad();
    break;
    default:
    break;
    }

    return 0;
}