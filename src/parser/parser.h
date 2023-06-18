/*
 *	toadb parser 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_PARSER_H_H
#define HAT_PARSER_H_H

#include "node.h"

List* raw_parser(char *sqlstr);
int ReleaseParserTreeResource(List *tree);

#endif