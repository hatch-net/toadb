%top{
/*
 *	toadb scanner 
 * Copyright (C) 2023-2023, senllang
*/
}

/* define list */
%{
#include <stdio.h>
#include <stdlib.h>

%}

%option noyywrap
%option yylineno
%option case-insensitive
%option reentrant
%option bison-bridge
%option header-file="scanner.h"

%{
#define YYSTYPE YYVAL_TYPE
#define YY_EXTRA_TYPE PSCANNER_DATA

#include "scanner_ext.h"
#include "grammar.h"


%}

/* operators */
operator    [-+&~|^/%*().!,;]
space       [ \t\n\r\f]
nonewline   [^\n\r]
comment     ("--"{nonewline}*)
whitespace		({space}+|{comment})
identify    [a-zA-Z][a-zA-Z0-9_]*

%%
SELECT  { 
            return SELECT; 
        }
FROM    {
            return FROM; 
        }
CREATE  {
            return CREATE; 
        }
TABLE	{
		return TABLE;
	}
UPDATE  {
            return UPDATE; 
        }
INSERT  {
            return INSERT; 
        }
DELETE  {
            return DELETE; 
        }
DROP    {
            return DROP; 
        }
{identify}        {
                    yylval->sval = strdup(yytext);
                    return IDENT;
                }
{operator}       {
                    return yytext[0];
                }
{whitespace}      {
                    /* ignore */
                }

%%
yyscan_t scanner_init(char *sqlStr, PSCANNER_DATA yyext)
{
    yyscan_t yyscaninfo;
	
    yylex_init(&yyscaninfo);
    yy_scan_string(sqlStr,yyscaninfo);

    /* set user data memory address */
    yyset_extra(yyext,yyscaninfo);

    return yyscaninfo;
}
