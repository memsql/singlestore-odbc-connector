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

ODBC_TEST(errors)
{
  ERR_SIMPLE_STMT(Stmt, "Some wrong query");
  // invalid handle
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, NULL, 1, NULL, NULL, NULL, 0, NULL) != SQL_INVALID_HANDLE,
          "Expected invalid handle error")

  // error
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, -1, NULL, NULL, NULL, 0, NULL) != SQL_ERROR,
          "Expected error")
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 0, NULL, NULL, NULL, 0, NULL) != SQL_ERROR,
          "Expected error")
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, NULL, NULL, NULL, -1, NULL) != SQL_ERROR,
          "Expected error")

  // no data error
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 2, NULL, NULL, NULL, 0, NULL) != SQL_NO_DATA,
          "Expected no data error")

  return OK;
}

#define BUFF_SIZE 1024
#define STATE_SIZE 5
int CheckRec(SQLSMALLINT handleType,
             SQLHANDLE handle,
             SQLSMALLINT recNumber,
             const char *expectedState,
             SQLINTEGER expectedNativeError,
             const char *expectedMessage)
{
  SQLCHAR state[BUFF_SIZE];
  SQLWCHAR stateW[BUFF_SIZE];
  SQLINTEGER nativeError;
  SQLCHAR message[BUFF_SIZE];
  SQLWCHAR messageW[BUFF_SIZE];
  SQLSMALLINT messageLength;

  CHECK_HANDLE_RC(handleType, handle, SQLGetDiagRec(handleType, handle, recNumber, state, &nativeError, message, BUFF_SIZE, &messageLength));
  IS_STR(state, expectedState, STATE_SIZE+1);
  is_num(nativeError, expectedNativeError);
  is_num(messageLength, strlen((char *)expectedMessage));
  IS_STR(message, expectedMessage, messageLength+1);

  CHECK_HANDLE_RC(handleType, handle, SQLGetDiagRecW(handleType, handle, recNumber, stateW, &nativeError, messageW, BUFF_SIZE, &messageLength));
  IS_WSTR(stateW, CW(expectedState), STATE_SIZE+1);
  is_num(nativeError, expectedNativeError);
  is_num(messageLength, strlen((char *)expectedMessage));
  IS_WSTR(messageW, CW(expectedMessage), messageLength+1);

  return OK;
}
#undef BUFF_SIZE

ODBC_TEST(sql_get_diag_rec)
{
  SQLHANDLE Hdbc;
  SQLCHAR catalog[2];
  SQLINTEGER catalogLength;
  char version[10];
  char errMsg[512];
  char errMsg2[512];

  // get S2 version
  OK_SIMPLE_STMT(Stmt, "SELECT @@memsql_version");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  my_fetch_str(Stmt, (SQLCHAR *)version, 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));


  // wrong SQL statement
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, NULL, NULL, NULL, 0, NULL) != SQL_NO_DATA,
          "Expected no data error")
  ERR_SIMPLE_STMT(Stmt, "Some wrong query");
  sprintf(errMsg, "%s[%s]You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'Some wrong query' at line 1", MARIADB_ODBC_ERR_PREFIX, version);
  IS_OK(CheckRec(SQL_HANDLE_STMT, Stmt, 1, "42000", 1064, errMsg))

  // output buffer is too small
  FAIL_IF(SQLGetDiagRec(SQL_HANDLE_DBC, Connection, 1, NULL, NULL, NULL, 0, NULL) != SQL_NO_DATA,
          "Expected no data error")
  FAIL_IF(SQLGetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG,
                             catalog, sizeof(catalog), &catalogLength) != SQL_SUCCESS_WITH_INFO, "expected SUCCESS_WITH_INFO");
  if ((iOdbc() || UnixOdbc()))
  {
    sprintf(errMsg, "%sString data, right-truncated", MARIADB_ODBC_ERR_PREFIX);
  } else
  {
    sprintf(errMsg, "[Microsoft][ODBC Driver Manager] Data truncated");
  }
  IS_OK(CheckRec(SQL_HANDLE_DBC, Connection, 1, "01004", 0, errMsg));

  // wrong DSN
  SQLAllocHandle(SQL_HANDLE_DBC, Env, &Hdbc);
  FAIL_IF(SQLConnect(Hdbc, (SQLCHAR *)"Wrong DSN", SQL_NTS, my_uid, SQL_NTS, my_pwd, SQL_NTS) != SQL_ERROR, "Failure expected");
  if (UnixOdbc())
  {
    sprintf(errMsg, "[unixODBC][Driver Manager]Data source name not found, and no default driver specified");
    sprintf(errMsg2, "[unixODBC][Driver Manager]Data source name not found and no default driver specified");

    // For Centos:8 and Debian:9-10 we are using the same version of UnixODBC but we are getting different error messages
    IS_OK(CheckRec(SQL_HANDLE_DBC, Hdbc, 1, "IM002", 0, errMsg) || CheckRec(SQL_HANDLE_DBC, Hdbc, 1, "IM002", 0, errMsg2));
  } else
  {
    sprintf(errMsg, iOdbc() ? "[iODBC][Driver Manager]Data source name not found and no default driver specified. Driver could not be loaded" :
                    "[Microsoft][ODBC Driver Manager] Data source name not found and no default driver specified");
    IS_OK(CheckRec(SQL_HANDLE_DBC, Hdbc, 1, "IM002", 0, errMsg));
  }

  return OK;
}

MA_ODBC_TESTS my_tests[]=
{
  {errors, "errors", NORMAL, ALL_DRIVERS},
  {sql_get_diag_rec, "sql_get_diag_rec", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
