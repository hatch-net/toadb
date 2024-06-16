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
#include "tsqlclient.h"

#include "ipc.h"

#define TOADB_VERSION "V100C001B001SPC001"

extern char *DataDir; 
char *serverAddr = "127.0.0.1";
int serverPort = 6389;

static PClientSharedInfo clientSharedInfo = NULL;
static enRunMode runMode = TSQL_RUN_CS_MODE;

static void showHelp();

void exitClientProc();

static void RunClient();
static void RunClientOnce(char *command);

static void ShowResult();
static int SendCommand(char *command);


int main(int argc, char *argv[])
{
    int	optindex;
	int	c;
    int digit_optind = 0;
    char *command = NULL;

    static struct option long_options[] =
    {
        {"help",    no_argument,    NULL, 'h'},
        {"version", no_argument,    NULL, 'v'},
        {"single", no_argument,    NULL, 'i'},
        {"client", no_argument,    NULL, 'd'},
        {"server-deamon", no_argument,    NULL, 's'},
        {NULL,      0,              NULL, 0  },
    };

    /* getopt error */
    opterr = 1;

    atexit(exitClientProc);

    while((c =getopt_long(argc, argv, "D:C:H:P:-", 
                            long_options, &optindex))!= -1)
    {  
        switch(c)
        {
            case 'D':
                DataDir = strdup(optarg);
            break;
            case 'C':
                command = strdup(optarg);
                if(runMode == TSQL_RUN_COMMAND)
                    runMode = TSQL_RUN_ONLY_CLIENT;
            break;
            case 'H':
                serverAddr = strdup(optarg);
            break;
            case 'P':
                serverPort = atoi(optarg);
            break;
            case 'v':
                printf("toadb version:%s", TOADB_VERSION);
                exit(0);
            break;
            case 'h':
                showHelp();
                exit(0);
            break;
            case 'i':
                runMode = TSQL_RUN_COMMAND;
            break;
            case 'd':
                runMode = TSQL_RUN_ONLY_CLIENT;
            break;
            case 's':
                runMode = TSQL_RUN_ONLY_SERVER;
            break;
            default:
                printf("unknow argument %c", c);
            return -1;
        }
    }

    switch(runMode)
    {
    case TSQL_RUN_COMMAND:
        RunClient();
    break;
    case TSQL_RUN_SINGLE_INPUT:
        InitToad();
        ToadMainEntry(command);
        ExitToad();
    break;
    case TSQL_RUN_ONLY_CLIENT:
        RunClientOnce(command);
    break;
    case TSQL_RUN_ONLY_SERVER:
        RunToadbServerDemon();
    break;
    case TSQL_RUN_CS_MODE:
        CSClient_main(argc, argv);
    break;
    default:
    break;
    }

    return 0;
}

static void showHelp()
{
    printf("Welcome to toadb client - toadsql. \n");
    printf("toadsql argments list: ");
    printf("-D datapath , enter toadb command client. \n");
    printf("-C \"sqlstring\" , execute sql once only. \n");
    printf("\nRunning Mode Select: \n");
    printf("--i , Single client Mode, Server/Client is the same process. \n");
    printf("--d , Single sql Mode, Server/Client is the same process. \n");
    printf("--s , Server running with deamon mode, only server start. \n");
    printf("default, Server running deamon mode, client/server communicate with network. \n");
    printf("--v , show version. \n");
    printf("--h , show help. \n");
}

static void RunClient()
{
    char command[MAX_COMMAND_LENGTH] = {0};

    /* init shared evironment */
    if(InitClientSharedEnv((char **)&clientSharedInfo) < 0)
        return ;

    do 
    {
        if(ReadClientCommand(command) < 0)
            break;
        
        OnlyClientRun(command);
    }while(1);

    DestoryClientSharedEnv((char **)&clientSharedInfo);
}

static void RunClientOnce(char *command)
{
    /* init shared evironment */
    if(InitClientSharedEnv((char **)&clientSharedInfo) < 0)
        return ;

    OnlyClientRun(command);

    DestoryClientSharedEnv((char **)&clientSharedInfo);
}

static void ShowResult()
{
    if(NULL == clientSharedInfo)
        return ;
    
    do 
    {
        WaitServerDataArrive();

        printf("%s",clientSharedInfo->data);

        if(SERVER_EXCUTOR_RESULT_FINISH != clientSharedInfo->command)
            NotifyServerReadData();
        else
            break;
    }while(1);
}

static int SendCommand(char *command)
{
    clientSharedInfo->command = CLIENT_EXCUTOR_COMMAND;
    clientSharedInfo->dataLen = strlen(command) + 1;
    memcpy(clientSharedInfo->data, command, clientSharedInfo->dataLen);

    return 0;
}

int ReadClientCommand(char *command)
{
    int readlen = 0;
    readlen = ReadCommandLine(command);
    if(readlen <= 0)
        return -1;

    if(strcmp(command, "quit") == 0)
    {
        return -1;
    }
    return readlen;
}

int OnlyClientRun(char *command)
{
    /* lock client */
    WaitClientControlLock();

    /* write data to share memory. */
    SendCommand(command);

    NotifyServerReadData();
    
    ShowResult();

    ReleaseClientControlLock();
    return 0;
}


/* 
 * callback function.
 * This function is called when exit process.
 */
void exitClientProc()
{
    switch(runMode)
    {
    case TSQL_RUN_COMMAND:
        DestoryClientSharedEnv((char**)&clientSharedInfo);
    break;
    case TSQL_RUN_SINGLE_INPUT:
    break;
    case TSQL_RUN_ONLY_CLIENT:
        DestoryClientSharedEnv((char**)&clientSharedInfo);
    break;
    case TSQL_RUN_ONLY_SERVER:
    break;
    case TSQL_RUN_CS_MODE:
    break;
    default:
    break;
    }
}