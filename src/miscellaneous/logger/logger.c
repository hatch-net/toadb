/*
 *	toadb logger
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
#include <pthread.h>
#include <unistd.h>

#include <stdarg.h>
#include <stdio.h>

#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "logger.h"
#include "public.h"


int logLevel = LOG_DEBUG;

SyslogInfo syslogInfo = {0};
FILE *sysLogFileDesc = NULL;

static int GetFileName(char *fileName, int len);
static FILE* OpenSyslogFile(char *fileName, char* mode);
static void FlushLogBuffer(char *buffer);

static void GetTimeString(char *buf, int len);

int SyslogInit()
{
    char logFileName[LOG_FILE_NAME_LEN];
    int fd = -1;
    int ret = 0;

    /* generator log file name */
    GetFileName(logFileName, LOG_FILE_NAME_LEN);
    sysLogFileDesc = OpenSyslogFile(logFileName, "a");
    if(NULL == sysLogFileDesc)
    {
        hat_error("open syslog file error!");
        return -1;
    }

    syslogInfo.syslogFd = fileno(sysLogFileDesc);
    /* redirect stdio/stderr to syslogfile. */
    if(((fd = dup2(syslogInfo.syslogFd, STDOUT_FILENO)) < 0) || (dup2(syslogInfo.syslogFd, STDERR_FILENO) < 0))
    {
        hat_error("redirect syslog file error!");
        ret = -1;
    }

    hat_log("logfile fd:%d redirect fd:%d", syslogInfo.syslogFd, fd);

    syslogInfo.syslogFd = fd;     
    fclose(sysLogFileDesc);
    sysLogFileDesc = NULL;

    return ret;
}

int SyslogDestroy()
{
    /* redirect, exit */
    if(syslogInfo.syslogFd > 0)
        close(syslogInfo.syslogFd);
    return 0;
}

/*
 * [threadid][year-month-day hour:minite:second.microsecond] message [function:line]
 */
void log_message(const char *filename, int lineno, const char *funcname, int level, const char *fmt, ...)
{
    char logBuffer[LOG_BUFFER_LEN] = {0};
    const char *pos = NULL;
    va_list vlist;
    int offset = 0;
    
    /* log level control */
    if(level < logLevel)
        return ;

    /* log header */
    snprintf(logBuffer, LOG_BUFFER_LEN, "[%d][%8u]", level, pthread_self());
    offset = strlen(logBuffer);
    
    GetTimeString(logBuffer + offset, LOG_BUFFER_LEN - offset);
    offset = strlen(logBuffer);

    /* log message */
    va_start(vlist, fmt);
    vsnprintf(logBuffer + offset, LOG_BUFFER_LEN - offset, fmt, vlist);
    va_end(vlist);
    offset = strlen(logBuffer);

    /* tail */
    pos = strrchr(filename, '/');
    if(NULL == pos)
    {
        pos = filename;
    }
    else 
        pos += 1;
    snprintf(logBuffer + offset, LOG_BUFFER_LEN - offset, " [%s][%s][%d]", pos, funcname, lineno);

    FlushLogBuffer(logBuffer);
    return ;
}

static void FlushLogBuffer(char *buffer)
{
    printf("%s\n", buffer);
}

#define MICOR_SCOND 1000
static void GetTimeString(char *buf, int len)
{
    struct tm* ptm;
    struct timeval tv;
    int offset = 0;

    gettimeofday(&tv, NULL);
    ptm = localtime (&(tv.tv_sec));
    strftime (buf, len, "[%Y-%m-%d %H:%M:%S", ptm);

    /* TODO: micro second. */
    offset = strlen(buf);
    snprintf(buf + offset, len - offset, ".%lu]", tv.tv_usec/MICOR_SCOND);
}

static int GetFileName(char *fileName, int len)
{
    time_t t;  
    struct tm *p;  
    
    time(&t);
    p=gmtime(&t);  

    strftime(fileName, len, "Toadb%Y_%m_%d_%H_%M_%S.log", p);  
    return 0;
}

static FILE* OpenSyslogFile(char *fileName, char* mode)
{
    FILE *fp = fopen(fileName, mode);
    return fp;
}




