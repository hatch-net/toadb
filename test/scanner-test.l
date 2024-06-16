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
int test = 0;
%}

%option outfile="scanner-test.c"
%option noyywrap
%option yylineno
%option case-insensitive
%option header-file="scanner-test.h"

/* operators */
endtag      [;]*
seperate    [,]
operator    [-+&~|^/%*().!]
space       [ \t\n\r\f]
nonewline   [^\n\r]
comment     ("--"{nonewline}*)
whitespace		({space}+|{comment})
identify    [a-zA-Z][a-zA-Z0-9_]*


%%
select  { 
            printf("--select "); 
        }
from    {
            printf("--from "); 
        }
create  {
            printf("--CREATE"); 
        }
update  {
            printf("--update"); 
        }
insert  {
           printf("--insert"); 
        }
delete  {
            printf("--delete"); 
        }
drop    {
           printf("--drop"); 
        }
{identify}        {
                    printf("ident :%s", yytext); 
                } 
{operator}        {
                    printf("op:%c",yytext[0]); 
                }
{endtag}          {
                    printf("end:%c",yytext[0]); 
                }
{seperate}        {
                    printf("se:%c",yytext[0]); 
                }               
{space}           {
                    /* ignore */
                }
{whitespace}      {
                    /* ignore */
                }
%%
int main(int argc, char * argv[])
{
    /*            | drop_stmt
                    {
                        printf("drop stmt");
                    }
            | insert_stmt
                    {
                        printf("insert stmt");
                    }
            | delete_stmt
                    {
                        printf("delete stmt");
                    }
*/    
    yylex();
    return 0;
}