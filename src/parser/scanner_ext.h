/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_SCANNER_EXT_H_H
#define HAT_SCANNER_EXT_H_H

#include "node.h"

typedef struct SCANNER_EXTRE_DATA
{
    int result;
    List *parserTree;
}SCANNER_DATA, *PSCANNER_DATA;

typedef union YYVAL_TYPE
{
    char *sval;
    char op;
    PList list;
    PNode node;
}YYVAL_TYPE;

#define YYSTYPE YYVAL_TYPE


/* yyerror declare is needed by grammar.c */
void yyerror(void* yyscaninfo, const char *msg);

#endif