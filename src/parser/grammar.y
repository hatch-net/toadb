%{
/*
 *	toadb grammar 
 * Copyright (C) 2023-2023, senllang
*/

#include <stdio.h>
#include <stdlib.h>

%}


%{

#include "scanner_ext.h"
#include "scanner.h"
#include "node.h"

%}

%pure-parser
%parse-param { yyscan_t yyscaninfo }
%lex-param { yyscan_t yyscaninfo }

%union {
    char *sval;
    char op;
    PList list;
    PNode node;
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

%type <sval> tablename attr_type attr_name 
%type <list> stmt_list column_list
%type <node> stmt create_stmt column_def

%start top_stmt

%{

#define log printf

%}

%%

top_stmt:  stmt_list
                {
                    PSCANNER_DATA pExtData = (PSCANNER_DATA)yyget_extra(yyscaninfo);
                    pExtData->parserTree = $1; /* root of tree */
                }
        ;
stmt_list:  ';'
                {
                    $$ = NULL;
                }
            | stmt ';'
                {
                    if($1 != NULL)
                    {
                        /* first node */
                        PList list = CreateCell(NULL);
                        list->tail->value.pValue = $1;

                        $$ = list;
                    }
                    else
                        $$ = NULL;
                }
            | stmt_list stmt ';'
                {
                    if($2 != NULL)
                    {
                        PList list = CreateCell($1);
                        list->tail->value.pValue = $2;
                        
                        $$ = list;
                    }
                    else
                        $$ = $1;
                }
        ;
stmt:       select_stmt 
                    {
                        printf("select stmt\n");
                        $$ = NULL;
                    }
            | create_stmt
                    {
                        $$ = $1;
                    }
            | drop_stmt
                    {
                        printf("drop stmt\n");
                        $$ = NULL;
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
                            PCreateStmt node = (PCreateStmt)CreateNode(sizeof(CreateStmt),T_CreateStmt);
                            node->tableName = $3;
                            node->ColList = $5;

                            $$ = (PNode)node;
                        }
        ;
drop_stmt:          DROP TABLE tablename
                    {
                        printf("sql drop \n");
                    }
        ;
tablename:      IDENT
                    {
                        $$ = $1;
                    }
        ;
column_list:    column_def
                    {
                        if(NULL != $1)
                        {
                            PColumnDef node;

                            /* first node */
                            PList list = CreateCell(NULL);
                            list->tail->value.pValue = $1;

                            $$ = list;
                        }
                        else
                            $$ = NULL;
                    }
                |    column_list ',' column_def
                    {
                        if($3 != NULL)
                        {
                            PList list = CreateCell($1);
                            list->tail->value.pValue = $3;
                            
                            $$ = list;
                        }
                        else
                            $$ = $1;
                    }
        ;
column_def:   attr_name attr_type
                    {
                        PColumnDef node = (PColumnDef)CreateNode(sizeof(ColumnDef),T_ColumnDef);
                        node->colName = $1;
                        node->colType = $2;

                        $$ = (PNode)node;
                    }
        ;
attr_name:      IDENT
                    {
                        $$ = $1;
                    }
        ;
attr_type:      IDENT
                    {
                        $$ = $1;
                    }
        ;

%%

void yyerror(yyscan_t yyscaninfo, const char *msg)
{
        fprintf(stderr, "error: %s\n",msg);
}
