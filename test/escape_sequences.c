#include "tap.h"

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
  char buffer[128];
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS scalar_function");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE scalar_function(col TEXT)");

  OK_SIMPLE_STMT(Stmt, "INSERT INTO scalar_function VALUES('a'), ('abc'), ('asasasas   asdads\n\r ;? asd')");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn CONCAT({fn UCASE(col)}, RTRIM(LTRIM('  !  '))) } FROM scalar_function ORDER BY col");
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
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LOCATE('abcabcd', 'abc')}");
  OK_SIMPLE_STMT(Stmt, "SELECT {fn LOCATE('abcabcd', 'abc', 2)}");
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
    char query[128];
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
    char buffer[128];
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

  size_t n = 10;
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
  char buffer[128];
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
  char buffer[128];
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
  char buffer[128];
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
  char buffer[128];
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
  char buffer[128];
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

ODBC_TEST(ansi_quotes) {
  OK_SIMPLE_STMT(Stmt, "SET sql_mode='ANSI_QUOTES'")
  OK_SIMPLE_STMT(Stmt, "SELECT {fn \"COS\"(1)}");
  OK_SIMPLE_STMT(Stmt, "SET sql_mode=''")
  return OK;
}

MA_ODBC_TESTS my_tests[]=
{
  {date_literal, "date_literal", NORMAL},
  {time_literal, "time_literal", NORMAL},
  {timestamp_literal, "timestamp_literal", NORMAL},
  {scalar_function, "scalar_function", NORMAL},
  {outer_join, "outer_join", NORMAL},
  {procedure_call, "procedure_call", NORMAL},
  {wrong_brackets_sequence, "wrong_brackets_sequence", NORMAL},
  {strings_with_escape_sequences, "strings_with_escape_sequences", NORMAL},
  {unsupported_escape_sequence, "unsupported_escape_sequence", NORMAL},
  {uppercase, "uppercase", NORMAL},
  {convert_failures, "convert_failures", NORMAL},
  {convert, "convert", NORMAL},
  {bit_length, "bit_length", NORMAL},
  {insert, "insert", NORMAL},
  {repeat, "repeat", NORMAL},
  {space, "space", NORMAL},
  {timestamp_add, "timestamp_add", NORMAL},
  {timestamp_diff, "timestamp_diff", NORMAL},
  {ansi_quotes, "ansi_quotes", TO_FIX},
  {NULL, NULL}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}