/*
 *	toadb main 
 *
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
 * 
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