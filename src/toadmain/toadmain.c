/*
 *	toadb toadmain 
 * Copyright (C) 2023-2023, senllang
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>

#include "parser.h"
#include "node.h"
#include "executor.h"
#include "buffer.h"

#define MAX_COMMAND_LENGTH 1024

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
	List *parserTree = NULL;
    int commandlen = 0;

    if (getuid() == 0) {
        prompt = "#";
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

        /* 对输入的SQL进行词法和语法解析，生成解析树 */
        parserTree = raw_parser(command);
	    // travelParserTree(parserTree);

        /* 执行器调用入口，根据解析树进行执行 */
	    ExecutorMain(parserTree);

        /* 释放解析树占用的内存资源 */
        ReleaseParserTreeResource(parserTree);
    }

    /* 释放系统字典占用的内存资源 */
    ReleaseTblInfoResource();
    return 0;
}