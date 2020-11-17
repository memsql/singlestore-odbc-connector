%{
#include "escape_sequences/parser.h"
#define YY_DECL int yylex(YYSTYPE * yylval_param, yyscan_t yyscanner, MADB_Stmt *Stmt)
int yylex();
%}

%option caseless
%option reentrant
%option bison-bridge
%option noyywrap
%s ANSI_QUOTES

%%

"d"                                       {
                                                if(Stmt->Query.AnsiQuotes)
                                                    BEGIN(ANSI_QUOTES);
                                                return DATE_ESCAPE;
                                          }
"t"                                       {
                                                if(Stmt->Query.AnsiQuotes)
                                                    BEGIN(ANSI_QUOTES);
                                                return TIME_ESCAPE;
                                          }
"ts"                                      {
                                                if(Stmt->Query.AnsiQuotes)
                                                    BEGIN(ANSI_QUOTES);
                                                return TIMESTAMP_ESCAPE;
                                          }
"fn"                                      {
                                                if(Stmt->Query.AnsiQuotes)
                                                    BEGIN(ANSI_QUOTES);
                                                return FN_ESCAPE;
                                          }
"oj"                                      {
                                                if(Stmt->Query.AnsiQuotes)
                                                    BEGIN(ANSI_QUOTES);
                                                return OUTER_JOIN_ESCAPE;
                                          }
(\?=)?([ \t\r\n\v\f]*)call                {
                                                if(Stmt->Query.AnsiQuotes)
                                                    BEGIN(ANSI_QUOTES);
                                                return CALL_ESCAPE;
                                          }
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
<ANSI_QUOTES>\"(\"\"|[^"])*\"             {
                                                yylval->string = strdup(yytext);
                                                if (yylval->string == NULL)
                                                {
                                                    return ALLOCATION_ERROR;
                                                }
                                                return LEXEME;
                                          }
<INITIAL>\"(\\.|[^"\\])*\"                {
                                                yylval->string = strdup(yytext);
                                                if (yylval->string == NULL)
                                                {
                                                    return ALLOCATION_ERROR;
                                                }
                                                return STRING_LITERAL;
                                          }
'(\\.|[^'\\])*'                           {
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