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
// #define log printf
#define log 
%}

/* operators */
space       [ \t\n\r\f]
nonewline   [^\n\r]
comment     ("--"{nonewline}*)
whitespace		({space}+|{comment})
identify    [a-zA-Z][a-zA-Z0-9_]*
string      [']+[a-zA-Z0-9_]*[']+
digest      [-]?[0-9]+

equals_greater	"=>"
less_equals		"<="
greater_equals	">="
less_greater	"<>"
not_equals		"!="

self			[,()\[\].;\:\+\-\*\/\%\^\<\>\=]
op_chars		[\~\!\@\#\^\&\|\`\?\+\-\*\/\%\<\>\=]
operator		{op_chars}+

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
INTO    {
            return INTO; 
        }
VALUES  {
            return VALUES; 
        }    
DELETE  {
            return DELETE; 
        }
DROP    {
            return DROP; 
        }
WHERE   {
            return WHERE; 
        }
AND    {
            return AND; 
        }
OR    {
            return OR; 
        }                        
{string}        {
                    int len = 0;

                    /* skip ' charactor */
                    yylval->sval = strdup(yytext+1);
                    len = strlen(yylval->sval);
                    yylval->sval[len-1] = '\0';
                    log("string :%s\n", yylval->sval);
                    return STRING;
                }
{digest}        {
                    yylval->ival = atoi(yytext);
                    log("digest :%d\n", yylval->ival);
                    return DIGEST;
                }
{identify}      {
                    yylval->sval = strdup(yytext);
                    return IDENT;
                }
{self}       {
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
