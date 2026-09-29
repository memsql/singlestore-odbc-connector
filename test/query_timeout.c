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

/*
  PLAT-8159: SQLExecute can hang indefinitely when the server stops responding
  mid-query (observed with ER_ROLLED_BACK_TRANSACTION / 1735 during
  CREATE TEMPORARY TABLE ... AS SELECT in a multi-statement transaction).

  The driver previously ignored SQL_ATTR_QUERY_TIMEOUT and never set
  MYSQL_OPT_READ_TIMEOUT, so a hung socket read waited forever.

  These tests verify:
  1) SQL_ATTR_QUERY_TIMEOUT is honored and SQLExecute returns (HYT00/08S01)
  2) DSN READ_TIMEOUT likewise bounds a hung/long query
  3) CREATE TEMPORARY TABLE AS SELECT inside a transaction surfaces a killed
     connection as SQL_ERROR (client must not hang on the in-flight execute)
*/

#include "tap.h"

#include <time.h>

static int reconnect_after_timeout(void)
{
  ODBC_Disconnect(Env, Connection, Stmt);
  return ODBC_Connect(&Env, &Connection, &Stmt);
}

/**
  SQL_ATTR_QUERY_TIMEOUT must interrupt a long-running query so SQLExecute
  returns instead of hanging forever.
*/
ODBC_TEST(t_query_timeout_attr)
{
  SQLRETURN rc;
  SQLULEN timeout= 2;
  SQLULEN got= 0;
  time_t started, finished;

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_QUERY_TIMEOUT,
                                     (SQLPOINTER)(SQLULEN)timeout, 0));
  CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_QUERY_TIMEOUT,
                                     &got, 0, NULL));
  is_num(got, timeout);

  started= time(NULL);
  rc= SQLExecDirect(Stmt, (SQLCHAR *)"SELECT SLEEP(30)", SQL_NTS);
  finished= time(NULL);

  FAIL_IF(rc != SQL_ERROR, "SQLExecDirect should fail when query timeout fires");
  /* Allow a little slack over the configured 2s timeout. */
  FAIL_IF((finished - started) > 15,
          "SQLExecDirect took too long; query timeout did not interrupt the wait");
  FAIL_IF((finished - started) < 1,
          "SQLExecDirect returned too quickly to be a real timeout");

  {
    SQLCHAR state[6]= {0};
    SQLINTEGER native= 0;
    SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH]= {0};
    CHECK_STMT_RC(Stmt, SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, state, &native,
                                      msg, sizeof(msg), NULL));
    diag("timeout diag: [%s] (%d) %s", state, native, msg);
    FAIL_IF(strcmp((char *)state, "HYT00") != 0 &&
            strcmp((char *)state, "08S01") != 0,
            "Expected HYT00 (query timeout) or 08S01 (server lost)");
  }

  IS(reconnect_after_timeout() == OK);
  return OK;
}

/**
  DSN READ_TIMEOUT must also bound SQLExecute when the app does not set
  SQL_ATTR_QUERY_TIMEOUT (typical for apps that never configured a timeout).
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
            "READ_TIMEOUT=2;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema,
            ma_strport, add_connstr);

  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS, NULL, 0, NULL,
                                      SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));

  started= time(NULL);
  rc= SQLExecDirect(hstmt, (SQLCHAR *)"SELECT SLEEP(30)", SQL_NTS);
  finished= time(NULL);

  FAIL_IF(rc != SQL_ERROR, "SQLExecDirect should fail when READ_TIMEOUT fires");
  FAIL_IF((finished - started) > 15,
          "SQLExecDirect took too long; READ_TIMEOUT did not interrupt the wait");

  SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  SQLDisconnect(hdbc);
  SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
  return OK;
}

/**
  Reproduce the GS Helios shape: multi-statement transaction with
  CREATE TEMPORARY TABLE ... AS SELECT. Kill the connection from another
  session while the statement is about to run / is in flight, and assert
  SQLExecute returns SQL_ERROR (does not hang).

  True ER_1735 (leaf disconnect) cannot be forced from a unit test; killing
  the client connection is the closest portable stand-in for "server side
  finished with an error / dropped the session while the client was waiting".
*/
ODBC_TEST(t_ctas_txn_killed_connection)
{
  SQLHENV henv2= NULL;
  SQLHDBC hdbc2= NULL;
  SQLHSTMT hstmt2= NULL;
  SQLINTEGER connection_id= 0, node_id= 0;
  SQLCHAR buf[256];
  SQLRETURN rc;
  time_t started, finished;

  /* Victim connection that will be killed. */
  IS(ODBC_Connect(&henv2, &hdbc2, &hstmt2) == OK);

  OK_SIMPLE_STMT(hstmt2, "SELECT CONNECTION_ID()");
  CHECK_STMT_RC(hstmt2, SQLFetch(hstmt2));
  connection_id= my_fetch_int(hstmt2, 1);
  CHECK_STMT_RC(hstmt2, SQLFreeStmt(hstmt2, SQL_CLOSE));

  _snprintf((char *)buf, sizeof(buf),
            "SELECT node_id FROM INFORMATION_SCHEMA.MV_PROCESSLIST WHERE id = %d",
            connection_id);
  OK_SIMPLE_STMT(hstmt2, buf);
  CHECK_STMT_RC(hstmt2, SQLFetch(hstmt2));
  node_id= my_fetch_int(hstmt2, 1);
  CHECK_STMT_RC(hstmt2, SQLFreeStmt(hstmt2, SQL_CLOSE));

  CHECK_DBC_RC(hdbc2, SQLSetConnectAttr(hdbc2, SQL_ATTR_AUTOCOMMIT,
                                        (SQLPOINTER)SQL_AUTOCOMMIT_OFF, 0));

  /* Mirror the customer pattern: work in a txn, then CTAS. */
  OK_SIMPLE_STMT(hstmt2, "DROP TABLE IF EXISTS t_plat8159_src");
  OK_SIMPLE_STMT(hstmt2, "CREATE TABLE t_plat8159_src (id INT, v VARCHAR(32))");
  OK_SIMPLE_STMT(hstmt2, "INSERT INTO t_plat8159_src VALUES (1, 'a'), (2, 'b')");
  CHECK_DBC_RC(hdbc2, SQLEndTran(SQL_HANDLE_DBC, hdbc2, SQL_COMMIT));

  OK_SIMPLE_STMT(hstmt2, "START TRANSACTION");
  OK_SIMPLE_STMT(hstmt2, "INSERT INTO t_plat8159_src VALUES (3, 'c')");

  /* Kill from the primary test connection before / as CTAS runs. */
  _snprintf((char *)buf, sizeof(buf), "KILL CONNECTION %d %d",
            connection_id, node_id);
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, buf, SQL_NTS));

  started= time(NULL);
  rc= SQLExecDirect(hstmt2, (SQLCHAR *)
                    "CREATE TEMPORARY TABLE t_plat8159_tmp AS "
                    "SELECT * FROM t_plat8159_src", SQL_NTS);
  finished= time(NULL);

  FAIL_IF(rc != SQL_ERROR,
          "CREATE TEMPORARY TABLE AS SELECT must return SQL_ERROR after kill");
  FAIL_IF((finished - started) > 30,
          "SQLExecDirect hung after connection kill (PLAT-8159 regression)");

  {
    SQLCHAR state[6]= {0};
    SQLINTEGER native= 0;
    SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH]= {0};
    SQLRETURN diag_rc= SQLGetDiagRec(SQL_HANDLE_STMT, hstmt2, 1, state, &native,
                                     msg, sizeof(msg), NULL);
    if (diag_rc == SQL_SUCCESS || diag_rc == SQL_SUCCESS_WITH_INFO)
      diag("killed CTAS diag: [%s] (%d) %s", state, native, msg);
  }

  /* Cleanup on the live connection. */
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_plat8159_src");

  SQLFreeHandle(SQL_HANDLE_STMT, hstmt2);
  SQLDisconnect(hdbc2);
  SQLFreeHandle(SQL_HANDLE_DBC, hdbc2);
  SQLFreeHandle(SQL_HANDLE_ENV, henv2);
  return OK;
}

/**
  Setting SQL_ATTR_QUERY_TIMEOUT=0 must leave the attribute readable as 0
  (no more silent "changed to default" SUCCESS_WITH_INFO).
*/
ODBC_TEST(t_query_timeout_get_set)
{
  SQLULEN got= 99;

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_QUERY_TIMEOUT,
                                     (SQLPOINTER)5, 0));
  CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_QUERY_TIMEOUT,
                                     &got, 0, NULL));
  is_num(got, 5);

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_QUERY_TIMEOUT,
                                     (SQLPOINTER)0, 0));
  CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_QUERY_TIMEOUT,
                                     &got, 0, NULL));
  is_num(got, 0);
  return OK;
}

MA_ODBC_TESTS my_tests[]=
{
  {t_query_timeout_get_set, "t_query_timeout_get_set", NORMAL, ALL_DRIVERS},
  {t_query_timeout_attr, "t_query_timeout_attr", NORMAL, ALL_DRIVERS},
  {t_read_timeout_dsn, "t_read_timeout_dsn", NORMAL, ALL_DRIVERS},
  {t_ctas_txn_killed_connection, "t_ctas_txn_killed_connection", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
