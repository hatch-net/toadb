/*
 *	toadb toadmain 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_TOADMAIN_H_H
#define HAT_TOADMAIN_H_H

#define MAX_COMMAND_LENGTH 1024

int toadbMain(int argc, char *argv[]) ;

int ToadMainEntry(char *query);

int ExitToad();

int args_opt(int argc, char *argv[]);

int checkDataDir();
void ShowToadbInfo();
#endif