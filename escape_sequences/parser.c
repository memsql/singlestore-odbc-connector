/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.3.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ma_odbc.h"
#include "escape_sequences/ast.h"
void yyerror (void *scanner, MADB_Error *error, MADB_DynString *res, char *s)
{
	if (strcmp(s, "syntax error") == 0)
	{
		MADB_SetError(error,  MADB_ERR_42000, "Incorrect syntax for the escape sequence", 0);
	} else
	{
		MADB_SetError(error,  MADB_ERR_42000, s, 0);
	}
}

// supportedForConversionDataTypes should be synced with appropriateSingleStoreDataTypes
//
const char *supportedForConversionDataTypes[] =
{
	"SQL_BIGINT",
	"SQL_BINARY",
	"SQL_BIT",
	"SQL_CHAR",
	"SQL_DATE",
	"SQL_DECIMAL",
	"SQL_DOUBLE",
	"SQL_FLOAT",
	"SQL_INTEGER",
	"SQL_LONGVARBINARY",
	"SQL_LONGVARCHAR",
	"SQL_NUMERIC",
	"SQL_REAL",
	"SQL_SMALLINT",
	"SQL_TIME",
	"SQL_TIMESTAMP",
	"SQL_TINYINT",
	"SQL_VARBINARY",
	"SQL_VARCHAR",
	"SQL_WCHAR",
	"SQL_WLONGVARCHAR",
	"SQL_WVARCHAR"
};

// appropriateSingleStoreDataTypes shoud be synced with supportedForConversionDataTypes
//
const char *appropriateSingleStoreDataTypes[] =
{
	"BIGINT",
	"BINARY",
	"BIT",
	"CHAR",
	"DATE",
	"DECIMAL",
	"DOUBLE",
	"FLOAT",
	"INTEGER",
	"LONGBLOB",
	"LONGTEXT",
	"NUMERIC",
	"DOUBLE",
	"SMALLINT",
	"TIME",
	"TIMESTAMP(6)",
	"TINYINT",
	"BLOB",
	"TEXT",
	"CHAR",
	"LONGTEXT",
	"TEXT"
};

const int supportedForConversionDataTypesSize = sizeof supportedForConversionDataTypes / sizeof supportedForConversionDataTypes[0];

// odbcIntervals should be synced with appropriateSingleStoreIntervals
//
const char *odbcIntervals[] =
{
	"SQL_TSI_FRAC_SECOND",
	"SQL_TSI_SECOND",
	"SQL_TSI_MINUTE",
	"SQL_TSI_HOUR",
	"SQL_TSI_DAY",
	"SQL_TSI_WEEK",
	"SQL_TSI_MONTH",
	"SQL_TSI_QUARTER",
	"SQL_TSI_YEAR"
};

// appropriateSingleStoreIntervals should be synced with odbcIntervals
//
const char *appropriateSingleStoreIntervals[] =
{
	"MICROSECOND",
	"SECOND",
	"MINUTE",
	"HOUR",
	"DAY",
	"WEEK",
	"MONTH",
	"QUARTER",
	"YEAR"
};

const int odbcIntervalsSize = sizeof odbcIntervals / sizeof odbcIntervals[0];

char *matchValue(const char **inputVar, const char **outputVar, size_t length, char *conversion)
{
	int correctMatchedIndex = -1;
	int i;
	for (i = 0; i < length; i++)
	{
		if (strcasecmp(conversion, inputVar[i]) == 0)
		{
			correctMatchedIndex = i;
		}
	}

	// Nothing matched
	//
	if (correctMatchedIndex == -1)
	{
		return conversion;
	}

	return outputVar[correctMatchedIndex];
}

char *getSingleStoreInterval(char *interval)
{
	return matchValue(odbcIntervals, appropriateSingleStoreIntervals, odbcIntervalsSize, interval);
}

char *getSingleStoreDataType(char *dataType)
{
	return matchValue(supportedForConversionDataTypes,
		appropriateSingleStoreDataTypes,
		supportedForConversionDataTypesSize,
		dataType);
}

void setRes(MADB_DynString *res, struct ASTNode *x)
{
	MADB_InitDynamicString(res, "", x->totalLength, 256);
	appendToString(res, x);
}

// allocateNode creates a new ASTNode using newNode
// If some error occurs during the memory allocation
// or at least one child node is null,
// it will free all children and return NULL
//
struct ASTNode* allocateNode(int childrenCount, ...);

#define YYALOCATION_ERROR do \
{ \
	yyerror(scanner, error, res, "Failed to allocate memory for escape sequence parsing");\
	YYABORT;\
} while(0)



# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
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
   by #include "parser.h".  */
#ifndef YY_YY_HOME_AMAKAROVYCH_UA_SINGLESTORE_ODBC_CONNECTOR_ESCAPE_SEQUENCES_PARSER_H_INCLUDED
# define YY_YY_HOME_AMAKAROVYCH_UA_SINGLESTORE_ODBC_CONNECTOR_ESCAPE_SEQUENCES_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */


#include "ma_odbc.h"
#include "ma_legacy_helpers.h"



/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    DATE_ESCAPE = 258,
    TIME_ESCAPE = 259,
    TIMESTAMP_ESCAPE = 260,
    OUTER_JOIN_ESCAPE = 261,
    FN_ESCAPE = 262,
    CALL_ESCAPE = 263,
    BIT_LENGTH = 264,
    INSERT = 265,
    TIMESTAMPADD = 266,
    TIMESTAMPDIFF = 267,
    CONVERT = 268,
    SPACE = 269,
    REPEAT = 270,
    ALLOCATION_ERROR = 271,
    OPENING_ROUND_BRACKET = 272,
    CLOSING_ROUND_BRACKET = 273,
    COMMA = 274,
    STRING_LITERAL = 275,
    LEXEME = 276
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{


	char *string;
	struct ASTNode *ast;


};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (void *scanner, MADB_Error *error, MADB_DynString *res);

#endif /* !YY_YY_HOME_AMAKAROVYCH_UA_SINGLESTORE_ODBC_CONNECTOR_ESCAPE_SEQUENCES_PARSER_H_INCLUDED  */



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
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
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
#  define YYSIZE_T unsigned
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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
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
#define YYFINAL  28
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   263

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  22
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  5
/* YYNRULES -- Number of rules.  */
#define YYNRULES  25
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  77

#define YYUNDEFTOK  2
#define YYMAXUTOK   276

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   231,   231,   247,   261,   274,   287,   297,   302,   315,
     323,   330,   352,   390,   430,   440,   455,   462,   477,   489,
     493,   504,   510,   516,   523,   527
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DATE_ESCAPE", "TIME_ESCAPE",
  "TIMESTAMP_ESCAPE", "OUTER_JOIN_ESCAPE", "FN_ESCAPE", "CALL_ESCAPE",
  "BIT_LENGTH", "INSERT", "TIMESTAMPADD", "TIMESTAMPDIFF", "CONVERT",
  "SPACE", "REPEAT", "ALLOCATION_ERROR", "OPENING_ROUND_BRACKET",
  "CLOSING_ROUND_BRACKET", "COMMA", "STRING_LITERAL", "LEXEME", "$accept",
  "res", "function_call", "lexemes", "lexeme", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276
};
# endif

#define YYPACT_NINF -13

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-13)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      51,   -12,    -9,    -5,     7,   229,   242,    23,   -13,   -13,
     -13,    15,    25,    28,    48,    49,    50,    59,     7,   -13,
      62,   -13,    70,     7,    62,   -13,    62,   -13,   -13,     7,
       7,    81,    83,     7,     7,     7,    -8,    26,     7,     7,
      60,    73,    72,    96,    86,    99,   112,   -13,   -13,    33,
       7,   -13,     7,     7,     7,    97,   -13,     7,   -13,   125,
     138,   151,   110,   164,     7,     7,     7,   -13,   -13,   177,
     190,   203,     7,   -13,   -13,   216,   -13
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     2,     3,
       4,     0,     0,     0,     0,     0,     0,     0,     0,    21,
      22,    24,     7,    19,     8,     9,     5,     6,     1,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    23,
       0,     0,     0,     0,     0,     0,     0,    25,    18,     0,
      20,    10,     0,     0,     0,     0,    16,     0,    17,     0,
       0,     0,     0,     0,     0,     0,     0,    14,    15,     0,
       0,     0,     0,    12,    13,     0,    11
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -13,   -13,    58,    93,    -4
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     7,    21,    22,    39
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      23,    11,    12,    13,    14,    15,    16,    17,     8,    18,
      47,     9,    19,    20,    36,    10,    11,    12,    13,    14,
      15,    16,    17,    28,    18,    40,    41,    19,    20,    44,
      45,    46,    29,    23,    50,    11,    12,    13,    14,    15,
      16,    17,    30,    18,    48,    31,    19,    20,    59,    60,
      61,    58,    38,    63,     1,     2,     3,     4,     5,     6,
      69,    70,    71,    25,    27,    32,    33,    34,    75,    11,
      12,    13,    14,    15,    16,    17,    35,    18,    51,    37,
      19,    20,    11,    12,    13,    14,    15,    16,    17,    38,
      18,    53,    52,    19,    20,    11,    12,    13,    14,    15,
      16,    17,    42,    18,    43,    55,    19,    20,    11,    12,
      13,    14,    15,    16,    17,    54,    18,    56,    62,    19,
      20,    11,    12,    13,    14,    15,    16,    17,    67,    18,
      49,    57,    19,    20,    11,    12,    13,    14,    15,    16,
      17,     0,    18,     0,    64,    19,    20,    11,    12,    13,
      14,    15,    16,    17,     0,    18,     0,    65,    19,    20,
      11,    12,    13,    14,    15,    16,    17,     0,    18,     0,
      66,    19,    20,    11,    12,    13,    14,    15,    16,    17,
       0,    18,    68,     0,    19,    20,    11,    12,    13,    14,
      15,    16,    17,     0,    18,     0,    72,    19,    20,    11,
      12,    13,    14,    15,    16,    17,     0,    18,    73,     0,
      19,    20,    11,    12,    13,    14,    15,    16,    17,     0,
      18,    74,     0,    19,    20,    11,    12,    13,    14,    15,
      16,    17,     0,    18,    76,     0,    19,    20,    11,    12,
      13,    14,    15,    16,    17,     0,     0,     0,     0,     0,
      24,    11,    12,    13,    14,    15,    16,    17,     0,     0,
       0,     0,     0,    26
};

static const yytype_int8 yycheck[] =
{
       4,     9,    10,    11,    12,    13,    14,    15,    20,    17,
      18,    20,    20,    21,    18,    20,     9,    10,    11,    12,
      13,    14,    15,     0,    17,    29,    30,    20,    21,    33,
      34,    35,    17,    37,    38,     9,    10,    11,    12,    13,
      14,    15,    17,    17,    18,    17,    20,    21,    52,    53,
      54,    18,    19,    57,     3,     4,     5,     6,     7,     8,
      64,    65,    66,     5,     6,    17,    17,    17,    72,     9,
      10,    11,    12,    13,    14,    15,    17,    17,    18,    17,
      20,    21,     9,    10,    11,    12,    13,    14,    15,    19,
      17,    19,    19,    20,    21,     9,    10,    11,    12,    13,
      14,    15,    21,    17,    21,    19,    20,    21,     9,    10,
      11,    12,    13,    14,    15,    19,    17,    18,    21,    20,
      21,     9,    10,    11,    12,    13,    14,    15,    18,    17,
      37,    19,    20,    21,     9,    10,    11,    12,    13,    14,
      15,    -1,    17,    -1,    19,    20,    21,     9,    10,    11,
      12,    13,    14,    15,    -1,    17,    -1,    19,    20,    21,
       9,    10,    11,    12,    13,    14,    15,    -1,    17,    -1,
      19,    20,    21,     9,    10,    11,    12,    13,    14,    15,
      -1,    17,    18,    -1,    20,    21,     9,    10,    11,    12,
      13,    14,    15,    -1,    17,    -1,    19,    20,    21,     9,
      10,    11,    12,    13,    14,    15,    -1,    17,    18,    -1,
      20,    21,     9,    10,    11,    12,    13,    14,    15,    -1,
      17,    18,    -1,    20,    21,     9,    10,    11,    12,    13,
      14,    15,    -1,    17,    18,    -1,    20,    21,     9,    10,
      11,    12,    13,    14,    15,    -1,    -1,    -1,    -1,    -1,
      21,     9,    10,    11,    12,    13,    14,    15,    -1,    -1,
      -1,    -1,    -1,    21
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    23,    20,    20,
      20,     9,    10,    11,    12,    13,    14,    15,    17,    20,
      21,    24,    25,    26,    21,    24,    21,    24,     0,    17,
      17,    17,    17,    17,    17,    17,    26,    17,    19,    26,
      26,    26,    21,    21,    26,    26,    26,    18,    18,    25,
      26,    18,    19,    19,    19,    19,    18,    19,    18,    26,
      26,    26,    21,    26,    19,    19,    19,    18,    18,    26,
      26,    26,    19,    18,    18,    26,    18
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    22,    23,    23,    23,    23,    23,    23,    23,    23,
      24,    24,    24,    24,    24,    24,    24,    24,    24,    25,
      25,    26,    26,    26,    26,    26
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       4,    10,     8,     8,     6,     6,     4,     4,     3,     1,
       3,     1,     1,     2,     1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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
        yyerror (scanner, error, res, YY_("syntax error: cannot back up")); \
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
                  Type, Value, scanner, error, res); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, void *scanner, MADB_Error *error, MADB_DynString *res)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (scanner);
  YYUSE (error);
  YYUSE (res);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, void *scanner, MADB_Error *error, MADB_DynString *res)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep, scanner, error, res);
  YYFPRINTF (yyo, ")");
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, void *scanner, MADB_Error *error, MADB_DynString *res)
{
  unsigned long yylno = yyrline[yyrule];
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
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              , scanner, error, res);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner, error, res); \
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
            else
              goto append;

          append:
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

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
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
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
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
    default: /* Avoid compiler warnings. */
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
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *scanner, MADB_Error *error, MADB_DynString *res)
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  YYUSE (error);
  YYUSE (res);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
    case 20: /* STRING_LITERAL  */

      { free (((*yyvaluep).string)); }

        break;

    case 21: /* LEXEME  */

      { free (((*yyvaluep).string)); }

        break;

    case 24: /* function_call  */

      { freeTree (((*yyvaluep).ast)); }

        break;

    case 25: /* lexemes  */

      { freeTree (((*yyvaluep).ast)); }

        break;

    case 26: /* lexeme  */

      { freeTree (((*yyvaluep).ast)); }

        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *scanner, MADB_Error *error, MADB_DynString *res)
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
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
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
# else /* defined YYSTACK_RELOCATE */
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
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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
      yychar = yylex (&yylval, scanner, error);
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
| yyreduce -- do a reduction.  |
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

    {
		struct ASTNode *stringLiteral = newLeaf((yyvsp[0].string));
		free((yyvsp[0].string));

		// "($2 :> DATE)"
		//
		struct ASTNode *x = allocateNode(3, newLeaf("("), stringLiteral, newLeaf(" :> DATE)"));
		if (x == NULL)
		{
			YYALOCATION_ERROR;
		}

		setRes(res, x);
		freeTree(x);
	}

    break;

  case 3:

    {
		struct ASTNode *stringLiteral = newLeaf((yyvsp[0].string));
		free((yyvsp[0].string));

		// "($2 :> TIME)"
		//
		struct ASTNode *x = allocateNode(3, newLeaf("("), stringLiteral, newLeaf(" :> TIME)"));
		if (x == NULL)
		{ YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}

    break;

  case 4:

    {
		struct ASTNode *stringLiteral = newLeaf((yyvsp[0].string));
		free((yyvsp[0].string));

		// "($2 :> TIMESTAMP(6))"
		//
		struct ASTNode *x = allocateNode(3, newLeaf("("), stringLiteral, newLeaf(" :> TIMESTAMP(6))"));
		if (x == NULL) { YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}

    break;

  case 5:

    {
		struct ASTNode *lexeme = newLeaf((yyvsp[0].string));
		free((yyvsp[0].string));

		// "CALL $2()"
		//
		struct ASTNode *x = allocateNode(3, newLeaf("CALL "), lexeme, newLeaf("()"));
		if (x == NULL) { YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}

    break;

  case 6:

    {
		// "CALL $2"
		//
		struct ASTNode *x = allocateNode(2, newLeaf("CALL "), (yyvsp[0].ast));
		if (x == NULL) { YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}

    break;

  case 7:

    {
		setRes(res, (yyvsp[0].ast));
		freeTree((yyvsp[0].ast));
	}

    break;

  case 8:

    {
		struct ASTNode *lexeme = newLeaf((yyvsp[0].string));
		free((yyvsp[0].string));

		// "$2()"
		//
		struct ASTNode *x = allocateNode(2, lexeme, newLeaf("()"));
		if (x == NULL) { YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}

    break;

  case 9:

    {
		setRes(res, (yyvsp[0].ast));
		freeTree((yyvsp[0].ast));
	}

    break;

  case 10:

    {
		// "LENGTH($3)*8"
		//
		(yyval.ast) = allocateNode(3, newLeaf("LENGTH("), (yyvsp[-1].ast), newLeaf(")*8"));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 11:

    {
		// CONCAT(CONCAT(LEFT($3, $5 - 1), $9), RIGHT($3, CHAR_LENGTH($3) - $7 - $5 + 1))
		//
		(yyval.ast) = allocateNode(15,
			newLeaf("CONCAT(CONCAT(LEFT("),
			(yyvsp[-7].ast),
			newLeaf(", "),
			(yyvsp[-5].ast),
			newLeaf(" - 1), "),
			(yyvsp[-1].ast),
			newLeaf("), RIGHT("),
			copyTree((yyvsp[-7].ast)),
			newLeaf(", CHAR_LENGTH("),
			copyTree((yyvsp[-7].ast)),
			newLeaf(") - "),
			(yyvsp[-3].ast),
			newLeaf(" - "),
			copyTree((yyvsp[-5].ast)),
			newLeaf(" + 1))"));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 12:

    {
		if (strcasecmp((yyvsp[-5].string), "SQL_TSI_FRAC_SECOND") == 0)
		{
			struct ASTNode *interval = newLeaf(getSingleStoreInterval((yyvsp[-5].string)));
			free((yyvsp[-5].string));

			// The interval is expected to be provided in the billionth of a second,
			// according to the ODBC documentation
			// "TIMESTAMPADD(getSingleStoreInterval($3), $5/1000, $7)"
			//
			(yyval.ast) = allocateNode(7,
				newLeaf("TIMESTAMPADD("),
				interval,
				newLeaf(", "),
				(yyvsp[-3].ast),
				newLeaf("/1000, "),
				(yyvsp[-1].ast),
				newLeaf(")"));
			if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
		} else
		{
			struct ASTNode *interval = newLeaf(getSingleStoreInterval((yyvsp[-5].string)));
			free((yyvsp[-5].string));

			// "TIMESTAMPADD(getSingleStoreInterval($3), $5, $7)"
			//
			(yyval.ast) = allocateNode(7,
				newLeaf("TIMESTAMPADD("),
				interval,
				newLeaf(", "),
				(yyvsp[-3].ast),
				newLeaf(", "),
				(yyvsp[-1].ast),
				newLeaf(")"));
			if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
		}
	}

    break;

  case 13:

    {
		if (strcasecmp((yyvsp[-5].string), "SQL_TSI_FRAC_SECOND") == 0)
		{
			struct ASTNode *interval = newLeaf(getSingleStoreInterval((yyvsp[-5].string)));
			free((yyvsp[-5].string));

			// The result is expected to be in the billionth of a second,
			// according to the ODBC documentation
			// "TIMESTAMPDIFF(getSingleStoreInterval($3), $5, $7)*1000"
			//
			(yyval.ast) = allocateNode(7,
				newLeaf("TIMESTAMPDIFF("),
				interval,
				newLeaf(", "),
				(yyvsp[-3].ast),
				newLeaf(", "),
				(yyvsp[-1].ast),
				newLeaf(")*1000")
			);
			if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
		} else
		{
			struct ASTNode *interval = newLeaf(getSingleStoreInterval((yyvsp[-5].string)));
			free((yyvsp[-5].string));

			// "TIMESTAMPDIFF(getSingleStoreInterval($3), $5, $7)"
			//
			(yyval.ast) = allocateNode(7,
				newLeaf("TIMESTAMPDIFF("),
				interval,
				newLeaf(", "),
				(yyvsp[-3].ast),
				newLeaf(", "),
				(yyvsp[-1].ast),
				newLeaf(")")
			);
			if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
		}
	}

    break;

  case 14:

    {
		struct ASTNode *correctDataTypeName = newLeaf(getSingleStoreDataType((yyvsp[-1].string)));
		free((yyvsp[-1].string));

		// "$3 :> correctDataTypeName"
		//
		(yyval.ast) = allocateNode(3, (yyvsp[-3].ast), newLeaf(" :> "), correctDataTypeName);
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 15:

    {
		// "LPAD('', CHAR_LENGTH($3)*$5, $3)"
		//
		(yyval.ast) = allocateNode(7,
			newLeaf("LPAD('', CHAR_LENGTH("),
			(yyvsp[-3].ast),
			newLeaf(")*"),
			(yyvsp[-1].ast),
			newLeaf(", "),
			copyTree((yyvsp[-3].ast)),
			newLeaf(")")
		);
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 16:

    {
		// "LPAD('', $3, ' ')"
		//
		(yyval.ast) = allocateNode(3, newLeaf("LPAD('', "), (yyvsp[-1].ast), newLeaf(", ' ')"));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 17:

    {
		struct ASTNode *lexeme = newLeaf((yyvsp[-3].string));
		free((yyvsp[-3].string));

		// "$1($3)"
		//
		(yyval.ast) = allocateNode(4,
			lexeme,
			newLeaf("("),
			(yyvsp[-1].ast),
			newLeaf(")")
		);
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 18:

    {
		struct ASTNode *lexeme = newLeaf((yyvsp[-2].string));
		free((yyvsp[-2].string));

		// "$1()"
		//
		(yyval.ast) = allocateNode(2, lexeme, newLeaf("()"));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 19:

    {
		(yyval.ast) = (yyvsp[0].ast);
	}

    break;

  case 20:

    {
		// "$1, $2"
		//
		(yyval.ast) = allocateNode(3, (yyvsp[-2].ast), newLeaf(", "), (yyvsp[0].ast));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 21:

    {
		(yyval.ast) = newLeaf((yyvsp[0].string));
		free((yyvsp[0].string));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 22:

    {
		(yyval.ast) = newLeaf((yyvsp[0].string));
		free((yyvsp[0].string));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 23:

    {
		// "$1 $2"
		//
		(yyval.ast) = allocateNode(3, (yyvsp[-1].ast), newLeaf(" "), (yyvsp[0].ast));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;

  case 24:

    {
		(yyval.ast) = (yyvsp[0].ast);
	}

    break;

  case 25:

    {
		// "($2)"
		//
		(yyval.ast) = allocateNode(3, newLeaf("("), (yyvsp[-1].ast), newLeaf(")"));
		if ((yyval.ast) == NULL) { YYALOCATION_ERROR; }
	}

    break;



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
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

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
      yyerror (scanner, error, res, YY_("syntax error"));
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
        yyerror (scanner, error, res, yymsgp);
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
                      yytoken, &yylval, scanner, error, res);
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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

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
                  yystos[yystate], yyvsp, scanner, error, res);
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
  yyerror (scanner, error, res, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner, error, res);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, scanner, error, res);
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



void freeChildren(int childrenCount, va_list children)
{
	int i;
	for (i = 0; i < childrenCount; i++)
	{
		struct ASTNode *child = va_arg(children, struct ASTNode*);
		if (child != NULL) { freeTree(child); }
	}
}

struct ASTNode* allocateNode(int childrenCount, ...)
{
	va_list valist;
	va_start(valist, childrenCount);
	my_bool children_allocated_successfully = TRUE;
	int i;
	for (i = 0; i < childrenCount; i++)
	{
		if (va_arg(valist, struct ASTNode*) == NULL)
		{
			children_allocated_successfully = FALSE;
		}
	}
	va_end(valist);

	if (!children_allocated_successfully)
	{
		va_start(valist, childrenCount);
		freeChildren(childrenCount, valist);
		va_end(valist);
		return NULL;
	}

	va_start(valist, childrenCount);
	struct ASTNode *res = newNode(childrenCount, valist);
	va_end(valist);

	if (res == NULL)
	{
		va_start(valist, childrenCount);
		freeChildren(childrenCount, valist);
		va_end(valist);
	}

	return res;
}
