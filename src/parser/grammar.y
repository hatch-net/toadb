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
    int  ival;
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
%token INTO
%token VALUES
%token UPDATE
%token WHERE
%token AND
%token OR
%token <sval> IDENT
%token <sval> STRING
%token <ival> DIGEST

%type <sval> tablename attr_type attr_name table_reference
%type <list> stmt_list columndef_list values_list attr_name_list select_opt_list table_reference_list
%type <node> stmt create_stmt column_def drop_stmt insert_stmt value_data select_stmt 
            

%start top_stmt

%{

// #define log printf
#define log

%}

%%

top_stmt:  stmt_list
                {
                    PSCANNER_DATA pExtData = (PSCANNER_DATA)yyget_extra(yyscaninfo);
                    pExtData->parserTree = $1; /* root of tree */
                    log("top stmt \n");
                }
        ;
stmt_list:  ';'
                {
                    log("null stmt \n");
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
                        log("stmt \n");
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
                        log("multi stmt \n");
                    }
                    else
                        $$ = $1;
                }
        ;
stmt:       select_stmt 
                    {
                        log("select stmt\n");
                        $$ = $1;
                    }
            | create_stmt
                    {
                        $$ = $1;
                    }
            | drop_stmt
                    {
                        $$ = $1;
                    }
            | insert_stmt
                    {
                        $$ = $1;
                    }

        ;
select_stmt:    SELECT select_opt_list
                    {
                        PSelectStmt node = (PSelectStmt)CreateNode(sizeof(SelectStmt),T_SelectStmt);
                        node->columnList = $2;
                        node->tblList = NULL;
                        node->selectAll = 0;

                        $$ = (PNode)node;

                        log("select \n");
                    }
            |   SELECT select_opt_list FROM table_reference_list
                    {
                        PSelectStmt node = (PSelectStmt)CreateNode(sizeof(SelectStmt),T_SelectStmt);
                        node->columnList = $2;
                        node->tblList = $4;

                        $$ = (PNode)node;

                        log("select  from \n");
                    }
            |   SELECT '*' FROM table_reference_list
                    {
                        PSelectStmt node = (PSelectStmt)CreateNode(sizeof(SelectStmt),T_SelectStmt);
                        node->columnList = NULL;
                        node->tblList = $4;
                        node->selectAll = 1;

                        $$ = (PNode)node;

                        log("select  from \n");
                    }
        ;
select_opt_list: attr_name
                    {
                        log("select  attr_name:%s \n", $1);
                        if(NULL != $1)
                        {
                            /* first node */
                            PList list = CreateCell(NULL);

                            PAttrName node = (PAttrName)CreateNode(sizeof(AttrName),T_AttrName);
                            node->attrName = $1;

                            list->tail->value.pValue = node;
                            
                            $$ = list;
                        }
                        else
                            $$ = NULL;
                    }
            | select_opt_list ',' attr_name
                    {
                        log("select  attr_name1:%s \n", $3);
                        if($3 != NULL)
                        {
                            PList list = CreateCell($1);

                            PAttrName node = (PAttrName)CreateNode(sizeof(AttrName),T_AttrName);
                            node->attrName = $3;

                            list->tail->value.pValue = node;
                            
                            $$ = list;
                        }
                        else
                            $$ = $1;
                    }
        ;

table_reference_list: table_reference
                    {
                        if(NULL != $1)
                        {
                            /* first node */
                            PList list = CreateCell(NULL);

                            PTableRefName node = (PTableRefName)CreateNode(sizeof(TableRefName),T_TableRefName);
                            node->tblRefName = $1;

                            list->tail->value.pValue = node;
                            
                            $$ = list;
                        }
                        else
                            $$ = NULL;
                    }
                | table_reference_list ',' table_reference
                    {
                        if($3 != NULL)
                        {
                            PList list = CreateCell($1);

                            PTableRefName node = (PTableRefName)CreateNode(sizeof(TableRefName),T_TableRefName);
                            node->tblRefName = $3;

                            list->tail->value.pValue = node;
                            
                            $$ = list;
                        }
                        else
                            $$ = $1;
                    }
        ;

table_reference:   tablename
                    {
                        $$ = $1;
                        log("table ref name:%s\n", $1);
                    }
        ;

create_stmt:        CREATE TABLE tablename '(' columndef_list ')'
                        {
                            PCreateStmt node = (PCreateStmt)CreateNode(sizeof(CreateStmt),T_CreateStmt);
                            node->tableName = $3;
                            node->ColList = $5;

                            $$ = (PNode)node;
                        }
        ;
drop_stmt:          DROP TABLE tablename
                    {
                        PDropStmt node = (PDropStmt)CreateNode(sizeof(DropStmt),T_DropStmt);
                        node->tableName = $3;

                        $$ = (PNode)node;
                    }
        ;
insert_stmt:    INSERT INTO tablename '(' attr_name_list ')' VALUES '(' values_list ')'
                    {                        
                        PInsertStmt node = (PInsertStmt)CreateNode(sizeof(InsertStmt),T_InsertStmt);
                        node->tableName = $3;
                        node->attrNameList = $5;
                        node->valuesList = $9;
                        
                        $$ = (PNode)node;
                        log("insert stmt %s\n", $3);
                    }
                | INSERT INTO tablename VALUES '(' values_list ')'
                    {
                        PInsertStmt node = (PInsertStmt)CreateNode(sizeof(InsertStmt),T_InsertStmt);
                        node->tableName = $3;
                        node->attrNameList = NULL;
                        node->valuesList = $6;
                        
                        $$ = (PNode)node;

                        log("insert noattr stmt %s\n", $3);
                    }
        ;
attr_name_list: attr_name
                    {
                        if(NULL != $1)
                        {
                            /* first node */
                            PList list = CreateCell(NULL);

                            PAttrName node = (PAttrName)CreateNode(sizeof(AttrName),T_AttrName);
                            node->attrName = $1;

                            list->tail->value.pValue = node;
                            
                            $$ = list;
                        }
                        else
                            $$ = NULL;
                        log("insert stmt attr_name:%s\n", $1);
                    }
                | attr_name_list ',' attr_name
                    {
                        if($3 != NULL)
                        {
                            PList list = CreateCell($1);

                            PAttrName node = (PAttrName)CreateNode(sizeof(AttrName),T_AttrName);
                            node->attrName = $3;

                            list->tail->value.pValue = node;
                            
                            $$ = list;
                        }
                        else
                            $$ = $1;
                        log("insert stmt multi attr_name:%s\n", $3);
                    }
        ;
values_list:    value_data
                    {
                        if(NULL != $1)
                        {
                            /* first node */
                            PList list = CreateCell(NULL);
                            list->tail->value.pValue = $1;                            

                            $$ = list;
                        }
                        else
                            $$ = NULL;
                        log("insert stmt value_data\n");
                    }
                | values_list ',' value_data
                    {
                        if($3 != NULL)
                        {
                            PList list = CreateCell($1);
                            list->tail->value.pValue = $3;
                            
                            $$ = list;
                        }
                        else
                            $$ = $1;                    
                        log("insert stmt multi value_data\n");
                    }
        ;
value_data:     STRING
                    {
                        PValuesData node = (PValuesData)CreateNode(sizeof(ValuesData),T_ValuesData);
                        node->value.pData = $1;
                        $$ = (PNode)node;

                        log("insert stmt value_data string %s \n", $1);
                    }
                | DIGEST
                    {
                        PValuesData node = (PValuesData)CreateNode(sizeof(ValuesData),T_ValuesData);
                        node->value.iData = $1;
                        $$ = (PNode)node;

                        log("insert stmt value_data int :%d \n", $1);
                    }
        ;
columndef_list:    column_def
                    {
                        if(NULL != $1)
                        {
                            /* first node */
                            PList list = CreateCell(NULL);
                            list->tail->value.pValue = $1;

                            $$ = list;
                        }
                        else
                            $$ = NULL;
                    }
                |    columndef_list ',' column_def
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
tablename:      IDENT
                    {
                        $$ = $1;
                    }
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
