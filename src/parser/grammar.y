/*
 *	toadb grammar 
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

%{

#include <stdio.h>
#include <stdlib.h>

%}


%{

#include "scanner_ext.h"
#include "scanner.h"
#include "parserNode.h"

%}

%pure-parser
%parse-param { yyscan_t yyscaninfo }
%lex-param { yyscan_t yyscaninfo }

%union {
    char *sval;
    int  ival;
    char op;
    float fval;
    PList list;
    PNode node;
}

/* tokens define */

/* keywords define */
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
%token SET
%token WHERE
%token AND
%token OR
%token NOT
%token ORDER
%token GROUP
%token LIMIT
%token OFFSET
%token AS
%token BY

/* types define */
%token <sval> IDENT
%token <sval> STRING
%token <ival> INTNUMBER
%token <fval> FLOATNUMBER

/* logical operators */
%token LESS_EQ
%token GREATER_EQ
%token NOT_EQ

%type <sval> tablename attr_type attr_name aliasname indirection_element

%type <list> stmt_list columndef_list values_list multi_values_list values_opt attr_name_list_opt attr_name_list 
             sort_clause limit_clause target_opt target_list 
             from_clause from_list set_clause_list set_target_list
             where_clause group_clause groupby_list

%type <node> stmt create_stmt column_def drop_stmt insert_stmt value_data select_stmt select_clause target_element a_expr c_expr 
             columnRef exprConst groupby_element update_stmt set_target set_clause
             alias_clause_opt alias_clause  table_ref update_table_ref relation_expr constValues
            

%start top_stmt

/* 优先级定义，从低到高优先级排列 */
%left		OR
%left		AND
%right		NOT
%nonassoc   '>' '<' '='  LESS_EQ GREATER_EQ NOT_EQ
%left		Op 
%left       '+' '-' 
%left       '*' '/' '%'
%right		UMINUS
%left		'(' ')'
%left		'.'

%{

#ifdef GRAMMAR_PARSER_LOG
#define hat_log printf
#else 
#define hat_log
#endif

%}

%%

top_stmt:  stmt_list
                {
                    PSCANNER_DATA pExtData = (PSCANNER_DATA)yyget_extra(yyscaninfo);
                    pExtData->parserTree = $1; /* root of tree */
                    hat_log("top stmt \n");
                }
        ;
stmt_list:   ';'
                {
                    /* empty */
                    hat_log("null stmt \n");
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
                        hat_log("stmt \n");
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
                        hat_log("multi stmt \n");
                    }
                    else
                        $$ = $1;
                }
        ;
stmt:       select_stmt 
                    {
                        hat_log("stmt select stmt\n");
                        $$ = $1;
                    }
            | insert_stmt
                    {
                        $$ = $1;
                    }
            | update_stmt
                    {
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


        ;
select_stmt: select_clause sort_clause limit_clause
                    {                        
                        PSelectStmt node = (PSelectStmt)$1;

                        node->sortList = $2;
                        node->limitClause = $3;

                        $$ = (PNode)node;

                        hat_log("select_stmt \n");
                    }
        ;
select_clause: SELECT target_opt from_clause where_clause group_clause
                    {
                        /* 创建selectstmt 节点 */
                        PSelectStmt node = (PSelectStmt)CreateNode(sizeof(SelectStmt),T_SelectStmt);

                        node->targetlist = $2;
                        node->fromList = $3;
                        node->whereList = $4;
                        node->groupList = $5;

                        $$ = (PNode)node;
                    
                        hat_log("select \n");
                    }
        ;
target_opt: target_list
                    {
                       $$ = $1;  
                       hat_log("target list \n");
                    }
            | /* empty */
                    {
                        $$ = NULL;
                    }
        ;
target_list: target_element
                    {
                        /* create list, and create element, then add element to the list;  */
                        /* first node */
                        PList list = CreateCell(NULL);
                        list->tail->value.pValue = $1;
                            
                        $$ = list;

                        hat_log("target element \n");
                    }
            | target_list ',' target_element 
                    {
                        /* create element, then add element to the list;  */
                        PList list = CreateCell($1);
                        list->tail->value.pValue = $3;
                            
                        $$ = list;
                    }
        ;
target_element:
            a_expr
                    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = NULL;
                        node->indirection = NULL;
                        node->val = $1;
                        $$ = (PNode)node;

                        hat_log("target_element a_expr\n");
                    }
            | a_expr AS aliasname
                    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = $3;  /* alias name */
                        node->indirection = NULL;
                        node->val = $1;
                        $$ = (PNode)node;

                        hat_log("target_element a_expr as aliasname\n"); 
                    }
            | a_expr aliasname
                    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = $2;            /* alias name */
                        node->indirection = NULL;
                        node->val = $1;
                        $$ = (PNode)node;

                        hat_log("target_element a_expr aliasname\n"); 
                    }
            | '*'
                    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = NULL;
                        node->indirection = NULL;
                        node->val = NULL;
                        node->all = 1;
                        $$ = (PNode)node;

                        hat_log("target_element * \n"); 
                    }
        ;


a_expr: c_expr
                    {
                        $$ = $1;
                        hat_log("a_expr -> c_expr  \n"); 
                    }
        | '+' a_expr %prec UMINUS
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "+";
                        node->lexpr = (PNode)$2;
                        node->rexpr = NULL;
                        node->exprOpType = POSITIVE;
                        $$ = (PNode)node;
                        hat_log("a_expr -> +  \n"); 
                    }
        | '-' a_expr %prec UMINUS
                    {
                        /* TODO: */
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "-";
                        node->lexpr = (PNode)$2;
                        node->rexpr = NULL;
                        node->exprOpType = NEGATIVE;
                        $$ = (PNode)node;
                        hat_log("a_expr -> -  \n"); 
                    }
        | a_expr '+' a_expr
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "+";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = PLUS;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr + a_expr \n"); 
                    }
        | a_expr '-' a_expr
                   {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "-";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = MINUS;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr - a_expr \n"); 
                    }
        | a_expr '*' a_expr
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "*";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = MULTIPLE;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr * a_expr \n"); 
                    }
        | a_expr '/' a_expr  
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "/";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = DIVISIION;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr / a_expr \n"); 
                    }
        | a_expr '%' a_expr  
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "%";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = MOD;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr % a_expr \n"); 
                    }
        /* logical operators */
        | a_expr '>' a_expr
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = ">";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = GREATER;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr > a_expr \n"); 
                    }
        | a_expr '<' a_expr     
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = LESS;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr < a_expr \n"); 
                    }  
        | a_expr '=' a_expr
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "=";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = EQUAL;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr = a_expr \n"); 
                    }
        | a_expr LESS_EQ a_expr
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<=";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = LESS_EQUAL;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr <= a_expr \n"); 
                    }
        | a_expr GREATER_EQ a_expr   
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = ">=";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = GREATER_EQUAL;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr >= a_expr \n"); 
                    }    
        | a_expr NOT_EQ a_expr
                    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<>";
                        node->lexpr = (PNode)$1;
                        node->rexpr = (PNode)$3;
                        node->exprOpType = NOT_EQUAL;
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr <> a_expr \n"); 
                    }
        | a_expr AND a_expr       
                    {
                        PList list = CreateList($1);
                        list = AppendNode(list, $3);

                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = AND_EXPR;
                        node->args = list;
                        
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr AND a_expr \n"); 
                    }
        | a_expr OR a_expr
                    {
                        PList list = CreateList($1);
                        list = AppendNode(list, $3);

                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = OR_EXPR;
                        node->args = list;
                        
                        $$ = (PNode)node;
                        hat_log("a_expr ->a_expr OR a_expr \n"); 
                    }
        | NOT a_expr
                    {
                        PList list = CreateList($2);
                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = NOT_EXPR;
                        node->args = list;
                        
                        $$ = (PNode)node;
                        hat_log("a_expr ->NOT a_expr \n"); 
                    }
        ;
c_expr: columnRef
                {
                    $$ = $1;
                }
        | exprConst
                {
                    $$ = $1;
                }
        | '(' a_expr ')' opt_indirection
                {
                    /* TODO: 暂不支持 */
                    $$ = NULL;
                }
    ;
opt_indirection: /* empty */
        | opt_indirection indirection_element 
            {
                ;
            }
    ;
indirection_element:
        '.' attr_name 
            {
                $$ = $2;
            }
        | '.' '*'
            {
                $$ = "*";
            }
    ;

from_clause: FROM from_list
                {
                    $$ = $2;
                    hat_log("from clause  \n");
                }
            | /* empty */ 
                {
                    $$ = NULL;
                }
        ;
from_list: table_ref 
                {
                    PList list = CreateList($1);
                    $$ = list;

                    hat_log("from_list table_ref  \n");
                }
            | from_list ',' table_ref
                {
                    PList list = AppendNode($1, $3);

                    $$ = list;
                }
        ;
table_ref: relation_expr alias_clause_opt
                {
                    PRangeVar node = (PRangeVar)$1;
                    node->alias = (PAlias)$2;
                    $$ = (PNode)node;
                }
        ;
relation_expr: tablename
                {
                    PRangeVar node = (PRangeVar)CreateNode(sizeof(RangeVar),T_RangeVar);
                    node->relname = $1;
                    node->alias = NULL;
                    $$ = (PNode)node;
                    hat_log("from relation_expr %s  \n", $1);
                }
        ;
alias_clause_opt: alias_clause
                    {
                        $$ = (PNode)$1;
                    }
                | /* empty */
                    {
                        $$ = NULL;
                    }
        ;
alias_clause: AS aliasname '(' name_list ')'
                {
                    /* TODO: 暂不支持  */
                }
            | aliasname '(' name_list ')'
                {
                    /* TODO: 暂不支持  */
                }
            | AS aliasname 
                {
                    PAlias node = (PAlias)CreateNode(sizeof(Alias),T_Alias);
                    node->aliasname = $2;
                    $$ = (PNode)node;
                    hat_log("alias_clause AS aliasname %s  \n", $2);
                }
            | aliasname
                {
                    PAlias node = (PAlias)CreateNode(sizeof(Alias),T_Alias);
                    node->aliasname = $1;
                    $$ = (PNode)node;
                    hat_log("alias_clause aliasname %s  \n", $1);
                }
        ;
name_list:
            {

            }
        ;
where_clause: WHERE a_expr 
                {
                    PList list = CreateList($2);
                    $$ = list;
                    hat_log("where clause \n");
                }
            | /* empty */
                {
                    $$ = NULL;
                }
        ;
group_clause: GROUP BY groupby_list
                {
                    $$ = NULL;
                    hat_log("group clause\n");
                }
            | /* empty */
                {
                    $$ = NULL;                    
                }
        ;
groupby_list: groupby_element
                {
                    hat_log("groupby_list \n"); 
                    $$ = NULL;
                }
            | groupby_list ',' groupby_element
                {
                    $$ = NULL;
                }
        ;
groupby_element: a_expr
                {
                    $$ = NULL;
                }
        ;
sort_clause: ORDER BY a_expr
                {
                    $$ = NULL;
                    hat_log("sort clause\n");
                }
            | /* empty */
                {
                    $$ = NULL;
                }
        ;
limit_clause: LIMIT INTNUMBER
                {
                    $$ = NULL;
                    hat_log("limit clause\n");
                }
            | /* empty */
                {
                    $$ = NULL;
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
update_stmt:    UPDATE update_table_ref SET set_clause_list from_clause where_clause 
                {
                    PUpdateStmt node = NewNode(UpdateStmt);
                    node->relation = $2;
                    node->targetlist = $4;
                    node->fromList = $5;
                    node->whereList = $6;
                    $$ = (PNode)node;
                }
        ;
update_table_ref:   table_ref
                    {                    
                        $$ = $1;
                    }
        ;
set_clause_list: set_clause 
                    {
                        PList list = CreateList($1);
                        $$ = list;
                    }
                | set_clause_list ',' set_clause 
                    {
                        PList list = AppendNode($1, $3);
                        $$ = list;
                    }
        ;
set_clause: set_target '=' a_expr
                    {
                        PResTarget node = (PResTarget)$1;
                        node->setValue = $3;
                        $$ = $1;
                    }
                | '(' set_target_list ')' '=' a_expr
                    {
                        PListCell tmpCell = NULL;
                        PList l = $2;

                        for(tmpCell = l->head; tmpCell != NULL; tmpCell = tmpCell->next)
                        {
                            PResTarget node = (PResTarget)GetCellNodeValue(tmpCell);
                            node->setValue = $5;
                        }

                        $$ = (PNode)$2;
                    } 
        ;
set_target: columnRef
                    {
                        PResTarget node = NewNode(ResTarget);
                        node->name = NULL;            /* without alias name */
                        node->indirection = NULL;
                        node->val = $1;
                        $$ = (PNode)node;
                    }
        ;
set_target_list: set_target
                    {
                        PList list = CreateList($1);
                        $$ = list;
                    }
                | set_target_list ',' set_target
                    {
                        PList list = AppendNode($1, $3);
                        $$ = list;
                    }
        ;
insert_stmt:    INSERT INTO tablename attr_name_list_opt VALUES multi_values_list
                    {                        
                        PInsertStmt node = (PInsertStmt)CreateNode(sizeof(InsertStmt),T_InsertStmt);
                        node->tableName = $3;
                        node->attrNameList = $4;
                        node->valuesList = $6;
                        
                        $$ = (PNode)node;
                        hat_log("insert stmt %s\n", $3);
                    }
        ;
attr_name_list_opt : /* empty */
                    {
                        $$ = NULL;
                    }
                | '(' attr_name_list ')' 
                    {
                        $$ = $2;
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
                        hat_log("insert stmt attr_name:%s\n", $1);
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
                        hat_log("insert stmt multi attr_name:%s\n", $3);
                    }
        ;
multi_values_list: values_opt
                    {
                        /* first node */
                        $$ = AppendNode(NULL, (PNode)$1);
                    }
                | multi_values_list ',' values_opt
                    {
                        $$ = AppendNode($1, (PNode)$3);
                    } 
        ;
values_opt : '(' values_list ')'
                    {
                        $$ = $2;
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
                        hat_log("insert stmt value_data\n");
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
                        hat_log("insert stmt multi value_data\n");
                    }
        ;
value_data:     constValues
                    {
                        PValuesData node = (PValuesData)CreateNode(sizeof(ValuesData),T_ValuesData);
                        node->valueNode = (PNode)$1;
                        $$ = (PNode)node;

                        hat_log("insert stmt value_data \n");
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
columnRef:  attr_name 
                    {
                        PColumnRef node = (PColumnRef)CreateNode(sizeof(ColumnRef),T_ColumnRef);
                        node->field = $1;
                        node->tableName = NULL;
                        $$ = (PNode)node;
                        hat_log("columnRef attr_name :%s \n", $1); 
                    }
            | attr_name indirection_element
                    {
                        PColumnRef node = (PColumnRef)CreateNode(sizeof(ColumnRef),T_ColumnRef);
                        node->tableName = $1;
                        node->field = $2;
                        $$ = (PNode)node;
                        hat_log("columnRef attr_name :%s.%s \n", node->tableName, node->field); 
                    }
        ;

exprConst : constValues
                    {
                        $$ = $1;
                    }
        ;

constValues :  STRING
                    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.pData = $1;
                        node->vt = VT_VARCHAR;

                        $$ = (PNode)node;     
                        hat_log("exprConst string :%s \n", $1);                   
                    }
            | INTNUMBER 
                    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.iData = $1;
                        node->vt = VT_INT;

                        $$ = (PNode)node;    
                        hat_log("exprConst int :%d \n", $1);      
                    }
            | FLOATNUMBER
                    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.fData = $1;
                        node->vt = VT_FLOAT;

                        $$ = (PNode)node;    
                        hat_log("exprConst float :%f \n", $1);      
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
aliasname:      IDENT
                    {
                        $$ = $1;
                    }
        ;
%%

void yyerror(yyscan_t yyscaninfo, const char *msg)
{
        fprintf(stderr, "error: %s\n",msg);
}
