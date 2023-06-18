/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toadmain.h"

char *DataDir = "./toadbtest";

int main(int argc, char *argv[])
{
	printf("Welcome to Toad Database Manage System.\n")	;

	if(argc > 1)
		DataDir = strdup(argv[1]);
	toadbMain(argc, argv);
	return 0;
}
