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
#include "query.h"
#include "plan.h"
#include "memStack.h"
#include "config_pub.h"

#define hat_log printf

// #define PARSER_TREE_PRINT 1

char *DataDir = "./toadbtest";

PMemContextNode dictionaryContext = NULL;

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
        hat_log("Data directory is invalid.\n");

        return -1;
    }

    InitToad();

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

        ToadMainEntry(command);
    }

    ExitToad();
    return 0;
}

int ToadMainEntry(char *query)
{
    List *parserTree = NULL;
    List *queryTree = NULL;
    List *planTree = NULL;
    PMemContextNode preContext = NULL, currContext = NULL;

    preContext = MemMangerNewContext("queryExcutor");

    /* 对输入的SQL进行词法和语法解析，生成解析树 */
    parserTree = raw_parser(query);
    if(NULL == parserTree)
    {
        return -1;
    }
#ifdef PARSER_TREE_PRINT
    /* 打印解析树 */    
    travelParserTree(parserTree, "parser Tree:");
#endif

    /* 对解析树进行分析和重写 */
    queryTree = QueryAnalyzeAndRewrite(parserTree);
    if(NULL == queryTree)
    {
        return -1;
    }

#ifdef PARSER_TREE_PRINT
    /* 打印查询树 */ 
    travelParserTree(queryTree, "Query Tree: ");
#endif

    /* 由查询树生成计划树 */
    planTree = QueryPlan(queryTree);
    if(NULL == planTree)
    {
        return -1;
    }
#ifdef PARSER_TREE_PRINT
    /* 打印计划树 */ 
    travelParserTree(planTree, "Plan Tree: ");
#endif

    /* 执行器调用入口，根据执行计划进行执行 */
    ExecutorMain(planTree);

    /* 释放解析树占用的内存资源 */
    ReleaseParserTreeResource(parserTree);
    
    /* relese memory context */
    currContext = MemMangerSwitchContext(preContext);
    MemMangerDeleteContext(preContext, currContext);

    return 0;
}

int InitToad()
{
    PMemContextNode oldContext = NULL;
    
    /* Memory Context Manager Initialize at the head. */
    MemMangerInit(); 

    /* initialize dictionary memory context. */
    oldContext = MemMangerNewContext("dictionaryTop");
    dictionaryContext = MemMangerSwitchContext(oldContext);

    return 0;
}

int ExitToad()
{
    /* 释放系统字典占用的内存资源 */
    ReleaseAllTblInfoResource();

    MemMangerDestroy();
    return 0;
}

int checkDataDir()
{
    // 检查文件是否存在
    if (access(DataDir, F_OK) != 0) 
    {
        // hat_log("table file %s is not exist. \n", filepath);
        return -1;
    }

    return 0;
}

/*
 * command line parser. 
 */
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
    // printf("cwd :%s\n", cwd);
    free(cwd);

    /* Datadir */
    printf("datadir: [%s]\n",DataDir);
}