
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
    SQLHANDLE henv1, Connection1, Stmt1;
    SQLLEN rc = 0;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt1, "CREATE TABLE test_rowcount_values (id INT, text VARCHAR(16));");
    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b');");
    OK_SIMPLE_STMT(Stmt1, "SELECT * FROM test_rowcount_values");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    FAIL_IF_NE_INT(rc, 2, "SQLRowCount should return row count for select statement");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

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

    OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt1, "CREATE TABLE test_rowcount_values (id INT, text VARCHAR(16));");
    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b');");
    OK_SIMPLE_STMT(Stmt1, "SELECT * FROM test_rowcount_values");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    // We return 0 since we don't cache result
    FAIL_IF_NE_INT(rc, -1, "SQLRowCount should return zero for select statement with no cache");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_sqlrowcnt_update) {
    SQLHANDLE henv1, Connection1, Stmt1;
    SQLLEN rc;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt1, "CREATE TABLE test_rowcount_values (id INT, text VARCHAR(16));");
    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b'), (3, 'a');");

    OK_SIMPLE_STMT(Stmt1, "UPDATE test_rowcount_values SET id=4 WHERE text='a';");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    FAIL_IF_NE_INT(rc, 2, "SQLRowCount should return updated rows count for update query");

    OK_SIMPLE_STMT(Stmt1, "UPDATE test_rowcount_values SET id=5 WHERE text='c';");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    FAIL_IF_NE_INT(rc, 0, "SQLRowCount should return updated rows count for update query");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

#define UPDATE_NO_EXISTING_ROW_CHANGED 0
#define UPDATE_NEW_ROW_INSERTED 1
#define UPDATE_EXISTING_ROW_UPDATED 2
#define INSERT_CNT 3

ODBC_TEST(t_sqlrowcnt_insert) {
    SQLHANDLE henv1, Connection1, Stmt1;
    SQLLEN rc;
    SQLCHAR conn[512];
    SQLINTEGER ids[INSERT_CNT] = {7, 8, 9};
    SQLCHAR texts[INSERT_CNT][16] = {"a", "b", "c"};

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt1, "CREATE TABLE test_rowcount_values (id INT PRIMARY KEY, text VARCHAR(16));");
    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b');");

    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (3, 'c'), (4, 'd');");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    FAIL_IF_NE_INT(rc, 2 * UPDATE_NEW_ROW_INSERTED, "SQLRowCount should return inserted rows count for insert query");

    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (2, 'b'), (5, 'a') ON DUPLICATE KEY UPDATE text = VALUES(text);");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    // SingleStore return 0 if column value is not changed
    FAIL_IF_NE_INT(rc, UPDATE_NO_EXISTING_ROW_CHANGED  + UPDATE_NEW_ROW_INSERTED,
                   "SQLRowCount should return inserted rows count for insert query when column is not changed");

    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (2, 'c'), (6, 'a') ON DUPLICATE KEY UPDATE text = VALUES(text);");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    // SingleStore return 2 if column value is not updated
    FAIL_IF_NE_INT(rc, UPDATE_EXISTING_ROW_UPDATED  + UPDATE_NEW_ROW_INSERTED,
                   "SQLRowCount should return inserted rows count for insert query when column is updated");

    SQLCHAR *insStmt = "INSERT INTO test_rowcount_values VALUES (?, ?);";
    CHECK_STMT_RC(Stmt1, SQLPrepare(Stmt1, insStmt, strlen(insStmt)));

    CHECK_STMT_RC(Stmt1, SQLBindParameter(Stmt1, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, ids, 0, NULL));
    CHECK_STMT_RC(Stmt1, SQLBindParameter(Stmt1, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 16, 0, texts, sizeof(texts[0]), NULL));

    CHECK_STMT_RC(Stmt1, SQLExecute(Stmt1));

    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    // We only return row count for last statement
    FAIL_IF_NE_INT(rc, 1, "SQLRowCount should return last statement inserted rows count for bulk insert query");


    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_sqlrowcnt_delete) {
    SQLHANDLE henv1, Connection1, Stmt1;
    SQLLEN rc;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt1, "CREATE TABLE test_rowcount_values (id INT PRIMARY KEY, text VARCHAR(16));");
    OK_SIMPLE_STMT(Stmt1, "INSERT INTO test_rowcount_values VALUES (1, 'a'), (2, 'b');");

    OK_SIMPLE_STMT(Stmt1, "DELETE FROM test_rowcount_values WHERE id=2;");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    FAIL_IF_NE_INT(rc, 1, "SQLRowCount should return deleted rows count for delete query");

    OK_SIMPLE_STMT(Stmt1, "DELETE FROM test_rowcount_values WHERE id=2;");
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    FAIL_IF_NE_INT(rc, 0, "SQLRowCount should return deleted rows count for delete query");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_sqlrowcnt_bulk_operation) {
    SQLHANDLE henv1, Connection1, Stmt1;
    SQLLEN rc;
    SQLCHAR conn[512];
    SQLINTEGER ids[INSERT_CNT] = {1, 2, 3};
    SQLCHAR texts[INSERT_CNT][16] = {"a", "b", "c"};

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    CHECK_STMT_RC(Stmt1, SQLSetStmtAttr(Stmt1, SQL_ATTR_CURSOR_TYPE,
                                       (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));
    CHECK_STMT_RC(Stmt1, SQLSetStmtAttr(Stmt1, SQL_ATTR_ROW_ARRAY_SIZE,
                                       (SQLPOINTER)INSERT_CNT, 0));

    OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt1, "CREATE TABLE test_rowcount_values (id INT PRIMARY KEY, text VARCHAR(16));");

    CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 1, SQL_C_LONG, ids, 0, NULL));
    CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 2, SQL_C_CHAR, texts, sizeof(texts[0]), NULL));

    OK_SIMPLE_STMT(Stmt1, "SELECT id, text FROM test_rowcount_values");

    FAIL_IF(SQLFetchScroll(Stmt1, SQL_FETCH_NEXT, 0)!= SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

    CHECK_STMT_RC(Stmt1, SQLBulkOperations(Stmt1, SQL_ADD));

    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    FAIL_IF_NE_INT(rc, INSERT_CNT, "SQLRowCount should return inserted rows count for SQLBulkOperations with SQL_ADD");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_sqlrowcnt_batch) {
    SQLHANDLE henv1, Connection1, Stmt1;
    SQLLEN rc;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s;OPTIONS=%lu",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr, my_options);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS test_rowcount_values;");
    OK_SIMPLE_STMT(Stmt1, "CREATE TABLE test_rowcount_values (id INT PRIMARY KEY, text VARCHAR(16));");

    SQLCHAR *multiStmt = "INSERT INTO test_rowcount_values VALUES (1, 'a');"
                         "INSERT INTO test_rowcount_values VALUES (2, 'b');"
                         "INSERT INTO test_rowcount_values VALUES (3, 'c');";

    CHECK_STMT_RC(Stmt1, SQLPrepare(Stmt1, multiStmt, strlen(multiStmt)));
    CHECK_STMT_RC(Stmt1, SQLExecute(Stmt1));
    CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rc));

    // We only return row count for last statement
    FAIL_IF_NE_INT(rc, 1, "SQLRowCount should return last statement inserted rows count for explicit batch insert");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

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
