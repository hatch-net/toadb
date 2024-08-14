/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 15 "sqlcore/parser/grammar.y" /* yacc.c:339  */


#include <stdio.h>
#include <stdlib.h>

#line 23 "sqlcore/parser/grammar.y" /* yacc.c:339  */


#include "scanner_ext.h"
#include "scanner.h"
#include "parserNode.h"


#line 80 "sqlcore/parser/grammar.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "grammar.h".  */
#ifndef YY_YY_SQLCORE_PARSER_GRAMMAR_H_INCLUDED
# define YY_YY_SQLCORE_PARSER_GRAMMAR_H_INCLUDED
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
    BEGIN_T = 279,
    END_T = 280,
    ROLLBACK = 281,
    COMMIT = 282,
    SAVEPOINT = 283,
    IDENT = 284,
    STRING = 285,
    INTNUMBER = 286,
    FLOATNUMBER = 287,
    LESS_EQ = 288,
    GREATER_EQ = 289,
    NOT_EQ = 290,
    Op = 291,
    UMINUS = 292
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 35 "sqlcore/parser/grammar.y" /* yacc.c:355  */

    char *sval;
    int  ival;
    char op;
    float fval;
    PList list;
    PNode node;

#line 167 "sqlcore/parser/grammar.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (yyscan_t yyscaninfo);

#endif /* !YY_YY_SQLCORE_PARSER_GRAMMAR_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 112 "sqlcore/parser/grammar.y" /* yacc.c:358  */


#ifdef GRAMMAR_PARSER_LOG
#define hat_log printf
#else 
#define hat_log
#endif


#line 192 "sqlcore/parser/grammar.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  53
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   252

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  51
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  54
/* YYNRULES -- Number of rules.  */
#define YYNRULES  115
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  193

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   292

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    44,     2,     2,
      46,    47,    42,    40,    50,    41,    48,    43,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    49,
      37,    38,    36,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    39,    45
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   124,   124,   131,   137,   151,   165,   169,   173,   177,
     181,   185,   189,   196,   208,   223,   229,   233,   244,   254,
     264,   274,   284,   298,   303,   315,   328,   340,   352,   364,
     376,   389,   401,   413,   425,   437,   449,   461,   474,   487,
     499,   503,   507,   512,   517,   518,   524,   528,   534,   540,
     544,   551,   558,   565,   574,   579,   583,   587,   591,   598,
     607,   611,   618,   622,   628,   632,   637,   642,   647,   653,
     657,   663,   668,   675,   683,   692,   700,   710,   715,   720,
     726,   732,   746,   755,   760,   766,   778,   781,   786,   804,
     822,   827,   832,   837,   851,   865,   874,   883,   890,   897,
     904,   911,   919,   932,   945,   954,   962,   972,   978,   987,
     996,  1006,  1011,  1016,  1021,  1026
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SELECT", "CREATE", "TABLE", "FROM",
  "DELETE", "DROP", "INSERT", "INTO", "VALUES", "UPDATE", "SET", "WHERE",
  "AND", "OR", "NOT", "ORDER", "GROUP", "LIMIT", "OFFSET", "AS", "BY",
  "BEGIN_T", "END_T", "ROLLBACK", "COMMIT", "SAVEPOINT", "IDENT", "STRING",
  "INTNUMBER", "FLOATNUMBER", "LESS_EQ", "GREATER_EQ", "NOT_EQ", "'>'",
  "'<'", "'='", "Op", "'+'", "'-'", "'*'", "'/'", "'%'", "UMINUS", "'('",
  "')'", "'.'", "';'", "','", "$accept", "top_stmt", "stmt_list", "stmt",
  "select_stmt", "select_clause", "target_opt", "target_list",
  "target_element", "a_expr", "c_expr", "opt_indirection",
  "indirection_element", "from_clause", "from_list", "table_ref",
  "relation_expr", "alias_clause_opt", "alias_clause", "name_list",
  "where_clause", "group_clause", "groupby_list", "groupby_element",
  "sort_clause", "limit_clause", "function_expr", "create_stmt",
  "drop_stmt", "update_stmt", "update_table_ref", "set_clause_list",
  "set_clause", "set_target", "set_target_list", "insert_stmt",
  "attr_name_list_opt", "attr_name_list", "multi_values_list",
  "values_opt", "values_list", "value_data", "delete_stmt",
  "transactionStmt", "columndef_list", "column_def", "columnRef",
  "exprConst", "constValues", "function_name", "tablename", "attr_name",
  "attr_type", "aliasname", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,    62,    60,    61,   291,
      43,    45,    42,    47,    37,   292,    40,    41,    46,    59,
      44
};
# endif

#define YYPACT_NINF -156

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-156)))

#define YYTABLE_NINF -112

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-112)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      56,   -10,     9,    23,    10,    28,    37,  -156,  -156,  -156,
    -156,  -156,  -156,    39,   200,    18,  -156,    55,  -156,  -156,
    -156,  -156,  -156,  -156,   170,    32,  -156,  -156,  -156,   170,
     170,  -156,   170,    81,    42,  -156,   105,  -156,  -156,  -156,
    -156,  -156,    43,    53,    37,    37,    37,    37,  -156,  -156,
       6,    89,  -156,  -156,    54,  -156,    83,    84,   196,  -156,
    -156,    75,    37,   100,   -10,   170,   170,    94,  -156,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
    -156,   -30,   -18,  -156,    78,    81,  -156,    79,    94,  -156,
    -156,    80,   -19,  -156,   170,    97,  -156,  -156,    82,  -156,
     170,   110,  -156,   196,   148,  -156,   208,   208,   208,   208,
     208,   208,    52,    52,  -156,  -156,  -156,    88,  -156,  -156,
    -156,  -156,   102,   100,   102,   125,    91,  -156,   102,     7,
    -156,   106,  -156,   135,  -156,    53,    37,   135,   129,  -156,
    -156,    -7,  -156,   124,  -156,    24,  -156,   108,  -156,   109,
    -156,    25,   -19,   100,   170,  -156,  -156,   170,  -156,   102,
    -156,  -156,  -156,   102,    68,   107,  -156,   111,  -156,   117,
     102,  -156,  -156,   135,   135,   114,  -156,  -156,  -156,    38,
    -156,  -156,   108,  -156,   170,  -156,   170,  -156,    68,  -156,
     135,  -156,  -156
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    16,     0,     0,     0,     0,     0,    97,    98,    99,
     100,   101,     3,     0,     2,     0,     6,    69,    10,    11,
       8,     7,     9,    12,     0,   113,   108,   109,   110,     0,
       0,    22,     0,    49,    15,    17,    19,    23,    43,    40,
      41,   107,     0,   105,     0,     0,     0,     0,   112,    77,
      55,     0,    53,     1,     0,     4,     0,    71,    39,    24,
      25,     0,     0,    62,     0,     0,     0,     0,   115,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      21,     0,     0,   106,     0,    49,    75,    86,     0,    52,
      54,    59,     0,     5,     0,     0,    13,    44,    48,    50,
       0,    64,    18,    37,    38,    20,    34,    35,    36,    31,
      32,    33,    26,    27,    28,    29,    30,     0,    72,   113,
      47,    46,     0,    62,     0,     0,    58,    60,     0,    49,
      78,     0,    82,    68,    70,    42,     0,    61,     0,    14,
      73,     0,   102,     0,    96,     0,    88,     0,    60,     0,
      83,     0,     0,    62,     0,    45,    51,     0,    74,     0,
     114,   104,    87,     0,     0,    85,    90,     0,    57,     0,
       0,    79,    76,    80,    67,    63,    65,   103,    89,     0,
      93,    95,     0,    56,     0,    84,     0,    92,     0,    91,
      81,    66,    94
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -156,  -156,  -156,   145,  -156,  -156,  -156,  -156,   101,   -24,
    -156,  -156,    26,   -67,  -156,   -59,  -156,  -156,  -156,    45,
    -119,  -156,  -156,   -20,  -156,  -156,  -156,  -156,  -156,  -156,
     122,  -156,    22,  -126,  -156,  -156,  -156,  -156,  -156,    -2,
    -156,     8,  -156,  -156,  -156,    35,   -91,  -156,  -155,  -156,
     -21,   -66,  -156,    19
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    13,    14,    15,    16,    17,    33,    34,    35,    36,
      37,   135,    83,    63,    98,    49,    50,    89,    90,   149,
     101,   139,   175,   176,    57,    96,    38,    18,    19,    20,
      51,   129,   130,   131,   151,    21,   125,   145,   165,   166,
     179,   180,    22,    23,   141,   142,    39,    40,    41,    42,
      52,    43,   161,    80
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      58,   132,   150,    99,   144,    59,    60,    24,    61,   181,
     119,   119,   117,    62,    44,    46,   121,   118,   123,    25,
      26,    27,    28,    84,   120,    86,    87,   128,    88,    45,
      29,    30,    31,   181,   172,    68,    32,   132,    47,    53,
     158,   103,   104,   159,   185,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   143,   152,   146,     1,
       2,   132,   153,     3,     4,     5,    48,    55,     6,    91,
     133,   162,   169,    56,   163,   170,   137,   156,  -111,   132,
       7,     8,     9,    10,    11,   187,   105,    62,   188,    81,
      65,    66,    64,   143,    77,    78,    79,   178,    26,    27,
      28,    82,    92,    93,    95,    12,    94,   126,    69,    70,
      71,    72,    73,    74,   100,    75,    76,    77,    78,    79,
      65,    66,    97,    68,   122,   124,   127,    67,   134,   138,
     173,   119,   136,   174,    68,   140,   147,   148,    69,    70,
      71,    72,    73,    74,   154,    75,    76,    77,    78,    79,
      65,    66,   157,   160,   164,   184,   168,   182,   183,    54,
     190,   155,   174,    65,   186,   102,   191,    85,    69,    70,
      71,    72,    73,    74,   171,    75,    76,    77,    78,    79,
     189,    69,    70,    71,    72,    73,    74,    24,    75,    76,
      77,    78,    79,   167,   177,     0,   192,     0,     0,    25,
      26,    27,    28,     1,     2,     0,     0,     3,     4,     5,
      29,    30,     6,     0,     0,     0,    32,     0,     0,     0,
       0,     0,     0,     0,     7,     8,     9,    10,    11,    69,
      70,    71,    72,    73,    74,     0,    75,    76,    77,    78,
      79,  -112,  -112,  -112,  -112,  -112,  -112,     0,    75,    76,
      77,    78,    79
};

static const yytype_int16 yycheck[] =
{
      24,    92,   128,    62,   123,    29,    30,    17,    32,   164,
      29,    29,    42,     6,     5,     5,    82,    47,    85,    29,
      30,    31,    32,    44,    42,    46,    47,    46,    22,     6,
      40,    41,    42,   188,   153,    29,    46,   128,    10,     0,
      47,    65,    66,    50,   170,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,   122,    50,   124,     3,
       4,   152,   129,     7,     8,     9,    29,    49,    12,    50,
      94,    47,    47,    18,    50,    50,   100,   136,    46,   170,
      24,    25,    26,    27,    28,    47,    67,     6,    50,    46,
      15,    16,    50,   159,    42,    43,    44,   163,    30,    31,
      32,    48,    13,    49,    20,    49,    23,    88,    33,    34,
      35,    36,    37,    38,    14,    40,    41,    42,    43,    44,
      15,    16,    47,    29,    46,    46,    46,    22,    31,    19,
     154,    29,    50,   157,    29,    47,    11,    46,    33,    34,
      35,    36,    37,    38,    38,    40,    41,    42,    43,    44,
      15,    16,    23,    29,    46,    38,    47,    50,    47,    14,
     184,   135,   186,    15,    50,    64,   186,    45,    33,    34,
      35,    36,    37,    38,   152,    40,    41,    42,    43,    44,
     182,    33,    34,    35,    36,    37,    38,    17,    40,    41,
      42,    43,    44,   148,   159,    -1,   188,    -1,    -1,    29,
      30,    31,    32,     3,     4,    -1,    -1,     7,     8,     9,
      40,    41,    12,    -1,    -1,    -1,    46,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    24,    25,    26,    27,    28,    33,
      34,    35,    36,    37,    38,    -1,    40,    41,    42,    43,
      44,    33,    34,    35,    36,    37,    38,    -1,    40,    41,
      42,    43,    44
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     7,     8,     9,    12,    24,    25,    26,
      27,    28,    49,    52,    53,    54,    55,    56,    78,    79,
      80,    86,    93,    94,    17,    29,    30,    31,    32,    40,
      41,    42,    46,    57,    58,    59,    60,    61,    77,    97,
      98,    99,   100,   102,     5,     6,     5,    10,    29,    66,
      67,    81,   101,     0,    54,    49,    18,    75,    60,    60,
      60,    60,     6,    64,    50,    15,    16,    22,    29,    33,
      34,    35,    36,    37,    38,    40,    41,    42,    43,    44,
     104,    46,    48,    63,   101,    81,   101,   101,    22,    68,
      69,   104,    13,    49,    23,    20,    76,    47,    65,    66,
      14,    71,    59,    60,    60,   104,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    60,    42,    47,    29,
      42,   102,    46,    64,    46,    87,   104,    46,    46,    82,
      83,    84,    97,    60,    31,    62,    50,    60,    19,    72,
      47,    95,    96,   102,    71,    88,   102,    11,    46,    70,
      84,    85,    50,    64,    38,    63,    66,    23,    47,    50,
      29,   103,    47,    50,    46,    89,    90,    70,    47,    47,
      50,    83,    71,    60,    60,    73,    74,    96,   102,    91,
      92,    99,    50,    47,    38,    84,    50,    47,    50,    90,
      60,    74,    92
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    51,    52,    53,    53,    53,    54,    54,    54,    54,
      54,    54,    54,    55,    56,    57,    57,    58,    58,    59,
      59,    59,    59,    60,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    60,    60,    60,    60,
      61,    61,    61,    61,    62,    62,    63,    63,    64,    64,
      65,    65,    66,    67,    68,    68,    69,    69,    69,    69,
      70,    71,    71,    72,    72,    73,    73,    74,    75,    75,
      76,    76,    77,    77,    78,    79,    80,    81,    82,    82,
      83,    83,    84,    85,    85,    86,    87,    87,    88,    88,
      89,    89,    90,    91,    91,    92,    93,    94,    94,    94,
      94,    94,    95,    95,    96,    97,    97,    98,    99,    99,
      99,   100,   101,   102,   103,   104
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     3,     5,     1,     0,     1,     3,     1,
       3,     2,     1,     1,     2,     2,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       1,     1,     4,     1,     0,     2,     2,     2,     2,     0,
       1,     3,     2,     1,     1,     0,     5,     4,     2,     1,
       0,     2,     0,     3,     0,     1,     3,     1,     3,     0,
       2,     0,     3,     4,     6,     3,     6,     1,     1,     3,
       3,     5,     1,     1,     3,     6,     0,     3,     1,     3,
       1,     3,     3,     1,     3,     1,     5,     1,     1,     1,
       1,     1,     1,     3,     2,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (yyscaninfo, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, yyscaninfo); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, yyscan_t yyscaninfo)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yyscaninfo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, yyscan_t yyscaninfo)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yyscaninfo);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, yyscan_t yyscaninfo)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , yyscaninfo);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, yyscaninfo); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, yyscan_t yyscaninfo)
{
  YYUSE (yyvaluep);
  YYUSE (yyscaninfo);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (yyscan_t yyscaninfo)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, yyscaninfo);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 125 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PSCANNER_DATA pExtData = (PSCANNER_DATA)yyget_extra(yyscaninfo);
                    pExtData->parserTree = (yyvsp[0].list); /* root of tree */
                    hat_log("top stmt ");
                }
#line 1444 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 3:
#line 132 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    /* empty */
                    hat_log("null stmt ");
                    (yyval.list) = NULL;
                }
#line 1454 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 4:
#line 138 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    if((yyvsp[-1].node) != NULL)
                    {
                        /* first node */
                        PList list = CreateCell(NULL);
                        list->tail->value.pValue = (yyvsp[-1].node);

                        (yyval.list) = list;
                        hat_log("stmt ");
                    }
                    else
                        (yyval.list) = NULL;
                }
#line 1472 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 5:
#line 152 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    if((yyvsp[-1].node) != NULL)
                    {
                        PList list = CreateCell((yyvsp[-2].list));
                        list->tail->value.pValue = (yyvsp[-1].node);
                        
                        (yyval.list) = list;
                        hat_log("multi stmt ");
                    }
                    else
                        (yyval.list) = (yyvsp[-2].list);
                }
#line 1489 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 6:
#line 166 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1497 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 7:
#line 170 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1505 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 8:
#line 174 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1513 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 9:
#line 178 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1521 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 10:
#line 182 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1529 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 11:
#line 186 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1537 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 12:
#line 190 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1545 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 13:
#line 197 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {                        
                        PSelectStmt node = (PSelectStmt)(yyvsp[-2].node);

                        node->sortList = (yyvsp[-1].list);
                        node->limitClause = (yyvsp[0].list);

                        (yyval.node) = (PNode)node;

                        hat_log("select_stmt ");
                    }
#line 1560 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 14:
#line 209 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        /* 创建selectstmt 节点 */
                        PSelectStmt node = (PSelectStmt)CreateNode(sizeof(SelectStmt),T_SelectStmt);

                        node->targetlist = (yyvsp[-3].list);
                        node->fromList = (yyvsp[-2].list);
                        node->whereList = (yyvsp[-1].list);
                        node->groupList = (yyvsp[0].list);

                        (yyval.node) = (PNode)node;
                    
                        hat_log("select ");
                    }
#line 1578 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 15:
#line 224 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                       (yyval.list) = (yyvsp[0].list);  
                       hat_log("target list ");
                    }
#line 1587 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 16:
#line 229 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = NULL;
                    }
#line 1595 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 17:
#line 234 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        /* create list, and create element, then add element to the list;  */
                        /* first node */
                        PList list = CreateCell(NULL);
                        list->tail->value.pValue = (yyvsp[0].node);
                            
                        (yyval.list) = list;

                        hat_log("target element ");
                    }
#line 1610 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 18:
#line 245 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        /* create element, then add element to the list;  */
                        PList list = CreateCell((yyvsp[-2].list));
                        list->tail->value.pValue = (yyvsp[0].node);
                            
                        (yyval.list) = list;
                    }
#line 1622 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 19:
#line 255 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = NULL;
                        node->indirection = NULL;
                        node->val = (yyvsp[0].node);
                        (yyval.node) = (PNode)node;

                        hat_log("target_element a_expr");
                    }
#line 1636 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 20:
#line 265 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = (yyvsp[0].sval);  /* alias name */
                        node->indirection = NULL;
                        node->val = (yyvsp[-2].node);
                        (yyval.node) = (PNode)node;

                        hat_log("target_element a_expr as aliasname"); 
                    }
#line 1650 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 21:
#line 275 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = (yyvsp[0].sval);            /* alias name */
                        node->indirection = NULL;
                        node->val = (yyvsp[-1].node);
                        (yyval.node) = (PNode)node;

                        hat_log("target_element a_expr aliasname"); 
                    }
#line 1664 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 22:
#line 285 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = NULL;
                        node->indirection = NULL;
                        node->val = NULL;
                        node->all = 1;
                        (yyval.node) = (PNode)node;

                        hat_log("target_element * "); 
                    }
#line 1679 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 23:
#line 299 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                        hat_log("a_expr -> c_expr  "); 
                    }
#line 1688 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 24:
#line 304 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "+";
                        node->lexpr = (PNode)(yyvsp[0].node);
                        node->rexpr = NULL;
                        node->exprOpType = POSITIVE;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr -> +  "); 
                    }
#line 1704 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 25:
#line 316 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        /* TODO: */
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "-";
                        node->lexpr = (PNode)(yyvsp[0].node);
                        node->rexpr = NULL;
                        node->exprOpType = NEGATIVE;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr -> -  "); 
                    }
#line 1721 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 26:
#line 329 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "+";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = PLUS;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr + a_expr "); 
                    }
#line 1737 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 27:
#line 341 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "-";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = MINUS;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr - a_expr "); 
                    }
#line 1753 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 28:
#line 353 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "*";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = MULTIPLE;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr * a_expr "); 
                    }
#line 1769 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 29:
#line 365 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "/";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = DIVISIION;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr / a_expr "); 
                    }
#line 1785 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 30:
#line 377 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "%";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = MOD;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr % a_expr "); 
                    }
#line 1801 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 31:
#line 390 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = ">";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = GREATER;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr > a_expr "); 
                    }
#line 1817 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 32:
#line 402 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = LESS;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr < a_expr "); 
                    }
#line 1833 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 33:
#line 414 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "=";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = EQUAL;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr = a_expr "); 
                    }
#line 1849 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 34:
#line 426 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<=";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = LESS_EQUAL;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr <= a_expr "); 
                    }
#line 1865 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 35:
#line 438 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = ">=";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = GREATER_EQUAL;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr >= a_expr "); 
                    }
#line 1881 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 36:
#line 450 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<>";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = NOT_EQUAL;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr <> a_expr "); 
                    }
#line 1897 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 37:
#line 462 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[-2].node));
                        list = AppendNode(list, (yyvsp[0].node));

                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = AND_EXPR;
                        node->args = list;
                        
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr AND a_expr "); 
                    }
#line 1914 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 38:
#line 475 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[-2].node));
                        list = AppendNode(list, (yyvsp[0].node));

                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = OR_EXPR;
                        node->args = list;
                        
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr OR a_expr "); 
                    }
#line 1931 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 39:
#line 488 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[0].node));
                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = NOT_EXPR;
                        node->args = list;
                        
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->NOT a_expr "); 
                    }
#line 1946 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 40:
#line 500 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.node) = (yyvsp[0].node);
                }
#line 1954 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 41:
#line 504 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.node) = (yyvsp[0].node);
                }
#line 1962 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 42:
#line 508 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    /* TODO: 暂不支持 */
                    (yyval.node) = NULL;
                }
#line 1971 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 43:
#line 513 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.node) = (yyvsp[0].node);
                }
#line 1979 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 45:
#line 519 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                ;
            }
#line 1987 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 46:
#line 525 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                (yyval.sval) = (yyvsp[0].sval);
            }
#line 1995 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 47:
#line 529 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                (yyval.sval) = "*";
            }
#line 2003 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 48:
#line 535 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = (yyvsp[0].list);
                    hat_log("from clause  ");
                }
#line 2012 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 49:
#line 540 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2020 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 50:
#line 545 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PList list = CreateList((yyvsp[0].node));
                    (yyval.list) = list;

                    hat_log("from_list table_ref  ");
                }
#line 2031 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 51:
#line 552 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PList list = AppendNode((yyvsp[-2].list), (yyvsp[0].node));

                    (yyval.list) = list;
                }
#line 2041 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 52:
#line 559 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PRangeVar node = (PRangeVar)(yyvsp[-1].node);
                    node->alias = (PAlias)(yyvsp[0].node);
                    (yyval.node) = (PNode)node;
                }
#line 2051 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 53:
#line 566 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PRangeVar node = (PRangeVar)CreateNode(sizeof(RangeVar),T_RangeVar);
                    node->relname = (yyvsp[0].sval);
                    node->alias = NULL;
                    (yyval.node) = (PNode)node;
                    hat_log("from relation_expr %s  ", (yyvsp[0].sval));
                }
#line 2063 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 54:
#line 575 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (PNode)(yyvsp[0].node);
                    }
#line 2071 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 55:
#line 579 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = NULL;
                    }
#line 2079 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 56:
#line 584 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    /* TODO: 暂不支持  */
                }
#line 2087 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 57:
#line 588 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    /* TODO: 暂不支持  */
                }
#line 2095 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 58:
#line 592 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PAlias node = (PAlias)CreateNode(sizeof(Alias),T_Alias);
                    node->aliasname = (yyvsp[0].sval);
                    (yyval.node) = (PNode)node;
                    hat_log("alias_clause AS aliasname %s  ", (yyvsp[0].sval));
                }
#line 2106 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 59:
#line 599 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PAlias node = (PAlias)CreateNode(sizeof(Alias),T_Alias);
                    node->aliasname = (yyvsp[0].sval);
                    (yyval.node) = (PNode)node;
                    hat_log("alias_clause aliasname %s  ", (yyvsp[0].sval));
                }
#line 2117 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 60:
#line 607 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {

            }
#line 2125 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 61:
#line 612 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PList list = CreateList((yyvsp[0].node));
                    (yyval.list) = list;
                    hat_log("where clause ");
                }
#line 2135 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 62:
#line 618 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2143 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 63:
#line 623 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                    hat_log("group clause");
                }
#line 2152 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 64:
#line 628 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;                    
                }
#line 2160 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 65:
#line 633 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    hat_log("groupby_list "); 
                    (yyval.list) = NULL;
                }
#line 2169 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 66:
#line 638 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2177 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 67:
#line 643 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.node) = NULL;
                }
#line 2185 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 68:
#line 648 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                    hat_log("sort clause");
                }
#line 2194 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 69:
#line 653 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2202 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 70:
#line 658 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                    hat_log("limit clause");
                }
#line 2211 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 71:
#line 663 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2219 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 72:
#line 669 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PFunctionClause node = NewNode(FunctionClause);
                    node->functionName = (yyvsp[-2].sval);

                    (yyval.node) = (PNode)node;
                }
#line 2230 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 73:
#line 676 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PFunctionClause node = NewNode(FunctionClause);
                    node->functionName = (yyvsp[-3].sval);

                    (yyval.node) = (PNode)node;
                }
#line 2241 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 74:
#line 684 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                            PCreateStmt node = (PCreateStmt)CreateNode(sizeof(CreateStmt),T_CreateStmt);
                            node->tableName = (yyvsp[-3].sval);
                            node->ColList = (yyvsp[-1].list);

                            (yyval.node) = (PNode)node;
                        }
#line 2253 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 75:
#line 693 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PDropStmt node = (PDropStmt)CreateNode(sizeof(DropStmt),T_DropStmt);
                        node->tableName = (yyvsp[0].sval);

                        (yyval.node) = (PNode)node;
                    }
#line 2264 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 76:
#line 701 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                    PUpdateStmt node = NewNode(UpdateStmt);
                    node->relation = (yyvsp[-4].node);
                    node->targetlist = (yyvsp[-2].list);
                    node->fromList = (yyvsp[-1].list);
                    node->whereList = (yyvsp[0].list);
                    (yyval.node) = (PNode)node;
                }
#line 2277 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 77:
#line 711 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {                    
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 2285 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 78:
#line 716 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[0].node));
                        (yyval.list) = list;
                    }
#line 2294 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 79:
#line 721 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = AppendNode((yyvsp[-2].list), (yyvsp[0].node));
                        (yyval.list) = list;
                    }
#line 2303 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 80:
#line 727 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)(yyvsp[-2].node);
                        node->setValue = (yyvsp[0].node);
                        (yyval.node) = (yyvsp[-2].node);
                    }
#line 2313 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 81:
#line 733 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PListCell tmpCell = NULL;
                        PList l = (yyvsp[-3].list);

                        for(tmpCell = l->head; tmpCell != NULL; tmpCell = tmpCell->next)
                        {
                            PResTarget node = (PResTarget)GetCellNodeValue(tmpCell);
                            node->setValue = (yyvsp[0].node);
                        }

                        (yyval.node) = (PNode)(yyvsp[-3].list);
                    }
#line 2330 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 82:
#line 747 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = NewNode(ResTarget);
                        node->name = NULL;            /* without alias name */
                        node->indirection = NULL;
                        node->val = (yyvsp[0].node);
                        (yyval.node) = (PNode)node;
                    }
#line 2342 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 83:
#line 756 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[0].node));
                        (yyval.list) = list;
                    }
#line 2351 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 84:
#line 761 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = AppendNode((yyvsp[-2].list), (yyvsp[0].node));
                        (yyval.list) = list;
                    }
#line 2360 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 85:
#line 767 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {                        
                        PInsertStmt node = (PInsertStmt)CreateNode(sizeof(InsertStmt),T_InsertStmt);
                        node->tableName = (yyvsp[-3].sval);
                        node->attrNameList = (yyvsp[-2].list);
                        node->valuesList = (yyvsp[0].list);
                        
                        (yyval.node) = (PNode)node;
                        hat_log("insert stmt %s", (yyvsp[-3].sval));
                    }
#line 2374 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 86:
#line 778 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = NULL;
                    }
#line 2382 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 87:
#line 782 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = (yyvsp[-1].list);
                    }
#line 2390 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 88:
#line 787 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        if(NULL != (yyvsp[0].sval))
                        {
                            /* first node */
                            PList list = CreateCell(NULL);

                            PAttrName node = (PAttrName)CreateNode(sizeof(AttrName),T_AttrName);
                            node->attrName = (yyvsp[0].sval);

                            list->tail->value.pValue = node;
                            
                            (yyval.list) = list;
                        }
                        else
                            (yyval.list) = NULL;
                        hat_log("insert stmt attr_name:%s", (yyvsp[0].sval));
                    }
#line 2412 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 89:
#line 805 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        if((yyvsp[0].sval) != NULL)
                        {
                            PList list = CreateCell((yyvsp[-2].list));

                            PAttrName node = (PAttrName)CreateNode(sizeof(AttrName),T_AttrName);
                            node->attrName = (yyvsp[0].sval);

                            list->tail->value.pValue = node;
                            
                            (yyval.list) = list;
                        }
                        else
                            (yyval.list) = (yyvsp[-2].list);
                        hat_log("insert stmt multi attr_name:%s", (yyvsp[0].sval));
                    }
#line 2433 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 90:
#line 823 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        /* first node */
                        (yyval.list) = AppendNode(NULL, (PNode)(yyvsp[0].list));
                    }
#line 2442 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 91:
#line 828 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = AppendNode((yyvsp[-2].list), (PNode)(yyvsp[0].list));
                    }
#line 2450 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 92:
#line 833 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = (yyvsp[-1].list);
                    }
#line 2458 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 93:
#line 838 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        if(NULL != (yyvsp[0].node))
                        {
                            /* first node */
                            PList list = CreateCell(NULL);
                            list->tail->value.pValue = (yyvsp[0].node);                            

                            (yyval.list) = list;
                        }
                        else
                            (yyval.list) = NULL;
                        hat_log("insert stmt value_data");
                    }
#line 2476 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 94:
#line 852 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        if((yyvsp[0].node) != NULL)
                        {
                            PList list = CreateCell((yyvsp[-2].list));
                            list->tail->value.pValue = (yyvsp[0].node);
                            
                            (yyval.list) = list;
                        }
                        else
                            (yyval.list) = (yyvsp[-2].list);                    
                        hat_log("insert stmt multi value_data");
                    }
#line 2493 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 95:
#line 866 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PValuesData node = (PValuesData)CreateNode(sizeof(ValuesData),T_ValuesData);
                        node->valueNode = (PNode)(yyvsp[0].node);
                        (yyval.node) = (PNode)node;

                        hat_log("insert stmt value_data ");
                    }
#line 2505 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 96:
#line 875 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PDeleteStmt node = NewNode(DeleteStmt);
                        node->relation = (yyvsp[-2].node);
                        node->fromList = (yyvsp[-1].list);
                        node->whereList = (yyvsp[0].list);
                        (yyval.node) = (PNode)node;
                    }
#line 2517 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 97:
#line 884 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {                        
                        PTransactionStmt node = NewNode(TransactionStmt);
                        node->transactionTag = TF_BEGIN;

                        (yyval.node) = (PNode)node;
                    }
#line 2528 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 98:
#line 891 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PTransactionStmt node = NewNode(TransactionStmt);
                        node->transactionTag = TF_END;

                        (yyval.node) = (PNode)node;
                    }
#line 2539 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 99:
#line 898 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PTransactionStmt node = NewNode(TransactionStmt);
                        node->transactionTag = TF_ROLLBACK;

                        (yyval.node) = (PNode)node;
                    }
#line 2550 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 100:
#line 905 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PTransactionStmt node = NewNode(TransactionStmt);
                        node->transactionTag = TF_COMMIT;

                        (yyval.node) = (PNode)node;
                    }
#line 2561 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 101:
#line 912 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PTransactionStmt node = NewNode(TransactionStmt);
                        node->transactionTag = TF_SAVEPOINT;

                        (yyval.node) = (PNode)node;                        
                    }
#line 2572 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 102:
#line 920 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        if(NULL != (yyvsp[0].node))
                        {
                            /* first node */
                            PList list = CreateCell(NULL);
                            list->tail->value.pValue = (yyvsp[0].node);

                            (yyval.list) = list;
                        }
                        else
                            (yyval.list) = NULL;
                    }
#line 2589 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 103:
#line 933 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        if((yyvsp[0].node) != NULL)
                        {
                            PList list = CreateCell((yyvsp[-2].list));
                            list->tail->value.pValue = (yyvsp[0].node);
                            
                            (yyval.list) = list;
                        }
                        else
                            (yyval.list) = (yyvsp[-2].list);
                    }
#line 2605 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 104:
#line 946 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PColumnDef node = (PColumnDef)CreateNode(sizeof(ColumnDef),T_ColumnDef);
                        node->colName = (yyvsp[-1].sval);
                        node->colType = (yyvsp[0].sval);

                        (yyval.node) = (PNode)node;
                    }
#line 2617 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 105:
#line 955 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PColumnRef node = (PColumnRef)CreateNode(sizeof(ColumnRef),T_ColumnRef);
                        node->field = (yyvsp[0].sval);
                        node->tableName = NULL;
                        (yyval.node) = (PNode)node;
                        hat_log("columnRef attr_name :%s ", (yyvsp[0].sval)); 
                    }
#line 2629 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 106:
#line 963 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PColumnRef node = (PColumnRef)CreateNode(sizeof(ColumnRef),T_ColumnRef);
                        node->tableName = (yyvsp[-1].sval);
                        node->field = (yyvsp[0].sval);
                        (yyval.node) = (PNode)node;
                        hat_log("columnRef attr_name :%s.%s ", node->tableName, node->field); 
                    }
#line 2641 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 107:
#line 973 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 2649 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 108:
#line 979 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.pData = (yyvsp[0].sval);
                        node->vt = VT_VARCHAR;

                        (yyval.node) = (PNode)node;     
                        hat_log("exprConst string :%s ", (yyvsp[0].sval));                   
                    }
#line 2662 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 109:
#line 988 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.iData = (yyvsp[0].ival);
                        node->vt = VT_INT;

                        (yyval.node) = (PNode)node;    
                        hat_log("exprConst int :%d ", (yyvsp[0].ival));      
                    }
#line 2675 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 110:
#line 997 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.fData = (yyvsp[0].fval);
                        node->vt = VT_FLOAT;

                        (yyval.node) = (PNode)node;    
                        hat_log("exprConst float :%f ", (yyvsp[0].fval));      
                    }
#line 2688 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 111:
#line 1007 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2696 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1012 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2704 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1017 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2712 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1022 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2720 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1027 "sqlcore/parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2728 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
    break;


#line 2732 "sqlcore/parser/grammar.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (yyscaninfo, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yyscaninfo, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, yyscaninfo);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yyscaninfo);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (yyscaninfo, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, yyscaninfo);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yyscaninfo);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1031 "sqlcore/parser/grammar.y" /* yacc.c:1906  */


void yyerror(yyscan_t yyscaninfo, const char *msg)
{
        fprintf(stderr, "error: %s",msg);
}
