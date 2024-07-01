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

/* 
 * this macro must be less portal buffer len. 
 * backend service mode, it represent the share memory size. 
 * single mode, it represent the sql buffer size.
*/


typedef enum enServRunMode 
{
    TOADSERV_RUN_CLIENT_SERVER,
    TOADSERV_RUN_ONLY_SERVER,
    TOADSERV_CS_MODE_SERVER,
    TOADSERV_RUN_INVALID
}enServRunMode;

typedef enum enClientCommand
{
    CLIENT_EXCUTOR_COMMAND,
    CLIENT_SERVICE_STOP_COMMAND,
    CLIENT_SERVICE_INFO_COMMAND,

    SERVER_EXCUTOR_RESULT_COMMAND,
    SERVER_EXCUTOR_RESULT_FINISH,
    INVALID_COMMAND
}enClientCommand;

typedef struct SysGlobalData 
{
    unsigned int objectId;
}SysGlobalData, *PSysGlobalData;
#define SYSGLOBALDATA_SIZE sizeof(SysGlobalData)

typedef struct SysGlobalContext
{
    int fd;
    SysGlobalData globalData;
}SysGlobalContext, *PSysGlobalContext;

int toadbMain(int argc, char *argv[]) ;

int ToadbServerMain(int argc, char *argv[]) ;
int ToadbCSModeServerMain(int argc, char *argv[]) ;

int RunToadbServerDemon();
int ToadbServiceRunning();
int StartToadbService();

int ToadMainEntry(char *query);
int ReadCommandLine(char *command);

int SendServerResult(char *result);
int SendFinishAndNotifyClient();

int InitToad();
int ExitToad();

int args_opt(int argc, char *argv[]);

int checkDataDir();
void ShowToadbInfo();

int InitSysGlobal();
int DestroySysGlobal();
int ReadSysGlobalData(PSysGlobalContext sysContext);
int WriteSysGlobalData(PSysGlobalContext sysContext);
int GetAndIncObjectId();

#endif