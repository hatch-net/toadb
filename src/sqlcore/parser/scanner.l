%top{
/*
 *	toadb scanner 
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
#define hat_log printf
#else 
#define hat_log
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
SET     {
            return SET; 
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
BEGIN   {
            return BEGIN_T;
        }
END     {
            return END_T;
        }
ROLLBACK {
            return ROLLBACK;
        }
COMMIT  {
            return COMMIT;
        }
SAVEPOINT   {
            return SAVEPOINT;
        }

{string}        {
                    int len = 0;

                    /* skip ' charactor */
                    yylval->sval = strdup(yytext+1);
                    len = strlen(yylval->sval);
                    yylval->sval[len-1] = '\0';
                    hat_log("string :%s", yylval->sval);
                    return STRING;
                }
{intnumber}        {
                    yylval->ival = atoi(yytext);
                    hat_log("digest :%d", yylval->ival);
                    return INTNUMBER;
                }
{floatnumber}   {
                    yylval->fval = atof(yytext);
                    hat_log("float :%f", yylval->fval);
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
