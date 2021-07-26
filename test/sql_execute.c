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


#define QUERY_SELECT_WITH_PARAMS_SINGLE_ROW         "SELECT * FROM " TABLE " WHERE id = ?"
#define QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW      "SELECT * FROM " TABLE " WHERE LEFT(text, 1) = RIGHT(text, 1)"
#define QUERY_UPDATE_WITH_PARAMS                    "UPDATE " TABLE " SET text = 'ca' WHERE id = ?" //TODO: change when bug PLAT-5607 is fixed
#define QUERY_DELETE_WITHOUT_PARAMS                 "DELETE FROM " TABLE " WHERE text LIKE '___'"   //TODO: change when bug PLAT-5607 is fixed
#define QUERY_SELECT_WITHOUT_PARAMS_MULTIPLE_ROWS   "SELECT * FROM " TABLE " ORDER BY id"


ODBC_TEST(execute_1_before_fetch_1) {
#define TABLE "execute_1"
    SQLINTEGER in_id = 3;
    SQLINTEGER out_id;
    SQLCHAR out_text[10];
    SQLRETURN ret;

    OK_SIMPLE_STMT(Stmt, "CREATE TABLE " TABLE "(id INT, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO " TABLE " VALUES (1, 'aa'), (2, 'bc'), (3, 'ca')");
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &in_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_SLONG, &out_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, &out_text, sizeof(out_text), NULL));

    // Syntax checks
    PREPARE(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    PREPARE(Stmt, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    PREPARE(Stmt, QUERY_UPDATE_WITH_PARAMS);
    PREPARE(Stmt, QUERY_DELETE_WITHOUT_PARAMS);

    // Smoke test: basic use case + if the cursor is not closed changes do not apply
    PREPARE(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    EXECUTE(Stmt);
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    EXECUTE_CURSOR_ERR(Stmt);
    PREPARE_CURSOR_ERR(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    EXEC_DIRECT_CURSOR_ERR(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    CLOSE(Stmt);
    IS(out_id == 0 && *out_text == 0);
    EXECUTE(Stmt);
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);

    // Re-executing a statement discards the results from the previous execution (SQLExecute)
    PREPARE(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    EXECUTE(Stmt);
    PREPARE_SEQUENCE_ERR(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    PREPARE(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    EXECUTE(Stmt);
    FETCH_CURSOR_ERR(Stmt);
    PREPARE(Stmt, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    EXECUTE(Stmt);
    FETCH(Stmt);
    IS(out_id == 1 && !strcmp("aa", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);
    EXECUTE(Stmt);
    EXECUTE_SEQUENCE_ERR(Stmt);
    PREPARE_SEQUENCE_ERR(Stmt, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    EXECUTE_SEQUENCE_ERR(Stmt);
    PREPARE(Stmt, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    EXECUTE(Stmt);
    FETCH(Stmt);
    IS(out_id == 1 && !strcmp("aa", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);
    EXEC_DIRECT(Stmt, QUERY_UPDATE_WITH_PARAMS);
    FETCH_CURSOR_ERR(Stmt);
    EXECUTE_SEQUENCE_ERR(Stmt);
    ret = SQLFetch(Stmt);
    IS(ret == SQL_ERROR);
    switch(cPlatform) {
    case LINUX:
        CHECK_STMT_ERR(Stmt, ret, SEQUENCE_ERROR);
        break;
    case WINDOWS:
        CHECK_STMT_ERR(Stmt, ret, CURSOR_STATE_ERROR);
        break;
    case MAC:
        CHECK_STMT_ERR(Stmt, ret, "S1010", 0, "Function sequence error");
        break;
    default:
        assert(0);
    }
    EXEC_DIRECT(Stmt, QUERY_SELECT_WITHOUT_PARAMS_MULTIPLE_ROWS);
    FETCH(Stmt);
    IS(out_id == 1 && !strcmp("aa", out_text)); out_id = 0, *out_text = 0;
    ret = SQLExecute(Stmt);
    IS(ret == SQL_ERROR);
    switch(cPlatform) {
    case LINUX:
    case WINDOWS:
        CHECK_STMT_ERR(Stmt, ret, SEQUENCE_ERROR);
        break;
    case MAC:
        CHECK_STMT_ERR(Stmt, ret, CURSOR_STATE_ERROR);
        break;
    default:
        assert(0);
    }
    FETCH(Stmt);
    IS(out_id == 2 && !strcmp("bc", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);

    // Re-executing a statement discards the results from the previous execution (SQLExecDirect)
    EXEC_DIRECT(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    EXEC_DIRECT_SEQUENCE_ERR(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    EXEC_DIRECT(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    FETCH_CURSOR_ERR(Stmt);
    EXEC_DIRECT(Stmt, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    PREPARE_SEQUENCE_ERR(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    PREPARE(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    PREPARE(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    ret = SQLFetch(Stmt);
    IS(ret == SQL_ERROR);
    switch(cPlatform) {
    case LINUX:
    case WINDOWS:
        CHECK_STMT_ERR(Stmt, ret, SEQUENCE_ERROR);
        break;
    case MAC:
        CHECK_STMT_ERR(Stmt, ret, "S1010", 1, "Function sequence error");
        break;
    default:
        assert(0);
    }
    PREPARE(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    EXECUTE(Stmt);
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);
    EXEC_DIRECT(Stmt, QUERY_UPDATE_WITH_PARAMS);
    CLOSE_CURSOR_ERR(Stmt);

    // A prepared non-executed statement can be reused for SQLExecDirect
    PREPARE(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    EXEC_DIRECT(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);
    PREPARE(Stmt, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    EXEC_DIRECT(Stmt, QUERY_DELETE_WITHOUT_PARAMS);
    EXECUTE_SEQUENCE_ERR(Stmt);
    PREPARE(Stmt, QUERY_UPDATE_WITH_PARAMS);
    EXECUTE(Stmt);
    EXECUTE(Stmt);
    FETCH_CURSOR_ERR(Stmt);

    UNBIND(Stmt);
    return OK;
#undef TABLE
}

ODBC_TEST(execute_2_before_fetch_1) {
#define TABLE "execute_2"
    SQLINTEGER in_id = 3;
    SQLINTEGER out_id;
    SQLCHAR out_text[10];
    SQLHANDLE stmt2;
    SQLINTEGER in2_id = 2;
    SQLINTEGER out2_id;
    SQLCHAR out2_text[10];

    OK_SIMPLE_STMT(Stmt, "CREATE TABLE " TABLE "(id INT, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO " TABLE " VALUES (1, 'aa'), (2, 'bc'), (3, 'ca')");
    CHECK_DBC_RC(Connection, SQLAllocHandle(SQL_HANDLE_STMT, Connection, &stmt2));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &in_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_SLONG, &out_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, &out_text, sizeof(out_text), NULL));
    CHECK_STMT_RC(stmt2, SQLBindParameter(stmt2, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &in2_id, 0, NULL));
    CHECK_STMT_RC(stmt2, SQLBindCol(stmt2, 1, SQL_C_SLONG, &out2_id, 0, NULL));
    CHECK_STMT_RC(stmt2, SQLBindCol(stmt2, 2, SQL_C_CHAR, &out2_text, sizeof(out2_text), NULL));

    // Executing two statements does not corrupt their results (SQLExecute)
    PREPARE(Stmt, QUERY_SELECT_WITHOUT_PARAMS_MULTIPLE_ROWS);
    PREPARE(stmt2, QUERY_DELETE_WITHOUT_PARAMS);
    EXECUTE(Stmt);
    EXECUTE(stmt2);
    FETCH(Stmt);
    IS(out_id == 1 && !strcmp("aa", out_text)); out_id = 0, *out_text = 0;
    EXEC_DIRECT(stmt2, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    FETCH(stmt2);
    FETCH(Stmt);
    IS(out_id == 2 && !strcmp("bc", out_text)); out_id = 0, *out_text = 0;
    IS(out2_id == 1 && !strcmp("aa", out2_text)); out2_id = 0, *out2_text = 0;
    CLOSE(stmt2);
    PREPARE(stmt2, QUERY_SELECT_WITHOUT_PARAMS_MULTIPLE_ROWS);
    EXECUTE(stmt2);
    FETCH(stmt2);
    IS(out2_id == 1 && !strcmp("aa", out2_text)); out2_id = 0, *out2_text = 0;
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    FETCH(stmt2);
    IS(out2_id == 2 && !strcmp("bc", out2_text)); out2_id = 0, *out2_text = 0;
    CLOSE(Stmt);
    FETCH(stmt2);
    IS(out2_id == 3 && !strcmp("ca", out2_text)); out2_id = 0, *out2_text = 0;
    CLOSE(stmt2);

    UNBIND(Stmt);
    FREE_HANDLE(stmt2);
    return OK;
#undef TABLE
}

ODBC_TEST(execute_async) {
    SQLUINTEGER async_mode;

    CHECK_DBC_RC(Connection, SQLGetInfo(Connection, SQL_ASYNC_MODE, &async_mode, 0, NULL));
    IS(async_mode == SQL_AM_NONE); /* Not supported */

    return OK;
}

ODBC_TEST(execute_no_data) {
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE execute_3_to (id INT, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE execute_3_from (id INT, text VARCHAR(16))");

    OK_SIMPLE_STMT(Stmt, "INSERT INTO execute_3_to VALUES (1, 'aa'), (2, 'bc'), (3, 'ca')");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO execute_3_from VALUES (11, 'xy'), (12, 'yz'), (13, 'zz')");

    EXPECT_STMT(Stmt, SQLExecDirect(Stmt, "INSERT INTO execute_3_to SELECT * FROM execute_3_from WHERE id < 10", SQL_NTS), SQL_NO_DATA);
    EXPECT_STMT(Stmt, SQLExecDirect(Stmt, "UPDATE execute_3_to SET id = 10 WHERE text = 'vvv'", SQL_NTS), SQL_NO_DATA);
    EXPECT_STMT(Stmt, SQLExecDirect(Stmt, "DELETE FROM execute_3_to WHERE text LIKE '___'", SQL_NTS), SQL_NO_DATA);

    PREPARE(Stmt, "INSERT INTO execute_3_to SELECT * FROM execute_3_from WHERE id < 10");
    EXPECT_STMT(Stmt, SQLExecute(Stmt), SQL_NO_DATA);
    PREPARE(Stmt, "UPDATE execute_3_to SET id = 10 WHERE text = 'vvv'");
    EXPECT_STMT(Stmt, SQLExecute(Stmt), SQL_NO_DATA);
    PREPARE(Stmt, "DELETE FROM execute_3_to WHERE text LIKE '___'");
    EXPECT_STMT(Stmt, SQLExecute(Stmt), SQL_NO_DATA);

    return OK;
}

ODBC_TEST(execute_unbound) {
    SQLINTEGER in_id = 3;

    OK_SIMPLE_STMT(Stmt, "CREATE TABLE execute_4 (id INT, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO execute_4 VALUES (1, 'aa'), (2, 'bc'), (3, 'ca')");

    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &in_id, 0, NULL));

    CHECK_STMT_ERR(Stmt,
                   SQLExecDirect(Stmt, "SELECT COUNT(*) FROM execute_4 WHERE id = ? AND text = ?", SQL_NTS),
                   "07002",
                   0,
                   "Parameter was not bound before calling SQLExecute");

    PREPARE(Stmt, "SELECT COUNT(*) FROM execute_4 WHERE id = ? AND text = ?");
    CHECK_STMT_ERR(Stmt, SQLExecute(Stmt), "07002", 0, "Parameter was not bound before calling SQLExecute");

    return OK;
}

int test_transaction(SQLHANDLE conn_other, SQLHANDLE stmt_other) {
#define TABLE "execute_5"
    SQLINTEGER in_id = 3;
    SQLINTEGER out_id;
    SQLCHAR out_text[10];

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS " TABLE);
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE " TABLE "(id INT, text VARCHAR(16))");
    OK_SIMPLE_STMT(Stmt, "INSERT INTO " TABLE " VALUES (1, 'aa'), (2, 'bc'), (3, 'ca')");
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &in_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_SLONG, &out_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, &out_text, sizeof(out_text), NULL));

    // Manual commit should not break prepared statements and cursors
    PREPARE(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    OK_SIMPLE_STMT(stmt_other, "INSERT INTO " TABLE " VALUES (10, 'uvw')");
    CHECK_DBC_RC(conn_other, SQLEndTran(SQL_HANDLE_DBC, conn_other, SQL_COMMIT));
    EXECUTE(Stmt);
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);

    EXEC_DIRECT(Stmt, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    OK_SIMPLE_STMT(stmt_other, "INSERT INTO " TABLE " VALUES (11, 'xyz')");
    CHECK_DBC_RC(conn_other, SQLEndTran(SQL_HANDLE_DBC, conn_other, SQL_COMMIT));
    FETCH(Stmt);
    IS(out_id == 1 && !strcmp("aa", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);

    EXEC_DIRECT(Stmt, QUERY_SELECT_WITHOUT_PARAMS_MULTIPLE_ROWS);
    FETCH(Stmt);
    IS(out_id == 1 && !strcmp("aa", out_text)); out_id = 0, *out_text = 0;
    OK_SIMPLE_STMT(stmt_other, "INSERT INTO " TABLE " VALUES (12, 'null')");
    CHECK_DBC_RC(conn_other, SQLEndTran(SQL_HANDLE_DBC, conn_other, SQL_COMMIT));
    FETCH(Stmt);
    IS(out_id == 2 && !strcmp("bc", out_text)); out_id = 0, *out_text = 0;
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);

    // Rollback should not break prepared statements and cursors
    PREPARE(Stmt, QUERY_SELECT_WITH_PARAMS_SINGLE_ROW);
    OK_SIMPLE_STMT(stmt_other, "DELETE FROM " TABLE " WHERE id = 10");
    CHECK_DBC_RC(conn_other, SQLEndTran(SQL_HANDLE_DBC, conn_other, SQL_ROLLBACK));
    EXECUTE(Stmt);
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);

    EXEC_DIRECT(Stmt, QUERY_SELECT_WITHOUT_PARAMS_SINGLE_ROW);
    OK_SIMPLE_STMT(stmt_other, "DELETE FROM " TABLE " WHERE id = 11");
    CHECK_DBC_RC(conn_other, SQLEndTran(SQL_HANDLE_DBC, conn_other, SQL_ROLLBACK));
    FETCH(Stmt);
    IS(out_id == 1 && !strcmp("aa", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);

    EXEC_DIRECT(Stmt, QUERY_SELECT_WITHOUT_PARAMS_MULTIPLE_ROWS);
    FETCH(Stmt);
    IS(out_id == 1 && !strcmp("aa", out_text)); out_id = 0, *out_text = 0;
    OK_SIMPLE_STMT(stmt_other, "DELETE FROM " TABLE " WHERE id = 12");
    CHECK_DBC_RC(conn_other, SQLEndTran(SQL_HANDLE_DBC, conn_other, SQL_ROLLBACK));
    FETCH(Stmt);
    IS(out_id == 2 && !strcmp("bc", out_text)); out_id = 0, *out_text = 0;
    FETCH(Stmt);
    IS(out_id == 3 && !strcmp("ca", out_text)); out_id = 0, *out_text = 0;
    CLOSE(Stmt);

    return OK;
#undef TABLE
}

ODBC_TEST(execute_transaction) {
    SQLUSMALLINT transaction_mode;
    SQLCHAR multiple_transactions[3];

    SQLHANDLE Connection1, Stmt1, stmt2;
    SQLCHAR conn[512];

    int result = OK;

    /* Check for transaction support */
    CHECK_DBC_RC(Connection, SQLGetInfo(Connection, SQL_TXN_CAPABLE, &transaction_mode, 0, NULL));
    IS(transaction_mode == SQL_TC_DDL_COMMIT);

    CHECK_DBC_RC(Connection, SQLGetInfo(Connection,
                                        SQL_MULTIPLE_ACTIVE_TXN,
                                        multiple_transactions,
                                        sizeof(multiple_transactions),
                                        NULL));
    IS(!strcmp((const char*)multiple_transactions, "Y"));

    /* Prepare the environment */
    sprintf((char*)conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s;NO_CACHE=1",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);
    CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));
    CHECK_DBC_RC(Connection, SQLAllocHandle(SQL_HANDLE_STMT, Connection, &stmt2));
    CHECK_DBC_RC(Connection, SQLSetConnectOption(Connection, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF));
    CHECK_DBC_RC(Connection1, SQLSetConnectOption(Connection1, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF));

    /* Run the tests */
    result &= test_transaction(Connection, stmt2);
    result &= test_transaction(Connection1, Stmt1);

    /* Clean up the environment */
    CHECK_DBC_RC(Connection, SQLSetConnectOption(Connection, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON));
    CHECK_STMT_RC(stmt2, SQLFreeHandle(SQL_HANDLE_STMT, stmt2));
    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));

    return result;
}

MA_ODBC_TESTS my_tests[] = {
   {execute_1_before_fetch_1, "execute_1_before_fetch_1", NORMAL, ALL_DRIVERS},
   {execute_2_before_fetch_1, "execute_2_before_fetch_1", NORMAL, ALL_DRIVERS},
   {execute_async, "execute_async", NORMAL, ALL_DRIVERS},
   {execute_no_data, "execute_no_data", KNOWN_FAILURE, ALL_DRIVERS},    //TODO: bug PLAT-5607
   {execute_unbound, "execute_unbound", NORMAL, ALL_DRIVERS},
   {execute_transaction, "execute_transaction", NORMAL, ALL_DRIVERS},
   {NULL, NULL, NORMAL, ALL_DRIVERS}
};


int main(int argc, char **argv) {
    int tests = sizeof(my_tests) / sizeof(MA_ODBC_TESTS) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}
