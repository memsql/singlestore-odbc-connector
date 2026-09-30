/*************************************************************************************
  Copyright (c) 2026 SingleStore, Inc.

  PLAT-8159 repro helper. Matches the batch that never returned:

    1. Outside the transaction, seed src_staging (6 rows, two sharing
       source_key) and a small batch_log.
    2. START TRANSACTION, then a few UPDATEs.
    3. DROP TEMPORARY TABLE IF EXISTS tmp_latest. This must return before
       the orchestrator touches the leaf.
    4. Signal ready, wait for "go", then run the window-function CTAS.

  The orchestrator kills the leaf after the create is executing, then refuses
  the leaf port. memsql.log records ER_ROLLED_BACK_TRANSACTION (1857) and
  SQLExecDirect does not return.

  Sync protocol (files under --sync-dir):
    1. Client writes "ready" after DROP TEMPORARY TABLE returns.
    2. Client waits for "go".
    3. Client runs CREATE TEMPORARY TABLE tmp_latest AS SELECT ...

  Exit codes:
    0  SQLExecDirect returned SQL_ERROR (an error reached the driver)
    1  SQLExecDirect succeeded
    2  setup / usage failure
    (still running when the orchestrator gives up is the reproduced hang)
*************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <sql.h>
#include <sqlext.h>

#define SEQ_BASE 1000
#define ROWS 6

static void usage(const char *prog)
{
  fprintf(stderr,
          "Usage: %s --conn <connstr> --sync-dir <dir>\n",
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

static int exec_ok(SQLHSTMT stmt, const char *sql)
{
  SQLRETURN rc;

  printf(">> %s\n", sql);
  fflush(stdout);
  rc= SQLExecDirect(stmt, (SQLCHAR *)sql, SQL_NTS);
  if (!SQL_SUCCEEDED(rc))
  {
    fprintf(stderr, "statement failed\n");
    print_diag(SQL_HANDLE_STMT, stmt);
    SQLFreeStmt(stmt, SQL_CLOSE);
    return -1;
  }
  SQLFreeStmt(stmt, SQL_CLOSE);
  return 0;
}

/* Customer library checks select @@trancount after every statement in the txn. */
static int assert_in_transaction(SQLHSTMT stmt)
{
  SQLRETURN rc;
  SQLCHAR buf[64]= {0};
  SQLLEN ind= 0;
  int trancount= 0;
  const char *sql= "SELECT @@trancount";

  printf(">> %s\n", sql);
  fflush(stdout);
  rc= SQLExecDirect(stmt, (SQLCHAR *)sql, SQL_NTS);
  if (!SQL_SUCCEEDED(rc))
  {
    fprintf(stderr, "SELECT @@trancount failed\n");
    print_diag(SQL_HANDLE_STMT, stmt);
    SQLFreeStmt(stmt, SQL_CLOSE);
    return -1;
  }
  rc= SQLFetch(stmt);
  if (!SQL_SUCCEEDED(rc) ||
      !SQL_SUCCEEDED(SQLGetData(stmt, 1, SQL_C_CHAR, buf, sizeof(buf), &ind)))
  {
    fprintf(stderr, "failed to read @@trancount\n");
    print_diag(SQL_HANDLE_STMT, stmt);
    SQLFreeStmt(stmt, SQL_CLOSE);
    return -1;
  }
  SQLFreeStmt(stmt, SQL_CLOSE);
  trancount= atoi((char *)buf);
  printf("@@trancount=%d\n", trancount);
  fflush(stdout);
  if (trancount <= 0)
  {
    fprintf(stderr, "expected an open transaction after the batch statement\n");
    return -1;
  }
  return 0;
}

static int seed_schema(SQLHSTMT stmt)
{
  int i;
  char sql[512];

  if (exec_ok(stmt, "DROP TABLE IF EXISTS src_staging") != 0 ||
      exec_ok(stmt, "DROP TABLE IF EXISTS batch_log") != 0)
    return -1;

  if (exec_ok(stmt,
              "CREATE TABLE src_staging ("
              "event_seq BIGINT NOT NULL, "
              "source_key VARCHAR(64) NOT NULL, "
              "action_code VARCHAR(8) NOT NULL, "
              "v VARCHAR(64) NOT NULL)") != 0)
    return -1;

  if (exec_ok(stmt,
              "CREATE TABLE batch_log ("
              "id INT NOT NULL PRIMARY KEY, "
              "seq_no BIGINT NOT NULL)") != 0)
    return -1;

  /* Two rows share source_key so the window keeps an rn=1 row. */
  printf("Inserting %d rows into src_staging...\n", ROWS);
  for (i= 0; i < ROWS; i++)
  {
    if (i < 2)
    {
      snprintf(sql, sizeof(sql),
               "INSERT INTO src_staging VALUES (%d, 'src-a', '%s', 'row-%d')",
               SEQ_BASE + i, i == 0 ? "I" : "U", i);
    }
    else
    {
      snprintf(sql, sizeof(sql),
               "INSERT INTO src_staging VALUES (%d, 'src-%d', 'I', 'row-%d')",
               SEQ_BASE + i, i, i);
    }
    if (exec_ok(stmt, sql) != 0)
      return -1;
  }

  for (i= 1; i <= 3; i++)
  {
    snprintf(sql, sizeof(sql),
             "INSERT INTO batch_log VALUES (%d, 0)", i);
    if (exec_ok(stmt, sql) != 0)
      return -1;
  }
  return 0;
}

int main(int argc, char **argv)
{
  const char *connstr= NULL;
  const char *sync_dir= NULL;
  int i;

  SQLHENV env= SQL_NULL_HENV;
  SQLHDBC dbc= SQL_NULL_HDBC;
  SQLHSTMT stmt= SQL_NULL_HSTMT;
  SQLRETURN rc;
  time_t started, finished;
  int exit_code= 2;
  char sql[512];
  char ctas[2048];

  for (i= 1; i < argc; i++)
  {
    if (strcmp(argv[i], "--conn") == 0 && i + 1 < argc)
      connstr= argv[++i];
    else if (strcmp(argv[i], "--sync-dir") == 0 && i + 1 < argc)
      sync_dir= argv[++i];
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

  if (!connstr || !sync_dir)
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

  /* Autocommit off — the reported batch runs inside one transaction. */
  rc= SQLSetConnectAttr(dbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
                        0);
  if (!SQL_SUCCEEDED(rc))
  {
    fprintf(stderr, "SQLSetConnectAttr(AUTOCOMMIT_OFF) failed\n");
    print_diag(SQL_HANDLE_DBC, dbc);
    goto done;
  }

  if (seed_schema(stmt) != 0)
    goto done;
  if (!SQL_SUCCEEDED(SQLEndTran(SQL_HANDLE_DBC, dbc, SQL_COMMIT)))
  {
    fprintf(stderr, "COMMIT of seed data failed\n");
    print_diag(SQL_HANDLE_DBC, dbc);
    goto done;
  }
  printf("Seed data committed.\n");

  if (exec_ok(stmt, "START TRANSACTION") != 0)
    goto done;

  /* Prior DML in the same transaction. The hung log ran these before the drop. */
  printf("Running prior updates...\n");
  for (i= 0; i < 3; i++)
  {
    snprintf(sql, sizeof(sql),
             "UPDATE batch_log SET seq_no = %d WHERE id = %d",
             SEQ_BASE + i, i + 1);
    if (exec_ok(stmt, sql) != 0)
      goto done;
  }
  if (assert_in_transaction(stmt) != 0)
    goto done;

  if (exec_ok(stmt, "DROP TEMPORARY TABLE IF EXISTS tmp_latest") != 0)
    goto done;
  if (assert_in_transaction(stmt) != 0)
    goto done;

  printf("DROP TEMPORARY TABLE returned. Signaling ready.\n");
  fflush(stdout);
  if (write_flag(sync_dir, "ready") != 0)
    goto done;

  printf("Waiting for orchestrator 'go' to start CREATE TEMPORARY TABLE...\n");
  fflush(stdout);
  if (wait_flag(sync_dir, "go", 120) != 0)
  {
    fprintf(stderr, "timed out waiting for 'go'\n");
    goto done;
  }

  snprintf(ctas, sizeof(ctas),
           "CREATE TEMPORARY TABLE tmp_latest AS "
           "SELECT * FROM ("
           "SELECT *, "
           "row_number() OVER (PARTITION BY source_key "
           "ORDER BY event_seq DESC) AS rn, "
           "count(*) OVER (PARTITION BY source_key) AS grp_count "
           "FROM src_staging "
           "WHERE event_seq BETWEEN %d AND %d "
           "AND SLEEP(30) = 0"
           ") WHERE rn = 1",
           SEQ_BASE, SEQ_BASE + ROWS - 1);

  printf("Running CREATE TEMPORARY TABLE ... AS SELECT...\n");
  printf(">> %s\n", ctas);
  fflush(stdout);
  started= time(NULL);
  rc= SQLExecDirect(stmt, (SQLCHAR *)ctas, SQL_NTS);
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
