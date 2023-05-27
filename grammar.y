%{
/*
 *	toadb grammar 
 * Copyright (C) 2023-2023, senllang
*/

#include <stdio.h>
#include <stdlib.h>

int grammar_main();
%}

%union {
    char *sval;
    char op;
}

/* tokens define */
%token SELECT
%token CREATE
%token TABLE
%token FROM
%token DELETE
%token DROP
%token INSERT
%token UPDATE
%token <sval> IDENT


%%
stmt_list:  ;
            |stmt ';'
            | stmt_list stmt ';'
        ;
stmt:       select_stmt 
                    {
                        printf("select stmt\n");
                    }
            | create_stmt
                    {
                        printf("create stmt\n");
                    }
            | drop_stmt
                    {
                        printf("drop stmt\n");
                    }

        ;
select_stmt:    SELECT select_opt_list
                    {
                        printf("select \n");
                    }
            |   SELECT select_opt_list FROM table_reference_list
                    {
                        printf("select  from \n");
                    }
        ;
select_opt_list: attr_name
            | select_opt_list ',' attr_name
            | '*'
        ;

table_reference_list: table_reference
                | table_reference ',' table_reference
        ;

table_reference:   IDENT
                    {
                        printf("list ident :%s\n", $1);
                    }
        ;

create_stmt:        CREATE TABLE tablename '(' column_list ')'
                        {
                            printf("sql: create \n");
                        }
        ;
drop_stmt:          DROP TABLE tablename
                    {
                        printf("sql drop \n");
                    }
        ;
tablename:      IDENT
                    {
                        printf("ident :%s\n", $1);
                    }
        ;
column_list:    attr_define
                |    column_list ',' attr_define
        ;
attr_define:   attr_name attr_type
                    {
                        printf("attrdef \n");
                    }
        ;
attr_name:      IDENT
                    {
                        printf("ident :%s\n", $1);
                    }
        ;
attr_type:      IDENT
                    {
                        printf("ident :%s\n", $1);
                    }
        ;

%%
int grammar_main()
{
    yyparse();

    return 0;
}

yyerror(char *s)
{
        fprintf(stderr, "error: %s\n",s);
}
