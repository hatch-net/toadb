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
#include <signal.h>
#include <errno.h>
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
#include "logger.h"
#include "servprocess.h"
#include "workermain.h"
#include "atom.h"
#include "config_pub.h"
#include "transactionControl.h"



char *globalData = "globalData";
char *DataDir = "./toadbtest";
int runMode = TOADSERV_RUN_CLIENT_SERVER;
int pageNum = 128;

static SysGlobalContext sysGlobalData;

PMemContextNode topMemContext = NULL;
PMemContextNode dictionaryContext = NULL;
PMemContextNode memPoolContext = NULL;

PClientSharedInfo clientSharedInfo = NULL;

static void showHelp();

static int WaitClientData();
static int NotifyClientResultData();

static int ToadSingClientMain();
static void exitServerProc();

static int WriteResultData(char *result);

static int StartToadbServiceWithThread();
static int RunToadbCSModeServerDemon();

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
        hat_log("Data directory is invalid.");

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
        case TOADSERV_CS_MODE_SERVER:
            ToadbCSModeServerMain(argc,argv);
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

int ToadbCSModeServerMain(int argc, char *argv[]) 
{
    RunToadbCSModeServerDemon();
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
        printf("start toadb service ...");
        StartToadbService();
    }
    else if(pid < 0)
    {
        /* error */
        printf("start toadb service error, when process fork.");
        return -1;
    }

    return 0;
}

static int RunToadbCSModeServerDemon()
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
        printf("start toadb service ...");
        StartToadbServiceWithThread();
    }
    else if(pid < 0)
    {
        /* error */
        printf("start toadb service error, when process fork.");
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
    InitWorker();
    do
    {
        WaitClientData();

        if(clientSharedInfo->command == CLIENT_SERVICE_STOP_COMMAND)
        {
            hat_log("exit taodb service now...");
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
    
    ExitWorker();
    ExitToad();

    DestorySeverSharedEnv((char**)&clientSharedInfo);

    return 0;
}

static int StartToadbServiceWithThread()
{
    atexit(ExitToad);

    InitToad();
    WorkerMain();
    //ExitToad();

    exit(0);
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
    InitWorker();
    while (1) 
    {
        ReadCommandLine(command);
        if(strcmp(command, "quit") == 0)
        {
            break;
        }

        ToadMainEntry(command);
    }
    ExitWorker();
    ExitToad();

    return 0;
}

int ToadMainEntry(char *query)
{
    List *parserTree = NULL;
    List *queryTree = NULL;
    List *planTree = NULL;
    PListCell tmpCell = NULL;
    PMemContextNode preContext = NULL, currContext = NULL;

    StartTransaction();

    preContext = MemMangerNewContext("queryExcutor");

    /* 对输入的SQL进行词法和语法解析，生成解析树 */
    parserTree = raw_parser(query);
    if(NULL == parserTree)
    {
        SendToPortStr(GetServPortal(), "Sytax is wrong, parser error.");
        goto ENDRET;
    }
#ifdef PARSER_TREE_PRINT
    /* 打印解析树 */    
    travelParserTree(parserTree, "parser Tree:");
#endif

    /* 对解析树进行分析和重写 */
    queryTree = QueryAnalyzeAndRewrite(parserTree);
    if(NULL == queryTree)
    {
        SendToPortStr(GetServPortal(), "object may be not found, query error.");
        goto ENDRET;
    }

#ifdef PARSER_TREE_PRINT
    /* 打印查询树 */ 
    travelParserTree(queryTree, "Query Tree: ");
#endif

    for(tmpCell = queryTree->head; tmpCell != NULL; tmpCell = tmpCell->next) 
    {
        PQuery node = (PQuery)(tmpCell->value.pValue);

        /* 由查询树生成计划树 */
        planTree = QueryPlan(node);
        if(NULL == planTree)
        {
            SendToPortStr(GetServPortal(), "planner error.");
            break;
        }

#ifdef PARSER_TREE_PRINT
        /* 打印计划树 */ 
        travelParserTree(planTree, "Plan Tree: ");
#endif

        StartTransaction();

        /* 执行器调用入口，根据执行计划进行执行 */
        ExecutorMain(planTree);

        IncCommandCount();
        FinishTransaction();
    }

ENDRET:
    /* 释放解析树占用的内存资源 */
    ReleaseParserTreeResource(parserTree);
    ReleaseAllResourceOwner();
    
    /* relese memory context */
    currContext = MemMangerSwitchContext(preContext);
    MemMangerDeleteContext(preContext, currContext);

    FinishTransaction();

    return 0;
}

int InitToad()
{
    PMemContextNode oldContext = NULL;
    int ret = 0;
    
    /* logger init */
    ret = SyslogInit();
    if(ret < 0)
    {
        hat_error("syslog init error!");
        exit(0);
    }

    /* Memory Context Manager Initialize at the head. */
    MemMangerInit(); 

    /* initialize memory pool context. */
    topMemContext = MemMangerNewContext("memoryPool");
    memPoolContext = MemMangerSwitchContext(topMemContext);
    
    /* initialize dictionary memory context. */
    oldContext = MemMangerNewContext("dictionaryTop");
    dictionaryContext = MemMangerSwitchContext(oldContext);

    oldContext = MemMangerNewContext("bufferPoolTop");
    CreateBufferPool(pageNum);
    dictionaryContext = MemMangerSwitchContext(oldContext);

    /* initialize dictionary info */
    InitTblInfo();

    ret = InitSysGlobal();
    return ret;
}

void ExitToad()
{
    hat_log("ExitToad begin!");

    /* record global infomation */
    RecordSysGlobal();

    /* 释放系统字典占用的内存资源 */
    ReleaseAllTblInfoResource();
    
    /* 释放bufferpool 锁资源 */
    DestroyBufferPool();

    MemMangerDestroy();

    SyslogDestroy();
    return ;
}

int checkDataDir()
{
    // 检查文件是否存在
    if (access(DataDir, F_OK) != 0) 
    {
        hat_log("table file %s is not exist. ", DataDir);
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
    // printf("cwd :%s", cwd);
    free(cwd);

    /* Datadir */
    printf("datadir: [%s]\n",DataDir);
}

int WriteSysGlobalData(PSysGlobalContext sysContext)
{
    int ret = 0;

    if((NULL == sysContext) || (sysContext->fd <= 0))
        return -1;
    
    lseek(sysContext->fd, 0, SEEK_SET);
    ret = write(sysContext->fd, &sysContext->globalData, SYSGLOBALDATA_SIZE);
    if(ret != SYSGLOBALDATA_SIZE)
    {
        hat_error("write sysdata failure[%d]",ret);
        return -1;
    }

    lseek(sysContext->fd, SYSGLOBALDATA_SIZE, SEEK_SET);
    ret = write(sysContext->fd, sysContext->transInfo, TRANSACTION_INFO_SIZE);
    if(ret != TRANSACTION_INFO_SIZE)
    {
        hat_error("write transinfo data failure[%d]",ret);
        return -1;
    }
    
    fsync(sysContext->fd);

    return 0;
}

int ReadSysGlobalData(PSysGlobalContext sysContext)
{
    int ret = 0;

    if((NULL == sysContext) || (sysContext->fd <= 0))
        return -1;
    
    lseek(sysContext->fd, 0, SEEK_SET);
    ret = read(sysContext->fd, &sysContext->globalData, SYSGLOBALDATA_SIZE);
    if(ret != SYSGLOBALDATA_SIZE)
    {
        hat_error("read sysdata failure[%d]",ret);
        return -1;
    }

    lseek(sysContext->fd, SYSGLOBALDATA_SIZE, SEEK_SET);
    ret = read(sysContext->fd, sysContext->transInfo, TRANSACTION_INFO_SIZE);
    if(ret != TRANSACTION_INFO_SIZE)
    {
        hat_error("read transinfo data failure[%d]",ret);
        return -1;
    }

    /* transaction initialize */
    sysContext->transInfo->highLevelXtid = sysContext->transInfo->nextTransactionId;
    sysContext->transInfo->lowLevelXtid = sysContext->transInfo->highLevelXtid;

    return 0;
}

int OpenGlobalFile()
{
    int fd = 0;
    char filepath[FILE_PATH_MAX_LEN] = {0};
    int flag = O_RDWR|O_EXCL;

    snprintf(filepath, FILE_PATH_MAX_LEN, "%s/%s", DataDir, globalData);

    if (access(filepath, F_OK) != 0)
    {
        flag |= O_CREAT;
    }

    fd = open(filepath, flag, 0600);
    if(fd < 0)
    {
        hat_error("open %s failure. [%d]",filepath, errno);
        return -1;
    }
    return fd;
}

int InitSysGlobal()
{
    int ret = 0;
    
    sysGlobalData.fd = OpenGlobalFile();
    sysGlobalData.transInfo = GetTransDataInfo();

    ret = ReadSysGlobalData(&sysGlobalData);

    return ret;
}

int RecordSysGlobal()
{
    if(sysGlobalData.fd <= 0)
    {
        sysGlobalData.fd = OpenGlobalFile();
    }
        
    WriteSysGlobalData(&sysGlobalData);

    close(sysGlobalData.fd);
    return 0;
}

int GetAndIncObjectId()
{
    long long obj = atomic_fetch_add(&sysGlobalData.globalData.objectId, 1);
    return (int)obj;
}


