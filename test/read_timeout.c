/*************************************************************************************
  Copyright (c) 2026 SingleStore, Inc.

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

#include <time.h>

/**
  DSN READ_TIMEOUT must bound SQLExecute when the server does not respond,
  instead of blocking on the socket read forever.
*/
ODBC_TEST(t_read_timeout_dsn)
{
  SQLHDBC hdbc= NULL;
  SQLHSTMT hstmt= NULL;
  SQLCHAR conn[1024];
  SQLRETURN rc;
  time_t started, finished;

  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &hdbc));

  _snprintf((char *)conn, sizeof(conn),
            "DRIVER={%s};SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;%s;"
            "READ_TIMEOUT=2;WRITE_TIMEOUT=2;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema,
            ma_strport, add_connstr);

  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS, NULL, 0, NULL,
                                      SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));

  /* Queries finishing within the timeout are unaffected */
  CHECK_STMT_RC(hstmt, SQLExecDirect(hstmt, (SQLCHAR *)"SELECT 1", SQL_NTS));
  CHECK_STMT_RC(hstmt, SQLFreeStmt(hstmt, SQL_CLOSE));

  started= time(NULL);
  rc= SQLExecDirect(hstmt, (SQLCHAR *)"SELECT SLEEP(30)", SQL_NTS);
  finished= time(NULL);

  FAIL_IF(rc != SQL_ERROR, "SQLExecDirect should fail when READ_TIMEOUT fires");
  FAIL_IF((finished - started) > 15,
          "SQLExecDirect took too long; READ_TIMEOUT did not interrupt the wait");
  FAIL_IF((finished - started) < 1,
          "SQLExecDirect returned too quickly to be a real timeout");

  SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  SQLDisconnect(hdbc);
  SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
  return OK;
}

MA_ODBC_TESTS my_tests[]=
{
  {t_read_timeout_dsn, "t_read_timeout_dsn", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
