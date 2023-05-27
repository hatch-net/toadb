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
#include "grammar.h"
int test = 0;
%}

%option outfile="scanner.c"
%option noyywrap
%option yylineno
%option case-insensitive

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
                    yylval.sval = strdup(yytext);
                    return IDENT;
                }
{operator}       {
                    return yytext[0];
                }
{whitespace}      {
                    /* ignore */
                }

%%
