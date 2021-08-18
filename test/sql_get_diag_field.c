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

ODBC_TEST(sql_diag_cursor_row_count)
{
  SQLLEN rowCount;
  SQLHANDLE apd;
  SQLHANDLE Hdbc, HdbcNoCache;
  SQLCHAR conn[512];
  SQLHANDLE Hstmt, HstmtNoCache;
  SQLSMALLINT recordNumber = iOdbc() ? 1 : 0;
  int i;

  sprintf((char *)conn, "DSN=%s;DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;NO_CACHE=1;OPTIONS=%lu;%s;%s",
          my_dsn, my_drivername, my_servername, my_uid, my_pwd, my_schema, my_options|32 /* MADB_OPT_FLAG_DYNAMIC_CURSOR */, ma_strport, add_connstr);
  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &HdbcNoCache));
  CHECK_DBC_RC(HdbcNoCache, SQLDriverConnect(HdbcNoCache, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(HdbcNoCache, SQLAllocHandle(SQL_HANDLE_STMT, HdbcNoCache, &HstmtNoCache));

  sprintf((char *)conn, "DSN=%s;DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;OPTIONS=%lu;%s;%s",
          my_dsn, my_drivername, my_servername, my_uid, my_pwd, my_schema, my_options|32, /* MADB_OPT_FLAG_DYNAMIC_CURSOR */ma_strport, add_connstr);
  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &Hdbc));
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(Hdbc, SQLAllocHandle(SQL_HANDLE_STMT, Hdbc, &Hstmt));

  // SQLGetDiagField with a DiagIdentifier of SQL_DIAG_CURSOR_ROW_COUNT on other than a statement handle will return SQL_ERROR.
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_ENV, Env, 0, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL) != SQL_ERROR,
          "SQLGetDiagField should return SQL_ERROR with SQL_DIAG_CURSOR_ROW_COUNT and env handle");
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_DBC, Connection, 0, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL) != SQL_ERROR,
          "SQLGetDiagField should return SQL_ERROR with SQL_DIAG_CURSOR_ROW_COUNT and connection handle");
  CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_APP_PARAM_DESC, &apd, SQL_IS_POINTER, NULL));
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_DESC, apd, 0, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL) != SQL_ERROR,
          "SQLGetDiagField should return SQL_ERROR with SQL_DIAG_CURSOR_ROW_COUNT and descriptor handle");

  // Test SQL_DIAG_CURSOR_ROW_COUNT with simple select and different cursors
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS sql_diag_cursor_row_count");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE sql_diag_cursor_row_count(id INT, text VARCHAR(16))");
  OK_SIMPLE_STMT(Stmt, "INSERT INTO sql_diag_cursor_row_count VALUES (1, 'a'), (2, 'b')");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  SQLPOINTER cursors[3] = {(SQLPOINTER) SQL_CURSOR_FORWARD_ONLY, (SQLPOINTER) SQL_CURSOR_STATIC, (SQLPOINTER) SQL_CURSOR_DYNAMIC};

  for (i= 0; i < 3; i++)
  {
    CHECK_STMT_RC(Hstmt, SQLSetStmtAttr(Hstmt, SQL_ATTR_CURSOR_TYPE, cursors[i], 0));
    OK_SIMPLE_STMT(Hstmt, "SELECT * FROM sql_diag_cursor_row_count");
    CHECK_STMT_RC(Hstmt, SQLFetch(Hstmt)); // TODO PLAT-5583
    CHECK_STMT_RC(Hstmt, SQLGetDiagField(SQL_HANDLE_STMT, Hstmt, recordNumber, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL));
    FAIL_IF_NE_INT(rowCount, 2,
                   "SQLGetDiagField with SQL_DIAG_CURSOR_ROW_COUNT should return row count for select statement");
    CHECK_STMT_RC(Hstmt, SQLFreeStmt(Hstmt, SQL_CLOSE));

    CHECK_STMT_RC(HstmtNoCache, SQLSetStmtAttr(HstmtNoCache, SQL_ATTR_CURSOR_TYPE, cursors[i], 0));
    OK_SIMPLE_STMT(HstmtNoCache, "SELECT * FROM sql_diag_cursor_row_count");
    CHECK_STMT_RC(HstmtNoCache, SQLFetch(HstmtNoCache)); // TODO PLAT-5583
    CHECK_STMT_RC(HstmtNoCache, SQLGetDiagField(SQL_HANDLE_STMT, HstmtNoCache, recordNumber, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL));
    CHECK_STMT_RC(HstmtNoCache, SQLFreeStmt(HstmtNoCache, SQL_CLOSE));
    if (cursors[i] == SQL_CURSOR_FORWARD_ONLY)
    {
      // FAIL_IF_NE_INT(rowCount, 0,
      //               "SQLGetDiagField with SQL_DIAG_CURSOR_ROW_COUNT should return 0 when cursor is SQL_CURSOR_FORWARD_ONLY and NO_CACHE option is used"); TODO PLAT-5583
    } else
    {
      FAIL_IF_NE_INT(rowCount, 2,
                     "SQLGetDiagField with SQL_DIAG_CURSOR_ROW_COUNT should return row count for select statement");
    }
  }

  // Test SQL_DIAG_CURSOR_ROW_COUNT with multistatement
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM sql_diag_cursor_row_count; SELECT 1");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt)); // TODO PLAT-5583
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, recordNumber, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL));
  FAIL_IF_NE_INT(rowCount, 2,
                 "SQLGetDiagField with SQL_DIAG_CURSOR_ROW_COUNT should return correct values for multistatement");

  CHECK_STMT_RC(Stmt, SQLMoreResults(Stmt));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt)); // TODO PLAT-5583
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, recordNumber, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL));
  FAIL_IF_NE_INT(rowCount, 1,
                 "SQLGetDiagField with SQL_DIAG_CURSOR_ROW_COUNT should return correct values for multistatement");

  EXPECT_STMT(Stmt, SQLMoreResults(Stmt), SQL_NO_DATA);
  EXPECT_STMT(Stmt, SQLFetch(Stmt), SQL_ERROR);
  if (iOdbc()) {
    EXPECT_STMT(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, recordNumber, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL), SQL_ERROR);
  } else {
    CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, recordNumber, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL));
    FAIL_IF_NE_INT(rowCount, 0,
                 "SQLGetDiagField with SQL_DIAG_CURSOR_ROW_COUNT should return 0 when no data left");
  }

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  // Test SQL_DIAG_CURSOR_ROW_COUNT with several parameter sets
  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *)"SELECT * FROM sql_diag_cursor_row_count WHERE id = ?", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)3, 0));
  SQLINTEGER ids[3] = {3, 2, 1};
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, ids, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt)); // TODO PLAT-5583
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, recordNumber, SQL_DIAG_CURSOR_ROW_COUNT, &rowCount, 0, NULL));
  // SELECT statement with several sets of parameters returns result only for the last set
  FAIL_IF_NE_INT(rowCount, 1,
                 "SQLGetDiagField with SQL_DIAG_CURSOR_ROW_COUNT should return correct values when SQL_ATTR_PARAMSET_SIZE is not 1");

  return OK;
}

ODBC_TEST(sql_diag_row_count)
{
  SQLLEN rowCount;
  SQLHANDLE apd;

  // SQLGetDiagField with a DiagIdentifier of SQL_DIAG_ROW_COUNT on other than a statement handle will return SQL_ERROR.
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_ENV, Env, 0, SQL_DIAG_ROW_COUNT, &rowCount, 0, NULL) != SQL_ERROR,
          "SQLGetDiagField should return SQL_ERROR with SQL_DIAG_ROW_COUNT and env handle");
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_DBC, Connection, 0, SQL_DIAG_ROW_COUNT, &rowCount, 0, NULL) != SQL_ERROR,
          "SQLGetDiagField should return SQL_ERROR with SQL_DIAG_ROW_COUNT and connection handle");
  CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_APP_PARAM_DESC, &apd, SQL_IS_POINTER, NULL));
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_DESC, apd, 0, SQL_DIAG_ROW_COUNT, &rowCount, 0, NULL) != SQL_ERROR,
          "SQLGetDiagField should return SQL_ERROR with SQL_DIAG_ROW_COUNT and descriptor handle");

  // Test SQL_DIAG_CURSOR_ROW_COUNT with simple insert/update/delete statements
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS sql_diag_cursor_row_count");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE sql_diag_cursor_row_count(id INT, text VARCHAR(16))");

  OK_SIMPLE_STMT(Stmt, "INSERT INTO sql_diag_cursor_row_count VALUES (1, 'a'), (2, 'b')");
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 0, SQL_DIAG_ROW_COUNT, &rowCount, 0, NULL));
  FAIL_IF_NE_INT(rowCount, 2,
                 "SQLGetDiagField with SQL_DIAG_ROW_COUNT should return correct values for insert");

  OK_SIMPLE_STMT(Stmt, "DELETE FROM sql_diag_cursor_row_count WHERE id = 1");
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 0, SQL_DIAG_ROW_COUNT, &rowCount, 0, NULL));
  FAIL_IF_NE_INT(rowCount, 1,
                 "SQLGetDiagField with SQL_DIAG_ROW_COUNT should return correct values for delete");

  OK_SIMPLE_STMT(Stmt, "UPDATE sql_diag_cursor_row_count SET id=4 WHERE text='b'");
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 0, SQL_DIAG_ROW_COUNT, &rowCount, 0, NULL));
  FAIL_IF_NE_INT(rowCount, 1,
                 "SQLGetDiagField with SQL_DIAG_ROW_COUNT should return correct values for update");

  // SQLGetDiagField with a DiagIdentifier of SQL_DIAG_ROW_COUNT after SQLPrepare and before SQLExecute should not crash
  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *)"INSERT INTO sql_diag_cursor_row_count VALUES (3, 'a'), (4, 'b')", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 0, SQL_DIAG_ROW_COUNT, &rowCount, 0, NULL));
  FAIL_IF_NE_INT(rowCount, 0,
                 "SQLGetDiagField with SQL_DIAG_ROW_COUNT should return 0 before SQLExecute");
  CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 0, SQL_DIAG_ROW_COUNT, &rowCount, 0, NULL));
  FAIL_IF_NE_INT(rowCount, 2,
                 "SQLGetDiagField with SQL_DIAG_ROW_COUNT should return correct values for insert");

  return OK;
}

#define BUFF_SIZE 512
int CheckChar(SQLSMALLINT HandleType, SQLHANDLE Handle, SQLSMALLINT RecNumber, SQLUSMALLINT DiagIdentifier, char *CorrectValue) {
  // DM incorrectly handles string conversions for SQLGetDiagField(W) in the case,
  // if ANSI driver tries to use Unicode versions of the SQLGetDiagField function
  // and vice versa
  // It's ok, because users shouldn't use Unicode functions with ANSI driver
  // and ANSI functions with Unicode driver
  // Thus, we are testing only SQLGetDiagField with ANSI driver and SQLGetDiagFieldW with Unicode one
  if (is_ansi_driver())
  {
    SQLCHAR string_value[BUFF_SIZE];
    SQLSMALLINT length = 0;

    CHECK_STMT_RC(Stmt, SQLGetDiagField(HandleType, Handle, RecNumber, DiagIdentifier, string_value, BUFF_SIZE, &length));
    is_num(length, strlen(CorrectValue));
    IS_STR(string_value, CorrectValue, length + 1);
  } else
  {
    SQLWCHAR stringw_value[BUFF_SIZE];
    SQLSMALLINT length = 0;

    CHECK_STMT_RC(Stmt, SQLGetDiagFieldW(HandleType, Handle, RecNumber, DiagIdentifier, stringw_value, BUFF_SIZE*sizeof(SQLWCHAR), &length));
    if (UnixOdbc())
    {
      // in Unix ODBC 2.3.1 SQLGetDiagFieldW returned length in characters
      // this bug was fixed in 2.3.2 version
      IS(length == strlen(CorrectValue)*sizeof(SQLWCHAR) || length == strlen(CorrectValue));
    } else
    {
      IS(length == strlen(CorrectValue)*sizeof(SQLWCHAR));
    }
    IS_WSTR(stringw_value, CW(CorrectValue), strlen(CorrectValue)+1);
  }

  return OK;
}
#undef BUFF_SIZE

ODBC_TEST(sql_diag_record_fields)
{
  SQLHANDLE Hdbc;
  SQLUSMALLINT fExists;
  SQLINTEGER nativeErrorCode;
  SQLLEN diagRowNumber;
  SQLINTEGER diagColumnNumber;
  SQLINTEGER diagNumber;
  char version[10];
  char errMsg[512];

  // get S2 version
  OK_SIMPLE_STMT(Stmt, "SELECT @@memsql_version");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  my_fetch_str(Stmt, (SQLCHAR *)version, 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));


  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 0, SQL_DIAG_NUMBER, &diagNumber, 0, NULL));
  FAIL_IF_NE_INT(diagNumber, 0,
                 "SQLGetDiagField should return 0 for SQL_DIAG_NUMBER if no errors occur");


  // Execute some wrong query to get SQLSTATE defined by Open Group and ISO call-level interface.
  // Error code is 42000
  // native error is 1064
  ERR_SIMPLE_STMT(Stmt, "Some wrong query");
  IS_OK(CheckChar(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_SQLSTATE, "42000"));
  IS_OK(CheckChar(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_CLASS_ORIGIN, "ISO 9075"));
  // IS_OK(CheckChar(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_SUBCLASS_ORIGIN, "ISO 9075")); TODO PLAT-5595

  sprintf(errMsg, "%s[%s]You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'Some wrong query' at line 1", MARIADB_ODBC_ERR_PREFIX, version);
  IS_OK(CheckChar(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_MESSAGE_TEXT, errMsg));
  // IS_OK(CheckChar(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_CONNECTION_NAME, "")); TODO PLAT-5597
  // IS_OK(CheckChar(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_SERVER_NAME, (char *)my_dsn)); TODO PLAT-5597
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 0, SQL_DIAG_NUMBER, &diagNumber, 0, NULL));
  FAIL_IF_NE_INT(diagNumber, 1,
                 "SQLGetDiagField should return 1 for SQL_DIAG_NUMBER if some errors occur");
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_NATIVE, &nativeErrorCode, 0, NULL));
  FAIL_IF_NE_INT(nativeErrorCode, 1064,
                 "SQLGetDiagField should return correct native error code");
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_COLUMN_NUMBER, &diagColumnNumber, 0, NULL));
  FAIL_IF_NE_INT(diagColumnNumber, SQL_COLUMN_NUMBER_UNKNOWN,
                 "SQLGetDiagField should return correct column number");
  CHECK_STMT_RC(Stmt, SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_ROW_NUMBER, &diagRowNumber, 0, NULL));
  FAIL_IF_NE_INT(diagRowNumber, SQL_ROW_NUMBER_UNKNOWN,
                 "SQLGetDiagField should return correct row number");

  // Connect using wrong DSN to get ODBC-specific SQLSTATE
  // Error code is IM002
  SQLAllocHandle(SQL_HANDLE_DBC, Env, &Hdbc);
  FAIL_IF(SQLConnect(Hdbc, (SQLCHAR *)"Wrong DSN", SQL_NTS, my_uid, SQL_NTS, my_pwd, SQL_NTS) != SQL_ERROR, "Failure expected");
  IS_OK(CheckChar(SQL_HANDLE_DBC, Hdbc, 1, SQL_DIAG_SQLSTATE, "IM002"));
  IS_OK(CheckChar(SQL_HANDLE_DBC, Hdbc, 1, SQL_DIAG_CLASS_ORIGIN, "ODBC 3.0"));
  IS_OK(CheckChar(SQL_HANDLE_DBC, Hdbc, 1, SQL_DIAG_SUBCLASS_ORIGIN, "ODBC 3.0"));
  if (!iOdbc()) {
    IS_OK(CheckChar(SQL_HANDLE_DBC, Hdbc, 1, SQL_DIAG_SERVER_NAME, ""));
  }
  CHECK_DBC_RC(Hdbc, SQLGetDiagField(SQL_HANDLE_DBC, Hdbc, 1, SQL_DIAG_NATIVE, &nativeErrorCode, 0, NULL));
  FAIL_IF_NE_INT(nativeErrorCode, 0,
                 "SQLGetDiagField should return 0 if no native error code presented");

  // Set connection attribute that can't be changed
  // Error code is 01S02
  // for SQL_DIAG_CLASS_ORIGIN it should return "ISO 9075" but for SQL_DIAG_SUBCLASS_ORIGIN it should return "ODBC 3.0"
  FAIL_IF(SQLSetConnectAttr(Connection, SQL_ATTR_ACCESS_MODE, (SQLPOINTER)SQL_MODE_READ_ONLY, 0) != SQL_SUCCESS_WITH_INFO, "Success with info expected");
  IS_OK(CheckChar(SQL_HANDLE_DBC, Connection, 1, SQL_DIAG_SQLSTATE, "01S02"));
  IS_OK(CheckChar(SQL_HANDLE_DBC, Connection, 1, SQL_DIAG_CLASS_ORIGIN, "ISO 9075"));
  // IS_OK(CheckChar(SQL_HANDLE_DBC, Connection, 1, SQL_DIAG_SUBCLASS_ORIGIN, "ODBC 3.0")); TODO PLAT-5595

  // Old version of unixODBC DM doesn't check that FunctionId is valid
  if (!UnixOdbc())
  {
    // Execute SQLGetFunctions with wrong FunctionId
    // Error code is HY095
    // for SQL_DIAG_CLASS_ORIGIN it should return "ISO 9075" but for SQL_DIAG_SUBCLASS_ORIGIN it should return "ODBC 3.0"
    FAIL_IF(SQLGetFunctions(Connection, -100, &fExists) != SQL_ERROR, "Failure expected");
    IS_OK(CheckChar(SQL_HANDLE_DBC, Connection, 1, SQL_DIAG_SQLSTATE, "HY095"));
    IS_OK(CheckChar(SQL_HANDLE_DBC, Connection, 1, SQL_DIAG_CLASS_ORIGIN, "ISO 9075"));
    // IS_OK(CheckChar(SQL_HANDLE_DBC, Connection, 1, SQL_DIAG_SUBCLASS_ORIGIN, "ODBC 3.0")); TODO PLAT-5595
  }
  return OK;
}

#define BUFF_SIZE 256
ODBC_TEST(errors)
{
  ERR_SIMPLE_STMT(Stmt, "Some wrong query");
  SQLCHAR state[100];

  // invalid handle
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_STMT, NULL, 1, SQL_DIAG_SQLSTATE, state, BUFF_SIZE, NULL) != SQL_INVALID_HANDLE,
          "Expected invalid handle error");

  // error
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 1, -1, &state, BUFF_SIZE, NULL) != SQL_ERROR,
          "Expected error");
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_STMT, Stmt, -1, SQL_DIAG_SQLSTATE, state, BUFF_SIZE, NULL) != SQL_ERROR,
          "Expected error");
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 1, SQL_DIAG_SQLSTATE, state, -1, NULL) != SQL_ERROR,
          "Expected error");

  // no data
  FAIL_IF(SQLGetDiagField(SQL_HANDLE_STMT, Stmt, 2, SQL_DIAG_SQLSTATE, state, BUFF_SIZE, NULL) != SQL_NO_DATA,
          "Expected no data error");

  return OK;
}
#undef BUFF_SIZE

MA_ODBC_TESTS my_tests[]=
{
  {sql_diag_cursor_row_count, "sql_diag_cursor_row_count", NORMAL, ALL_DRIVERS},
  {sql_diag_row_count, "sql_diag_row_count", TO_FIX, ALL_DRIVERS}, // TODO PLAT-5590
  {sql_diag_record_fields, "sql_diag_record_fields", NORMAL, ALL_DRIVERS},
  {errors, "errors", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
