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

#define BUF_SIZE    1000

#if defined(_WIN32) || defined(__APPLE__)
  #define NULL_ARG_ERROR  "Invalid argument value"
#else
  #define NULL_ARG_ERROR  "Invalid use of null pointer"
#endif
#define INVALID_BUF_SIZE_ERROR  "Invalid string or buffer length"


ODBC_TEST(buffers_native_sql) {
    SQLCHAR in_buf[BUF_SIZE];
    SQLCHAR out_buf[BUF_SIZE];
    SQLINTEGER out_len, buff_size;
    const char* query;
    const char* long_query;

    if (iOdbc() && is_unicode_driver()) {
        // this test doesn't work for iODBC Unicode driver type
        return OK;
    }

    // Normal run
    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), out_buf, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, query));
    IS(!strcmp(in_buf, out_buf));
    IS(out_len == strlen(query));

    query = "DELETE FROM thetable WHERE id = 1";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, query));
    IS(!strcmp(in_buf, out_buf));
    IS(out_len == strlen(query));

    // Omit length output
    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), out_buf, BUF_SIZE, NULL));
    IS(!strcmp(in_buf, query));
    IS(!strcmp(in_buf, out_buf));

    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, BUF_SIZE, NULL));
    IS(!strcmp(in_buf, query));
    IS(!strcmp(in_buf, out_buf));

    // Omit out buf but keep length output
    query = "DELETE FROM thetable WHERE id = 1";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), NULL, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, query));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), NULL, 0, &out_len));
    IS(!strcmp(in_buf, query));
    IS(out_len == strlen(query));

    // Don't provide out buf length
    strcpy(out_buf, "DEADBEEF");

    query = "DELETE FROM thetable WHERE id = 1";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, 0, &out_len));
    IS(!strcmp(in_buf, query));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    if (iOdbc() && is_unicode_driver()) {
      CHECK_DBC_ERR(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, SQL_NTS, &out_len), "HY001", -1, "Memory allocation error");
    } else {
      CHECK_DBC_ERR(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, SQL_NTS, &out_len), "HY090", 0, "Invalid string or buffer length");
    }

    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, 0, NULL));
    IS(!strcmp(in_buf, query));

    IS(!strcmp("DEADBEEF", out_buf));

    // Shortcut in buff
    query = "SELECT (1, 2, 3)";
    long_query = "SELECT (1, 2, 3)BLAH";
    strcpy(in_buf, long_query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(query), out_buf, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, long_query));
    IS(!strcmp(query, out_buf));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, 0, out_buf, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, query));
    IS(out_buf[0] == 0);

    // Shortcut out buff
    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), out_buf, 10, &out_len));
    IS(!strcmp(in_buf, query));
    IS(strlen(out_buf) == 9)
    IS(!strncmp(in_buf, out_buf, 9));
    IS(out_len == strlen(query));   // The standard is not very clear there

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), out_buf, 15, NULL));
    IS(!strcmp(in_buf, query));
    IS(strlen(out_buf) == 14)
    IS(!strncmp(in_buf, out_buf, 14));

    // Bad use
    CHECK_DBC_ERR(Connection, SQLNativeSql(Connection, NULL, 0, out_buf, 15, NULL), "HY009", iOdbc() ? -1 : 0, NULL_ARG_ERROR);
    IS(strlen(out_buf) == 14);

    CHECK_DBC_ERR(Connection, SQLNativeSql(Connection, NULL, SQL_NTS, out_buf, 15, NULL), "HY009", iOdbc() ? -1 : 0, NULL_ARG_ERROR);
    IS(strlen(out_buf) == 14);

    out_len = 10;
    buff_size = 15;
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, 0, out_buf, buff_size, &out_len));
    IS(strlen(out_buf) == 0);
    IS(out_len == 0);

    *in_buf = 0;
    out_len = 10;
    out_buf[0] = 'b';
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, BUF_SIZE, out_buf, 15, &out_len));
    IS(strlen(out_buf) == 0);
    IS(out_len == 0);

    query = "DELETE FROM thetable WHERE id = 1";
    strcpy(in_buf, query);
    // S1090 is returned on MacOS instead of HY090, but error message is correct
    CHECK_DBC_ERR(Connection, SQLNativeSql(Connection, in_buf, -1, out_buf, BUF_SIZE, &out_len), iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, INVALID_BUF_SIZE_ERROR);

    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_ERR(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, -1, &out_len), "HY090", 0, INVALID_BUF_SIZE_ERROR);

    return OK;
}

ODBC_TEST(buffers_native_sql_w) {
    SQLWCHAR* in_buf;
    SQLWCHAR out_buf[BUF_SIZE];
    SQLINTEGER out_len, buff_size;
    const char* query;
    const char* long_query;

    if (iOdbc() && is_ansi_driver()) {
        // this test doesn't work for iODBC Ansi driver type
        return OK;
    }

    // Normal run
    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), out_buf, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharequal(in_buf, out_buf));
    IS(out_len == strlen(query));

    query = "DELETE FROM thetable WHERE id = 1";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharequal(in_buf, out_buf));
    IS(out_len == strlen(query));

    // Omit length output
    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), out_buf, BUF_SIZE, NULL));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharequal(in_buf, out_buf));

    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, BUF_SIZE, NULL));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharequal(in_buf, out_buf));

    // Omit out buf but keep length output
    query = "DELETE FROM thetable WHERE id = 1";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), NULL, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), NULL, 0, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(out_len == strlen(query));

    // Don't provide out buf length
    strcpy((char*)out_buf, "DEADBEEF");

    query = "DELETE FROM thetable WHERE id = 1";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, 0, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(out_len == strlen(query));

    if (!iOdbc()) {
        //This check fails on iODBC with `Memory Allocation Error`
        query = "INSERT INTO thetable VALUES (1, 2, none)";
        in_buf = CW(query);
        CHECK_DBC_ERR(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, SQL_NTS, &out_len), "HY090", 0, "Invalid string or buffer length");
    }

    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, 0, NULL));
    IS(sqlwcharequal(in_buf, CW(query)));

    IS(!strcmp("DEADBEEF", (char*)out_buf));

    // Shortcut in buff
    query = "SELECT (1, 2, 3)";
    long_query = "SELECT (1, 2, 3)BLAH";
    in_buf = CW(long_query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, strlen(query), out_buf, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(long_query)));
    IS(sqlwcharequal(out_buf, CW(query)));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, 0, out_buf, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(out_buf[0] == 0);

    // Shortcut out buff
    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), out_buf, 10, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharlen(out_buf) == 9)
    IS(!sqlwcharcmp(in_buf, out_buf, 9));
    IS(out_len == strlen(query));   // The standard is not very clear there

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), out_buf, 15, NULL));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharlen(out_buf) == 14)
    IS(!sqlwcharcmp(in_buf, out_buf, 14));

    // Bad use
    CHECK_DBC_ERR(Connection, SQLNativeSqlW(Connection, NULL, 0, out_buf, 15, NULL), "HY009", iOdbc() ? -1 : 0, NULL_ARG_ERROR);
    IS(sqlwcharlen(out_buf) == 14);

    CHECK_DBC_ERR(Connection, SQLNativeSqlW(Connection, NULL, SQL_NTS, out_buf, 15, NULL), "HY009", iOdbc() ? -1 : 0, NULL_ARG_ERROR);
    IS(sqlwcharlen(out_buf) == 14);

    out_len = 10;
    buff_size = 15;
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, 0, out_buf, buff_size, &out_len));
    IS(sqlwcharlen(out_buf) == 0);
    IS(out_len == 0);

    *in_buf = 0;
    out_len = 10;
    out_buf[0] = 'b';
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, BUF_SIZE, out_buf, 15, &out_len));
    IS(sqlwcharlen(out_buf) == 0);
    IS(out_len == 0);

    query = "DELETE FROM thetable WHERE id = 1";
    in_buf = CW(query);
    // S1090 is returned on MacOS instead of HY090, but error message is correct
    CHECK_DBC_ERR(Connection, SQLNativeSqlW(Connection, in_buf, -1, out_buf, BUF_SIZE, &out_len), iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, INVALID_BUF_SIZE_ERROR);

    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_ERR(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, -1, &out_len), "HY090", 0, INVALID_BUF_SIZE_ERROR);

    return OK;
}


ODBC_TEST(buffers_exec_direct) {
#define TABLE "buffers_exec_direct"
    SQLCHAR in_buf[BUF_SIZE];
    const char* query;
    const char* long_query;
    SQLINTEGER in_id = 3;
    SQLINTEGER out_id;
    SQLCHAR out_text[10];

    // Simple use
    query = "CREATE TABLE " TABLE "(id INT, text VARCHAR(16))";
    strcpy(in_buf, query);
    CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, in_buf, strlen(in_buf)));
    IS(!strcmp(in_buf, query));

    query = "INSERT INTO " TABLE " VALUES (1, 'aa'), (2, 'bc'), (3, 'ca')";
    strcpy(in_buf, query);
    CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, in_buf, SQL_NTS));
    IS(!strcmp(in_buf, query));

    // Shortcut in buff
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &in_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_SLONG, &out_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, &out_text, sizeof(out_text), NULL));

    query = "SELECT * FROM " TABLE " WHERE id = ?";
    long_query = "SELECT * FROM " TABLE " WHERE id = ?BLAH";
    strcpy(in_buf, long_query);
    CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, in_buf, strlen(query)));
    IS(!strcmp(in_buf, long_query));

    FETCH(Stmt);
    IS(in_id == 3);
    IS(out_id == 3);
    IS(!strcmp(out_text, "ca"));
    CLOSE(Stmt);

    // Bad use
    CHECK_STMT_ERR(Stmt, SQLExecDirect(Stmt, NULL, 0), "HY009", iOdbc() ? -1 : 0, NULL_ARG_ERROR);

    CHECK_STMT_ERR(Stmt, SQLExecDirect(Stmt, NULL, SQL_NTS), "HY009", iOdbc() ? -1 : 0, NULL_ARG_ERROR);

    if (iOdbc()) {
      CHECK_STMT_ERR(Stmt, SQLExecDirect(Stmt, in_buf, 0), "42000", 0, "Syntax error or access violation");
    } else {
      CHECK_STMT_ERR(Stmt, SQLExecDirect(Stmt, in_buf, 0), "HY090", 0, "Invalid string or buffer length");
    }
// native error is not set properly by iODBC with Xcode 11.7 libraries
#if !defined(__APPLE__)
    // S1090 is returned on MacOS instead of HY090, but error message is correct
    CHECK_STMT_ERR(Stmt, SQLExecDirect(Stmt, in_buf, -1), iOdbc() ? "S1090" : "HY090", 0, "Invalid string or buffer length");
#else
    SQLCHAR SQLState[6];
    SQLINTEGER NativeError;
    SQLCHAR SQLMessage[SQL_MAX_MESSAGE_LENGTH];
    SQLExecDirect(Stmt, in_buf, -1);
    const SQLRETURN result = SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, SQLState, &NativeError, SQLMessage, SQL_MAX_MESSAGE_LENGTH, NULL);
    if (!endswith(SQLMessage, "Invalid string or buffer length"))
      return FAIL;
#endif

    *in_buf = 0;
    CHECK_STMT_ERR(Stmt, SQLExecDirect(Stmt, in_buf, BUF_SIZE), "42000", 0, "Syntax error or access violation");

    UNBIND(Stmt);
    return OK;
#undef TABLE
}

ODBC_TEST(buffers_exec_direct_w) {

    //This test doesn't work with iODBC Ansi Driver
    if (iOdbc() && is_ansi_driver()) {
        return OK;
    }

#define TABLE "buffers_exec_direct_w"
    SQLWCHAR* in_buf;
    const char* query;
    const char* long_query;
    SQLINTEGER in_id = 3;
    SQLINTEGER out_id;
    SQLWCHAR out_text[10];

    // Simple use
    query = "CREATE TABLE " TABLE "(id INT, text VARCHAR(16))";
    in_buf = CW(query);
    CHECK_STMT_RC(Stmt, SQLExecDirectW(Stmt, in_buf, sqlwcharlen(in_buf)));
    IS(sqlwcharequal(in_buf, CW(query)));

    query = "INSERT INTO " TABLE " VALUES (1, 'aa'), (2, 'bc'), (3, 'ca')";
    in_buf = CW(query);
    CHECK_STMT_RC(Stmt, SQLExecDirectW(Stmt, in_buf, SQL_NTS));
    IS(sqlwcharequal(in_buf, CW(query)));

    SQLLEN len1;
    SQLLEN len2;

    // Shortcut in buff
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &in_id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_SLONG, &out_id, 0, iOdbc() ? &len1 : NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_WCHAR, &out_text, sizeof(out_text), iOdbc() ? &len2 : NULL));

    query = "SELECT * FROM " TABLE " WHERE id = ?";
    long_query = "SELECT * FROM " TABLE " WHERE id = ?BLAH";
    in_buf = CW(long_query);
    CHECK_STMT_RC(Stmt, SQLExecDirectW(Stmt, in_buf, strlen(query)));
    IS(sqlwcharequal(in_buf, CW(long_query)));

    FETCH(Stmt);
    IS(in_id == 3);
    IS(out_id == 3);
    IS(sqlwcharequal(out_text, CW("ca")));
    CLOSE(Stmt);

    // Bad use
    CHECK_STMT_ERR(Stmt, SQLExecDirectW(Stmt, NULL, 0), "HY009", iOdbc() ? -1 : 0, NULL_ARG_ERROR);

    CHECK_STMT_ERR(Stmt, SQLExecDirectW(Stmt, NULL, SQL_NTS), "HY009", iOdbc() ? -1 : 0, NULL_ARG_ERROR);

    if (iOdbc()) {
      CHECK_STMT_ERR(Stmt, SQLExecDirectW(Stmt, in_buf, 0), "42000", 0, "Syntax error or access violation");
    } else {
      CHECK_STMT_ERR(Stmt, SQLExecDirectW(Stmt, in_buf, 0), "HY090", 0, "Invalid string or buffer length");
    }
// native error is not set properly by iODBC with Xcode 11.7 libraries
#if !defined(__APPLE__)
    // S1090 is returned on MacOS instead of HY090, but error message is correct
    CHECK_STMT_ERR(Stmt, SQLExecDirectW(Stmt, in_buf, -1), iOdbc() ? "S1090" : "HY090", 0, "Invalid string or buffer length");
#else
    SQLCHAR SQLState[6];
    SQLINTEGER NativeError;
    SQLCHAR SQLMessage[SQL_MAX_MESSAGE_LENGTH];
    SQLExecDirect(Stmt, in_buf, -1);
    const SQLRETURN result = SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, SQLState, &NativeError, SQLMessage, SQL_MAX_MESSAGE_LENGTH, NULL);
    if (!endswith(SQLMessage, "Invalid string or buffer length"))
      return FAIL;
#endif

    *in_buf = 0;
    CHECK_STMT_ERR(Stmt, SQLExecDirectW(Stmt, in_buf, BUF_SIZE), "42000", 0, "Syntax error or access violation");

    UNBIND(Stmt);
    return OK;
#undef TABLE
}


#define NUM_PROC_FIELDS 4

static int test_proc_columns() {
    SQLCHAR buf[BUF_SIZE];
    SQLULEN num_rows = 0;

    // We'll check only the dataset size
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)(NUM_PROC_FIELDS + 1), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &num_rows, 0));

    // With id mode OFF query strings are treated as patterns
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)SQL_FALSE, SQL_IS_UINTEGER));

    // NULL buffers
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 3);

    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 3);

    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, NULL, 1000, NULL, 1000, NULL, 1000, NULL, 1000));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 3);

    // Empty strings
    *buf = 0;

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, buf, 0, buf, 0, buf, 0, buf, 0));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, buf, SQL_NTS, buf, SQL_NTS, buf, SQL_NTS, buf, SQL_NTS));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, buf, BUF_SIZE, buf, BUF_SIZE, buf, BUF_SIZE, buf, BUF_SIZE));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    IS(*buf == 0);

    // Invalid use
    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt,
                                            (SQLCHAR*)"odbc_test", -1000,
                                            NULL, 0,
                                            (SQLCHAR*)"aaa", strlen("aaa"),
                                            (SQLCHAR*)"a11", strlen("a11")),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt,
                                             (SQLCHAR*)"odbc_test", strlen("odbc_test"),
                                             NULL, 0,
                                             (SQLCHAR*)"aaa", -1000,
                                             (SQLCHAR*)"a11", strlen("a11")),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt,
                                             (SQLCHAR*)"odbc_test", strlen("odbc_test"),
                                             NULL, 0,
                                             (SQLCHAR*)"aaa", strlen("aaa"),
                                             (SQLCHAR*)"a11", -1000),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    // Normal runs
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt,
                                            (SQLCHAR*)"odbc_test", strlen("odbc_test"),
                                            NULL, 0,
                                            (SQLCHAR*)"aaa", strlen("aaa"),
                                            (SQLCHAR*)"a11", strlen("a11")));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt,
                                            (SQLCHAR*)"odbc_test", SQL_NTS,
                                            NULL, 0,
                                            (SQLCHAR*)"aaa", SQL_NTS,
                                            (SQLCHAR*)"a11", SQL_NTS));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    // Shortcut buffers
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt,
                                            (SQLCHAR*)"odbc_testBLAH", strlen("odbc_test"),
                                            NULL, 0,
                                            (SQLCHAR*)"aaaBLAH", strlen("aaa"),
                                            (SQLCHAR*)"a11BLAH", strlen("a11")));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, SQL_IS_UINTEGER));

    // NULL buffers
    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0), NULL_PTR_ERROR);
    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS), NULL_PTR_ERROR);
    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt, NULL, 1000, NULL, 1000, NULL, 1000, NULL, 1000), NULL_PTR_ERROR);

    // Empty strings
    *buf = 0;

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, buf, 0, buf, 0, buf, 0, buf, 0));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, buf, SQL_NTS, buf, SQL_NTS, buf, SQL_NTS, buf, SQL_NTS));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, buf, BUF_SIZE, buf, BUF_SIZE, buf, BUF_SIZE, buf, BUF_SIZE));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    IS(*buf == 0);

    // Invalid use
    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt,
                                             (SQLCHAR*)"odbc_test", -1000,
                                             NULL, 0,
                                             (SQLCHAR*)"aaa", strlen("aaa"),
                                             (SQLCHAR*)"a11", strlen("a11")),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt,
                                             (SQLCHAR*)"odbc_test", strlen("odbc_test"),
                                             NULL, 0,
                                             (SQLCHAR*)"aaa", -1000,
                                             (SQLCHAR*)"a11", strlen("a11")),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    CHECK_STMT_ERR(Stmt, SQLProcedureColumns(Stmt,
                                             (SQLCHAR*)"odbc_test", strlen("odbc_test"),
                                             NULL, 0,
                                             (SQLCHAR*)"aaa", strlen("aaa"),
                                             (SQLCHAR*)"a11", -1000),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    // Normal runs
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt,
                                            (SQLCHAR*)"odbc_test", strlen("odbc_test"),
                                            "", 0,
                                            (SQLCHAR*)"aaa", strlen("aaa"),
                                            (SQLCHAR*)"a11", strlen("a11")));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt,
                                            (SQLCHAR*)"odbc_test", SQL_NTS,
                                            "", 0,
                                            (SQLCHAR*)"aaa", SQL_NTS,
                                            (SQLCHAR*)"a11", SQL_NTS));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    // Shortcut buffers
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt,
                                            (SQLCHAR*)"odbc_testBLAH", strlen("odbc_test"),
                                            "", 0,
                                            (SQLCHAR*)"aaaBLAH", strlen("aaa"),
                                            (SQLCHAR*)"a11BLAH", strlen("a11")));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    return OK;
}

static int test_proc_columns_w() {
    SQLWCHAR buf[BUF_SIZE];
    SQLULEN num_rows = 0;

    // We'll check only the dataset size
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)(NUM_PROC_FIELDS + 1), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &num_rows, 0));

    // With id mode OFF query strings are treated as patterns
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER)SQL_FALSE, SQL_IS_UINTEGER));

    // NULL buffers
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 3);

    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 3);

    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, NULL, 1000, NULL, 1000, NULL, 1000, NULL, 1000));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 3);

    // Empty strings
    *buf = 0;

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, buf, 0, buf, 0, buf, 0, buf, 0));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, buf, SQL_NTS, buf, SQL_NTS, buf, SQL_NTS, buf, SQL_NTS));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, buf, BUF_SIZE, buf, BUF_SIZE, buf, BUF_SIZE, buf, BUF_SIZE));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    IS(*buf == 0);

    // Invalid use
    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(  Stmt,
                                                CW("odbc_test"), -1000,
                                                NULL, 0,
                                                CW("aaa"), strlen("aaa"),
                                                CW("a11"), strlen("a11")),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(  Stmt,
                                                CW("odbc_test"), strlen("odbc_test"),
                                                NULL, 0,
                                                CW("aaa"), -1000,
                                                CW("a11"), strlen("a11")),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(  Stmt,
                                                CW("odbc_test"), strlen("odbc_test"),
                                                NULL, 0,
                                                CW("aaa"), strlen("aaa"),
                                                CW("a11"), -1000),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    // Normal runs
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(   Stmt,
                                                CW("odbc_test"), strlen("odbc_test"),
                                                NULL, 0,
                                                CW("aaa"), strlen("aaa"),
                                                CW("a11"), strlen("a11")));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(   Stmt,
                                                CW("odbc_test"), SQL_NTS,
                                                NULL, 0,
                                                CW("aaa"), SQL_NTS,
                                                CW("a11"), SQL_NTS));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    // Shortcut buffers
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(   Stmt,
                                                CW("odbc_testBLAH"), strlen("odbc_test"),
                                                NULL, 0,
                                                CW("aaaBLAH"), strlen("aaa"),
                                                CW("a11BLAH"), strlen("a11")));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, SQL_IS_UINTEGER));

    // NULL buffers
    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(Stmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0), NULL_PTR_ERROR);
    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(Stmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS), NULL_PTR_ERROR);
    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(Stmt, NULL, 1000, NULL, 1000, NULL, 1000, NULL, 1000), NULL_PTR_ERROR);

    // Empty strings
    *buf = 0;

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, buf, 0, buf, 0, buf, 0, buf, 0));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, buf, SQL_NTS, buf, SQL_NTS, buf, SQL_NTS, buf, SQL_NTS));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, buf, BUF_SIZE, buf, BUF_SIZE, buf, BUF_SIZE, buf, BUF_SIZE));
    FETCH_NO_DATA(Stmt);
    CLOSE(Stmt);

    IS(*buf == 0);

    // Invalid use
    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(  Stmt,
                                                CW("odbc_test"), -1000,
                                                NULL, 0,
                                                CW("aaa"), strlen("aaa"),
                                                CW("a11"), strlen("a11")),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(  Stmt,
                                                CW("odbc_test"), strlen("odbc_test"),
                                                NULL, 0,
                                                CW("aaa"), -1000,
                                                CW("a11"), strlen("a11")),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    CHECK_STMT_ERR(Stmt, SQLProcedureColumnsW(  Stmt,
                                                CW("odbc_test"), strlen("odbc_test"),
                                                NULL, 0,
                                                CW("aaa"), strlen("aaa"),
                                                CW("a11"), -1000),
                   iOdbc() ? "S1090" : "HY090", iOdbc() ? -1 : 0, "Invalid string or buffer length");

    // Normal runs
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(   Stmt,
                                                CW("odbc_test"), strlen("odbc_test"),
                                                CW(""), 0,
                                                CW("aaa"), strlen("aaa"),
                                                CW("a11"), strlen("a11")));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(   Stmt,
                                                CW("odbc_test"), SQL_NTS,
                                                CW(""), 0,
                                                CW("aaa"), SQL_NTS,
                                                CW("a11"), SQL_NTS));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    // Shortcut buffers
    num_rows = 0;
    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(   Stmt,
                                                CW("odbc_testBLAH"), strlen("odbc_test"),
                                                CW(""), 0,
                                                CW("aaaBLAH"), strlen("aaa"),
                                                CW("a11BLAH"), strlen("a11")));
    FETCH(Stmt);
    CLOSE(Stmt);
    IS(num_rows == 1);

    return OK;
}

ODBC_TEST(buffers_proc_columns) {
    int res;

    OK_SIMPLE_STMT(Stmt, "CREATE FUNCTION aaa (a11 INT, aa1 INT) RETURNS INT AS BEGIN RETURN 0; END");
    OK_SIMPLE_STMT(Stmt, "CREATE PROCEDURE bbb (b22 INT) AS BEGIN SET b22 = 0; END");

    res = test_proc_columns();
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    UNBIND(Stmt);

    res &= test_proc_columns_w();
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    UNBIND(Stmt);

    OK_SIMPLE_STMT(Stmt, "DROP FUNCTION aaa");
    OK_SIMPLE_STMT(Stmt, "DROP PROCEDURE bbb");

    return res;
}


MA_ODBC_TESTS my_tests[] = {
   {buffers_native_sql, "buffers_native_sql", NORMAL, ALL_DRIVERS},
   {buffers_native_sql_w, "buffers_native_sql_w", NORMAL, ALL_DRIVERS},
   {buffers_exec_direct, "buffers_exec_direct", NORMAL, ALL_DRIVERS},
   {buffers_exec_direct_w, "buffers_exec_direct_w", NORMAL, ALL_DRIVERS},
   {buffers_proc_columns, "buffers_proc_columns", NORMAL, ALL_DRIVERS},
   {NULL, NULL, NORMAL, ALL_DRIVERS}
};


int main(int argc, char **argv) {
    int tests = LENGTHOF(my_tests) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}
