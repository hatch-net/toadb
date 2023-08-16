/*
 *	toadb toadmain 
 * Copyright (C) 2023-2023, senllang
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <getopt.h>      

#include <sys/types.h>
#include <fcntl.h> 

#include "toadmain.h"
#include "parser.h"
#include "node.h"
#include "executor.h"
#include "buffer.h"

#define log printf

char *DataDir = "./toadbtest";

/* 
 * toadb数据库服务入口，进行SQL解析，执行 
 */
int toadbMain(int argc, char *argv[]) 
{
    char command[MAX_COMMAND_LENGTH];
    //struct passwd *pw = getpwuid(getuid());
    //char *username = pw->pw_name;
    char *username = "toadb";
    char *prompt = ">";
    int commandlen = 0;

    if (getuid() == 0) {
        prompt = "#";
    }

    /* argments process */
    args_opt(argc, argv);

    ShowToadbInfo();
    if(checkDataDir() < 0)
    {
        log("Data directory is invalid.\n");

        return -1;
    }

    while (1) 
    {
        printf("%s%s ", username, prompt);
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) 
        {
            continue;    
        }

        commandlen = strlen(command);
        if(commandlen <= 1)
            continue;
        
        command[commandlen-1] = '\0'; /* skip \n */
        if(strcmp(command, "quit") == 0)
        {
            break;
        }

        ToadMainEtry(command);
    }

    ExitToad();
    return 0;
}

int ToadMainEtry(char *query)
{
    List *parserTree = NULL;

    /* 对输入的SQL进行词法和语法解析，生成解析树 */
    parserTree = raw_parser(query);
    // travelParserTree(parserTree);

    /* 执行器调用入口，根据解析树进行执行 */
    ExecutorMain(parserTree);

    /* 释放解析树占用的内存资源 */
    ReleaseParserTreeResource(parserTree);
    return 0;
}

int ExitToad()
{
    /* 释放系统字典占用的内存资源 */
    ReleaseAllTblInfoResource();
    return 0;
}

int checkDataDir()
{
    // 检查文件是否存在
    if (access(DataDir, F_OK) != 0) 
    {
        // log("table file %s is not exist. \n", filepath);
        return -1;
    }

    return 0;
}

int args_opt(int argc, char *argv[])
{
    int	optindex;
	int	c;
    int digit_optind = 0;

    static struct option long_options[] =
    {
        {"reqarg", required_argument, NULL, 'r'},
        {"optarg", optional_argument, NULL, 'o'},
        {"noarg", no_argument, NULL, 'n'},
        {NULL, 0, NULL, 0},
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
                
            break;
            default:

            break;
        }
    }

    return 0;
}

void ShowToadbInfo()
{
    char *cwd = NULL;
    
    /* current director path */
    cwd = getcwd(NULL, 0);
    printf("cwd :%s\n", cwd);
    free(cwd);

    /* Datadir */
    printf("datadir: [%s]\n",DataDir);
}