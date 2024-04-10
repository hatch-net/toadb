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
#line 15 "parser/grammar.y" /* yacc.c:339  */


#include <stdio.h>
#include <stdlib.h>

#line 23 "parser/grammar.y" /* yacc.c:339  */


#include "scanner_ext.h"
#include "scanner.h"
#include "parserNode.h"


#line 80 "parser/grammar.c" /* yacc.c:339  */

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
#line 35 "parser/grammar.y" /* yacc.c:355  */

    char *sval;
    int  ival;
    char op;
    float fval;
    PList list;
    PNode node;

#line 162 "parser/grammar.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (yyscan_t yyscaninfo);

#endif /* !YY_YY_PARSER_GRAMMAR_H_INCLUDED  */

/* Copy the second part of user declarations.  */
#line 106 "parser/grammar.y" /* yacc.c:358  */


#ifdef GRAMMAR_PARSER_LOG
#define hat_log printf
#else 
#define hat_log
#endif


#line 187 "parser/grammar.c" /* yacc.c:358  */

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
#define YYFINAL  42
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   229

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  50
/* YYNRULES -- Number of rules.  */
#define YYNRULES  103
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  174

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   287

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    39,     2,     2,
      41,    42,    37,    35,    45,    36,    43,    38,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
      32,    33,    31,     2,     2,     2,     2,     2,     2,     2,
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
      25,    26,    27,    28,    29,    30,    34,    40
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   118,   118,   125,   131,   145,   159,   164,   168,   172,
     176,   183,   195,   210,   216,   220,   231,   241,   251,   261,
     271,   285,   290,   302,   315,   327,   339,   351,   363,   376,
     388,   400,   412,   424,   436,   448,   461,   474,   486,   490,
     494,   500,   501,   507,   511,   517,   523,   527,   534,   541,
     548,   557,   562,   566,   570,   574,   581,   590,   594,   601,
     605,   611,   615,   620,   625,   630,   636,   640,   646,   651,
     660,   668,   678,   683,   688,   694,   700,   714,   723,   728,
     734,   746,   749,   754,   772,   790,   795,   800,   805,   819,
     833,   842,   855,   868,   877,   885,   895,   901,   910,   919,
     930,   934,   939,   944
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
  "IDENT", "STRING", "INTNUMBER", "FLOATNUMBER", "LESS_EQ", "GREATER_EQ",
  "NOT_EQ", "'>'", "'<'", "'='", "Op", "'+'", "'-'", "'*'", "'/'", "'%'",
  "UMINUS", "'('", "')'", "'.'", "';'", "','", "$accept", "top_stmt",
  "stmt_list", "stmt", "select_stmt", "select_clause", "target_opt",
  "target_list", "target_element", "a_expr", "c_expr", "opt_indirection",
  "indirection_element", "from_clause", "from_list", "table_ref",
  "relation_expr", "alias_clause_opt", "alias_clause", "name_list",
  "where_clause", "group_clause", "groupby_list", "groupby_element",
  "sort_clause", "limit_clause", "create_stmt", "drop_stmt", "update_stmt",
  "update_table_ref", "set_clause_list", "set_clause", "set_target",
  "set_target_list", "insert_stmt", "attr_name_list_opt", "attr_name_list",
  "multi_values_list", "values_opt", "values_list", "value_data",
  "columndef_list", "column_def", "columnRef", "exprConst", "constValues",
  "tablename", "attr_name", "attr_type", "aliasname", YY_NULLPTR
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
     285,    62,    60,    61,   286,    43,    45,    42,    47,    37,
     287,    40,    41,    46,    59,    44
};
# endif

#define YYPACT_NINF -143

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-143)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
       9,   143,     5,     6,    18,    -9,  -143,    16,    22,   -21,
    -143,    45,  -143,  -143,  -143,  -143,   164,  -143,  -143,  -143,
    -143,   164,   164,  -143,   164,    29,    23,  -143,    76,  -143,
    -143,  -143,  -143,    -7,    -9,    -9,    -9,  -143,  -143,    75,
      56,  -143,  -143,    28,  -143,    62,    90,   178,  -143,  -143,
      51,    -9,   104,   143,   164,   164,    96,  -143,   164,   164,
     164,   164,   164,   164,   164,   164,   164,   164,   164,  -143,
     -15,  -143,    80,  -143,    82,    96,  -143,  -143,    83,   -17,
    -143,   164,    99,  -143,  -143,    81,  -143,   164,   108,  -143,
     178,   126,  -143,   190,   190,   190,   190,   190,   190,    38,
      38,  -143,  -143,  -143,  -143,  -143,   111,   111,   117,   102,
    -143,   111,    13,  -143,   109,  -143,   101,  -143,    -7,    -9,
     101,   121,  -143,   -13,  -143,   122,    15,  -143,   106,  -143,
     103,  -143,    17,   -17,   104,   164,  -143,  -143,   164,  -143,
     111,  -143,  -143,  -143,   111,    69,   105,  -143,   110,  -143,
     115,   111,  -143,  -143,   101,   101,   127,  -143,  -143,  -143,
      19,  -143,  -143,   106,  -143,   164,  -143,   164,  -143,    69,
    -143,   101,  -143,  -143
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    14,     0,     0,     0,     0,     3,     0,     2,     0,
       6,    66,     9,    10,     8,     7,     0,   101,    97,    98,
      99,     0,     0,    20,     0,    46,    13,    15,    17,    21,
      38,    39,    96,    94,     0,     0,     0,   100,    72,    52,
       0,    50,     1,     0,     4,     0,    68,    37,    22,    23,
       0,     0,    59,     0,     0,     0,     0,   103,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    19,
       0,    95,     0,    70,    81,     0,    49,    51,    56,     0,
       5,     0,     0,    11,    41,    45,    47,     0,    61,    16,
      35,    36,    18,    32,    33,    34,    29,    30,    31,    24,
      25,    26,    27,    28,    44,    43,     0,     0,     0,    55,
      57,     0,    46,    73,     0,    77,    65,    67,    40,     0,
      58,     0,    12,     0,    91,     0,     0,    83,     0,    57,
       0,    78,     0,     0,    59,     0,    42,    48,     0,    69,
       0,   102,    93,    82,     0,     0,    80,    85,     0,    54,
       0,     0,    74,    71,    75,    64,    60,    62,    92,    84,
       0,    88,    90,     0,    53,     0,    79,     0,    87,     0,
      86,    76,    63,    89
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -143,  -143,  -143,   145,  -143,  -143,  -143,  -143,   113,   -16,
    -143,  -143,    53,    61,  -143,   -49,  -143,  -143,  -143,    46,
      40,  -143,  -143,    10,  -143,  -143,  -143,  -143,  -143,  -143,
    -143,    43,   -97,  -143,  -143,  -143,  -143,  -143,    20,  -143,
      24,  -143,    42,   -78,  -143,  -142,    67,   -66,  -143,   -19
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     7,     8,     9,    10,    11,    25,    26,    27,    28,
      29,   118,    71,    52,    85,    38,    39,    76,    77,   130,
      88,   122,   156,   157,    46,    83,    12,    13,    14,    40,
     112,   113,   114,   132,    15,   108,   126,   146,   147,   160,
     161,   123,   124,    30,    31,    32,    41,    33,   142,    69
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      47,   115,    86,   162,   105,    48,    49,    17,    50,    17,
      34,    35,     1,     2,   131,    37,    42,     3,     4,    51,
      78,     5,   104,    44,   111,     1,     2,   162,    36,   139,
       3,     4,   140,   115,     5,    51,    70,    92,    90,    91,
     125,   127,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,     6,   166,   115,   109,   143,   133,   150,
     144,   168,   151,    45,   169,   116,    54,    55,    53,    79,
     137,   120,    80,   115,   125,    66,    67,    68,   159,    58,
      59,    60,    61,    62,    63,    81,    64,    65,    66,    67,
      68,    54,    55,    84,    18,    19,    20,    75,    56,    57,
      57,    72,    73,    74,    58,    59,    60,    61,    62,    63,
      82,    64,    65,    66,    67,    68,    54,    55,    87,   154,
      57,   106,   155,   107,   110,   117,   119,   121,   128,    58,
      59,    60,    61,    62,    63,    17,    64,    65,    66,    67,
      68,    54,   135,   129,   138,   149,   141,   145,   165,   171,
     163,   155,   164,    43,    58,    59,    60,    61,    62,    63,
      16,    64,    65,    66,    67,    68,    89,    17,    18,    19,
      20,   136,   167,   134,   153,   148,   152,   172,    21,    22,
      23,    16,   158,   170,    24,     0,     0,     0,    17,    18,
      19,    20,     0,   173,     0,     0,     0,     0,     0,    21,
      22,     0,     0,     0,     0,    24,    58,    59,    60,    61,
      62,    63,     0,    64,    65,    66,    67,    68,    -1,    -1,
      -1,    -1,    -1,    -1,     0,    64,    65,    66,    67,    68
};

static const yytype_int16 yycheck[] =
{
      16,    79,    51,   145,    70,    21,    22,    24,    24,    24,
       5,     5,     3,     4,   111,    24,     0,     8,     9,     6,
      39,    12,    37,    44,    41,     3,     4,   169,    10,    42,
       8,     9,    45,   111,    12,     6,    43,    56,    54,    55,
     106,   107,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    44,   151,   133,    75,    42,    45,    42,
      45,    42,    45,    18,    45,    81,    15,    16,    45,    13,
     119,    87,    44,   151,   140,    37,    38,    39,   144,    28,
      29,    30,    31,    32,    33,    23,    35,    36,    37,    38,
      39,    15,    16,    42,    25,    26,    27,    22,    22,    24,
      24,    34,    35,    36,    28,    29,    30,    31,    32,    33,
      20,    35,    36,    37,    38,    39,    15,    16,    14,   135,
      24,    41,   138,    41,    41,    26,    45,    19,    11,    28,
      29,    30,    31,    32,    33,    24,    35,    36,    37,    38,
      39,    15,    33,    41,    23,    42,    24,    41,    33,   165,
      45,   167,    42,     8,    28,    29,    30,    31,    32,    33,
      17,    35,    36,    37,    38,    39,    53,    24,    25,    26,
      27,   118,    45,   112,   134,   129,   133,   167,    35,    36,
      37,    17,   140,   163,    41,    -1,    -1,    -1,    24,    25,
      26,    27,    -1,   169,    -1,    -1,    -1,    -1,    -1,    35,
      36,    -1,    -1,    -1,    -1,    41,    28,    29,    30,    31,
      32,    33,    -1,    35,    36,    37,    38,    39,    28,    29,
      30,    31,    32,    33,    -1,    35,    36,    37,    38,    39
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     8,     9,    12,    44,    47,    48,    49,
      50,    51,    72,    73,    74,    80,    17,    24,    25,    26,
      27,    35,    36,    37,    41,    52,    53,    54,    55,    56,
      89,    90,    91,    93,     5,     5,    10,    24,    61,    62,
      75,    92,     0,    49,    44,    18,    70,    55,    55,    55,
      55,     6,    59,    45,    15,    16,    22,    24,    28,    29,
      30,    31,    32,    33,    35,    36,    37,    38,    39,    95,
      43,    58,    92,    92,    92,    22,    63,    64,    95,    13,
      44,    23,    20,    71,    42,    60,    61,    14,    66,    54,
      55,    55,    95,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    37,    93,    41,    41,    81,    95,
      41,    41,    76,    77,    78,    89,    55,    26,    57,    45,
      55,    19,    67,    87,    88,    93,    82,    93,    11,    41,
      65,    78,    79,    45,    59,    33,    58,    61,    23,    42,
      45,    24,    94,    42,    45,    41,    83,    84,    65,    42,
      42,    45,    77,    66,    55,    55,    68,    69,    88,    93,
      85,    86,    91,    45,    42,    33,    78,    45,    42,    45,
      84,    55,    69,    86
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    46,    47,    48,    48,    48,    49,    49,    49,    49,
      49,    50,    51,    52,    52,    53,    53,    54,    54,    54,
      54,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    56,    56,
      56,    57,    57,    58,    58,    59,    59,    60,    60,    61,
      62,    63,    63,    64,    64,    64,    64,    65,    66,    66,
      67,    67,    68,    68,    69,    70,    70,    71,    71,    72,
      73,    74,    75,    76,    76,    77,    77,    78,    79,    79,
      80,    81,    81,    82,    82,    83,    83,    84,    85,    85,
      86,    87,    87,    88,    89,    89,    90,    91,    91,    91,
      92,    93,    94,    95
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     3,     1,     1,     1,     1,
       1,     3,     5,     1,     0,     1,     3,     1,     3,     2,
       1,     1,     2,     2,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     1,     1,
       4,     0,     2,     2,     2,     2,     0,     1,     3,     2,
       1,     1,     0,     5,     4,     2,     1,     0,     2,     0,
       3,     0,     1,     3,     1,     3,     0,     2,     0,     6,
       3,     6,     1,     1,     3,     3,     5,     1,     1,     3,
       6,     0,     3,     1,     3,     1,     3,     3,     1,     3,
       1,     1,     3,     2,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1
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
#line 119 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PSCANNER_DATA pExtData = (PSCANNER_DATA)yyget_extra(yyscaninfo);
                    pExtData->parserTree = (yyvsp[0].list); /* root of tree */
                    hat_log("top stmt \n");
                }
#line 1417 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 3:
#line 126 "parser/grammar.y" /* yacc.c:1646  */
    {
                    /* empty */
                    hat_log("null stmt \n");
                    (yyval.list) = NULL;
                }
#line 1427 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 4:
#line 132 "parser/grammar.y" /* yacc.c:1646  */
    {
                    if((yyvsp[-1].node) != NULL)
                    {
                        /* first node */
                        PList list = CreateCell(NULL);
                        list->tail->value.pValue = (yyvsp[-1].node);

                        (yyval.list) = list;
                        hat_log("stmt \n");
                    }
                    else
                        (yyval.list) = NULL;
                }
#line 1445 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 5:
#line 146 "parser/grammar.y" /* yacc.c:1646  */
    {
                    if((yyvsp[-1].node) != NULL)
                    {
                        PList list = CreateCell((yyvsp[-2].list));
                        list->tail->value.pValue = (yyvsp[-1].node);
                        
                        (yyval.list) = list;
                        hat_log("multi stmt \n");
                    }
                    else
                        (yyval.list) = (yyvsp[-2].list);
                }
#line 1462 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 6:
#line 160 "parser/grammar.y" /* yacc.c:1646  */
    {
                        hat_log("stmt select stmt\n");
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1471 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 7:
#line 165 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1479 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 8:
#line 169 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1487 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 9:
#line 173 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1495 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 10:
#line 177 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 1503 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 11:
#line 184 "parser/grammar.y" /* yacc.c:1646  */
    {                        
                        PSelectStmt node = (PSelectStmt)(yyvsp[-2].node);

                        node->sortList = (yyvsp[-1].list);
                        node->limitClause = (yyvsp[0].list);

                        (yyval.node) = (PNode)node;

                        hat_log("select_stmt \n");
                    }
#line 1518 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 12:
#line 196 "parser/grammar.y" /* yacc.c:1646  */
    {
                        /* 创建selectstmt 节点 */
                        PSelectStmt node = (PSelectStmt)CreateNode(sizeof(SelectStmt),T_SelectStmt);

                        node->targetlist = (yyvsp[-3].list);
                        node->fromList = (yyvsp[-2].list);
                        node->whereList = (yyvsp[-1].list);
                        node->groupList = (yyvsp[0].list);

                        (yyval.node) = (PNode)node;
                    
                        hat_log("select \n");
                    }
#line 1536 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 13:
#line 211 "parser/grammar.y" /* yacc.c:1646  */
    {
                       (yyval.list) = (yyvsp[0].list);  
                       hat_log("target list \n");
                    }
#line 1545 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 14:
#line 216 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = NULL;
                    }
#line 1553 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 15:
#line 221 "parser/grammar.y" /* yacc.c:1646  */
    {
                        /* create list, and create element, then add element to the list;  */
                        /* first node */
                        PList list = CreateCell(NULL);
                        list->tail->value.pValue = (yyvsp[0].node);
                            
                        (yyval.list) = list;

                        hat_log("target element \n");
                    }
#line 1568 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 16:
#line 232 "parser/grammar.y" /* yacc.c:1646  */
    {
                        /* create element, then add element to the list;  */
                        PList list = CreateCell((yyvsp[-2].list));
                        list->tail->value.pValue = (yyvsp[0].node);
                            
                        (yyval.list) = list;
                    }
#line 1580 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 17:
#line 242 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = NULL;
                        node->indirection = NULL;
                        node->val = (yyvsp[0].node);
                        (yyval.node) = (PNode)node;

                        hat_log("target_element a_expr\n");
                    }
#line 1594 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 18:
#line 252 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = (yyvsp[0].sval);  /* alias name */
                        node->indirection = NULL;
                        node->val = (yyvsp[-2].node);
                        (yyval.node) = (PNode)node;

                        hat_log("target_element a_expr as aliasname\n"); 
                    }
#line 1608 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 19:
#line 262 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = (yyvsp[0].sval);            /* alias name */
                        node->indirection = NULL;
                        node->val = (yyvsp[-1].node);
                        (yyval.node) = (PNode)node;

                        hat_log("target_element a_expr aliasname\n"); 
                    }
#line 1622 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 20:
#line 272 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)CreateNode(sizeof(ResTarget),T_ResTarget);
                        node->name = NULL;
                        node->indirection = NULL;
                        node->val = NULL;
                        node->all = 1;
                        (yyval.node) = (PNode)node;

                        hat_log("target_element * \n"); 
                    }
#line 1637 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 21:
#line 286 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                        hat_log("a_expr -> c_expr  \n"); 
                    }
#line 1646 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 22:
#line 291 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "+";
                        node->lexpr = (PNode)(yyvsp[0].node);
                        node->rexpr = NULL;
                        node->exprOpType = POSITIVE;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr -> +  \n"); 
                    }
#line 1662 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 23:
#line 303 "parser/grammar.y" /* yacc.c:1646  */
    {
                        /* TODO: */
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "-";
                        node->lexpr = (PNode)(yyvsp[0].node);
                        node->rexpr = NULL;
                        node->exprOpType = NEGATIVE;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr -> -  \n"); 
                    }
#line 1679 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 24:
#line 316 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "+";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = PLUS;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr + a_expr \n"); 
                    }
#line 1695 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 25:
#line 328 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "-";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = MINUS;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr - a_expr \n"); 
                    }
#line 1711 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 26:
#line 340 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "*";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = MULTIPLE;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr * a_expr \n"); 
                    }
#line 1727 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 27:
#line 352 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "/";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = DIVISIION;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr / a_expr \n"); 
                    }
#line 1743 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 28:
#line 364 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "%";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = MOD;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr % a_expr \n"); 
                    }
#line 1759 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 29:
#line 377 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = ">";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = GREATER;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr > a_expr \n"); 
                    }
#line 1775 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 30:
#line 389 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = LESS;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr < a_expr \n"); 
                    }
#line 1791 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 31:
#line 401 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "=";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = EQUAL;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr = a_expr \n"); 
                    }
#line 1807 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 32:
#line 413 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<=";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = LESS_EQUAL;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr <= a_expr \n"); 
                    }
#line 1823 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 33:
#line 425 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = ">=";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = GREATER_EQUAL;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr >= a_expr \n"); 
                    }
#line 1839 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 34:
#line 437 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PA_Expr node = (PA_Expr)CreateNode(sizeof(A_Expr),T_A_Expr);
                        
                        node->exprType = AEXPR_OP;
                        node->name = "<>";
                        node->lexpr = (PNode)(yyvsp[-2].node);
                        node->rexpr = (PNode)(yyvsp[0].node);
                        node->exprOpType = NOT_EQUAL;
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr <> a_expr \n"); 
                    }
#line 1855 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 35:
#line 449 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[-2].node));
                        list = AppendNode(list, (yyvsp[0].node));

                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = AND_EXPR;
                        node->args = list;
                        
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr AND a_expr \n"); 
                    }
#line 1872 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 36:
#line 462 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[-2].node));
                        list = AppendNode(list, (yyvsp[0].node));

                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = OR_EXPR;
                        node->args = list;
                        
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->a_expr OR a_expr \n"); 
                    }
#line 1889 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 37:
#line 475 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[0].node));
                        PBoolExpr node = (PBoolExpr)CreateNode(sizeof(BoolExpr),T_BoolExpr);
                        
                        node->boolop = NOT_EXPR;
                        node->args = list;
                        
                        (yyval.node) = (PNode)node;
                        hat_log("a_expr ->NOT a_expr \n"); 
                    }
#line 1904 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 38:
#line 487 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.node) = (yyvsp[0].node);
                }
#line 1912 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 39:
#line 491 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.node) = (yyvsp[0].node);
                }
#line 1920 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 40:
#line 495 "parser/grammar.y" /* yacc.c:1646  */
    {
                    /* TODO: 暂不支持 */
                    (yyval.node) = NULL;
                }
#line 1929 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 42:
#line 502 "parser/grammar.y" /* yacc.c:1646  */
    {
                ;
            }
#line 1937 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 43:
#line 508 "parser/grammar.y" /* yacc.c:1646  */
    {
                (yyval.sval) = (yyvsp[0].sval);
            }
#line 1945 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 44:
#line 512 "parser/grammar.y" /* yacc.c:1646  */
    {
                (yyval.sval) = "*";
            }
#line 1953 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 45:
#line 518 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = (yyvsp[0].list);
                    hat_log("from clause  \n");
                }
#line 1962 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 46:
#line 523 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 1970 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 47:
#line 528 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PList list = CreateList((yyvsp[0].node));
                    (yyval.list) = list;

                    hat_log("from_list table_ref  \n");
                }
#line 1981 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 48:
#line 535 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PList list = AppendNode((yyvsp[-2].list), (yyvsp[0].node));

                    (yyval.list) = list;
                }
#line 1991 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 49:
#line 542 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PRangeVar node = (PRangeVar)(yyvsp[-1].node);
                    node->alias = (PAlias)(yyvsp[0].node);
                    (yyval.node) = (PNode)node;
                }
#line 2001 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 50:
#line 549 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PRangeVar node = (PRangeVar)CreateNode(sizeof(RangeVar),T_RangeVar);
                    node->relname = (yyvsp[0].sval);
                    node->alias = NULL;
                    (yyval.node) = (PNode)node;
                    hat_log("from relation_expr %s  \n", (yyvsp[0].sval));
                }
#line 2013 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 51:
#line 558 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (PNode)(yyvsp[0].node);
                    }
#line 2021 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 52:
#line 562 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = NULL;
                    }
#line 2029 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 53:
#line 567 "parser/grammar.y" /* yacc.c:1646  */
    {
                    /* TODO: 暂不支持  */
                }
#line 2037 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 54:
#line 571 "parser/grammar.y" /* yacc.c:1646  */
    {
                    /* TODO: 暂不支持  */
                }
#line 2045 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 55:
#line 575 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PAlias node = (PAlias)CreateNode(sizeof(Alias),T_Alias);
                    node->aliasname = (yyvsp[0].sval);
                    (yyval.node) = (PNode)node;
                    hat_log("alias_clause AS aliasname %s  \n", (yyvsp[0].sval));
                }
#line 2056 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 56:
#line 582 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PAlias node = (PAlias)CreateNode(sizeof(Alias),T_Alias);
                    node->aliasname = (yyvsp[0].sval);
                    (yyval.node) = (PNode)node;
                    hat_log("alias_clause aliasname %s  \n", (yyvsp[0].sval));
                }
#line 2067 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 57:
#line 590 "parser/grammar.y" /* yacc.c:1646  */
    {

            }
#line 2075 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 58:
#line 595 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PList list = CreateList((yyvsp[0].node));
                    (yyval.list) = list;
                    hat_log("where clause \n");
                }
#line 2085 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 59:
#line 601 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2093 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 60:
#line 606 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                    hat_log("group clause\n");
                }
#line 2102 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 61:
#line 611 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;                    
                }
#line 2110 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 62:
#line 616 "parser/grammar.y" /* yacc.c:1646  */
    {
                    hat_log("groupby_list \n"); 
                    (yyval.list) = NULL;
                }
#line 2119 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 63:
#line 621 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2127 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 64:
#line 626 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.node) = NULL;
                }
#line 2135 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 65:
#line 631 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                    hat_log("sort clause\n");
                }
#line 2144 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 66:
#line 636 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2152 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 67:
#line 641 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                    hat_log("limit clause\n");
                }
#line 2161 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 68:
#line 646 "parser/grammar.y" /* yacc.c:1646  */
    {
                    (yyval.list) = NULL;
                }
#line 2169 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 69:
#line 652 "parser/grammar.y" /* yacc.c:1646  */
    {
                            PCreateStmt node = (PCreateStmt)CreateNode(sizeof(CreateStmt),T_CreateStmt);
                            node->tableName = (yyvsp[-3].sval);
                            node->ColList = (yyvsp[-1].list);

                            (yyval.node) = (PNode)node;
                        }
#line 2181 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 70:
#line 661 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PDropStmt node = (PDropStmt)CreateNode(sizeof(DropStmt),T_DropStmt);
                        node->tableName = (yyvsp[0].sval);

                        (yyval.node) = (PNode)node;
                    }
#line 2192 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 71:
#line 669 "parser/grammar.y" /* yacc.c:1646  */
    {
                    PUpdateStmt node = NewNode(UpdateStmt);
                    node->relation = (yyvsp[-4].node);
                    node->targetlist = (yyvsp[-2].list);
                    node->fromList = (yyvsp[-1].list);
                    node->whereList = (yyvsp[0].list);
                    (yyval.node) = (PNode)node;
                }
#line 2205 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 72:
#line 679 "parser/grammar.y" /* yacc.c:1646  */
    {                    
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 2213 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 73:
#line 684 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[0].node));
                        (yyval.list) = list;
                    }
#line 2222 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 74:
#line 689 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = AppendNode((yyvsp[-2].list), (yyvsp[0].node));
                        (yyval.list) = list;
                    }
#line 2231 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 75:
#line 695 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = (PResTarget)(yyvsp[-2].node);
                        node->setValue = (yyvsp[0].node);
                        (yyval.node) = (yyvsp[-2].node);
                    }
#line 2241 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 76:
#line 701 "parser/grammar.y" /* yacc.c:1646  */
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
#line 2258 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 77:
#line 715 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PResTarget node = NewNode(ResTarget);
                        node->name = NULL;            /* without alias name */
                        node->indirection = NULL;
                        node->val = (yyvsp[0].node);
                        (yyval.node) = (PNode)node;
                    }
#line 2270 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 78:
#line 724 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = CreateList((yyvsp[0].node));
                        (yyval.list) = list;
                    }
#line 2279 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 79:
#line 729 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PList list = AppendNode((yyvsp[-2].list), (yyvsp[0].node));
                        (yyval.list) = list;
                    }
#line 2288 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 80:
#line 735 "parser/grammar.y" /* yacc.c:1646  */
    {                        
                        PInsertStmt node = (PInsertStmt)CreateNode(sizeof(InsertStmt),T_InsertStmt);
                        node->tableName = (yyvsp[-3].sval);
                        node->attrNameList = (yyvsp[-2].list);
                        node->valuesList = (yyvsp[0].list);
                        
                        (yyval.node) = (PNode)node;
                        hat_log("insert stmt %s\n", (yyvsp[-3].sval));
                    }
#line 2302 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 81:
#line 746 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = NULL;
                    }
#line 2310 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 82:
#line 750 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = (yyvsp[-1].list);
                    }
#line 2318 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 83:
#line 755 "parser/grammar.y" /* yacc.c:1646  */
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
                        hat_log("insert stmt attr_name:%s\n", (yyvsp[0].sval));
                    }
#line 2340 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 84:
#line 773 "parser/grammar.y" /* yacc.c:1646  */
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
                        hat_log("insert stmt multi attr_name:%s\n", (yyvsp[0].sval));
                    }
#line 2361 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 85:
#line 791 "parser/grammar.y" /* yacc.c:1646  */
    {
                        /* first node */
                        (yyval.list) = AppendNode(NULL, (PNode)(yyvsp[0].list));
                    }
#line 2370 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 86:
#line 796 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = AppendNode((yyvsp[-2].list), (PNode)(yyvsp[0].list));
                    }
#line 2378 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 87:
#line 801 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.list) = (yyvsp[-1].list);
                    }
#line 2386 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 88:
#line 806 "parser/grammar.y" /* yacc.c:1646  */
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
                        hat_log("insert stmt value_data\n");
                    }
#line 2404 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 89:
#line 820 "parser/grammar.y" /* yacc.c:1646  */
    {
                        if((yyvsp[0].node) != NULL)
                        {
                            PList list = CreateCell((yyvsp[-2].list));
                            list->tail->value.pValue = (yyvsp[0].node);
                            
                            (yyval.list) = list;
                        }
                        else
                            (yyval.list) = (yyvsp[-2].list);                    
                        hat_log("insert stmt multi value_data\n");
                    }
#line 2421 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 90:
#line 834 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PValuesData node = (PValuesData)CreateNode(sizeof(ValuesData),T_ValuesData);
                        node->valueNode = (PNode)(yyvsp[0].node);
                        (yyval.node) = (PNode)node;

                        hat_log("insert stmt value_data \n");
                    }
#line 2433 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 91:
#line 843 "parser/grammar.y" /* yacc.c:1646  */
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
#line 2450 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 92:
#line 856 "parser/grammar.y" /* yacc.c:1646  */
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
#line 2466 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 93:
#line 869 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PColumnDef node = (PColumnDef)CreateNode(sizeof(ColumnDef),T_ColumnDef);
                        node->colName = (yyvsp[-1].sval);
                        node->colType = (yyvsp[0].sval);

                        (yyval.node) = (PNode)node;
                    }
#line 2478 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 94:
#line 878 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PColumnRef node = (PColumnRef)CreateNode(sizeof(ColumnRef),T_ColumnRef);
                        node->field = (yyvsp[0].sval);
                        node->tableName = NULL;
                        (yyval.node) = (PNode)node;
                        hat_log("columnRef attr_name :%s \n", (yyvsp[0].sval)); 
                    }
#line 2490 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 95:
#line 886 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PColumnRef node = (PColumnRef)CreateNode(sizeof(ColumnRef),T_ColumnRef);
                        node->tableName = (yyvsp[-1].sval);
                        node->field = (yyvsp[0].sval);
                        (yyval.node) = (PNode)node;
                        hat_log("columnRef attr_name :%s.%s \n", node->tableName, node->field); 
                    }
#line 2502 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 96:
#line 896 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.node) = (yyvsp[0].node);
                    }
#line 2510 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 97:
#line 902 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.pData = (yyvsp[0].sval);
                        node->vt = VT_VARCHAR;

                        (yyval.node) = (PNode)node;     
                        hat_log("exprConst string :%s \n", (yyvsp[0].sval));                   
                    }
#line 2523 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 98:
#line 911 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.iData = (yyvsp[0].ival);
                        node->vt = VT_INT;

                        (yyval.node) = (PNode)node;    
                        hat_log("exprConst int :%d \n", (yyvsp[0].ival));      
                    }
#line 2536 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 99:
#line 920 "parser/grammar.y" /* yacc.c:1646  */
    {
                        PConstValue node = (PConstValue)CreateNode(sizeof(ConstValue),T_ConstValue);
                        node->val.fData = (yyvsp[0].fval);
                        node->vt = VT_FLOAT;

                        (yyval.node) = (PNode)node;    
                        hat_log("exprConst float :%f \n", (yyvsp[0].fval));      
                    }
#line 2549 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 100:
#line 931 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2557 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 101:
#line 935 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2565 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 102:
#line 940 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2573 "parser/grammar.c" /* yacc.c:1646  */
    break;

  case 103:
#line 945 "parser/grammar.y" /* yacc.c:1646  */
    {
                        (yyval.sval) = (yyvsp[0].sval);
                    }
#line 2581 "parser/grammar.c" /* yacc.c:1646  */
    break;


#line 2585 "parser/grammar.c" /* yacc.c:1646  */
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
#line 949 "parser/grammar.y" /* yacc.c:1906  */


void yyerror(yyscan_t yyscaninfo, const char *msg)
{
        fprintf(stderr, "error: %s\n",msg);
}
