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
#include "ipc.h"
#include "resourceMgr.h"

#define hat_log printf

// #define PARSER_TREE_PRINT 1

char *DataDir = "./toadbtest";
int runMode = TOADSERV_RUN_CLIENT_SERVER;
int pageNum = 16;

PMemContextNode dictionaryContext = NULL;
PMemContextNode memPoolContext = NULL;

PClientSharedInfo clientSharedInfo = NULL;

static void showHelp();

static int WaitClientData();
static int NotifyClientResultData();

static int ToadSingClientMain();
static void exitServerProc();

static int WriteResultData(char *result);

/* 
 * toadb数据库服务入口，进行SQL解析，执行 
 */
int toadbMain(int argc, char *argv[]) 
{
    /* argments process */
    args_opt(argc, argv);

    ShowToadbInfo();
    if(checkDataDir() < 0)
    {
        hat_log("Data directory is invalid.\n");

        return -1;
    }

    printf("start toadb runMode %d ...\n", runMode);
    switch(runMode)
    {
        case TOADSERV_RUN_CLIENT_SERVER:
            ToadSingClientMain();
        break;
        case TOADSERV_RUN_ONLY_SERVER:
            ToadbServerMain(argc, argv);
        break;
        default:
        break;
    }
    return 0;
}

int ToadbServerMain(int argc, char *argv[]) 
{
    RunToadbServerDemon();
    return 0;
}

int RunToadbServerDemon()
{
    int pid = 0;

    /* check server demon is running. */
    if(ToadbServiceRunning())
    {
        return 0;
    }

    pid = fork();
    if(pid == 0)
    {
        /* child process */
        printf("start toadb service ...\n");
        StartToadbService();
    }
    else if(pid < 0)
    {
        /* error */
        printf("start toadb service error, when process fork.\n");
        return -1;
    }

    return 0;
}

int ToadbServiceRunning()
{

    return 0;
}

int SendServerResult(char *result)
{
    int writeLen = 0;

    if(clientSharedInfo->dataLen >= MAX_COMMAND_LENGTH)
    {
        NotifyClientReadData();
        WaitClientDataArrive();

        clientSharedInfo->dataLen = 0;
    }

    writeLen = WriteResultData(result);

    return writeLen;
}

static int WriteResultData(char *result)
{
    int dataLen = 0;
    char *pData = NULL;
    char endFlag = '\n';
    
    if(NULL == result)
        return 0;

    dataLen = strlen(result) + 1;
    
    /* multiple result */
    if(clientSharedInfo->dataLen > 0)
        clientSharedInfo->data[clientSharedInfo->dataLen-1] = endFlag;

    pData = clientSharedInfo->data + clientSharedInfo->dataLen;

    clientSharedInfo->command = SERVER_EXCUTOR_RESULT_COMMAND;    
    memcpy(pData, result, dataLen);

    clientSharedInfo->dataLen += dataLen ;

    return dataLen;
}

int SendFinishAndNotifyClient()
{
    clientSharedInfo->command = SERVER_EXCUTOR_RESULT_FINISH;

    NotifyClientResultData();
    return 0;
}

int StartToadbService()
{
    char command[MAX_COMMAND_LENGTH] = {0};

    InitServerSharedEnv((char **)&clientSharedInfo);
    clientSharedInfo->command = 0;
    clientSharedInfo->dataLen = 0;
    
    atexit(exitServerProc);

    InitToad();
    
    do
    {
        WaitClientData();

        if(clientSharedInfo->command == CLIENT_SERVICE_STOP_COMMAND)
        {
            hat_log("exit taodb service now...\n");
            break;
        }

        if(clientSharedInfo->dataLen > 0)
        {
            memcpy(command, clientSharedInfo->data, clientSharedInfo->dataLen);
            clientSharedInfo->dataLen = 0;

            ToadMainEntry(command);
        }

        SendFinishAndNotifyClient();
    } while (1);
    
    ExitToad();

    DestorySeverSharedEnv((char**)&clientSharedInfo);

    return 0;
}

static int WaitClientData()
{
    WaitClientDataArrive();
    return 0;
}

static int NotifyClientResultData()
{
    NotifyClientReadData();
    return 0;
}

/* 
 * callback function.
 * This function is called when exit process.
 */
void exitServerProc()
{
    DestorySeverSharedEnv((char**)&clientSharedInfo);
}

int ReadCommandLine(char *command)
{
    //struct passwd *pw = getpwuid(getuid());
    //char *username = pw->pw_name;
    char *username = "toadb";
    char *prompt = ">";
    int commandlen = 0;

    if (getuid() == 0) 
    {
        prompt = "#";
    }

    do
    {
        printf("%s%s ", username, prompt);
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) 
        {
            continue;    
        }

        commandlen = strlen(command);
    } while (commandlen <= 1);
    
    command[commandlen-1] = '\0'; /* skip \n */
    return commandlen;
}

static int ToadSingClientMain()
{
    char command[MAX_COMMAND_LENGTH];

    InitToad();

    while (1) 
    {
        ReadCommandLine(command);
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
    ReleaseAllResourceOwner();
    
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

    /* initialize memory pool context. */
    oldContext = MemMangerNewContext("memoryPool");
    memPoolContext = MemMangerSwitchContext(oldContext);
    
    /* initialize dictionary memory context. */
    oldContext = MemMangerNewContext("dictionaryTop");
    dictionaryContext = MemMangerSwitchContext(oldContext);

    oldContext = MemMangerNewContext("bufferPoolTop");
    CreateBufferPool(pageNum);
    dictionaryContext = MemMangerSwitchContext(oldContext);

    CreateResourceOwnerPool();

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

    while((c =getopt_long(argc, argv, "D:M:-", 
                            long_options, &optindex))!= -1)
    {  
        switch(c)
        {
            case 'D':
                DataDir = strdup(optarg);
            break;
            case 'M':
                runMode = atoi(optarg);
            break;
            case 'r':
            break;
            case 'o':
            break;
            case 'n':
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