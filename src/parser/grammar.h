/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_PARSER_GRAMMAR_H_INCLUDED
# define YY_YY_PARSER_GRAMMAR_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SELECT = 258,
    CREATE = 259,
    TABLE = 260,
    FROM = 261,
    DELETE = 262,
    DROP = 263,
    INSERT = 264,
    INTO = 265,
    VALUES = 266,
    UPDATE = 267,
    SET = 268,
    WHERE = 269,
    AND = 270,
    OR = 271,
    NOT = 272,
    ORDER = 273,
    GROUP = 274,
    LIMIT = 275,
    OFFSET = 276,
    AS = 277,
    BY = 278,
    IDENT = 279,
    STRING = 280,
    INTNUMBER = 281,
    FLOATNUMBER = 282,
    LESS_EQ = 283,
    GREATER_EQ = 284,
    NOT_EQ = 285,
    Op = 286,
    UMINUS = 287
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 35 "parser/grammar.y" /* yacc.c:1909  */

    char *sval;
    int  ival;
    char op;
    float fval;
    PList list;
    PNode node;

#line 96 "./parser/grammar.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (yyscan_t yyscaninfo);

#endif /* !YY_YY_PARSER_GRAMMAR_H_INCLUDED  */
