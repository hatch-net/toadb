/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/

#ifndef HAT_SCANNER_EXT_H_H
#define HAT_SCANNER_EXT_H_H

#include "parserNode.h"


#ifdef PARSER_DEBUG_LOG
#define GRAMMAR_PARSER_LOG 1
#define SCANNER_PARSER_LOG 1

#endif


typedef struct SCANNER_EXTRE_DATA
{
    List *parserTree;       /* It must be the first member, which address is used to free memory. */
    int result;
}SCANNER_DATA, *PSCANNER_DATA;

typedef union YYVAL_TYPE
{
    char *sval;
    int ival;
    char op;
    float fval;
    PList list;
    PNode node;
}YYVAL_TYPE;

#define YYSTYPE YYVAL_TYPE


/* yyerror declare is needed by grammar.c */
void yyerror(void* yyscaninfo, const char *msg);

#endif