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

#define SQL_COLUMNS_BUFFER_LEN 256

SQLCHAR *params_no_cache_false = "NO_CACHE=0";

int run_sql_special_columns(SQLHANDLE Stmt, const SQLSMALLINT *ExpDataType) {
    const int ExpNumOfRowsFetched = 10;

    char *ExpTypeName[10] = {"int unsigned", "bigint", "double", "decimal", "date", "datetime", "timestamp", "char", "binary", "text"};
    SQLINTEGER ExpColSize[10] = {10, 19, 22, 10, 10, 19, 26, 11, 1,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 21845 : 65535};
    SQLSMALLINT ExpDecimalDigits[10] = {0, 0, 6, 5, 0, 0, 6, -1, -1, -1};

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_types");
    OK_SIMPLE_STMT(Stmt, ServerNotOlderThan(Connection, 7, 3, 0) ?
                                                                 "CREATE ROWSTORE TABLE t_types (a INT UNSIGNED, b BIGINT, c DOUBLE, d DECIMAL(10, 5), "
                                                                 "e DATE, f DATETIME, g TIMESTAMP(6), h CHAR(11), i BINARY, j TEXT, "
                                                                 "aa TINYINT, ab SMALLINT, ac FLOAT, ad TIME, ae TINYTEXT, af VARBINARY(15), ag BLOB, "
                                                                 "PRIMARY KEY(a, b, c, d, e, f, g, h, i, j))" :
                                                                 "CREATE TABLE t_types (a INT UNSIGNED, b BIGINT, c DOUBLE, d DECIMAL(10, 5), "
                                                                 "e DATE, f DATETIME, g TIMESTAMP(6), h CHAR(11), i BINARY, j TEXT, "
                                                                 "aa TINYINT, ab SMALLINT, ac FLOAT, ad TIME, ae TINYTEXT, af VARBINARY(15), ag BLOB, "
                                                                 "PRIMARY KEY(a, b, c, d, e, f, g, h, i, j))");

    // Fetch only primary key columns.
    CHECK_STMT_RC(Stmt, SQLSpecialColumns(Stmt, SQL_BEST_ROWID, my_schema, SQL_NTS, NULL, 0,
                                                      (SQLCHAR *) "t_types", SQL_NTS, SQL_SCOPE_CURROW, SQL_NULLABLE));

    SQLCHAR colName[SQL_COLUMNS_BUFFER_LEN], typeName[SQL_COLUMNS_BUFFER_LEN];
    SQLLEN scopeInd, cnSize, dtSize, tnSize, csSize, blSize, ddSize, pcLen;
    SQLSMALLINT scope, decimalDigits, pseudoColumn;
    SQLINTEGER dataType, columnSize, bufferLength;

    // Bind columns in result set to buffers
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_SHORT, &scope, sizeof(SQLSMALLINT), &scopeInd));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, colName, SQL_COLUMNS_BUFFER_LEN, &cnSize));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_LONG, &dataType, sizeof(SQLINTEGER), &dtSize));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, typeName, SQL_COLUMNS_BUFFER_LEN, &tnSize));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_LONG, &columnSize, sizeof(SQLINTEGER), &csSize));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 6, SQL_C_LONG, &bufferLength, sizeof(SQLINTEGER), &blSize));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 7, SQL_C_SHORT, &decimalDigits, sizeof(SQLSMALLINT), &ddSize));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 8, SQL_C_SHORT, &pseudoColumn, sizeof(SQLSMALLINT), &pcLen));

    diag("Checking primary key columns...");
    int numOfRowsFetched = 0;
    while (SQL_SUCCEEDED(SQLFetch(Stmt)))
    {
        FAIL_IF(scopeInd != SQL_NULL_DATA, "Wrong SCOPE returned");

        SQLCHAR ExpColName[2] = { numOfRowsFetched + 'a', '\0'};
        FAIL_IF(_strnicmp(colName, ExpColName, cnSize) != 0, "Wrong COLUMN_NAME returned!");
        FAIL_IF(dataType != ExpDataType[numOfRowsFetched], "Wrong DATA_TYPE returned!");
        FAIL_IF(_strnicmp(typeName, ExpTypeName[numOfRowsFetched], tnSize) != 0, "Wrong TYPE_NAME returned!");
        FAIL_IF(columnSize != ExpColSize[numOfRowsFetched], "Wrong COLUMN_SIZE returned!");

        if (ExpDecimalDigits[numOfRowsFetched] != SQL_NULL_DATA) {
            FAIL_IF(decimalDigits != ExpDecimalDigits[numOfRowsFetched], "Wrong DECIMAL_DIGITS returned!");
        }
        FAIL_IF(pseudoColumn != SQL_PC_UNKNOWN, "Wrong PREUDO_COLUMN returned!");
        numOfRowsFetched++;
    }

    diag("Fetched %d rows", numOfRowsFetched);
    FAIL_IF(numOfRowsFetched != ExpNumOfRowsFetched, "wrong number of rows fetched");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    diag("Checking all columns...");
    // Fetch all columns.
    CHECK_STMT_RC(Stmt, SQLSpecialColumns(Stmt, SQL_ROWVER, my_schema, SQL_NTS, NULL, 0,
                                                             (SQLCHAR *) "t_types", SQL_NTS, SQL_SCOPE_CURROW, SQL_NULLABLE));

    SQLLEN rowCount;
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowCount));
    is_num(rowCount, 17); // Full table has 17 columns.

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_types");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}

ODBC_TEST(t_specialcolumns3U) {
    SQLSMALLINT ExpDataType[10] = {SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE, SQL_DECIMAL,
                                   SQL_TYPE_DATE, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   SQL_WCHAR, SQL_BINARY, SQL_WLONGVARCHAR};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLWCHAR    *connw, connw_out[1024];
    SQLSMALLINT conn_out_len;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, params_no_cache_false);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));

    connw= CW(conn);
    CHECK_DBC_RC(Connection1, SQLDriverConnectW(Connection1, NULL, connw, SQL_NTS, connw_out,
                                                sizeof(connw_out)/sizeof(SQLWCHAR), &conn_out_len,
                                                SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_special_columns(Stmt1, ExpDataType) != OK, "error running SQLSpecialColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_specialcolumns3A) {
    SQLSMALLINT ExpDataType[10] = {SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE, SQL_DECIMAL,
                                   SQL_TYPE_DATE, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   SQL_CHAR, SQL_BINARY, SQL_LONGVARCHAR};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, params_no_cache_false);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_special_columns(Stmt1, ExpDataType) != OK, "error running SQLSpecialColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_specialcolumns2U) {
    SQLSMALLINT ExpDataType[10] = {SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE, SQL_DECIMAL,
                                   SQL_DATE, SQL_TIMESTAMP, SQL_TIMESTAMP,
                                   SQL_WCHAR, SQL_BINARY, SQL_WLONGVARCHAR};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLWCHAR    *connw, connw_out[1024];
    SQLSMALLINT conn_out_len;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, params_no_cache_false);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC2, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));

    connw= CW(conn);
    CHECK_DBC_RC(Connection1, SQLDriverConnectW(Connection1, NULL, connw, SQL_NTS, connw_out,
                                                sizeof(connw_out)/sizeof(SQLWCHAR), &conn_out_len,
                                                SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_special_columns(Stmt1, ExpDataType) != OK, "error running SQLSpecialColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_specialcolumns2A) {
    SQLSMALLINT ExpDataType[10] = {SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE, SQL_DECIMAL,
                                   SQL_DATE, SQL_TIMESTAMP, SQL_TIMESTAMP,
                                   SQL_CHAR, SQL_BINARY, SQL_LONGVARCHAR};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, params_no_cache_false);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC2, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_special_columns(Stmt1, ExpDataType) != OK, "error running SQLSpecialColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}


MA_ODBC_TESTS my_tests[] =
        {
                {t_specialcolumns3U, "t_specialcolumns3U", NORMAL, UNICODE_DRIVER},
                {t_specialcolumns3A, "t_specialcolumns3A", NORMAL, ANSI_DRIVER},
                {t_specialcolumns2U, "t_specialcolumns2U", NORMAL, UNICODE_DRIVER},
                {t_specialcolumns2A, "t_specialcolumns2A", NORMAL, ANSI_DRIVER},
                {NULL, NULL, NORMAL, ALL_DRIVERS}
        };


int main(int argc, char **argv) {
    int tests = sizeof(my_tests) / sizeof(MA_ODBC_TESTS) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}