/*************************************************************************************
  Copyright (c) 2026 SingleStore, Inc.

  PLAT-8159 repro helper: open a multi-statement transaction over ODBC, signal
  readiness, then run CREATE TEMPORARY TABLE ... AS SELECT while the orchestrator
  kills the leaf. Prints whether SQLExecDirect returned an error (and the
  diagnostics) or succeeded.

  Sync protocol (files under --sync-dir):
    1. Client writes "ready" after START TRANSACTION + setup, before CTAS.
    2. Orchestrator kills the leaf, then writes "go".
    3. Client starts CTAS once "go" appears (or immediately if --no-wait-go).

  Exit codes:
    0  SQLExecDirect returned SQL_ERROR (error reached the driver)
    1  SQLExecDirect succeeded (unexpected after leaf kill)
    2  setup / usage failure
*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <sql.h>
#include <sqlext.h>

static void usage(const char *prog)
{
  fprintf(stderr,
          "Usage: %s --conn <connstr> --sync-dir <dir> [--query-timeout N]\n"
          "          [--no-wait-go] [--rows N]\n",
          prog);
}

static void print_diag(SQLSMALLINT htype, SQLHANDLE h)
{
  SQLCHAR state[6]= {0};
  SQLINTEGER native= 0;
  SQLCHAR msg[SQL_MAX_MESSAGE_LENGTH]= {0};
  SQLSMALLINT msglen= 0;
  SQLRETURN rc;
  SQLSMALLINT i= 1;

  while ((rc= SQLGetDiagRec(htype, h, i, state, &native, msg, sizeof(msg),
                            &msglen)) == SQL_SUCCESS ||
         rc == SQL_SUCCESS_WITH_INFO)
  {
    printf("  diag[%d]: SQLSTATE=%s native=%d msg=%s\n", i, state, (int)native,
           msg);
    i++;
  }
  if (i == 1)
    printf("  (no diagnostic records)\n");
}

static int write_flag(const char *dir, const char *name)
{
  char path[512];
  FILE *f;

  snprintf(path, sizeof(path), "%s/%s", dir, name);
  f= fopen(path, "w");
  if (!f)
  {
    fprintf(stderr, "failed to write %s: %s\n", path, strerror(errno));
    return -1;
  }
  fprintf(f, "1\n");
  fclose(f);
  return 0;
}

static int wait_flag(const char *dir, const char *name, int timeout_sec)
{
  char path[512];
  time_t start= time(NULL);

  snprintf(path, sizeof(path), "%s/%s", dir, name);
  while (access(path, F_OK) != 0)
  {
    if (timeout_sec > 0 && (time(NULL) - start) >= timeout_sec)
      return -1;
    usleep(50 * 1000);
  }
  return 0;
}

static SQLRETURN exec_direct(SQLHSTMT stmt, const char *sql)
{
  printf(">> %s\n", sql);
  return SQLExecDirect(stmt, (SQLCHAR *)sql, SQL_NTS);
}

int main(int argc, char **argv)
{
  const char *connstr= NULL;
  const char *sync_dir= NULL;
  int query_timeout= 0;
  int wait_go= 1;
  int rows= 5000;
  int i;

  SQLHENV env= SQL_NULL_HENV;
  SQLHDBC dbc= SQL_NULL_HDBC;
  SQLHSTMT stmt= SQL_NULL_HSTMT;
  SQLRETURN rc;
  time_t started, finished;
  int exit_code= 2;
  char sql[256];

  for (i= 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--conn") == 0 && i + 1 < argc)
      connstr= argv[++i];
    else if (strcmp(argv[i], "--sync-dir") == 0 && i + 1 < argc)
      sync_dir= argv[++i];
    else if (strcmp(argv[i], "--query-timeout") == 0 && i + 1 < argc)
      query_timeout= atoi(argv[++i]);
    else if (strcmp(argv[i], "--no-wait-go") == 0)
      wait_go= 0;
    else if (strcmp(argv[i], "--rows") == 0 && i + 1 < argc)
      rows= atoi(argv[++i]);
    else if (strcmp(argv[i], "--help") == 0)
    {
      usage(argv[0]);
      return 2;
    }
    else
    {
      fprintf(stderr, "unknown arg: %s\n", argv[i]);
      usage(argv[0]);
      return 2;
    }
  }

  if (!connstr || !sync_dir || rows <= 0)
  {
    usage(argv[0]);
    return 2;
  }

  printf("Connecting...\n");
  if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env)) ||
      !SQL_SUCCEEDED(SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
                                   (SQLPOINTER)SQL_OV_ODBC3, 0)) ||
      !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc)))
  {
    fprintf(stderr, "failed to allocate ODBC env/dbc\n");
    goto done;
  }

  rc= SQLDriverConnect(dbc, NULL, (SQLCHAR *)connstr, SQL_NTS, NULL, 0, NULL,
                       SQL_DRIVER_NOPROMPT);
  if (!SQL_SUCCEEDED(rc))
  {
    fprintf(stderr, "SQLDriverConnect failed\n");
    print_diag(SQL_HANDLE_DBC, dbc);
    goto done;
  }

  if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt)))
  {
    fprintf(stderr, "SQLAllocHandle(STMT) failed\n");
    print_diag(SQL_HANDLE_DBC, dbc);
    goto done;
  }

  if (query_timeout > 0)
  {
    printf("Setting SQL_ATTR_QUERY_TIMEOUT=%d\n", query_timeout);
    rc= SQLSetStmtAttr(stmt, SQL_ATTR_QUERY_TIMEOUT,
                       (SQLPOINTER)(SQLULEN)query_timeout, 0);
    if (!SQL_SUCCEEDED(rc))
    {
      fprintf(stderr, "SQLSetStmtAttr(QUERY_TIMEOUT) failed\n");
      print_diag(SQL_HANDLE_STMT, stmt);
      goto done;
    }
  }

  /* Autocommit off — matches GS Helios multi-statement transaction shape. */
  rc= SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                        0);
  if (!SQL_SUCCEEDED(rc))
  {
    fprintf(stderr, "SQLSetConnectAttr(AUTOCOMMIT_OFF) failed\n");
    print_diag(SQL_HANDLE_DBC, dbc);
    goto done;
  }

  (void)exec_direct(stmt, "DROP TABLE IF EXISTS t_plat8159_src");
  SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(exec_direct(
          stmt, "CREATE TABLE t_plat8159_src (id INT, v VARCHAR(64))")))
  {
    fprintf(stderr, "CREATE TABLE failed\n");
    print_diag(SQL_HANDLE_STMT, stmt);
    goto done;
  }
  SQLFreeStmt(stmt, SQL_CLOSE);

  /* Build a non-trivial source so CTAS touches the leaf and takes a moment. */
  printf("Inserting %d rows into t_plat8159_src...\n", rows);
  for (i= 1; i <= rows; i++)
  {
    snprintf(sql, sizeof(sql),
             "INSERT INTO t_plat8159_src VALUES (%d, 'row-%d')", i, i);
    if (!SQL_SUCCEEDED(SQLExecDirect(stmt, (SQLCHAR *)sql, SQL_NTS)))
    {
      fprintf(stderr, "INSERT failed at row %d\n", i);
      print_diag(SQL_HANDLE_STMT, stmt);
      goto done;
    }
    SQLFreeStmt(stmt, SQL_CLOSE);
  }

  if (!SQL_SUCCEEDED(SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT)))
  {
    fprintf(stderr, "COMMIT of seed data failed\n");
    print_diag(SQL_HANDLE_DBC, dbc);
    goto done;
  }

  if (!SQL_SUCCEEDED(exec_direct(stmt, "START TRANSACTION")))
  {
    fprintf(stderr, "START TRANSACTION failed\n");
    print_diag(SQL_HANDLE_STMT, stmt);
    goto done;
  }
  SQLFreeStmt(stmt, SQL_CLOSE);

  if (!SQL_SUCCEEDED(exec_direct(
          stmt, "INSERT INTO t_plat8159_src VALUES (-1, 'in-txn')")))
  {
    fprintf(stderr, "in-txn INSERT failed\n");
    print_diag(SQL_HANDLE_STMT, stmt);
    goto done;
  }
  SQLFreeStmt(stmt, SQL_CLOSE);

  printf("Transaction open. Signaling ready.\n");
  fflush(stdout);
  if (write_flag(sync_dir, "ready") != 0)
    goto done;

  if (wait_go)
  {
    printf("Waiting for orchestrator 'go' (leaf kill)...\n");
    fflush(stdout);
    if (wait_flag(sync_dir, "go", 120) != 0)
    {
      fprintf(stderr, "timed out waiting for 'go'\n");
      goto done;
    }
  }

  printf("Running CREATE TEMPORARY TABLE ... AS SELECT (may hang if no "
         "timeout)...\n");
  fflush(stdout);
  started= time(NULL);
  rc= SQLExecDirect(stmt,
                    (SQLCHAR *)"CREATE TEMPORARY TABLE t_plat8159_tmp AS "
                               "SELECT * FROM t_plat8159_src",
                    SQL_NTS);
  finished= time(NULL);

  printf("SQLExecDirect returned rc=%d (%s) after %ld seconds\n", (int)rc,
         rc == SQL_SUCCESS
             ? "SQL_SUCCESS"
             : rc == SQL_SUCCESS_WITH_INFO
                   ? "SQL_SUCCESS_WITH_INFO"
                   : rc == SQL_ERROR ? "SQL_ERROR"
                                     : rc == SQL_NO_DATA ? "SQL_NO_DATA"
                                                         : "OTHER",
         (long)(finished - started));
  print_diag(SQL_HANDLE_STMT, stmt);

  if (rc == SQL_ERROR)
  {
    printf("RESULT: error WAS delivered to the ODBC driver\n");
    exit_code= 0;
  }
  else
  {
    printf("RESULT: no error — statement succeeded (rc=%d)\n", (int)rc);
    exit_code= 1;
  }

done:
  if (stmt != SQL_NULL_HSTMT)
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
  if (dbc != SQL_NULL_HDBC)
  {
    SQLDisconnect(dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, dbc);
  }
  if (env != SQL_NULL_HENV)
    SQLFreeHandle(SQL_HANDLE_ENV, env);
  return exit_code;
}
