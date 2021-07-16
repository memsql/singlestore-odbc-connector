/*************************************************************************************
  Copyright (c) 2021 SingleStore, Inc.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not see <http://www.gnu.org/licenses>
  or write to the Free Software Foundation, Inc.,
  51 Franklin St., Fifth Floor, Boston, MA 02110, USA
*************************************************************************************/
%{
#include "escape_sequences/parser.h"
#define YY_DECL int yylex(YYSTYPE * yylval_param, yyscan_t yyscanner, MADB_Error *error, MADB_Dbc *Dbc)
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