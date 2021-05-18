
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
#include <sql.h>

ODBC_TEST(t_sqlrowcnt_select) {
    SQLLEN rc = 0;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS test_rowcount_values");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE test_rowcount_values (id INT, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b')");
    OK_SIMPLE_STMT(Stmt, "SELECT * FROM test_rowcount_values");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    FAIL_IF_NE_INT(rc, 2, "SQLRowCount should return row count for select statement");

    return OK;
}

ODBC_TEST(t_sqlrowcnt_select_nocache) {
    SQLHANDLE henv1, Connection1, Stmt1;
    SQLLEN rc;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s;NO_CACHE=1",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS test_rowcount_values");
    OK_SIMPLE_STMT(Stmt1, "CREATE TABLE test_rowcount_values (id INT, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b')");
    OK_SIMPLE_STMT(Stmt1, "SELECT * FROM test_rowcount_values");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    // We return -1 since we don't cache result
    FAIL_IF_NE_INT(rc, -1, "SQLRowCount should return -1 for select statement with no cache");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_sqlrowcnt_update) {
    SQLLEN rc;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS test_rowcount_values");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE test_rowcount_values (id INT, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b'), (3, 'a')");

    OK_SIMPLE_STMT(Stmt, "UPDATE test_rowcount_values SET id=4 WHERE text='a'");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    FAIL_IF_NE_INT(rc, 2, "SQLRowCount should return updated rows count for update query");

    OK_SIMPLE_STMT(Stmt, "UPDATE test_rowcount_values SET id=5 WHERE text='c'");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    FAIL_IF_NE_INT(rc, 0, "SQLRowCount should return updated rows count for update query");

    return OK;
}

#define UPDATE_NO_EXISTING_ROW_CHANGED 0
#define UPDATE_NEW_ROW_INSERTED 1
#define UPDATE_EXISTING_ROW_UPDATED 2
#define INSERT_CNT 3

ODBC_TEST(t_sqlrowcnt_insert) {
    SQLLEN rc;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS test_rowcount_values");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE test_rowcount_values (id INT PRIMARY KEY, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b')");

    OK_SIMPLE_STMT(Stmt, "INSERT INTO test_rowcount_values VALUES (3, 'c'), (4, 'd')");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    FAIL_IF_NE_INT(rc, 2 * UPDATE_NEW_ROW_INSERTED, "SQLRowCount should return inserted rows count for insert query");

    OK_SIMPLE_STMT(Stmt, "INSERT INTO test_rowcount_values VALUES (2, 'b'), (5, 'a') ON DUPLICATE KEY UPDATE text = VALUES(text)");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    // TODO: PLAT-5518

    // SingleStore return 0 if column value is not changed
    // FAIL_IF_NE_INT(rc, UPDATE_NO_EXISTING_ROW_CHANGED  + UPDATE_NEW_ROW_INSERTED,
    //               "SQLRowCount should return inserted rows count for insert query when column is not changed");

    OK_SIMPLE_STMT(Stmt, "INSERT INTO test_rowcount_values VALUES (2, 'c'), (6, 'a') ON DUPLICATE KEY UPDATE text = VALUES(text)");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    // SingleStore return 2 if column value is not updated
    FAIL_IF_NE_INT(rc, UPDATE_EXISTING_ROW_UPDATED  + UPDATE_NEW_ROW_INSERTED,
                   "SQLRowCount should return inserted rows count for insert query when column is updated");

    return OK;
}

ODBC_TEST(t_sqlrowcnt_delete) {
    SQLLEN rc;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE test_rowcount_values (id INT PRIMARY KEY, text VARCHAR(16));");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b');");

    OK_SIMPLE_STMT(Stmt, "DELETE FROM test_rowcount_values WHERE id=2;");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    FAIL_IF_NE_INT(rc, 1, "SQLRowCount should return deleted rows count for delete query");

    OK_SIMPLE_STMT(Stmt, "DELETE FROM test_rowcount_values WHERE id=2;");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    FAIL_IF_NE_INT(rc, 0, "SQLRowCount should return deleted rows count for delete query");

    return OK;
}

ODBC_TEST(t_sqlrowcnt_bulk_operation) {
    SQLLEN rc;
    SQLINTEGER ids[INSERT_CNT] = {1, 2, 3};
    SQLCHAR texts[INSERT_CNT][16] = {"a", "b", "c"};

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                       (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                       (SQLPOINTER)INSERT_CNT, 0));

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE test_rowcount_values (id INT PRIMARY KEY, text VARCHAR(16));");

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, ids, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, texts, sizeof(texts[0]), NULL));

    OK_SIMPLE_STMT(Stmt, "SELECT id, text FROM test_rowcount_values");

    FAIL_IF(SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0)!= SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

    CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    FAIL_IF_NE_INT(rc, INSERT_CNT, "SQLRowCount should return inserted rows count for SQLBulkOperations with SQL_ADD");

    return OK;
}

ODBC_TEST(t_sqlrowcnt_batch) {
    SQLLEN rc;
    SQLINTEGER ids[INSERT_CNT] = {4, 5, 6};
    SQLCHAR texts[INSERT_CNT][16] = {"a", "b", "c"};

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS test_rowcount_values");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE test_rowcount_values (id INT PRIMARY KEY, text VARCHAR(16))");

    SQLCHAR *multiStmt = "INSERT INTO test_rowcount_values VALUES (1, 'a');"
                         "INSERT INTO test_rowcount_values VALUES (2, 'b'), (3, 'c');";

    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, multiStmt, strlen(multiStmt)));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));
    FAIL_IF_NE_INT(rc, 1, "SQLRowCount should return each statement inserted rows count for explicit batch insert");

    CHECK_STMT_RC(Stmt, SQLMoreResults(Stmt));
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));
    FAIL_IF_NE_INT(rc, 2, "SQLRowCount should return each statement inserted rows count for explicit batch insert");

    SQLCHAR *insStmt = "INSERT INTO test_rowcount_values VALUES (?, ?)";
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, insStmt, strlen(insStmt)));

    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, ids, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 16, 0, texts, sizeof(texts[0]), NULL));

    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    // We only return row count for last statement
    FAIL_IF_NE_INT(rc, 1, "SQLRowCount should return last statement inserted rows count for bulk insert query");

    OK_SIMPLE_STMT(Stmt, "DROP PROCEDURE IF EXISTS t_sqlrowcnt_batch")
    OK_SIMPLE_STMT(Stmt, " CREATE PROCEDURE t_sqlrowcnt_batch () AS BEGIN "
                          "INSERT INTO test_rowcount_values VALUES (7, 'd');"
                          "INSERT INTO test_rowcount_values VALUES (8, 'e');"
                          "END;"
                  );
    OK_SIMPLE_STMT(Stmt, "CALL t_sqlrowcnt_batch();")

    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rc));

    // We don't support row count for procedures
    FAIL_IF_NE_INT(rc, 0, "SQLRowCount should return 0 for stored procedures");

    return OK;
}


MA_ODBC_TESTS my_tests[] =
{
    {t_sqlrowcnt_select, "t_sqlrowcnt_select", NORMAL, ALL_DRIVERS},
    {t_sqlrowcnt_select_nocache, "t_sqlrowcnt_select_nocache", NORMAL, ALL_DRIVERS},
    {t_sqlrowcnt_update, "t_sqlrowcnt_update", NORMAL, ALL_DRIVERS},
    {t_sqlrowcnt_insert, "t_sqlrowcnt_insert", NORMAL, ALL_DRIVERS},
    {t_sqlrowcnt_delete, "t_sqlrowcnt_delete", NORMAL, ALL_DRIVERS},
    {t_sqlrowcnt_bulk_operation, "t_sqlrowcnt_bulk_operation", NORMAL, ALL_DRIVERS},
    {t_sqlrowcnt_batch, "t_sqlrowcnt_batch", NORMAL, ALL_DRIVERS},
    {NULL, NULL, NORMAL, ALL_DRIVERS}
};


int main(int argc, char **argv) {
    int tests = sizeof(my_tests) / sizeof(MA_ODBC_TESTS) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}
