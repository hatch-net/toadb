/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "node.h"

int main(int argc, char *argv[])
{
	char str[1024];
	List *parserTree = NULL;

	printf("input sql please \n");

	if(argc > 1)
		sprintf(str, "%s", argv[1]);
	else
		return -1;
		
	parserTree = raw_parser(str);
	travelParserTree(parserTree);
	
	return 0;
}
