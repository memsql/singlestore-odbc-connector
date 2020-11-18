%code requires {
#include "ma_odbc.h"
#include "ma_legacy_helpers.h"
}

%{
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ma_odbc.h"
#include "escape_sequences/ast.h"
void yyerror (void *scanner, MADB_Stmt *Stmt, MADB_DynString *res, char *s)
{
	if (strcmp(s, "syntax error") == 0)
	{
		MADB_SetError(&Stmt->Error,  MADB_ERR_42000, "Incorrect syntax for the escape sequence", 0);
	} else
	{
		MADB_SetError(&Stmt->Error,  MADB_ERR_42000, s, 0);
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
	yyerror(scanner, Stmt, res, "Failed to allocate memory for escape sequence parsing");\
	YYABORT;\
} while(0)

%}


%pure-parser
%lex-param {void *scanner}
%lex-param {MADB_Stmt *Stmt}
%parse-param {void *scanner}
%parse-param {MADB_Stmt *Stmt}
%parse-param {MADB_DynString *res}

%union
{
	char *string;
	struct ASTNode *ast;
}

%token DATE_ESCAPE
%token TIME_ESCAPE
%token TIMESTAMP_ESCAPE
%token OUTER_JOIN_ESCAPE
%token FN_ESCAPE
%token CALL_ESCAPE

%token BIT_LENGTH
%token INSERT
%token TIMESTAMPADD
%token TIMESTAMPDIFF
%token CONVERT
%token SPACE
%token REPEAT
%token ALLOCATION_ERROR

%token OPENING_ROUND_BRACKET
%token CLOSING_ROUND_BRACKET
%token COMMA
%token <string> STRING_LITERAL
%token <string> LEXEME

%type <ast> function_call lexemes lexeme

%destructor { freeTree ($$); } <ast>
%destructor { free ($$); } <string>

%%
res:
	DATE_ESCAPE STRING_LITERAL
	{
		struct ASTNode *stringLiteral = newLeaf($2);
		free($2);

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
	|  TIME_ESCAPE STRING_LITERAL
	{
		struct ASTNode *stringLiteral = newLeaf($2);
		free($2);

		// "($2 :> TIME)"
		//
		struct ASTNode *x = allocateNode(3, newLeaf("("), stringLiteral, newLeaf(" :> TIME)"));
		if (x == NULL)
		{ YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}
	|  TIMESTAMP_ESCAPE STRING_LITERAL
	{
		struct ASTNode *stringLiteral = newLeaf($2);
		free($2);

		// "($2 :> TIMESTAMP(6))"
		//
		struct ASTNode *x = allocateNode(3, newLeaf("("), stringLiteral, newLeaf(" :> TIMESTAMP(6))"));
		if (x == NULL) { YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}
	|  CALL_ESCAPE LEXEME
	{
		struct ASTNode *lexeme = newLeaf($2);
		free($2);

		// "CALL $2()"
		//
		struct ASTNode *x = allocateNode(3, newLeaf("CALL "), lexeme, newLeaf("()"));
		if (x == NULL) { YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}
	|  CALL_ESCAPE function_call
	{
		// "CALL $2"
		//
		struct ASTNode *x = allocateNode(2, newLeaf("CALL "), $2);
		if (x == NULL) { YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}
	|  OUTER_JOIN_ESCAPE lexemes
	{
		setRes(res, $2);
		freeTree($2);
	}
	|  FN_ESCAPE LEXEME
	{
		struct ASTNode *lexeme = newLeaf($2);
		free($2);

		// "$2()"
		//
		struct ASTNode *x = allocateNode(2, lexeme, newLeaf("()"));
		if (x == NULL) { YYALOCATION_ERROR; }

		setRes(res, x);
		freeTree(x);
	}
	|  FN_ESCAPE function_call
	{
		setRes(res, $2);
		freeTree($2);
	}
	;

function_call:
	BIT_LENGTH OPENING_ROUND_BRACKET lexeme CLOSING_ROUND_BRACKET
	{
		// "LENGTH($3)*8"
		//
		$$ = allocateNode(3, newLeaf("LENGTH("), $3, newLeaf(")*8"));
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| INSERT OPENING_ROUND_BRACKET lexeme COMMA lexeme COMMA lexeme COMMA lexeme CLOSING_ROUND_BRACKET
	{
		// CONCAT(CONCAT(LEFT($3, $5 - 1), $9), RIGHT($3, CHAR_LENGTH($3) - $7 - $5 + 1))
		//
		$$ = allocateNode(15,
			newLeaf("CONCAT(CONCAT(LEFT("),
			$3,
			newLeaf(", "),
			$5,
			newLeaf(" - 1), "),
			$9,
			newLeaf("), RIGHT("),
			copyTree($3),
			newLeaf(", CHAR_LENGTH("),
			copyTree($3),
			newLeaf(") - "),
			$7,
			newLeaf(" - "),
			copyTree($5),
			newLeaf(" + 1))"));
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| TIMESTAMPADD OPENING_ROUND_BRACKET LEXEME COMMA lexeme COMMA lexeme CLOSING_ROUND_BRACKET
	{
		if (strcasecmp($3, "SQL_TSI_FRAC_SECOND") == 0)
		{
			struct ASTNode *interval = newLeaf(getSingleStoreInterval($3));
			free($3);

			// The interval is expected to be provided in the billionth of a second,
			// according to the ODBC documentation
			// "TIMESTAMPADD(getSingleStoreInterval($3), $5/1000, $7)"
			//
			$$ = allocateNode(7,
				newLeaf("TIMESTAMPADD("),
				interval,
				newLeaf(", "),
				$5,
				newLeaf("/1000, "),
				$7,
				newLeaf(")"));
			if ($$ == NULL) { YYALOCATION_ERROR; }
		} else
		{
			struct ASTNode *interval = newLeaf(getSingleStoreInterval($3));
			free($3);

			// "TIMESTAMPADD(getSingleStoreInterval($3), $5, $7)"
			//
			$$ = allocateNode(7,
				newLeaf("TIMESTAMPADD("),
				interval,
				newLeaf(", "),
				$5,
				newLeaf(", "),
				$7,
				newLeaf(")"));
			if ($$ == NULL) { YYALOCATION_ERROR; }
		}
	}
	| TIMESTAMPDIFF OPENING_ROUND_BRACKET LEXEME COMMA lexeme COMMA lexeme CLOSING_ROUND_BRACKET
	{
		if (strcasecmp($3, "SQL_TSI_FRAC_SECOND") == 0)
		{
			struct ASTNode *interval = newLeaf(getSingleStoreInterval($3));
			free($3);

			// The result is expected to be in the billionth of a second,
			// according to the ODBC documentation
			// "TIMESTAMPDIFF(getSingleStoreInterval($3), $5, $7)*1000"
			//
			$$ = allocateNode(7,
				newLeaf("TIMESTAMPDIFF("),
				interval,
				newLeaf(", "),
				$5,
				newLeaf(", "),
				$7,
				newLeaf(")*1000")
			);
			if ($$ == NULL) { YYALOCATION_ERROR; }
		} else
		{
			struct ASTNode *interval = newLeaf(getSingleStoreInterval($3));
			free($3);

			// "TIMESTAMPDIFF(getSingleStoreInterval($3), $5, $7)"
			//
			$$ = allocateNode(7,
				newLeaf("TIMESTAMPDIFF("),
				interval,
				newLeaf(", "),
				$5,
				newLeaf(", "),
				$7,
				newLeaf(")")
			);
			if ($$ == NULL) { YYALOCATION_ERROR; }
		}
	}
	| CONVERT OPENING_ROUND_BRACKET lexeme COMMA LEXEME CLOSING_ROUND_BRACKET
	{
		struct ASTNode *correctDataTypeName = newLeaf(getSingleStoreDataType($5));
		free($5);

		// "$3 :> correctDataTypeName"
		//
		$$ = allocateNode(3, $3, newLeaf(" :> "), correctDataTypeName);
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| REPEAT OPENING_ROUND_BRACKET lexeme COMMA lexeme CLOSING_ROUND_BRACKET
	{
		// "LPAD('', CHAR_LENGTH($3)*$5, $3)"
		//
		$$ = allocateNode(7,
			newLeaf("LPAD('', CHAR_LENGTH("),
			$3,
			newLeaf(")*"),
			$5,
			newLeaf(", "),
			copyTree($3),
			newLeaf(")")
		);
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| SPACE OPENING_ROUND_BRACKET lexeme CLOSING_ROUND_BRACKET
	{
		// "LPAD('', $3, ' ')"
		//
		$$ = allocateNode(3, newLeaf("LPAD('', "), $3, newLeaf(", ' ')"));
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| LEXEME OPENING_ROUND_BRACKET lexemes CLOSING_ROUND_BRACKET
	{
		struct ASTNode *lexeme = newLeaf($1);
		free($1);

		// "$1($3)"
		//
		$$ = allocateNode(4,
			lexeme,
			newLeaf("("),
			$3,
			newLeaf(")")
		);
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| LEXEME OPENING_ROUND_BRACKET CLOSING_ROUND_BRACKET
	{
		struct ASTNode *lexeme = newLeaf($1);
		free($1);

		// "$1()"
		//
		$$ = allocateNode(2, lexeme, newLeaf("()"));
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	;

lexemes : lexeme
	{
		$$ = $1;
	}
	| lexemes COMMA lexeme
	{
		// "$1, $2"
		//
		$$ = allocateNode(3, $1, newLeaf(", "), $3);
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	;


lexeme :
	STRING_LITERAL
	{
		$$ = newLeaf($1);
		free($1);
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| LEXEME
	{
		$$ = newLeaf($1);
		free($1);
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| lexeme lexeme
	{
		// "$1 $2"
		//
		$$ = allocateNode(3, $1, newLeaf(" "), $2);
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	| function_call
	{
		$$ = $1;
	}
	| OPENING_ROUND_BRACKET lexeme OPENING_ROUND_BRACKET
	{
		// "($2)"
		//
		$$ = allocateNode(3, newLeaf("("), $2, newLeaf(")"));
		if ($$ == NULL) { YYALOCATION_ERROR; }
	}
	;

allocation_error:
	ALLOCATION_ERROR
	{ YYALOCATION_ERROR; }
%%

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