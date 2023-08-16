/*
 *	toadb tsqlmain  
 * Copyright (C) 2023-2023, senllang
*/
#include <unistd.h>
#include <getopt.h>          /*所在头文件 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toadmain.h"

extern char *DataDir; 

int main(int argc, char *argv[])
{
    int	optindex;
	int	c;
    int digit_optind = 0;
    char *command = NULL;
    int runMode = 0;

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
                runMode = 1;
            break;
            default:
                printf("unknow argument %c\n", c);
            return -1;
        }
    }

    if(runMode)
    {
        ToadMainEtry(command);
        ExitToad();
    }
    else
    {
        toadbMain(argc, argv);
    }

    return 0;
}