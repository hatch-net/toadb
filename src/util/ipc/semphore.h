/*
 *	toadb semphore
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
#ifndef HAT_TOADB_SEMPHORE_H_H
#define HAT_TOADB_SEMPHORE_H_H


#include <semaphore.h>

typedef struct SemLock
{
    char name[32];
    sem_t semLock;
}SemLock, *PSemLock;

PSemLock InitializeSem(char *name, int initValue, PSemLock sem);

int DestorySem(char *name, PSemLock sem);
int CloseSem(PSemLock sem);

int WaitSem(PSemLock sem);
int PostSem(PSemLock sem);

#endif 