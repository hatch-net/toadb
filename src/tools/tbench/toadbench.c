/*
 *	toadb toadbench 
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

#include "toadInterface.h"

#include <stdio.h>  
#include <stdlib.h>  
#include <time.h>  
#include <string.h>

char sqlcommand[][128] = 
{
    "UPDATE toad_accounts SET abalance = abalance + %d WHERE aid = %d;",
    "SELECT abalance FROM toad_accounts WHERE aid = %d;",
    "UPDATE toad_tellers SET tbalance = tbalance + %d WHERE tid = %d;",
    "UPDATE toad_branches SET bbalance = bbalance + %d WHERE bid = %d;",
    "INSERT INTO toad_history (tid, bid, aid, delta, mtime) VALUES (%d, %d, %d, %d, '%s');"
};

int nbranches=1;
int ntellers=10;
int naccounts=100000;

#define UPDATE_ACC      0x01
#define SELECT_ACC      0x02
#define INSERT_HIS      0x04
#define UPDATE_TEL      0x10
#define UPDATE_BRA      0x20

int mode = 0xFF;

void BenchmarkMain(PConnectionContext conn, int scale);

int main(int argc, char *argv[])
{
    ConnectionContext conn = {0};
    char *serverAddr = "127.0.0.1";
    int serverPort = 6389;
    int scale = 1;

    if(argc > 1)
        scale = atoi(argv[1]);

    if(argc > 2)
        mode = atoi(argv[2]);

    if(argc > 3)
    {
        serverAddr = strdup(argv[3]);
        serverPort = atoi(argv[4]);
    }

    InitToadbInterface(&conn);

    ConnectToadbServer(&conn, serverAddr, serverPort);

    nbranches = nbranches * scale;
    ntellers = ntellers * scale;
    naccounts = naccounts * scale;

    BenchmarkMain(&conn, scale);

    CloseConnectionToadb(&conn);
    DestoryToadbInterface(&conn);

    return 0;
}

void BenchmarkMain(PConnectionContext conn, int scale)
{
    // 获取当前时间（秒）作为种子  
    time_t rawtime;  
    struct tm * timeinfo;  
    char tbuf[80], sqlbuf[128];  
    int aid, bid, tid, delta ;
    int count = 0;

    time(&rawtime);  
  
    // 使用当前时间作为随机数生成器的种子  
    srand((unsigned int)rawtime);  
  
    timeinfo = localtime(&rawtime);  
  
    // 将当前时间转换为字符串格式  
    strftime(tbuf, 80, "%Y-%m-%d %H:%M:%S", timeinfo);  
    printf("当前时间: %s\n", tbuf);  
  
    do
    {
        aid = rand() % naccounts + 1;      
        bid = rand() % nbranches + 1;
        tid = rand() % ntellers + 1;
        delta = rand() % 10000 - 5000;

        if(mode & UPDATE_ACC)
        {
            snprintf(sqlbuf, 128, sqlcommand[0], delta, aid);
            ExecQery(conn, sqlbuf);
        }

        if(mode & SELECT_ACC)
        {
            snprintf(sqlbuf, 128, sqlcommand[1], aid);
            ExecQery(conn, sqlbuf);
        }

        if(mode & UPDATE_TEL)
        {
            snprintf(sqlbuf, 128, sqlcommand[2], delta, tid);
            ExecQery(conn, sqlbuf);
        }

        if(mode & UPDATE_BRA)
        {
            snprintf(sqlbuf, 128, sqlcommand[3], delta, bid);
            ExecQery(conn, sqlbuf);
        }

        if(mode & INSERT_HIS)
        {
            time(&rawtime);
            timeinfo = localtime(&rawtime);  
            strftime(tbuf, 80, "%Y-%m-%d %H:%M:%S", timeinfo);  
            snprintf(sqlbuf, 128, sqlcommand[4], tid, bid, aid, delta, tbuf);
            ExecQery(conn, sqlbuf);
        }

        count += 5;

        if((count % 1000) == 0)
        {
            printf("excutor %d commands, time %s\n", count, tbuf);
        }
    } while (1);
     
    return;
}