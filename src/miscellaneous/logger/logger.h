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
#ifndef HAT_LOGGER_H_H
#define HAT_LOGGER_H_H



#define LOG_FILE_NAME_LEN 128
#define LOG_BUFFER_LEN    4096
#define LOG_HEADER_INFO_LEN    128

typedef struct SyslogInfo 
{
    int syslogFd;
    int doneRedirect;

}SyslogInfo, *PSyslogInfo;

typedef enum LogLevel
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_LOGGING,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
}LogLevel;


#define log_report(level, ...) log_message(__FILE__,__LINE__,__FUNCTION__, level, __VA_ARGS__)

#define hat_debugExcutorPathSelect(...)  
#define hat_debug1(...)  
#define hat_debug(...) 
#define hat_log(...)   log_report(LOG_LOGGING, __VA_ARGS__) 
#define hat_error(...) log_report(LOG_ERROR, __VA_ARGS__) 



int SyslogInit();
int SyslogDestroy();


void log_message(const char *filename, int lineno, const char *funcname, int level, const char *fmt, ...);

#endif 