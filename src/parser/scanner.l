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

#ifdef SCANNER_PARSER_LOG
#define log printf
#else 
#define log
#endif

%}

/* 空白符定义 */
horiz_space		[ \t\f]
newline			[\n\r]
space       [ \t\n\r\f]
nonewline   [^\n\r]

/* sql的注释，空白符，进行过滤 */
comment     ("--"{nonewline}*)
whitespace		({space}+|{comment})

special_whitespace		({space}+|{comment}{newline})
horiz_whitespace		({horiz_space}|{comment})
whitespace_with_newline	({horiz_whitespace}*{newline}{special_whitespace}*)

/* 标识符，字符串，整型数，浮点数 */
identify    [a-zA-Z][a-zA-Z0-9_]* 

string      '(\\.|''|[^'\n])*'|\"(\\.|\"\"|[^\"\n])*\"

intnumber   [-]?[0-9]+ 

/* 匹配 1.12 / 12.  ; .12 ;  和 带指数小数 */
floatnumber  -?[0-9]+"."[0-9]*|-?"."[0-9]+|-?[0-9]+"."[0-9]*E[-+]?[0-9]+|-?[0-9]+E[+-]?[0-9]+|-?[0-9]*"."[0-9]+E[+-]?[0-9]+

/* operators */
less_equals		"<="
greater_equals	">="
not_equals		"<>"

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
NOT    {
            return NOT; 
      }
ORDER {
            return ORDER;
        }                  
GROUP {
            return GROUP;
        }  
BY      {
            return BY;
        }       
LIMIT   {
            return LIMIT;
        }
OFFSET  {
            return OFFSET;
        }
AS      {
            return AS;
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
{intnumber}        {
                    yylval->ival = atoi(yytext);
                    log("digest :%d\n", yylval->ival);
                    return INTNUMBER;
                }
{floatnumber}   {
                    yylval->fval = atof(yytext);
                    log("float :%f\n", yylval->fval);
                    return FLOATNUMBER;
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
{less_equals}	{
                    return LESS_EQ;
                }
{greater_equals} {
                    return GREATER_EQ;
                }
{not_equals}	{
                    return NOT_EQ;
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
