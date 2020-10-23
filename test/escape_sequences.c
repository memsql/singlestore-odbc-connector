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

  size_t n = 8;
  char **end = queries + n;
  for (char **query = queries; query < end; query++) {
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

  size_t n = 8;
  char **end = escapeSequences + n;
  for (char **escapeSequence = escapeSequences; escapeSequence < end; escapeSequence++) {
    // build "SELECT "..."" query
    char query[128];
    char *queryIterator = query;
    strcpy(queryIterator, "SELECT \"");
    queryIterator += 8;

    // escape escapeSequence
    for (char *c = *escapeSequence; *c != '\0'; c++) {
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

  size_t n = 3;
  char **end = queries + n;
  for (char **query = queries; query < end; query++) {
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

  size_t n = 5;
  char **end = queries + n;
  for (char **query = queries; query < end; query++) {
    OK_SIMPLE_STMT(Stmt, *query);
  }

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
  {NULL, NULL}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}