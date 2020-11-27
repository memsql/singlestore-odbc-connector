%{
#include "escape_sequences/parser.h"
#define YY_DECL int yylex(YYSTYPE * yylval_param, yyscan_t yyscanner, MADB_Stmt *Stmt)
int yylex();
%}

%option caseless
%option reentrant
%option bison-bridge
%option noyywrap

%%

"d"                                       { return DATE_ESCAPE; }
"t"                                       { return TIME_ESCAPE; }
"ts"                                      { return TIMESTAMP_ESCAPE; }
"fn"                                      { return FN_ESCAPE; }
"oj"                                      { return OUTER_JOIN_ESCAPE; }
(\?=)?([ \t\r\n\v\f]*)call                { return CALL_ESCAPE; }
"BIT_LENGTH"                              { return BIT_LENGTH; }
"INSERT"                                  { return INSERT; }
"TIMESTAMPADD"                            { return TIMESTAMPADD; }
"TIMESTAMPDIFF"                           { return TIMESTAMPDIFF; }
"CONVERT"                                 { return CONVERT; }
"SPACE"                                   { return SPACE; }
"REPEAT"                                  { return REPEAT; }
"("                                       { return OPENING_ROUND_BRACKET; }
")"                                       { return CLOSING_ROUND_BRACKET; }
,                                         { return COMMA; }
\"((\"\")|\\.|[^"\\])*\"                  {
                                                yylval->string = strdup(yytext);
                                                if (yylval->string == NULL)
                                                {
                                                    return ALLOCATION_ERROR;
                                                }
                                                // When ANSI_QUOTES is disabled, it should be a STRING_LITERAL.
                                                // To check it before running each query,
                                                // we need SingleStore to support server trackers.
                                                // This feature is not implemented now.
                                                // See discussion on PLAT-4916.
                                                // With current behaviour,
                                                // it is possible, that we will send to the SingleStore a query,
                                                // where ANSI_QUOTES mode is disabled and string literal is used as
                                                // function call.
                                                // In this case SingleStore will return an appropriate error.
                                                // Also, it is possible, that when ANSI_QUOTES is enabled,
                                                // it will parse incorrectly queries with backslash at the end of identifier.
                                                //
                                                return LEXEME;
                                          }
'(('')|\\.|[^'\\])*'                      {
                                                yylval->string = strdup(yytext);
                                                if (yylval->string == NULL)
                                                {
                                                    return ALLOCATION_ERROR;
                                                }
                                                return STRING_LITERAL;
                                          }
`(``|[^`])*`                              {
                                                yylval->string = strdup(yytext);
                                                if (yylval->string == NULL)
                                                {
                                                    return ALLOCATION_ERROR;
                                                }
                                                return LEXEME;
                                          }
[^\(\)\\ \t\r\n\v\f,'`]*                  {
                                                yylval->string = strdup(yytext);
                                                if (yylval->string == NULL)
                                                {
                                                    return ALLOCATION_ERROR;
                                                }
                                                return LEXEME;
                                          }
[ \t\r\n\v\f]*                            ;
%%