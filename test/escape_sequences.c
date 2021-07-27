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

#include "tap.h"
#define BUFFER_SIZE 1024

ODBC_TEST(date_literal)
{
  OK_SIMPLE_STMT(Stmt, "SELECT {d '2001-10-1' }");
  return OK;
}

ODBC_TEST(time_literal)
{
  OK_SIMPLE_STMT(Stmt, "SELECT {t '01:10:10' }");
  return OK;
}

ODBC_TEST(timestamp_literal)
{
  OK_SIMPLE_STMT(Stmt, "SELECT {ts '2001-10-1 01:10:10'  }");
  return OK;
}

ODBC_TEST(scalar_function) {
  char buffer[BUFFER_SIZE];
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS scalar_function");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE scalar_function(col TEXT)");

  OK_SIMPLE_STMT(Stmt, "INSERT INTO scalar_function VALUES('a'), ('abc'), ('asasasas   asdads\n\r ;? asd')");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONCAT({fn UCASE((col))}, RTRIM((LTRIM('  !  ')))) } FROM scalar_function ORDER BY col");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "A!", 3);
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "ABC!", 5);
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "ASASASAS   ASDADS\n\r ;? ASD!", sizeof("ASASASAS   ASDADS\n\r ;? ASD!"));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE scalar_function");

  OK_SIMPLE_STMT(Stmt, "SELECT {fn ASCII('abc')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CHAR(10)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CHAR_LENGTH('abc')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CHARACTER_LENGTH('abc')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONCAT('abc', 'def')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LCASE('AbC')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LEFT('AbC', 1)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LOCATE('abcabcd', (('abc')))}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LOCATE('abcabcd', ('abc'), (2))}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LTRIM('    abc')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn OCTET_LENGTH('abc')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn POSITION('bbabcbb' in 'abc')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn REPLACE('abcabc', 'b', 'cc')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn RIGHT('abcabc', 3)}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn RTRIM('abc   ')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn SUBSTRING('abcabc', 3, 5)}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn UCASE('AbC')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn BIT_LENGTH('AbC')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn INSERT('AbCabc', 2, 2, 'qwe')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn REPEAT('abc', 4)}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn SPACE(4)}")

  OK_SIMPLE_STMT(Stmt, "SELECT {fn ABS(-10)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn ACOS(0.5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn ASIN(0.5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn ATAN(0.5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn ATAN2(0.5, -0.5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CEILING(0.5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn COS(2)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn COT(2)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn DEGREES(2)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn EXP(2)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn FLOOR(2.6)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LOG(2.6)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LOG10(2.6)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn MOD(13, 3)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn PI()}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn POWER(3, 4.5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn RADIANS(4.5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn RAND()}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn RAND(10)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn ROUND(2.5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn SIGN(-4)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn SIN(-4)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn SQRT(5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TAN(5)}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TRUNCATE(2.5555, 1)}");


  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURRENT_DATE()}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURRENT_TIME(0)}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURRENT_TIME(6)}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURRENT_TIME}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURRENT_TIMESTAMP(0)}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURRENT_TIMESTAMP(6)}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURRENT_TIMESTAMP}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURDATE()}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CURTIME()}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn DAYNAME('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn DAYNAME('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn DAYOFMONTH('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn DAYOFWEEK('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn DAYOFYEAR('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn EXTRACT(SECOND FROM '1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn EXTRACT(MINUTE FROM '1998-01-01 01:01:01.001222')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn EXTRACT(HOUR FROM '1998-01-01 01:01:01.001222')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn EXTRACT(DAY FROM '1998-01-01 01:01:01.001222')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn EXTRACT(MONTH FROM '1998-01-01 01:01:01.001222')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn EXTRACT(YEAR FROM '1998-01-01 01:01:01.001222')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn HOUR('838:54:59')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn MINUTE('838:54:59')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn MONTH('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn MONTHNAME('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn NOW()}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn QUARTER('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn SECOND('838:54:59')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn WEEK('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn YEAR('1998-01-01')}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_SECOND, 2, '1981-01-01 00:00:01')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_SECOND, 2, '1981-01-01 00:00:01')}");

  OK_SIMPLE_STMT(Stmt, "SELECT {fn DATABASE()}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn IFNULL(NULL, 1)}")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn USER()}")

  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONVERT(23, SQL_BIGINT)}")

  EXPECT_STMT(Stmt, SQLExecDirect(Stmt, (SQLCHAR*)("SELECT {fn \"USER\"()}"), SQL_NTS), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "42000");
  EXPECT_STMT(Stmt, SQLExecDirect(Stmt, (SQLCHAR*)("SELECT {fn 'USER'()}"), SQL_NTS), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "42000");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn `USER`()}");
  return OK;
}

ODBC_TEST(outer_join) {
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS l");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS r");

  OK_SIMPLE_STMT(Stmt, "CREATE TABLE l(a INT)");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE r(b INT)");

  OK_SIMPLE_STMT(Stmt, "INSERT INTO l VALUES(1), (2)");
  OK_SIMPLE_STMT(Stmt, "INSERT INTO r VALUES(2), (3)");

  OK_SIMPLE_STMT(Stmt, "SELECT * FROM {oj l LEFT OUTER JOIN r ON l.a=r.b  }");
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM {oj l RIGHT OUTER JOIN r ON l.a=r.b }");
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM {oj l FULL OUTER JOIN r ON l.a=r.b  }");

  OK_SIMPLE_STMT(Stmt, "DROP TABLE l");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE r");

  return OK;
}

ODBC_TEST(procedure_call) {
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS procedure_call_data");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE procedure_call_data(col INT)");

  OK_SIMPLE_STMT(Stmt,"CREATE OR REPLACE PROCEDURE ins (a INT) AS\n"
                       "BEGIN\n"
                       "    INSERT INTO procedure_call_data VALUES (a);\n"
                       "END")
  OK_SIMPLE_STMT(Stmt, "CREATE OR REPLACE PROCEDURE ins100 () AS\n"
                       "BEGIN\n"
                       "    INSERT INTO procedure_call_data VALUES (100);\n"
                       "END")

  OK_SIMPLE_STMT(Stmt, "{call ins(1)   }")
  OK_SIMPLE_STMT(Stmt, "{?=call ins(2) }")
  OK_SIMPLE_STMT(Stmt, "{?=call ins100  }")

  OK_SIMPLE_STMT(Stmt, "SELECT * FROM procedure_call_data ORDER BY col");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), 1);
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), 2);
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), 100);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE procedure_call_data")

  EXPECT_STMT(Stmt, SQLExecDirect(Stmt, (SQLCHAR*)("{call    }"), SQL_NTS), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "42000");

  return OK;
}

ODBC_TEST(wrong_brackets_sequence) {
  char * queries[] = {
    "SELECT {d '2001-04-11'",
    "SELECT '2001-04-11'}",
    "SELECT {d {d }}} {",
    "SELECT {d } {d } {d } }{",
    "SELECT {d {d {d {d } } } {d }",
    "SELECT } } {d {d {d {d } } } {d }",
    "SELECT }",
    "SELECT {{d} {d}} }",
  };
  char **query;

  size_t n = 8;
  char **end = queries + n;
  for (query = queries; query < end; query++) {
    EXPECT_STMT(Stmt, SQLExecDirect(Stmt, (SQLCHAR*)(*query), SQL_NTS), SQL_ERROR);
    CHECK_SQLSTATE(Stmt, "42000");
  }

  return OK;
}

ODBC_TEST(strings_with_escape_sequences) {
  char* escapeSequences[] = {
    "{}",
    "{d '2001-10-1' }",
    "{ts '2001-10-1 01:10:10'  }",
    "{fn CONCAT({fn UCASE(col)}, RTRIM(LTRIM('  !  '))) }",
    "{oj l LEFT OUTER JOIN r ON l.a=r.b  }",
    "{?=call ins(2) }",
    "{{\\}}",
    "{\"{}\"}",
  };
  char **escapeSequence;
  char *c;

  size_t n = 8;
  char **end = escapeSequences + n;
  for (escapeSequence = escapeSequences; escapeSequence < end; escapeSequence++) {
    // build "SELECT "..."" query
    char query[BUFFER_SIZE];
    char *queryIterator = query;
    strcpy(queryIterator, "SELECT \"");
    queryIterator += 8;

    // escape escapeSequence
    for (c = *escapeSequence; *c != '\0'; c++) {
      if (*c == '\\' || *c == '"' || *c == '\'' ) {
        *queryIterator++ = '\\';
      }
      *queryIterator++ = *c;
    }
    *queryIterator++ = '\"';
    *queryIterator = '\0';

    OK_SIMPLE_STMT(Stmt, query);
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    // check that we get correct string
    char buffer[BUFFER_SIZE];
    IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), *escapeSequence,strlen(*escapeSequence));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  }

  return OK;
}

ODBC_TEST(unsupported_escape_sequence) {
  char * queries[] = {
    "SELECT 'Name' LIKE '\\%AAA%' {escape '\\'}",
    "SELECT {guid 'nnnnnnnn-nnnn-nnnn-nnnn-nnnnnnnnnnnn'}",
    "SELECT {INTERVAL 1 day}",
  };
  char ** query;

  size_t n = 3;
  char **end = queries + n;
  for (query = queries; query < end; query++) {
    EXPECT_STMT(Stmt, SQLExecDirect(Stmt, (SQLCHAR*)(*query), SQL_NTS), SQL_ERROR);
    CHECK_SQLSTATE(Stmt, "42000");
  }

  return OK;
}

ODBC_TEST(uppercase) {
  char * queries[] = {
    "SELECT {D '2001-10-1' }",
    "SELECT {T '01:10:10' }",
    "SELECT {TS '2001-10-1 01:10:10'  }",
    "SELECT {tS '2001-10-1 01:10:10'  }",
    "SELECT {Ts '2001-10-1 01:10:10'  }"
  };
  char **query;

  size_t n = 5;
  char **end = queries + n;
  for (query = queries; query < end; query++) {
    OK_SIMPLE_STMT(Stmt, *query);
  }

  return OK;
}

ODBC_TEST(convert) {
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_BIGINT  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONVERT(123,SQL_BINARY)},123")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONVERT(123   ,SQL_BIT)},123")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONVERT(123   ,SQL_CHAR )},123")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONVERT(123   ,SQL_CHAR )},123")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONVERT(123   , SQL_DATE )}  ,123")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn ConVErT(123   , SQL_decIMAL )   }  ,123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_DOUBLE  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_FLOAT  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_INTEGER  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_LONGVARBINARY  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_LONGVARCHAR  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_NUMERIC  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_REAL  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_SMALLINT  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_TIME  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_TIMESTAMP  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_TINYINT  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_VARBINARY  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_VARCHAR  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_WCHAR  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_WLONGVARCHAR  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , SQL_WVARCHAR  ) } , 123")
  OK_SIMPLE_STMT(Stmt, "SELECT { fn CONVERT (  123 , BIGINT  ) } , 123")
  return OK;
}

ODBC_TEST(convert_failures) {
  char * queries[] = {
    "SELECT {fn CONVERT(123 SQL_BIGINT) }",
    "SELECT {fn CONVERT(123, SQL_BIGINT }",
    "SELECT {fn CONVERT(1,   ) }",
    "SELECT {fn CONVERT 123, SQL_BIGINT) }",
    "SELECT {fn CONVERT 123, SQL_BIGINT }",
    "SELECT {fn CONVERT 123 }",
    "SELECT {fn CONVERT(, SQL_BIGINT) }",
    "SELECT {fn CONVERT(  ,   ) }",
    "SELECT {fn CONVERT(  ,  SQL_BIGINT ) }",
  };
  char **query;

  size_t n = sizeof(queries) / sizeof(queries[0]);
  char **end = queries + n;
  for (query = queries; query < end; query++) {
    EXPECT_STMT(Stmt, SQLExecDirect(Stmt, (SQLCHAR*)(*query), SQL_NTS), SQL_ERROR);
    CHECK_SQLSTATE(Stmt, "42000");
  }

  return OK;
}

ODBC_TEST(bit_length) {
  OK_SIMPLE_STMT(Stmt, "SELECT {fn BIT_LENGTH('')}, {fn BIT_LENGTH('asd')}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), 0);
  is_num(my_fetch_int(Stmt, 2), 24);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}

ODBC_TEST(insert) {
  char buffer[BUFFER_SIZE];
  OK_SIMPLE_STMT(Stmt, "SELECT {fn INSERT('abcde', 2, 2, 'eee')}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "aeeede", 7);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn INSERT('abcde', 3, 10, 'eee')}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "abeee", 6);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn INSERT('abcde', -1, 4, 'eee')}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "eeecde", 6);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn INSERT('ßßßßß', 2, 2, 'ééé')}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "ßéééßß", 13);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}

ODBC_TEST(repeat) {
  char buffer[BUFFER_SIZE];
  OK_SIMPLE_STMT(Stmt, "SELECT {fn REPEAT('abc', 3)}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "abcabcabc", 10);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn REPEAT('abc', 0)}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "", 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn REPEAT('', 10)}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "", 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn REPEAT('ßß', 3)}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "ßßßßßß", 13);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}

ODBC_TEST(space) {
  char buffer[BUFFER_SIZE];
  OK_SIMPLE_STMT(Stmt, "SELECT {fn SPACE(3)}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "   ", 4);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn SPACE(0)}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "", 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}

ODBC_TEST(timestamp_add) {
  char buffer[BUFFER_SIZE];
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_FRAC_SECOND, 2000, '1998-01-01 01:01:01.001222')}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "1998-01-01 01:01:01.001224", 27);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_SECOND, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_MINUTE, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_HOUR, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_DAY, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_WEEK, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_MONTH, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_QUARTER, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SQL_TSI_YEAR, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(MICROSECOND, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(SECOND, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(MINUTE, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(HOUR, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(DAY, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(WEEK, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(MONTH, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(QUARTER, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPADD(YEAR, 2, '1998-01-01 01:01:01.001222')}");

  return OK;
}

ODBC_TEST(timestamp_diff) {
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_FRAC_SECOND, '1998-01-01 01:01:01.001222', '1998-01-01 01:01:01.001225')}");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), 3000);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_SECOND, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_MINUTE, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_HOUR, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_DAY, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_WEEK, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_MONTH, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_QUARTER, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SQL_TSI_YEAR, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(MICROSECOND, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(SECOND, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(MINUTE, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(HOUR, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(DAY, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(WEEK, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(MONTH, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(QUARTER, 2, '1998-01-01 01:01:01.001222')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn TIMESTAMPDIFF(YEAR, 2, '1998-01-01 01:01:01.001222')}");

  return OK;
}

ODBC_TEST(quotes) {
  char buffer[BUFFER_SIZE];
  // This query should be successfully sent to SingleStore and validated there
  //
  EXPECT_STMT(Stmt, SQLExecDirect(Stmt, (SQLCHAR*)("SELECT {fn \"COS\"(1)}"), SQL_NTS), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "42000");

  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONCAT(\"asd\"\"asd\", \"a\") }")
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "asd\"asda", 9);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONCAT('asd''asd', 'a') }")
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "asd'asda", 9);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONCAT('\\'''', 'a') }")
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, (SQLCHAR*)buffer, 1), "''a", 4);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}

ODBC_TEST(ansi_quotes) {
  OK_SIMPLE_STMT(Stmt, "SET sql_mode='ANSI_QUOTES'")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn \"COS\"(1)}");
  OK_SIMPLE_STMT(Stmt, "SET sql_mode=''")
  return OK;
}

// TODO: this should be fixed after adding of the session trackers support to the SingleStore DB-46842
//
ODBC_TEST(ansi_quotes_to_fix) {
  OK_SIMPLE_STMT(Stmt, "SET sql_mode='ANSI_QUOTES'")
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t(`\\` int)");

  OK_SIMPLE_STMT(Stmt, "SELECT {fn \"COS\"(\"\\\")} FROM t");

  OK_SIMPLE_STMT(Stmt, "DROP TABLE t");
  OK_SIMPLE_STMT(Stmt, "SET sql_mode=''")
  return OK;
}

// sql_native_sql executes SQLNativeSql a lot of times with different valid queries and checks that it
// works correctly with them
//
ODBC_TEST(sql_native_sql) {
  int i, n;
  SQLCHAR buffer[BUFFER_SIZE];
  SQLWCHAR bufferW[BUFFER_SIZE];
  char * queries[197] = {
    "SELECT {d '2001-10-1' }",
    "SELECT {t '01:10:10' }",
    "SELECT {ts '2001-10-1 01:10:10'  }",
    "DROP TABLE IF EXISTS scalar_function",
    "CREATE TABLE scalar_function(col TEXT)",
    "INSERT INTO scalar_function VALUES('a'), ('abc'), ('asasasas   asdads\n\r ;? asd')",
    "SELECT {fn CONCAT({fn UCASE((col))}, RTRIM((LTRIM('  !  ')))) } FROM scalar_function ORDER BY col",
    "DROP TABLE scalar_function",
    "SELECT {fn ASCII('abc')}",
    "SELECT {fn CHAR(10)}",
    "SELECT {fn CHAR_LENGTH('abc')}",
    "SELECT {fn CHARACTER_LENGTH('abc')}",
    "SELECT {fn CONCAT('abc', 'def')}",
    "SELECT {fn LCASE('AbC')}",
    "SELECT {fn LEFT('AbC', 1)}",
    "SELECT {fn LOCATE('abcabcd', (('abc')))}",
    "SELECT {fn LOCATE('abcabcd', ('abc'), (2))}",
    "SELECT {fn LTRIM('    abc')}",
    "SELECT {fn OCTET_LENGTH('abc')}",
    "SELECT {fn POSITION('bbabcbb' in 'abc')}",
    "SELECT {fn REPLACE('abcabc', 'b', 'cc')}",
    "SELECT {fn RIGHT('abcabc', 3)}",
    "SELECT {fn RTRIM('abc   ')}",
    "SELECT {fn SUBSTRING('abcabc', 3, 5)}",
    "SELECT {fn UCASE('AbC')}",
    "SELECT {fn BIT_LENGTH('AbC')}",
    "SELECT {fn INSERT('AbCabc', 2, 2, 'qwe')}",
    "SELECT {fn REPEAT('abc', 4)}",
    "SELECT {fn SPACE(4)}",
    "SELECT {fn ABS(-10)}",
    "SELECT {fn ACOS(0.5)}",
    "SELECT {fn ASIN(0.5)}",
    "SELECT {fn ATAN(0.5)}",
    "SELECT {fn ATAN2(0.5, -0.5)}",
    "SELECT {fn CEILING(0.5)}",
    "SELECT {fn COS(2)}",
    "SELECT {fn COT(2)}",
    "SELECT {fn DEGREES(2)}",
    "SELECT {fn EXP(2)}",
    "SELECT {fn FLOOR(2.6)}",
    "SELECT {fn LOG(2.6)}",
    "SELECT {fn LOG10(2.6)}",
    "SELECT {fn MOD(13, 3)}",
    "SELECT {fn PI()}",
    "SELECT {fn POWER(3, 4.5)}",
    "SELECT {fn RADIANS(4.5)}",
    "SELECT {fn RAND()}",
    "SELECT {fn RAND(10)}",
    "SELECT {fn ROUND(2.5)}",
    "SELECT {fn SIGN(-4)}",
    "SELECT {fn SIN(-4)}",
    "SELECT {fn SQRT(5)}",
    "SELECT {fn TAN(5)}",
    "SELECT {fn TRUNCATE(2.5555, 1)}",
    "SELECT {fn CURRENT_DATE()}",
    "SELECT {fn CURRENT_TIME(0)}",
    "SELECT {fn CURRENT_TIME(6)}",
    "SELECT {fn CURRENT_TIME}",
    "SELECT {fn CURRENT_TIMESTAMP(0)}",
    "SELECT {fn CURRENT_TIMESTAMP(6)}",
    "SELECT {fn CURRENT_TIMESTAMP}",
    "SELECT {fn CURDATE()}",
    "SELECT {fn CURTIME()}",
    "SELECT {fn DAYNAME('1998-01-01')}",
    "SELECT {fn DAYNAME('1998-01-01')}",
    "SELECT {fn DAYOFMONTH('1998-01-01')}",
    "SELECT {fn DAYOFWEEK('1998-01-01')}",
    "SELECT {fn DAYOFYEAR('1998-01-01')}",
    "SELECT {fn EXTRACT(SECOND FROM '1998-01-01')}",
    "SELECT {fn EXTRACT(MINUTE FROM '1998-01-01 01:01:01.001222')}",
    "SELECT {fn EXTRACT(HOUR FROM '1998-01-01 01:01:01.001222')}",
    "SELECT {fn EXTRACT(DAY FROM '1998-01-01 01:01:01.001222')}",
    "SELECT {fn EXTRACT(MONTH FROM '1998-01-01 01:01:01.001222')}",
    "SELECT {fn EXTRACT(YEAR FROM '1998-01-01 01:01:01.001222')}",
    "SELECT {fn HOUR('838:54:59')}",
    "SELECT {fn MINUTE('838:54:59')}",
    "SELECT {fn MONTH('1998-01-01')}",
    "SELECT {fn MONTHNAME('1998-01-01')}",
    "SELECT {fn NOW()}",
    "SELECT {fn QUARTER('1998-01-01')}",
    "SELECT {fn SECOND('838:54:59')}",
    "SELECT {fn WEEK('1998-01-01')}",
    "SELECT {fn YEAR('1998-01-01')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_SECOND, 2, '1981-01-01 00:00:01')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_SECOND, 2, '1981-01-01 00:00:01')}",
    "SELECT {fn DATABASE()}",
    "SELECT {fn IFNULL(NULL, 1)}",
    "SELECT {fn USER()}",
    "SELECT {fn CONVERT(23, SQL_BIGINT)}",
    "SELECT {fn `USER`()}",
    "DROP TABLE IF EXISTS l",
    "DROP TABLE IF EXISTS r",
    "CREATE TABLE l(a INT)",
    "CREATE TABLE r(b INT)",
    "INSERT INTO l VALUES(1), (2)",
    "INSERT INTO r VALUES(2), (3)",
    "SELECT * FROM {oj l LEFT OUTER JOIN r ON l.a=r.b  }",
    "SELECT * FROM {oj l RIGHT OUTER JOIN r ON l.a=r.b }",
    "SELECT * FROM {oj l FULL OUTER JOIN r ON l.a=r.b  }",
    "DROP TABLE l",
    "DROP TABLE r",
    "DROP TABLE IF EXISTS procedure_call_data",
    "CREATE TABLE procedure_call_data(col INT)",
    "CREATE OR REPLACE PROCEDURE ins (a INT) AS\nBEGIN\nINSERT INTO procedure_call_data VALUES (a,\nEND",
    "CREATE OR REPLACE PROCEDURE ins100 () AS\nBEGIN\nINSERT INTO procedure_call_data VALUES (100,\nEND",
    "{call ins(1)   }",
    "{?=call ins(2) }",
    "{?=call ins100  }",
    "SELECT * FROM procedure_call_data ORDER BY col",
    "DROP TABLE procedure_call_data",
    "SELECT \"{}\"",
    "SELECT \"{d \\'2001-10-1\\' }\"",
    "SELECT \"{ts \\'2001-10-1 01:10:10\\'  }\"",
    "SELECT \"{fn CONCAT({fn UCASE(col)}, RTRIM(LTRIM(\\'  !  \\'))) }\"",
    "SELECT \"{oj l LEFT OUTER JOIN r ON l.a=r.b  }\"",
    "SELECT \"{?=call ins(2) }\"",
    "SELECT \"{{\\\\}}\"",
    "SELECT \"{\\\"{}\\\"}\"",
    "SELECT {D '2001-10-1' }",
    "SELECT {T '01:10:10' }",
    "SELECT {TS '2001-10-1 01:10:10'  }",
    "SELECT {tS '2001-10-1 01:10:10'  }",
    "SELECT {Ts '2001-10-1 01:10:10'  }",
    "SELECT { fn CONVERT (  123 , SQL_BIGINT  ) } , 123",
    "SELECT {fn CONVERT(123,SQL_BINARY)},123",
    "SELECT {fn CONVERT(123   ,SQL_BIT)},123",
    "SELECT {fn CONVERT(123   ,SQL_CHAR )},123",
    "SELECT {fn CONVERT(123   ,SQL_CHAR )},123",
    "SELECT {fn CONVERT(123   , SQL_DATE )}  ,123",
    "SELECT {fn ConVErT(123   , SQL_decIMAL )   }  ,123",
    "SELECT { fn CONVERT (  123 , SQL_DOUBLE  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_FLOAT  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_INTEGER  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_LONGVARBINARY  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_LONGVARCHAR  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_NUMERIC  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_REAL  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_SMALLINT  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_TIME  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_TIMESTAMP  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_TINYINT  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_VARBINARY  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_VARCHAR  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_WCHAR  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_WLONGVARCHAR  ) } , 123",
    "SELECT { fn CONVERT (  123 , SQL_WVARCHAR  ) } , 123",
    "SELECT { fn CONVERT (  123 , BIGINT  ) } , 123",
    "SELECT {fn BIT_LENGTH('')}, {fn BIT_LENGTH('asd')}",
    "SELECT {fn INSERT('abcde', 2, 2, 'eee')}",
    "SELECT {fn INSERT('abcde', 3, 10, 'eee')}",
    "SELECT {fn INSERT('abcde', -1, 4, 'eee')}",
    "SELECT {fn INSERT('ßßßßß', 2, 2, 'ééé')}",
    "SELECT {fn REPEAT('abc', 3)}",
    "SELECT {fn REPEAT('abc', 0)}",
    "SELECT {fn REPEAT('', 10)}",
    "SELECT {fn REPEAT('ßß', 3)}",
    "SELECT {fn SPACE(3)}",
    "SELECT {fn SPACE(0)}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_FRAC_SECOND, 2000, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_SECOND, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_MINUTE, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_HOUR, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_DAY, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_WEEK, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_MONTH, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_QUARTER, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SQL_TSI_YEAR, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(MICROSECOND, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(SECOND, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(MINUTE, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(HOUR, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(DAY, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(WEEK, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(MONTH, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(QUARTER, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPADD(YEAR, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_FRAC_SECOND, '1998-01-01 01:01:01.001222', '1998-01-01 01:01:01.001225')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_SECOND, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_MINUTE, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_HOUR, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_DAY, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_WEEK, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_MONTH, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_QUARTER, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SQL_TSI_YEAR, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(MICROSECOND, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(SECOND, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(MINUTE, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(HOUR, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(DAY, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(WEEK, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(MONTH, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(QUARTER, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn TIMESTAMPDIFF(YEAR, 2, '1998-01-01 01:01:01.001222')}",
    "SELECT {fn CONCAT(\"asd\"\"asd\", \"a\") }",
    "SELECT {fn CONCAT('asd''asd', 'a') }",
    "SELECT {fn CONCAT('\\'''', 'a') }",
  };

  char * expected_results[197] = {
    "SELECT ('2001-10-1' :> DATE)",
    "SELECT ('01:10:10' :> TIME)",
    "SELECT ('2001-10-1 01:10:10' :> TIMESTAMP(6))",
    "DROP TABLE IF EXISTS scalar_function",
    "CREATE TABLE scalar_function(col TEXT)",
    "INSERT INTO scalar_function VALUES('a'), ('abc'), ('asasasas   asdads\n\r ;? asd')",
    "SELECT CONCAT(UCASE((col)), RTRIM((LTRIM('  !  ')))) FROM scalar_function ORDER BY col",
    "DROP TABLE scalar_function",
    "SELECT ASCII('abc')",
    "SELECT CHAR(10)",
    "SELECT CHAR_LENGTH('abc')",
    "SELECT CHARACTER_LENGTH('abc')",
    "SELECT CONCAT('abc', 'def')",
    "SELECT LCASE('AbC')",
    "SELECT LEFT('AbC', 1)",
    "SELECT LOCATE('abcabcd', (('abc')))",
    "SELECT LOCATE('abcabcd', ('abc'), (2))",
    "SELECT LTRIM('    abc')",
    "SELECT OCTET_LENGTH('abc')",
    "SELECT POSITION('bbabcbb' in 'abc')",
    "SELECT REPLACE('abcabc', 'b', 'cc')",
    "SELECT RIGHT('abcabc', 3)",
    "SELECT RTRIM('abc   ')",
    "SELECT SUBSTRING('abcabc', 3, 5)",
    "SELECT UCASE('AbC')",
    "SELECT LENGTH('AbC')*8",
    "SELECT CONCAT(CONCAT(LEFT('AbCabc', 2 - 1), 'qwe'), RIGHT('AbCabc', CHAR_LENGTH('AbCabc') - 2 - 2 + 1))",
    "SELECT LPAD('', CHAR_LENGTH('abc')*4, 'abc')",
    "SELECT LPAD('', 4, ' ')",
    "SELECT ABS(-10)",
    "SELECT ACOS(0.5)",
    "SELECT ASIN(0.5)",
    "SELECT ATAN(0.5)",
    "SELECT ATAN2(0.5, -0.5)",
    "SELECT CEILING(0.5)",
    "SELECT COS(2)",
    "SELECT COT(2)",
    "SELECT DEGREES(2)",
    "SELECT EXP(2)",
    "SELECT FLOOR(2.6)",
    "SELECT LOG(2.6)",
    "SELECT LOG10(2.6)",
    "SELECT MOD(13, 3)",
    "SELECT PI()",
    "SELECT POWER(3, 4.5)",
    "SELECT RADIANS(4.5)",
    "SELECT RAND()",
    "SELECT RAND(10)",
    "SELECT ROUND(2.5)",
    "SELECT SIGN(-4)",
    "SELECT SIN(-4)",
    "SELECT SQRT(5)",
    "SELECT TAN(5)",
    "SELECT TRUNCATE(2.5555, 1)",
    "SELECT CURRENT_DATE()",
    "SELECT CURRENT_TIME(0)",
    "SELECT CURRENT_TIME(6)",
    "SELECT CURRENT_TIME()",
    "SELECT CURRENT_TIMESTAMP(0)",
    "SELECT CURRENT_TIMESTAMP(6)",
    "SELECT CURRENT_TIMESTAMP()",
    "SELECT CURDATE()",
    "SELECT CURTIME()",
    "SELECT DAYNAME('1998-01-01')",
    "SELECT DAYNAME('1998-01-01')",
    "SELECT DAYOFMONTH('1998-01-01')",
    "SELECT DAYOFWEEK('1998-01-01')",
    "SELECT DAYOFYEAR('1998-01-01')",
    "SELECT EXTRACT(SECOND FROM '1998-01-01')",
    "SELECT EXTRACT(MINUTE FROM '1998-01-01 01:01:01.001222')",
    "SELECT EXTRACT(HOUR FROM '1998-01-01 01:01:01.001222')",
    "SELECT EXTRACT(DAY FROM '1998-01-01 01:01:01.001222')",
    "SELECT EXTRACT(MONTH FROM '1998-01-01 01:01:01.001222')",
    "SELECT EXTRACT(YEAR FROM '1998-01-01 01:01:01.001222')",
    "SELECT HOUR('838:54:59')",
    "SELECT MINUTE('838:54:59')",
    "SELECT MONTH('1998-01-01')",
    "SELECT MONTHNAME('1998-01-01')",
    "SELECT NOW()",
    "SELECT QUARTER('1998-01-01')",
    "SELECT SECOND('838:54:59')",
    "SELECT WEEK('1998-01-01')",
    "SELECT YEAR('1998-01-01')",
    "SELECT TIMESTAMPADD(SECOND, 2, '1981-01-01 00:00:01')",
    "SELECT TIMESTAMPDIFF(SECOND, 2, '1981-01-01 00:00:01')",
    "SELECT DATABASE()",
    "SELECT IFNULL(NULL, 1)",
    "SELECT USER()",
    "SELECT 23 :> BIGINT",
    "SELECT `USER`()",
    "DROP TABLE IF EXISTS l",
    "DROP TABLE IF EXISTS r",
    "CREATE TABLE l(a INT)",
    "CREATE TABLE r(b INT)",
    "INSERT INTO l VALUES(1), (2)",
    "INSERT INTO r VALUES(2), (3)",
    "SELECT * FROM l LEFT OUTER JOIN r ON l.a=r.b",
    "SELECT * FROM l RIGHT OUTER JOIN r ON l.a=r.b",
    "SELECT * FROM l FULL OUTER JOIN r ON l.a=r.b",
    "DROP TABLE l",
    "DROP TABLE r",
    "DROP TABLE IF EXISTS procedure_call_data",
    "CREATE TABLE procedure_call_data(col INT)",
    "CREATE OR REPLACE PROCEDURE ins (a INT) AS\nBEGIN\nINSERT INTO procedure_call_data VALUES (a,\nEND",
    "CREATE OR REPLACE PROCEDURE ins100 () AS\nBEGIN\nINSERT INTO procedure_call_data VALUES (100,\nEND",
    "CALL ins(1)",
    "CALL ins(2)",
    "CALL ins100()",
    "SELECT * FROM procedure_call_data ORDER BY col",
    "DROP TABLE procedure_call_data",
    "SELECT \"{}\"",
    "SELECT \"{d \\'2001-10-1\\' }\"",
    "SELECT \"{ts \\'2001-10-1 01:10:10\\'  }\"",
    "SELECT \"{fn CONCAT({fn UCASE(col)}, RTRIM(LTRIM(\\'  !  \\'))) }\"",
    "SELECT \"{oj l LEFT OUTER JOIN r ON l.a=r.b  }\"",
    "SELECT \"{?=call ins(2) }\"",
    "SELECT \"{{\\\\}}\"",
    "SELECT \"{\\\"{}\\\"}\"",
    "SELECT ('2001-10-1' :> DATE)",
    "SELECT ('01:10:10' :> TIME)",
    "SELECT ('2001-10-1 01:10:10' :> TIMESTAMP(6))",
    "SELECT ('2001-10-1 01:10:10' :> TIMESTAMP(6))",
    "SELECT ('2001-10-1 01:10:10' :> TIMESTAMP(6))",
    "SELECT 123 :> BIGINT , 123",
    "SELECT 123 :> BINARY,123",
    "SELECT 123 :> BIT,123",
    "SELECT 123 :> CHAR,123",
    "SELECT 123 :> CHAR,123",
    "SELECT 123 :> DATE  ,123",
    "SELECT 123 :> DECIMAL  ,123",
    "SELECT 123 :> DOUBLE , 123",
    "SELECT 123 :> FLOAT , 123",
    "SELECT 123 :> INTEGER , 123",
    "SELECT 123 :> LONGBLOB , 123",
    "SELECT 123 :> LONGTEXT , 123",
    "SELECT 123 :> NUMERIC , 123",
    "SELECT 123 :> DOUBLE , 123",
    "SELECT 123 :> SMALLINT , 123",
    "SELECT 123 :> TIME , 123",
    "SELECT 123 :> TIMESTAMP(6) , 123",
    "SELECT 123 :> TINYINT , 123",
    "SELECT 123 :> BLOB , 123",
    "SELECT 123 :> TEXT , 123",
    "SELECT 123 :> CHAR , 123",
    "SELECT 123 :> LONGTEXT , 123",
    "SELECT 123 :> TEXT , 123",
    "SELECT 123 :> BIGINT , 123",
    "SELECT LENGTH('')*8, LENGTH('asd')*8",
    "SELECT CONCAT(CONCAT(LEFT('abcde', 2 - 1), 'eee'), RIGHT('abcde', CHAR_LENGTH('abcde') - 2 - 2 + 1))",
    "SELECT CONCAT(CONCAT(LEFT('abcde', 3 - 1), 'eee'), RIGHT('abcde', CHAR_LENGTH('abcde') - 10 - 3 + 1))",
    "SELECT CONCAT(CONCAT(LEFT('abcde', -1 - 1), 'eee'), RIGHT('abcde', CHAR_LENGTH('abcde') - 4 - -1 + 1))",
    "SELECT CONCAT(CONCAT(LEFT('ßßßßß', 2 - 1), 'ééé'), RIGHT('ßßßßß', CHAR_LENGTH('ßßßßß') - 2 - 2 + 1))",
    "SELECT LPAD('', CHAR_LENGTH('abc')*3, 'abc')",
    "SELECT LPAD('', CHAR_LENGTH('abc')*0, 'abc')",
    "SELECT LPAD('', CHAR_LENGTH('')*10, '')",
    "SELECT LPAD('', CHAR_LENGTH('ßß')*3, 'ßß')",
    "SELECT LPAD('', 3, ' ')",
    "SELECT LPAD('', 0, ' ')",
    "SELECT TIMESTAMPADD(MICROSECOND, 2000/1000, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(SECOND, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(MINUTE, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(HOUR, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(DAY, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(WEEK, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(MONTH, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(QUARTER, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(YEAR, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(MICROSECOND, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(SECOND, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(MINUTE, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(HOUR, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(DAY, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(WEEK, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(MONTH, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(QUARTER, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPADD(YEAR, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(MICROSECOND, '1998-01-01 01:01:01.001222', '1998-01-01 01:01:01.001225')*1000",
    "SELECT TIMESTAMPDIFF(SECOND, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(MINUTE, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(HOUR, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(DAY, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(WEEK, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(MONTH, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(QUARTER, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(YEAR, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(MICROSECOND, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(SECOND, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(MINUTE, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(HOUR, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(DAY, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(WEEK, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(MONTH, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(QUARTER, 2, '1998-01-01 01:01:01.001222')",
    "SELECT TIMESTAMPDIFF(YEAR, 2, '1998-01-01 01:01:01.001222')",
    "SELECT CONCAT(\"asd\"\"asd\", \"a\")",
    "SELECT CONCAT('asd''asd', 'a')",
    "SELECT CONCAT('\\'''', 'a')",
  };

  n = sizeof(queries)/sizeof(queries[0]);
  for (i = 0; i < n; i++)
  {
    SQLINTEGER len;

    CHECK_STMT_RC(Stmt, SQLNativeSql(Connection, (SQLCHAR*)queries[i], SQL_NTS, buffer, BUFFER_SIZE, &len));
    is_num(len, strlen(expected_results[i]));
    IS_STR(buffer, expected_results[i], len);

    if (!is_ansi_driver())
    {
      len = 0;
      CHECK_STMT_RC(Stmt, SQLNativeSqlW(Connection, CW(queries[i]), SQL_NTS, bufferW, BUFFER_SIZE*sizeof(SQLWCHAR), &len));
      is_num(len, sqlwcharlen(CW(expected_results[i])));
      IS_WSTR(bufferW, CW(expected_results[i]), len+1);
    }
  }
  return OK;
}

// sql_native_sql_buffers checks that SQLNativeSql handle input and output buffers correctly
//
ODBC_TEST(sql_native_sql_buffers) {
  int len;
  SQLCHAR buffer[BUFFER_SIZE];

    // This test failed on iODBC Unicode driver type
    if (iOdbc() && is_unicode_driver()) {
        return OK;
    }

  // Terminate the input statement by the number of bytes
  //
  CHECK_STMT_RC(Stmt, SQLNativeSql(Connection, (SQLCHAR*)"SELECT 1 some incorrect query suffix", 8, buffer, BUFFER_SIZE, &len));
  is_num(len, 8);
  IS_STR(buffer, "SELECT 1", 9);

  // Terminate the input statement by null character
  //
  CHECK_STMT_RC(Stmt, SQLNativeSql(Connection, (SQLCHAR*)"SELECT 1", SQL_NTS, buffer, BUFFER_SIZE, &len));
  is_num(len, 8);
  IS_STR(buffer, "SELECT 1", 9);

  // Should return the correct length even if output buffer is null
  //
  CHECK_STMT_RC(Stmt, SQLNativeSql(Connection, (SQLCHAR*)"SELECT 1", SQL_NTS, NULL, 0, &len));
  is_num(len, 8);

  // Should terminate the output buffer with null character and correctly truncate it
  //
  CHECK_STMT_RC(Stmt, SQLNativeSql(Connection, (SQLCHAR*)"SELECT 1", SQL_NTS, buffer, 8, &len));
  is_num(len, 8);
  IS_STR(buffer, "SELECT ", 8);

  CHECK_STMT_RC(Stmt, SQLNativeSql(Connection, (SQLCHAR*)"SELECT 1", SQL_NTS, buffer, 3, &len));
  is_num(len, 8);
  IS_STR(buffer, "SE", 3);

  return OK;
}

ODBC_TEST(sql_native_sql_buffers_unicode) {
    int len;
    SQLWCHAR buffer[BUFFER_SIZE];

    // This test failed on iODBC Ansi driver type
    if (iOdbc() && is_ansi_driver()) {
        return OK;
    }

    // Terminate the input statement by the number of bytes
    //
    CHECK_STMT_RC(Stmt, SQLNativeSqlW(Connection, CW("SELECT 1 some incorrect query suffix"), 8, buffer, BUFFER_SIZE, &len));
    is_num(len, 8);
    IS_WSTR(buffer, CW("SELECT 1"), 9);

    // Terminate the input statement by null character
    //
    CHECK_STMT_RC(Stmt, SQLNativeSqlW(Connection, CW("SELECT 1"), SQL_NTS, buffer, BUFFER_SIZE, &len));
    is_num(len, 8);
    IS_WSTR(buffer, CW("SELECT 1"), 9);

    // Should return the correct length even if output buffer is null
    //
    CHECK_STMT_RC(Stmt, SQLNativeSqlW(Connection, CW("SELECT 1"), SQL_NTS, NULL, 0, &len));
    is_num(len, 8);

    // Should terminate the output buffer with null character and correctly truncate it
    //
    CHECK_STMT_RC(Stmt, SQLNativeSqlW(Connection, CW("SELECT 1"), SQL_NTS, buffer, 8, &len));
    is_num(len, 8);
    IS_WSTR(buffer, CW("SELECT "), 8);

    CHECK_STMT_RC(Stmt, SQLNativeSqlW(Connection, CW("SELECT 1"), SQL_NTS, buffer, 3, &len));
    is_num(len, 8);
    IS_WSTR(buffer, CW("SE"), 3);

    return OK;
}

// sql_native_sql_errors checks that SQLNativeSql returns appropriate errors for invalid statements
//
ODBC_TEST(sql_native_sql_errors) {
  int i, n;
  SQLCHAR buffer[BUFFER_SIZE];
  SQLWCHAR bufferW[BUFFER_SIZE];
  char * queries[24] = {
    "{",
    "SELECT {fn \"USER\"()}",
    "SELECT {fn 'USER'()}",
    "{call    }",
    "SELECT {d '2001-04-11'",
    "SELECT '2001-04-11'}",
    "SELECT {d {d }}} {",
    "SELECT {d } {d } {d } }{",
    "SELECT {d {d {d {d } } } {d }",
    "SELECT } } {d {d {d {d } } } {d }",
    "SELECT }",
    "SELECT {{d} {d}} }",
    "SELECT 'Name' LIKE '\\%AAA%' {escape '\\'}",
    "SELECT {guid 'nnnnnnnn-nnnn-nnnn-nnnn-nnnnnnnnnnnn'}",
    "SELECT {INTERVAL 1 day}",
    "SELECT {fn CONVERT(123 SQL_BIGINT) }",
    "SELECT {fn CONVERT(123, SQL_BIGINT }",
    "SELECT {fn CONVERT(1,   ) }",
    "SELECT {fn CONVERT 123, SQL_BIGINT) }",
    "SELECT {fn CONVERT 123, SQL_BIGINT }",
    "SELECT {fn CONVERT 123 }",
    "SELECT {fn CONVERT(, SQL_BIGINT) }",
    "SELECT {fn CONVERT(  ,   ) }",
    "SELECT {fn CONVERT(  ,  SQL_BIGINT ) }",
  };

  n = sizeof(queries)/sizeof(queries[0]);
  for (i = 0; i < n; i++) {
    int len;
    EXPECT_DBC(Stmt, SQLNativeSql(Connection, (SQLCHAR *)queries[i], SQL_NTS, buffer, BUFFER_SIZE, &len), SQL_ERROR);

    if (!is_ansi_driver())
    {
      len = 0;
      EXPECT_DBC(Stmt, SQLNativeSqlW(Connection, CW(queries[i]), SQL_NTS, bufferW, BUFFER_SIZE*sizeof(SQLWCHAR), &len), SQL_ERROR);
    }
  }

  return OK;
}

#define BUFF_SIZE 512
ODBC_TEST(decimal_conversion)
{
  SQLHANDLE Hdbc, Hstmt;
  SQLCHAR   conn[BUFF_SIZE];
  SQLCHAR res[BUFF_SIZE];
  char query[BUFF_SIZE];
  char *value = "123451234512.1234";
  sprintf(query, "SELECT {fn ConVErT(%s, SQL_decIMAL )   }", value);
  OK_SIMPLE_STMT(Stmt, query);
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, &res, BUFF_SIZE, NULL));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  // by default the precision is 10 and scale is 0
  IS_STR(res, "9999999999", strlen("9999999999") + 1);

  sprintf((char *)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;APP={SingleStore Power BI Direct Query Connector}",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema);
  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &Hdbc));
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(Hdbc, SQLAllocHandle(SQL_HANDLE_STMT, Hdbc, &Hstmt));
  OK_SIMPLE_STMT(Hstmt, query);
  CHECK_STMT_RC(Hstmt, SQLBindCol(Hstmt, 1, SQL_C_CHAR, &res, BUFF_SIZE, NULL));
  CHECK_STMT_RC(Hstmt, SQLFetch(Hstmt));

  // when APP is "SingleStore Power BI Direct Query Connector" SQL_DECIMAL is converted to DOUBLE
  IS_STR(res, value, strlen(value) + 1);

  return OK;
}
#undef BUFF_SIZE

MA_ODBC_TESTS my_tests[]=
{
  {date_literal, "date_literal", NORMAL, ALL_DRIVERS},
  {time_literal, "time_literal", NORMAL, ALL_DRIVERS},
  {timestamp_literal, "timestamp_literal", NORMAL, ALL_DRIVERS},
  {scalar_function, "scalar_function", NORMAL, ALL_DRIVERS},
  {outer_join, "outer_join", NORMAL, ALL_DRIVERS},
  {procedure_call, "procedure_call", NORMAL, ALL_DRIVERS},
  {wrong_brackets_sequence, "wrong_brackets_sequence", NORMAL, ALL_DRIVERS},
  {strings_with_escape_sequences, "strings_with_escape_sequences", NORMAL, ALL_DRIVERS},
  {unsupported_escape_sequence, "unsupported_escape_sequence", NORMAL, ALL_DRIVERS},
  {uppercase, "uppercase", NORMAL, ALL_DRIVERS},
  {convert_failures, "convert_failures", NORMAL, ALL_DRIVERS},
  {convert, "convert", NORMAL, ALL_DRIVERS},
  {bit_length, "bit_length", NORMAL, ALL_DRIVERS},
  {insert, "insert", NORMAL, ANSI_DRIVER},
  {repeat, "repeat", NORMAL, ANSI_DRIVER},
  {space, "space", NORMAL, ALL_DRIVERS},
  {timestamp_add, "timestamp_add", NORMAL, ALL_DRIVERS},
  {timestamp_diff, "timestamp_diff", NORMAL, ALL_DRIVERS},
  {quotes, "quotes", NORMAL, ALL_DRIVERS},
  {ansi_quotes, "ansi_quotes", NORMAL, ALL_DRIVERS},
  {ansi_quotes_to_fix, "ansi_quotes_to_fix", TO_FIX, ALL_DRIVERS},
  {sql_native_sql, "sql_native_sql", NORMAL, ALL_DRIVERS},
  {sql_native_sql_buffers, "sql_native_sql_buffers_ansi", NORMAL, ALL_DRIVERS},
  {sql_native_sql_buffers_unicode, "sql_native_sql_buffers_unicode", NORMAL, ALL_DRIVERS},
  {sql_native_sql_errors, "sql_native_sql_errors", NORMAL, ALL_DRIVERS},
  {decimal_conversion, "decimal_conversion", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
#undef BUFFER_SIZE
