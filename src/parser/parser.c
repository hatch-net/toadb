/*
 *	toadb main 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>

/* header file must be order by scanner_ext, parser, scanner, grammar. */
#include "scanner_ext.h"
#include "parser.h"
#include "scanner.h"
#include "grammar.h"


/* scanner_init declare is needed by raw_parser */
extern void* scanner_init(char *sqlStr, PSCANNER_DATA yyext);

#define log printf
List* raw_parser(char *sqlstr)
{
    PSCANNER_DATA parserinfo = NULL;
    yyscan_t scannerinfo = NULL;
    YYVAL_TYPE value;
    int tag = 0;

    /* parserinfo memory will be released by caller, include members malloc .*/
    parserinfo = (PSCANNER_DATA)malloc(sizeof(SCANNER_DATA));
    if(parserinfo == NULL)
    {
        log("Memory is not enough.\n");
        return NULL;
    }

    /* scanner initialize with sqlstr and userdata. */
    scannerinfo = scanner_init(sqlstr, parserinfo);

#if SINGLE_TEST_FLEX
    /* flex test single. */
    while(tag != ';')
    {
        value.sval = NULL;
        tag = yylex(&value, scannerinfo);
        printf("tag:%d, value:%s\n", tag, value.sval == NULL? "" : value.sval);
    }
#endif

    parserinfo->parserTree = NULL;
    parserinfo->result = 1;

    yyparse(scannerinfo);

    yylex_destroy(scannerinfo);
    return parserinfo->parserTree;
}
