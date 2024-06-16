/*
 *	toadb parser 
 * Copyright (C) 2023-2023, senllang
*/
#include <stdio.h>
#include <stdlib.h>

/* header file must be order by scanner_ext, parser, scanner, grammar. */
#include "scanner_ext.h"
#include "parser.h"
#include "scanner.h"
#include "grammar.h"
#include "buffer.h"
#include "memStack.h"

#define hat_log printf

/* scanner_init declare is needed by raw_parser */
extern void* scanner_init(char *sqlStr, PSCANNER_DATA yyext);


List* raw_parser(char *sqlstr)
{
    PSCANNER_DATA parserinfo = NULL;
    yyscan_t scannerinfo = NULL;
    YYVAL_TYPE value;
    int tag = 0;

    /* parserinfo memory will be released by caller, include members malloc .*/
    parserinfo = (PSCANNER_DATA)AllocMem(sizeof(SCANNER_DATA));
    if(parserinfo == NULL)
    {
        hat_log("Memory is not enough.");
        return NULL;
    }

    /* scanner initialize with sqlstr and userdata. */
    scannerinfo = scanner_init(sqlstr, parserinfo);

#if SINGLE_TEST_FLEX
    /* flex test single. */
    while(tag != ';')
    {
        value.sval = NULL;
        value.ival = 0;
        tag = yylex(&value, scannerinfo);
        printf("tag:%d, value:%s", tag, value.sval == NULL? "" : value.sval);
    }
#endif

    parserinfo->parserTree = NULL;
    parserinfo->result = 1;

    yyparse(scannerinfo);

    yylex_destroy(scannerinfo);
    return parserinfo->parserTree;
}

int ReleaseParserTreeResource(List *tree)
{
    if(NULL == tree)
        return -1;
    /* release parser tree */

    /* release parser arg */
    FreeMem(tree);
    return 0;
}