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
            printf("--select \n"); 
        }
from    {
            printf("--from \n"); 
        }
create  {
            printf("--CREATE\n"); 
        }
update  {
            printf("--update\n"); 
        }
insert  {
           printf("--insert\n"); 
        }
delete  {
            printf("--delete\n"); 
        }
drop    {
           printf("--drop\n"); 
        }
{identify}        {
                    printf("ident :%s\n", yytext); 
                } 
{operator}        {
                    printf("op:%c\n",yytext[0]); 
                }
{endtag}          {
                    printf("end:%c\n",yytext[0]); 
                }
{seperate}        {
                    printf("se:%c\n",yytext[0]); 
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
                        printf("drop stmt\n");
                    }
            | insert_stmt
                    {
                        printf("insert stmt\n");
                    }
            | delete_stmt
                    {
                        printf("delete stmt\n");
                    }
*/    
    yylex();
    return 0;
}