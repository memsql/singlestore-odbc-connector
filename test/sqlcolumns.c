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

int run_sql_columns(SQLHANDLE Stmt, const SQLSMALLINT *ExpDataType, const SQLSMALLINT *ExpSqlDataType) {
  const int ExpNumOfRowsFetched = 33;
  SQLCHAR *ExpTableCat = my_schema;
  SQLCHAR *ExpTableName = "t_types";
  int utf8_char_size = getDbCharSize();

  char *ExpTypeName[33] = {"tinyint", "smallint", "mediumint", "int(10) unsigned", "bigint(20) unsigned", "double", "float",
                             "decimal", "date", "time", "datetime", "datetime", "timestamp", "timestamp", "year",
                             "char", "binary", "varchar", "varbinary", "longtext", "mediumtext", "text", "tinytext",
                             "longblob", "mediumblob", "blob", "tinyblob", "bit",
                             "json", "geography", "geographypoint", "enum", "set"};
  SQLINTEGER ExpColSize[33] = {3, 5, 7, 10, 20, 15, 7, 10, 10, 8, 19, 26, 19, 26, 4, 11, 1, 13, 17,
                               // values below are computed as maximum memory for type divided by the char size in the default charset.
                               // Default charset is UTF8MB3 for server older than 7.5.0, UTF8MB4 starting with 7.5
                               utf8_char_size > 3 ? 1073741823 : 1431655765,
                               utf8_char_size > 3 ? 4194303 : 5592405,
                               utf8_char_size > 3 ? 16383 : 21845,
                               utf8_char_size > 3 ? 63 : 85, 2147483647, 16777215, 65535, 255, 8, -1, -1, -1, 1,
                               1};
  SQLSMALLINT ExpDecimalDigits[33] = {0, 0, 0, 0, 0, -1, -1, 5, -1, 0, 0, 6, 0, 6,
                                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  SQLSMALLINT ExpNumPrecRadix[33] = {10, 10, 10, 10, 10, 10, 10, 10, -1, -1, -1, -1, -1, -1,
                                     10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  SQLSMALLINT ExpDatetimeSub[33] = {-1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 3, 3, 3, 3,
                                    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_types");
  OK_SIMPLE_STMT(Stmt, ServerNotOlderThan(Connection, 7, 3, 0) ?
                                                               "CREATE ROWSTORE TABLE t_types (a TINYINT, b SMALLINT, c MEDIUMINT, d INT UNSIGNED, e BIGINT UNSIGNED, f DOUBLE, g FLOAT,"
                                                               "h DECIMAL(10, 5), i DATE, j TIME, k DATETIME, l DATETIME(6), m TIMESTAMP, n TIMESTAMP(6), o YEAR,"
                                                               "p CHAR(11), q BINARY, r VARCHAR(13), s VARBINARY(17), t LONGTEXT, u MEDIUMTEXT, v TEXT, w TINYTEXT,"
                                                               "x LONGBLOB, y MEDIUMBLOB, z BLOB, aa TINYBLOB, ab BIT(1),"
                                                               "ac JSON, ad GEOGRAPHY, ae GEOGRAPHYPOINT, af ENUM('e'), ag SET('s'))" :

                                                               "CREATE TABLE t_types (a TINYINT, b SMALLINT, c MEDIUMINT, d INT UNSIGNED, e BIGINT UNSIGNED, f DOUBLE, g FLOAT,"
                                                               "h DECIMAL(10, 5), i DATE, j TIME, k DATETIME, l DATETIME(6), m TIMESTAMP, n TIMESTAMP(6), o YEAR,"
                                                               "p CHAR(11), q BINARY, r VARCHAR(13), s VARBINARY(17), t LONGTEXT, u MEDIUMTEXT, v TEXT, w TINYTEXT,"
                                                               "x LONGBLOB, y MEDIUMBLOB, z BLOB, aa TINYBLOB, ab BIT(1),"
                                                               "ac JSON, ad GEOGRAPHY, ae GEOGRAPHYPOINT, af ENUM('e'), ag SET('s'))");

  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLColumns(Stmt, ExpTableCat, SQL_NTS, NULL, 0,
                                                    (SQLCHAR *) "t_types", SQL_NTS, NULL, 0));

  SQLCHAR tableCat[SQL_COLUMNS_BUFFER_LEN], tableSchema[SQL_COLUMNS_BUFFER_LEN], tableName[SQL_COLUMNS_BUFFER_LEN], colName[SQL_COLUMNS_BUFFER_LEN], typeName[SQL_COLUMNS_BUFFER_LEN];
  SQLCHAR remarks[SQL_COLUMNS_BUFFER_LEN], colDefault[SQL_COLUMNS_BUFFER_LEN], isNullable[SQL_COLUMNS_BUFFER_LEN];
  SQLLEN tsSize, csSize, blSize, ddSize, nprSize, rSize, cdSize, dscSize, colSize;
  SQLSMALLINT dataType, decimalDigits, numPrecRadix, nullable, sqlDataType, datetimeSubtypeCode;
  SQLINTEGER columnSize, charOctetLength, bufferLength, ordinalPosition;

  // Bind columns in result set to buffers
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, tableCat, SQL_COLUMNS_BUFFER_LEN, NULL));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, tableSchema, SQL_COLUMNS_BUFFER_LEN, &tsSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, tableName, SQL_COLUMNS_BUFFER_LEN, NULL));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, colName, SQL_COLUMNS_BUFFER_LEN, NULL));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &dataType, 0, NULL));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 6, SQL_C_CHAR, typeName, SQL_COLUMNS_BUFFER_LEN, NULL));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 7, SQL_C_LONG, &columnSize, 0, &csSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 8, SQL_C_LONG, &bufferLength, 0, &blSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 9, SQL_C_SHORT, &decimalDigits, 0, &ddSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 10, SQL_C_SHORT, &numPrecRadix, 0, &nprSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 11, SQL_C_SHORT, &nullable, 0, NULL));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 12, SQL_C_CHAR, remarks, SQL_COLUMNS_BUFFER_LEN, &rSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 13, SQL_C_CHAR, colDefault, SQL_COLUMNS_BUFFER_LEN, &cdSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 14, SQL_C_SHORT, &sqlDataType, 0, NULL));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 15, SQL_C_SHORT, &datetimeSubtypeCode, 0, &dscSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 16, SQL_C_LONG, &charOctetLength, 0, &colSize));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 17, SQL_C_LONG, &ordinalPosition, 0, NULL));
  CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 18, SQL_C_CHAR, isNullable, SQL_COLUMNS_BUFFER_LEN, NULL));

  int rc;
  int numOfRowsFetched = 0;
  while ((rc = SQLFetch(Stmt)) == SQL_SUCCESS) {
    FAIL_IF(_stricmp(tableCat, ExpTableCat) != 0, "Wrong TABLE_CAT returned!");
    FAIL_IF(tsSize != SQL_NULL_DATA, "TABLE_SCHEMA must be null!");
    FAIL_IF(_stricmp(tableName, ExpTableName) != 0, "Wrong TABLE_NAME returned!")

    SQLCHAR ExpColName[3] = {0};
    ExpColName[0] = numOfRowsFetched >= 26 ? (numOfRowsFetched - 26) / 26 + 'a' : (numOfRowsFetched % 26) + 'a';
    ExpColName[1] = numOfRowsFetched >= 26 ? (numOfRowsFetched % 26) + 'a' : '\0';
    FAIL_IF(_stricmp(colName, ExpColName) != 0, "Wrong COLUMN_NAME returned!");
    is_num(dataType, ExpDataType[numOfRowsFetched]);
    FAIL_IF(_strnicmp(typeName, ExpTypeName[numOfRowsFetched], strlen(ExpTypeName[numOfRowsFetched])) != 0, "Wrong TYPE_NAME returned!");

    if (ExpColSize[numOfRowsFetched] != SQL_NULL_DATA) {
        is_num(columnSize, ExpColSize[numOfRowsFetched]);
    }
    if (ExpDecimalDigits[numOfRowsFetched] != SQL_NULL_DATA) {
        is_num(decimalDigits, ExpDecimalDigits[numOfRowsFetched]);
    }
    if (ExpNumPrecRadix[numOfRowsFetched] != SQL_NULL_DATA) {
        FAIL_IF(numPrecRadix != ExpNumPrecRadix[numOfRowsFetched], "Wrong NUM_PREC_RADIX returned!");
    }
    FAIL_IF(sqlDataType != ExpSqlDataType[numOfRowsFetched], "Wrong SQL_DATA_TYPE returned!");
    if (ExpDatetimeSub[numOfRowsFetched] != SQL_NULL_DATA) {
        FAIL_IF(datetimeSubtypeCode != ExpDatetimeSub[numOfRowsFetched], "Wrong SQL_DATETIME_SUB returned!");
    }
    FAIL_IF(ordinalPosition != ++numOfRowsFetched, "Wrong ORDINAL_POSITION returned!");
  }

  diag("Fetched %d rows", numOfRowsFetched);
  FAIL_IF(numOfRowsFetched != ExpNumOfRowsFetched, "wrong number of rows fetched");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_types");

  return OK;
}

#define N_COLUMNS_LARGE 520
int run_sql_columns_large(SQLHANDLE Stmt, const SQLSMALLINT *ExpDataType, const SQLSMALLINT *ExpSqlDataType) {
    char query[N_COLUMNS_LARGE * 20 + 40] = "CREATE TABLE t_types (";
    int i;
    for (i = 0; i < N_COLUMNS_LARGE; i++)
    {
        sprintf(query + strlen(query), "c_%d INT,", i);
    }
    sprintf(query + strlen(query), "c_%d INT)", i);
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_types");
    OK_SIMPLE_STMT(Stmt, query);


    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLColumns(Stmt, my_schema, SQL_NTS, NULL, 0,
                                                    (SQLCHAR *) "t_types", SQL_NTS, NULL, 0));

    SQLCHAR tableCat[SQL_COLUMNS_BUFFER_LEN], tableSchema[SQL_COLUMNS_BUFFER_LEN], tableName[SQL_COLUMNS_BUFFER_LEN], colName[SQL_COLUMNS_BUFFER_LEN], typeName[SQL_COLUMNS_BUFFER_LEN];
    SQLCHAR remarks[SQL_COLUMNS_BUFFER_LEN], colDefault[SQL_COLUMNS_BUFFER_LEN], isNullable[SQL_COLUMNS_BUFFER_LEN];
    SQLLEN tsSize, csSize, blSize, ddSize, nprSize, rSize, cdSize, dscSize, colSize;
    SQLSMALLINT dataType, decimalDigits, numPrecRadix, nullable, sqlDataType, datetimeSubtypeCode;
    SQLINTEGER columnSize, charOctetLength, bufferLength, ordinalPosition;

    // Bind columns in result set to buffers
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, tableCat, SQL_COLUMNS_BUFFER_LEN, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, tableSchema, SQL_COLUMNS_BUFFER_LEN, &tsSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, tableName, SQL_COLUMNS_BUFFER_LEN, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, colName, SQL_COLUMNS_BUFFER_LEN, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &dataType, 0, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 6, SQL_C_CHAR, typeName, SQL_COLUMNS_BUFFER_LEN, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 7, SQL_C_LONG, &columnSize, 0, &csSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 8, SQL_C_LONG, &bufferLength, 0, &blSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 9, SQL_C_SHORT, &decimalDigits, 0, &ddSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 10, SQL_C_SHORT, &numPrecRadix, 0, &nprSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 11, SQL_C_SHORT, &nullable, 0, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 12, SQL_C_CHAR, remarks, SQL_COLUMNS_BUFFER_LEN, &rSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 13, SQL_C_CHAR, colDefault, SQL_COLUMNS_BUFFER_LEN, &cdSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 14, SQL_C_SHORT, &sqlDataType, 0, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 15, SQL_C_SHORT, &datetimeSubtypeCode, 0, &dscSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 16, SQL_C_LONG, &charOctetLength, 0, &colSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 17, SQL_C_LONG, &ordinalPosition, 0, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 18, SQL_C_CHAR, isNullable, SQL_COLUMNS_BUFFER_LEN, NULL));

    int rc;
    int numOfRowsFetched = 0;
    while ((rc = SQLFetch(Stmt)) == SQL_SUCCESS)
    {
        char expColName[7];
        sprintf(expColName, "c_%d", numOfRowsFetched);

        FAIL_IF(_stricmp(colName, expColName) != 0, "Wrong COLUMN_NAME returned!");
        is_num(ordinalPosition, ++numOfRowsFetched);
        is_num(dataType, SQL_INTEGER);
    }
    diag("Fetched %d rows", numOfRowsFetched);
    return OK;
}

ODBC_TEST(t_columns3U) {
    SQLSMALLINT ExpDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                   SQL_REAL,
                                   SQL_DECIMAL, SQL_TYPE_DATE, SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_SMALLINT,
                                   SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                   SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                   SQL_WLONGVARCHAR, SQL_WCHAR, SQL_WCHAR, SQL_WVARCHAR, SQL_WVARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_REAL,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                      SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                      SQL_WLONGVARCHAR, SQL_WCHAR, SQL_WCHAR, SQL_WVARCHAR, SQL_WVARCHAR};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLWCHAR    *connw, connw_out[1024];
    SQLSMALLINT conn_out_len;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));

    connw= CW(conn);
    CHECK_DBC_RC(Connection1, SQLDriverConnectW(Connection1, NULL, connw, SQL_NTS, connw_out,
                                                sizeof(connw_out)/sizeof(SQLWCHAR), &conn_out_len,
                                                SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_columns(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_columns3A) {
    SQLSMALLINT ExpDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                   SQL_REAL,
                                   SQL_DECIMAL, SQL_TYPE_DATE, SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_SMALLINT,
                                   SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                   SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                   SQL_LONGVARCHAR, SQL_CHAR, SQL_CHAR, SQL_VARCHAR, SQL_VARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_REAL,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                      SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                      SQL_LONGVARCHAR, SQL_CHAR, SQL_CHAR, SQL_VARCHAR, SQL_VARCHAR};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));

    CHECK_DBC_RC(Connection1, SQLConnect(Connection1, my_dsn, SQL_NTS, my_uid, SQL_NTS,
                                         my_pwd, SQL_NTS));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_columns(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_columns2U) {
    SQLSMALLINT ExpDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                   SQL_REAL,
                                   SQL_DECIMAL, SQL_DATE, SQL_TIME, SQL_TIMESTAMP, SQL_TIMESTAMP,
                                   SQL_TIMESTAMP, SQL_TIMESTAMP, SQL_SMALLINT,
                                   SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                   SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                   SQL_WLONGVARCHAR, SQL_WCHAR, SQL_WCHAR, SQL_WVARCHAR, SQL_WVARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_REAL,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                      SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                      SQL_WLONGVARCHAR, SQL_WCHAR, SQL_WCHAR, SQL_WVARCHAR, SQL_WVARCHAR};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLWCHAR    *connw, connw_out[1024];
    SQLSMALLINT conn_out_len;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC2, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));

    connw= CW(conn);
    CHECK_DBC_RC(Connection1, SQLDriverConnectW(Connection1, NULL, connw, SQL_NTS, connw_out,
                                                sizeof(connw_out)/sizeof(SQLWCHAR), &conn_out_len,
                                                SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_columns(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_columns2A) {
  SQLSMALLINT ExpDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                   SQL_REAL,
                                   SQL_DECIMAL, SQL_DATE, SQL_TIME, SQL_TIMESTAMP, SQL_TIMESTAMP,
                                   SQL_TIMESTAMP, SQL_TIMESTAMP, SQL_SMALLINT,
                                   SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                   SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                   SQL_LONGVARCHAR, SQL_CHAR, SQL_CHAR, SQL_VARCHAR, SQL_VARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_REAL,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                      SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                      SQL_LONGVARCHAR, SQL_CHAR, SQL_CHAR, SQL_VARCHAR, SQL_VARCHAR};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC2, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_columns(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_columns_long) {
    SQLSMALLINT ExpDataType[N_COLUMNS_LARGE] = {SQL_INTEGER};
    SQLSMALLINT ExpSqlDataType[N_COLUMNS_LARGE] = {SQL_INTEGER};

    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    sprintf((char *) conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                        (SQLPOINTER) SQL_OV_ODBC2, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                    SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                    NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_columns_large(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLColumns with 300 columns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}


MA_ODBC_TESTS my_tests[] =
{
    {t_columns3U, "t_columns3U", NORMAL, UNICODE_DRIVER},
    {t_columns3A, "t_columns3A", NORMAL, ANSI_DRIVER},
    {t_columns2U, "t_columns2U", NORMAL, UNICODE_DRIVER},
    {t_columns2A, "t_columns2A", NORMAL, ANSI_DRIVER},
    {t_columns_long, "t_columns_long", NORMAL, ALL_DRIVERS},
    {NULL, NULL, NORMAL, ALL_DRIVERS}
};


int main(int argc, char **argv) {
    int tests = sizeof(my_tests) / sizeof(MA_ODBC_TESTS) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}