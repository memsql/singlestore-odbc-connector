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
#include <stdint.h>

#define SQL_COLUMNS_BUFFER_LEN 256
#define PROC_ON_FUNC_NATIVE_ERR_CODE 1347

#define CREATE_ROUTINE_TEMPLATE \
"CREATE %s test_procedure_columns(a TINYINT , b SMALLINT NOT NULL, c MEDIUMINT UNSIGNED , d INT NOT NULL, e BIGINT UNSIGNED , f DOUBLE NOT NULL, g FLOAT ,"\
"h DECIMAL(10, 5) NOT NULL, i DATE, j TIME NOT NULL , k DATETIME, l DATETIME(6) NOT NULL , m TIMESTAMP, n TIMESTAMP(6) NOT NULL , o YEAR,"\
"p CHAR(11) NOT NULL, q BINARY, r VARCHAR(13) NOT NULL , s VARBINARY(17), t LONGTEXT NOT NULL , u MEDIUMTEXT, v TEXT NOT NULL , w TINYTEXT,"\
"x LONGBLOB NOT NULL, y MEDIUMBLOB, z BLOB NOT NULL, aa TINYBLOB, ab BIT(1) NOT NULL,"\
"ac JSON DEFAULT '{}', ad GEOGRAPHY NOT NULL DEFAULT 'POINT(1, 1)', ae GEOGRAPHYPOINT DEFAULT 'POINT(1, 1)', af ENUM('e') NOT NULL DEFAULT 'e', ag SET('s') DEFAULT 's') "\
"RETURNS INT AS BEGIN RETURN 0; END"


static int process_drop_error(const char* const command) {
    SQLCHAR SQLState[6];
    SQLINTEGER NativeError;
    SQLCHAR SQLMessage[SQL_MAX_MESSAGE_LENGTH];
    SQLSMALLINT TextLengthPtr;

    const SQLRETURN result = SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, SQLState, &NativeError, SQLMessage, SQL_MAX_MESSAGE_LENGTH, &TextLengthPtr);
    if(result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO) {
        if(NativeError == PROC_ON_FUNC_NATIVE_ERR_CODE) {
            /* [HY000] (1347) [ss-0.8.2][7.1.12]Cannot DROP PROCEDURE on FUNCTION `test_procedure_columns` */
            return OK;
        }
        fprintf(stdout, "[%s] (%d) %s\n", SQLState, NativeError, SQLMessage);
    } else {
        fprintf(stdout, "Error %d getting diagnostic records with SQLGetDiagRec()\n", result);
    }

    return FAIL;
}

static int try_drop_check_error(const char* const command) {
    if(!SQL_SUCCEEDED(SQLExecDirect(Stmt, (SQLCHAR*)(command), (SQLINTEGER)strlen(command))))
    {
        IS_OK(process_drop_error(command));
    }
    /*	TODO: PLAT-5561
    if(!SQL_SUCCEEDED(SQLExecDirectW(Stmt, CW(command), SQL_NTS)))
    {
        IS_OK(process_drop_error(command));
    }
    */
    return OK;
}

/* This should delete all the procedures created by tests in this file so that test failures do not chain up.
 * Call at the beginning of each test.
 */
static int run_cleanup() {
    const char* const c_proc_names[] = {
        "test_procedure_columns",
        "TEST_GET_PRECISION_AND_ROUND_UP",
        "НАЙТИ_ПЕТЮ_И_ОКРУГЛИТЬ"
    };
    const char* const c_func_names[] = {
        "test_procedure_columns",
        "test_get_name",
        "`test set and get precision`",
        "узнать_имя_васи",
        "`взять и положить кого-то куда-то`"
    };
    char buffer[1000];
    unsigned i;

    for(i = 0; i < LENGTHOF(c_func_names); ++i) {
        _snprintf(buffer, sizeof(buffer), "DROP FUNCTION IF EXISTS %s", c_func_names[i]);
        IS_OK(try_drop_check_error(buffer));
    }
    for(i = 0; i < LENGTHOF(c_proc_names); ++i) {
        _snprintf(buffer, sizeof(buffer), "DROP PROCEDURE IF EXISTS %s", c_proc_names[i]);
        IS_OK(try_drop_check_error(buffer));
    }

    return OK;
}

int run_sql_procedurecolumns_routine_type(SQLHANDLE Stmt, const SQLSMALLINT *ExpDataType, const SQLSMALLINT *ExpSqlDataType, const char* RoutineType) {
    const int ExpNumOfRowsFetched = 33;
    SQLCHAR *ExpTableCat = my_schema;
    SQLCHAR *ExpRoutineName = "test_procedure_columns";
    char createStmtStr[1024];
    unsigned long crLength = strlen(CREATE_ROUTINE_TEMPLATE) + strlen(RoutineType) + 1;
    _snprintf(createStmtStr, crLength, CREATE_ROUTINE_TEMPLATE, RoutineType);

    char *ExpTypeName[33] = {"tinyint", "smallint", "mediumint unsigned", "int", "bigint unsigned", "double", "float",
                             ServerNotOlderThan(Connection, 7, 5, 0) ? "decimal" : "newdecimal", "date", "time", "datetime", "datetime", "timestamp", "timestamp", "year",
                             "char", "binary", "varchar", "varbinary", "longtext", "mediumtext", "text", "tinytext",
                             "longblob", "mediumblob", "blob", "tinyblob", "bit",
                             "json", "geography", "geographypoint", "enum", "set"};
    SQLINTEGER ExpColSize[33] = {3, 5, 8, 10, 20,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 22 : 50,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 12 : 50, 10, 10, 8, 19, 26, 19, 26, 4,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 11 : 33, 1,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 13 : 39, 17,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 1431655765 : 2147483647,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 5592405 : 50331645,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 21845 : 196605,
                                 ServerNotOlderThan(Connection, 7, 5, 0) ? 85 : 765, 2147483647, 16777215, 65535, 255,
                                 -1, -1, -1, -1, -1, -1};
    SQLSMALLINT ExpDecimalDigits[33] = {0, 0, 0, 0, 0,
                                        ServerNotOlderThan(Connection, 7, 5, 0) ? 6 : 31,
                                        ServerNotOlderThan(Connection, 7, 5, 0) ? 0 : 31, 5, 0, 0, 0, 6, 0, 6,
                                        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    SQLSMALLINT ExpNumPrecRadix[33] = {10, 10, 10, 10, 10, 10, 10, 10, -1, -1, -1, -1, -1, -1,
                                       10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    SQLSMALLINT ExpDatetimeSub[33] = {-1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 3, 3, 3, 3,
                                      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    SQLSMALLINT ExpNullable[33] = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
                                   0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
    char *ExpDefault[33] = {"", "", "", "", "", "", "", "", "", "", "", "", "", "",
                            "", "", "", "", "", "", "", "", "", "", "", "", "", "",
                            "'{}'", "'POINT(1, 1)'", "'POINT(1, 1)'", "'e'", "'s'"};

    IS_OK(run_cleanup());
    OK_SIMPLE_STMT(Stmt, createStmtStr);

    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLProcedureColumns(Stmt, ExpTableCat, SQL_NTS, (SQLCHAR *)"", 0,
                                                               (SQLCHAR *) ExpRoutineName, SQL_NTS, NULL, 0));

    SQLCHAR procedureCat[SQL_COLUMNS_BUFFER_LEN], procedureSchema[SQL_COLUMNS_BUFFER_LEN], procedureName[SQL_COLUMNS_BUFFER_LEN], colName[SQL_COLUMNS_BUFFER_LEN];
    SQLCHAR typeName[SQL_COLUMNS_BUFFER_LEN], remarks[SQL_COLUMNS_BUFFER_LEN], colDefault[SQL_COLUMNS_BUFFER_LEN], isNullable[SQL_COLUMNS_BUFFER_LEN];
    SQLLEN tsSize, csSize, blSize, ddSize, nprSize, rSize, cdSize, dscSize, colSize, dtSize, sdtSize;
    SQLSMALLINT colType, dataType, decimalDigits, numPrecRadix, nullable, sqlDataType, datetimeSubtypeCode;
    SQLINTEGER columnSize, charOctetLength, bufferLength, ordinalPosition;

    // Bind columns in result set to buffers
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, procedureCat, SQL_COLUMNS_BUFFER_LEN, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, procedureSchema, SQL_COLUMNS_BUFFER_LEN, &tsSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, procedureName, SQL_COLUMNS_BUFFER_LEN, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, colName, SQL_COLUMNS_BUFFER_LEN, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &colType, 0, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 6, SQL_C_SHORT, &dataType, 0, &dtSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 7, SQL_C_CHAR, typeName, SQL_COLUMNS_BUFFER_LEN, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 8, SQL_C_LONG, &columnSize, 0, &csSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 9, SQL_C_LONG, &bufferLength, 0, &blSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 10, SQL_C_SHORT, &decimalDigits, 0, &ddSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 11, SQL_C_SHORT, &numPrecRadix, 0, &nprSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 12, SQL_C_SHORT, &nullable, 0, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 13, SQL_C_CHAR, remarks, SQL_COLUMNS_BUFFER_LEN, &rSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 14, SQL_C_CHAR, colDefault, SQL_COLUMNS_BUFFER_LEN, &cdSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 15, SQL_C_SHORT, &sqlDataType, 0, &sdtSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 16, SQL_C_SHORT, &datetimeSubtypeCode, 0, &dscSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 17, SQL_C_LONG, &charOctetLength, 0, &colSize));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 18, SQL_C_LONG, &ordinalPosition, 0, NULL));
    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLBindCol(Stmt, 19, SQL_C_CHAR, isNullable, SQL_COLUMNS_BUFFER_LEN, NULL));

    int numOfRowsFetched = 0;
    while (SQLFetch(Stmt) == SQL_SUCCESS) {
        FAIL_IF(_stricmp(procedureCat, ExpTableCat) != 0, "Wrong PROCEDURE_CAT returned!");
        FAIL_IF(tsSize != SQL_NULL_DATA, "PROCEDURE_SCHEM must be null!");
        FAIL_IF(_stricmp(procedureName, ExpRoutineName) != 0, "Wrong PROCEDURE_NAME returned!")

        SQLCHAR ExpColName[3] = {0};
        ExpColName[0] = numOfRowsFetched >= 26 ? (numOfRowsFetched - 26) / 26 + 'a' : (numOfRowsFetched % 26) + 'a';
        ExpColName[1] = numOfRowsFetched >= 26 ? (numOfRowsFetched % 26) + 'a' : '\0';
        FAIL_IF(_stricmp(colName, ExpColName) != 0, "Wrong COLUMN_NAME returned!");
        FAIL_IF(colType != SQL_PARAM_INPUT, "Wrong PARAMETER_MODE returned!");
        is_num(dataType, ExpDataType[numOfRowsFetched]);
        FAIL_IF(_stricmp(typeName, ExpTypeName[numOfRowsFetched]) != 0, "Wrong TYPE_NAME returned!");

        FAIL_IF(nullable != ExpNullable[numOfRowsFetched], "Wrong NULLABLE returned!");
        if (ExpNullable[numOfRowsFetched]) {
            FAIL_IF(_stricmp(isNullable, "YES") != 0, "Wrong IS_NULLABLE returned!");
        } else {
            FAIL_IF(_stricmp(isNullable, "NO") != 0, "Wrong IS_NULLABLE returned!");
        }

        if (ExpColSize[numOfRowsFetched] != SQL_NULL_DATA) {
            FAIL_IF(columnSize != ExpColSize[numOfRowsFetched], "Wrong COLUMN_SIZE returned!");
        }
        if (ExpDecimalDigits[numOfRowsFetched] != SQL_NULL_DATA) {
            FAIL_IF(decimalDigits != ExpDecimalDigits[numOfRowsFetched], "Wrong DECIMAL_DIGITS returned!");
        }
        if (ExpNumPrecRadix[numOfRowsFetched] != SQL_NULL_DATA) {
            FAIL_IF(numPrecRadix != ExpNumPrecRadix[numOfRowsFetched], "Wrong NUM_PREC_RADIX returned!");
        }
        FAIL_IF(sqlDataType != ExpSqlDataType[numOfRowsFetched], "Wrong SQL_DATA_TYPE returned!");
        if (ExpDatetimeSub[numOfRowsFetched] != SQL_NULL_DATA) {
            FAIL_IF(datetimeSubtypeCode != ExpDatetimeSub[numOfRowsFetched], "Wrong SQL_DATETIME_SUB returned!");
        }
        if(strlen(ExpDefault[numOfRowsFetched]) != 0) {
            FAIL_IF(_stricmp(colDefault, ExpDefault[numOfRowsFetched]) != 0, "Wrong COLUMN_DEF returned!");
        }
        FAIL_IF(ordinalPosition != ++numOfRowsFetched, "Wrong ORDINAL_POSITION returned!");
    }

    diag("Fetched %d rows for '%s' routine type", numOfRowsFetched, RoutineType);
    FAIL_IF(numOfRowsFetched != ExpNumOfRowsFetched, "wrong number of rows fetched");

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}

int run_sql_procedurecolumns(SQLHANDLE Stmt, const SQLSMALLINT *ExpDataType, const SQLSMALLINT *ExpSqlDataType) {
    int rc;
    if((rc = run_sql_procedurecolumns_routine_type(Stmt, ExpDataType, ExpSqlDataType, "PROCEDURE")) != OK) {
        return rc;
    };
    return run_sql_procedurecolumns_routine_type(Stmt, ExpDataType, ExpSqlDataType, "FUNCTION");
}

ODBC_TEST(t_procedurecolumns3U) {
    SQLSMALLINT ExpDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                   SQL_REAL,
                                   SQL_DECIMAL, SQL_TYPE_DATE, SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_SMALLINT,
                                   SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                   SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                   SQL_WLONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_WVARCHAR, SQL_WVARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_REAL,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                      SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                      SQL_WLONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_WVARCHAR, SQL_WVARCHAR};

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

    FAIL_IF(run_sql_procedurecolumns(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLProcedureColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_procedurecolumns3A) {
    SQLSMALLINT ExpDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                   SQL_REAL,
                                   SQL_DECIMAL, SQL_TYPE_DATE, SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_SMALLINT,
                                   SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                   SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                   SQL_LONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_VARCHAR, SQL_VARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_REAL,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                      SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                      SQL_LONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_VARCHAR, SQL_VARCHAR};

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
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    FAIL_IF(run_sql_procedurecolumns(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLProcedureColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_procedurecolumns2U) {
    SQLSMALLINT ExpDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                   SQL_REAL,
                                   SQL_DECIMAL, SQL_DATE, SQL_TIME, SQL_TIMESTAMP, SQL_TIMESTAMP,
                                   SQL_TIMESTAMP, SQL_TIMESTAMP, SQL_SMALLINT,
                                   SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                   SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                   SQL_WLONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_WVARCHAR, SQL_WVARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_REAL,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                      SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                      SQL_WLONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_WVARCHAR, SQL_WVARCHAR};

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

    FAIL_IF(run_sql_procedurecolumns(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLProcedureColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_procedurecolumns2A) {
    SQLSMALLINT ExpDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                   SQL_REAL,
                                   SQL_DECIMAL, SQL_DATE, SQL_TIME, SQL_TIMESTAMP, SQL_TIMESTAMP,
                                   SQL_TIMESTAMP, SQL_TIMESTAMP, SQL_SMALLINT,
                                   SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                   SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                   SQL_LONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_VARCHAR, SQL_VARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_REAL,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                      SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BINARY,
                                      SQL_LONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_VARCHAR, SQL_VARCHAR};

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

    FAIL_IF(run_sql_procedurecolumns(Stmt1, ExpDataType, ExpSqlDataType) != OK, "error running SQLProcedureColumns");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}


#define NUM_PROC_FIELDS 8
#define ONE_MASK 0x07		/* all parameters for the first procedure in the expected results table */
#define TWO_MASK 0x18		/* all parameters for the second procedure in the expected results table */
#define THREE_MASK 0xE0		/* all parameters for the third procedure in the expected results table */
#define FULL_MASK (ONE_MASK | TWO_MASK | THREE_MASK)	/* all procedures in the table */


typedef uint8_t Mask;

typedef struct {
    const char* catalog;
    const char* procedure;
    const char* column_name;
    uint8_t column_type;
} ProcData;

typedef struct {
    const char* catalog;
    const char* procedure;
    const char* column;
    Mask mask;
} QueryDesc;

typedef int (*ProcColWorkerFn)(const char* const catalog, const char* const procedure, const char* const column, const Mask mask);

static const ProcData c_proc_expected[NUM_PROC_FIELDS] = {
    {"odbc_test", "test_get_name", "test_input_number", SQL_PARAM_INPUT},
    {"odbc_test", "test_get_name", "test_input_width", SQL_PARAM_INPUT},
    {"odbc_test", "test_get_name", "RESULT", SQL_RETURN_VALUE},
    {"odbc_test", "TEST_GET_PRECISION_AND_ROUND_UP", "TEST_IN", SQL_PARAM_INPUT},
    {"odbc_test", "TEST_GET_PRECISION_AND_ROUND_UP", "TEST_OUT", SQL_PARAM_INPUT},
    {"odbc_test", "test set and get precision", "test precision", SQL_PARAM_INPUT},
    {"odbc_test", "test set and get precision", "precision result", SQL_RESULT_COL},
    {"odbc_test", "test set and get precision", "test precision", SQL_RESULT_COL}
};
static const uint8_t c_proc_expected_sorting[NUM_PROC_FIELDS] = {5, 6, 7, 2, 0, 1, 3, 4};
/*
 * SQL_RETURN_VALUE and SQL_RESULT_COL are not implemented by the driver.
 * TODO: remove the use of the mask after PLAT-5588
 */
static const Mask c_implemented_mask = 0x3B;

static const QueryDesc c_queries_with_id_off[] = {
    // Get all fields
    {NULL, NULL, NULL, FULL_MASK},
    // Ordinary argument
    {"odbc_test", NULL, NULL, FULL_MASK},
    {"ODBC_TEST", NULL, NULL, FULL_MASK},
    {"odbc_test ", NULL, NULL, FULL_MASK},
    {"`odbc_test`", NULL, NULL, 0},
    // Pattern value arguments - full search
    {"", NULL, NULL, 0},
    {"odbc_test", "", NULL, 0},
    {NULL, "", NULL, 0},
    {NULL, NULL, "", 0},
    {"%_%", NULL, NULL, 0},
    {NULL, "%_%", "TEST_IN", 0x08},
    {NULL, "%\\_%", "TEST\\_IN", 0x08},
    {NULL, NULL, "%_%", FULL_MASK},
    {"odbc_test", "test_get_name", "", 0},
    {"%e%", NULL, NULL, 0},
    {NULL, "%e%", NULL, FULL_MASK},
    {NULL, NULL, "%e%", FULL_MASK},
    {NULL, "%", "%", FULL_MASK},
    {NULL, "_", NULL, 0},
    {NULL, NULL, "_", 0},
    // Pattern value arguments - patterns
    {NULL, "%get%", NULL, FULL_MASK},
    {NULL, "%g_t%", NULL, FULL_MASK},
    {NULL, "%\\_get%", NULL, ONE_MASK | TWO_MASK},
    {NULL, "test\\_%", NULL, ONE_MASK | TWO_MASK},
    {NULL, "test_%", NULL, FULL_MASK},
    {NULL, "% and %", NULL, THREE_MASK},
    {NULL, "%and%", NULL, TWO_MASK | THREE_MASK},
    {NULL, NULL, "%test%", 0xBB},
    {NULL, NULL, "%t__t%", 0xBB},
    {NULL, NULL, "%test\\_%", 0x1B},
    {NULL, NULL, "%test %", 0xA0},
    {NULL, "%_et%", "%in%", 0x0B},
    {NULL, NULL, "%in", 0x08},
    {NULL, NULL, "_______P%", 0x03},
    {NULL, NULL, "%t___", 0x08},
    {NULL, "_____get_____", NULL, ONE_MASK},
    {NULL, NULL, "%st%p%n", 0x20},
    {NULL, NULL, "%st%p%n%", 0x21},
    {NULL, NULL, "%st%p%n_%", 0x01},
    {"OdBc_TEST", "TEST_set_AND_get_PRECISION", "TeSt_PrEcIsIoN", 0x20},
};

static const QueryDesc c_queries_with_id_on[] = {
    // Get all fields
    {"", "", "", 0},
    // Basic use
    {"odbc_test", "", "", 0},
    {"test_get_name", "", "", 0},
    {"test_get_name", "test_get_name", "", 0},
    {"", "test_get_name", "", 0},
    {"", "", "test_input_number", 0},
    {"odbc_test", "test_get_name", "test_input_number", 0x01},
    {"odbc_test", "test set and get precision", "test precision", 0x20},
    // Remove trailing blanks in unquoted
    {"odbc_test ", "test_get_name ", "test_input_number ", 0x01},
    {"odbc_test \t", "test_get_name \t", "test_input_number \t", 0x01},
    {"odbc_test \r", "test_get_name \r", "test_input_number \r", 0x01},
    {"odbc_test \n", "test_get_name \n", "test_input_number \n", 0x01},
    {"odbc_test \t\r\n\t ", "test_get_name \t\r\n\t", "test_input_number \t\r\n\t", 0x01},
    {" odbc_test \t", "test_get_name", "test_input_number", 0},
    {" odbc_test", "test_get_name", "test_input_number", 0},
    {" odbc_test ", "test_get_name", "test_input_number", 0},
    {"odbc_test", "test_get_name ", "test_input_number", 0x01},
    {"odbc_test", " test_get_name", "test_input_number", 0},
    {"odbc_test", "test_get_name", "test_input_number  " , 0x01},
    {"odbc_test", "test_get_name", " test_input_number", 0},
     // Remove leading and trailing blanks in quoted - not tested because ` is not escapable
    {"odbc_test", "`test set and get precision`", "`test precision`", 0x20},
    {" `odbc_test` ", " `test set and get precision` ", " `test precision` ", 0x20},
    {"` odbc_test `", "` test set and get precision `", "` test precision `", 0},
    // Escape sequences
    {"", "", "", 0},
    {"odbc_test", "", "", 0},
    {"", "", "", 0},
    {"", "", "", 0},
    {"%_%", "", "", 0},
    {"", "%_%", "TEST_IN", 0},
    {"", "", "%_%", 0},
    {"odbc_test", "test_get_name", "", 0},
    {"%e%", "", "", 0},
    {"", "%e%", "", 0},
    {"", "", "%e%", 0},
};

static const ProcData c_proc_expected_non_ascii[NUM_PROC_FIELDS] = {
    {"odbc_test", "узнать_имя_васи", "вася_пупкин", SQL_PARAM_INPUT},
    {"odbc_test", "узнать_имя_васи", "петя_васин", SQL_PARAM_INPUT},
    {"odbc_test", "узнать_имя_васи", "RESULT", SQL_RETURN_VALUE},
    {"odbc_test", "НАЙТИ_ПЕТЮ_И_ОКРУГЛИТЬ", "ПЕТЯ", SQL_PARAM_INPUT},
    {"odbc_test", "НАЙТИ_ПЕТЮ_И_ОКРУГЛИТЬ", "НЕ_ПЕТЯ", SQL_PARAM_INPUT},
    {"odbc_test", "взять и положить кого-то куда-то", "кто-то там", SQL_PARAM_INPUT},
    {"odbc_test", "взять и положить кого-то куда-то", "не там", SQL_RESULT_COL},
    {"odbc_test", "взять и положить кого-то куда-то", "кто-то там", SQL_RESULT_COL}
};
static const uint8_t c_proc_expected_non_ascii_sorting[NUM_PROC_FIELDS] = {5, 6, 7, 3, 4, 2, 0, 1};

static const QueryDesc c_queries_non_ascii_with_id_off[] = {
    // Get all fields
    {NULL, NULL, NULL, FULL_MASK},
    // Case insensitive
    {NULL, "НАЙТИ_ПЕТЮ_И_ОКРУГЛИТЬ", NULL, TWO_MASK},
    {NULL, "найти_петю_и_округлить", NULL, TWO_MASK},
    {NULL, "найти\\_петю\\_и\\_округлить", NULL, TWO_MASK},
    {NULL, NULL, "ВАСЯ\\_ПУПКИН", 0x01},
    // Search patterns
    {NULL, "%", NULL, FULL_MASK},
    {NULL, NULL, "%", FULL_MASK},
    {"odbc_test", "% и %", NULL, THREE_MASK},
    {"odbc_test", NULL, "%-то %", 0x20},
    {"odbc_test", "%\\____\\_%", NULL, ONE_MASK},
    {"odbc_test", NULL, "___ПЕТЯ", 0x10},
};

static const QueryDesc c_queries_non_ascii_with_id_on[] = {
    // Get all fields
    {"", "", "", FULL_MASK},
    // Case insensitive
    {"", "найти_петю_и_округлить", "", TWO_MASK},
    {"", "", "ВАСЯ_ПУПКИН", 0x01},
    // Remove trailing blanks
    {"odbc_test  ", "узнать_имя_васи  ", "петя_васин  ", 0x02},
};


static int check_sorting(const uint8_t* const order, const size_t numitems, const size_t cur, const size_t prev) {
    size_t i;

    if(prev == numitems)
        return OK;

    /* previous value should be in the sorted data before the current value */
    for(i = 0; i < numitems; ++i) {
        if(prev == order[i])
            return OK;
        if(cur == order[i])
            return FAIL;
    }

    assert(0);	/* inputs were not found in the sorted array */
    return FAIL;
}

static int query_and_check_N(const char* const catalog, const char* const procedure, const char* const column, const Mask mask) {
    typedef struct {
        SQLCHAR catalog[20];
        SQLCHAR procedure[50];
        SQLCHAR column_name[50];
        SQLSMALLINT column_type;
        SQLLEN catLen;
        SQLLEN procLen;
        SQLLEN colLen;
    } ProcDataN;

    static ProcDataN result[NUM_PROC_FIELDS + 1];
    SQLULEN numrows;
    SQLRETURN fetched;
    Mask remaining = (mask & c_implemented_mask);
    size_t i, j;
    size_t prev = NUM_PROC_FIELDS;

    memset(result, 0, sizeof(result));

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, (SQLCHAR*)catalog, SQL_NTS, (SQLCHAR *)"", 0, (SQLCHAR*)procedure, SQL_NTS, (SQLCHAR*)column, SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, result[0].catalog, sizeof(result[0].catalog), &result[0].catLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, result[0].procedure, sizeof(result[0].procedure), &result[0].procLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, result[0].column_name, sizeof(result[0].column_name), &result[0].colLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &result[0].column_type, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(result[0]), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)(NUM_PROC_FIELDS + 1), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &numrows, 0));
    fetched = SQLFetch(Stmt);
    if(!remaining) {
        if(fetched == SQL_NO_DATA) {
            SQLCloseCursor(Stmt);
            return OK;
        }

        CHECK_STMT_RC(Stmt, fetched);

        printf("Fetched %d rows, expected 0\n", numrows);
        return FAIL;
    }
    CHECK_STMT_RC(Stmt, fetched);

    for(i = 0; i < numrows; ++i) {
        for(j = 0; j < NUM_PROC_FIELDS; ++j) {
            if(remaining & (1 << j)) {
                if(!strcmp(result[i].catalog, c_proc_expected[j].catalog)
                    && !strcmp(result[i].procedure, c_proc_expected[j].procedure)
                    && !strcmp(result[i].column_name, c_proc_expected[j].column_name)
                    && result[i].column_type == c_proc_expected[j].column_type) {
                    is_num(strlen(c_proc_expected[j].catalog), result[i].catLen);
                    is_num(strlen(c_proc_expected[j].procedure), result[i].procLen);
                    is_num(strlen(c_proc_expected[j].column_name), result[i].colLen);
                    remaining &= ~(1 << j);
                    IS_OK(check_sorting(c_proc_expected_sorting, NUM_PROC_FIELDS, j, prev));
                    prev = j;
                    goto query_and_check_n_found;
                }
            }
        }
        printf("QUERY %s %s %s %02X\n", catalog, procedure, column, mask);
        printf("RESULT%d %s %s %s %d\n", i, result[i].catalog, result[i].procedure, result[i].column_name, result[i].column_type);
        FAIL_IF(1, "Unexpected SQLProcedureColumns result ^^\n");
query_and_check_n_found:
        continue;
    }
    IS(!remaining);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    return OK;
}

static int query_and_check_W(const char* const catalog, const char* const procedure, const char* const column, const Mask mask) {
    typedef struct {
        SQLWCHAR catalog[20];
        SQLWCHAR procedure[50];
        SQLWCHAR column_name[50];
        SQLSMALLINT column_type;
        SQLLEN catLen;
        SQLLEN procLen;
        SQLLEN colLen;
    } ProcDataW;

    static ProcDataW result[NUM_PROC_FIELDS + 1];
    SQLWCHAR cat[20];
    SQLWCHAR proc[50];
    SQLWCHAR col[50];
    SQLWCHAR *pCat = NULL, *pProc = NULL, *pCol = NULL;
    SQLULEN numrows;
    SQLRETURN fetched;
    Mask remaining = (mask & c_implemented_mask);
    size_t i, j;
    size_t prev = NUM_PROC_FIELDS;

    memset(result, 0, sizeof(result));
    if(catalog) {
        latin_as_sqlwchar(catalog, cat);
        pCat = cat;
    }
    if(procedure) {
        latin_as_sqlwchar(procedure, proc);
        pProc = proc;
    }
    if(column) {
        latin_as_sqlwchar(column, col);
        pCol = col;
    }

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, pCat, SQL_NTS, CW(""), 0, pProc, SQL_NTS, pCol, SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_WCHAR, result[0].catalog, sizeof(result[0].catalog), &result[0].catLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_WCHAR, result[0].procedure, sizeof(result[0].procedure), &result[0].procLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_WCHAR, result[0].column_name, sizeof(result[0].column_name), &result[0].colLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &result[0].column_type, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(result[0]), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)(NUM_PROC_FIELDS + 1), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &numrows, 0));
    fetched = SQLFetch(Stmt);
    if(!remaining) {
        if(fetched == SQL_NO_DATA) {
            SQLCloseCursor(Stmt);
            return OK;
        }

        CHECK_STMT_RC(Stmt, fetched);

        printf("Fetched %d rows, expected 0\n", numrows);
        return FAIL;
    }
    CHECK_STMT_RC(Stmt, fetched);

    for(i = 0; i < numrows; ++i) {
        for(j = 0; j < NUM_PROC_FIELDS; ++j) {
            if(remaining & (1 << j)) {
                if(!sqlwcharcmp(result[i].catalog, CW(c_proc_expected[j].catalog), -1)
                   && !sqlwcharcmp(result[i].procedure, CW(c_proc_expected[j].procedure), -1)
                   && !sqlwcharcmp(result[i].column_name, CW(c_proc_expected[j].column_name), -1)
                   && result[i].column_type == c_proc_expected[j].column_type) {
                    is_num(strlen(c_proc_expected[j].catalog) * sizeof(SQLWCHAR), result[i].catLen);
                    is_num(strlen(c_proc_expected[j].procedure) * sizeof(SQLWCHAR), result[i].procLen);
                    is_num(strlen(c_proc_expected[j].column_name) * sizeof(SQLWCHAR), result[i].colLen);
                    remaining &= ~(1 << j);
                    IS_OK(check_sorting(c_proc_expected_sorting, NUM_PROC_FIELDS, j, prev));
                    prev = j;
                    goto query_and_check_w_found;
                }
            }
        }
        printf("QUERY %s %s %s %02X\n", catalog, procedure, column, mask);
        printf("RESULT%d ", i);
        printHex((char*)result[i].catalog, result[i].catLen);
        printf(" ");
        printHex((char*)result[i].procedure, result[i].procLen);
        printf(" ");
        printHex((char*)result[i].column_name, result[i].colLen);
        printf("\n");
        FAIL_IF(1, "Unexpected SQLProcedureColumns (W) result ^^\n");
query_and_check_w_found:
        continue;
    }
    IS(!remaining);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    return OK;
}

static int query_and_check_WN(const char* const catalog, const char* const procedure, const char* const column, const Mask mask) {
    typedef struct {
        SQLCHAR catalog[20];
        SQLWCHAR procedure[50];
        SQLCHAR column_name[50];
        SQLSMALLINT column_type;
        SQLLEN catLen;
        SQLLEN procLen;
        SQLLEN colLen;
    } ProcDataWN;

    static ProcDataWN result[NUM_PROC_FIELDS + 1];
    SQLWCHAR cat[20];
    SQLWCHAR proc[50];
    SQLWCHAR col[50];
    SQLWCHAR *pCat = NULL, *pProc = NULL, *pCol = NULL;
    SQLULEN numrows;
    SQLRETURN fetched;
    Mask remaining = (mask & c_implemented_mask);
    size_t i, j;
    size_t prev = NUM_PROC_FIELDS;

    memset(result, 0, sizeof(result));
    if(catalog) {
        latin_as_sqlwchar(catalog, cat);
        pCat = cat;
    }
    if(procedure) {
        latin_as_sqlwchar(procedure, proc);
        pProc = proc;
    }
    if(column) {
        latin_as_sqlwchar(column, col);
        pCol = col;
    }

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, pCat, SQL_NTS, CW(""), 0, pProc, SQL_NTS, pCol, SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, result[0].catalog, sizeof(result[0].catalog), &result[0].catLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_WCHAR, result[0].procedure, sizeof(result[0].procedure), iOdbc() ? &result[0].procLen : NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, result[0].column_name, sizeof(result[0].column_name), iOdbc() ? &result[0].colLen : NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &result[0].column_type, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(result[0]), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)(NUM_PROC_FIELDS + 1), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &numrows, 0));
    fetched = SQLFetch(Stmt);
    if(!remaining) {
        if(fetched == SQL_NO_DATA) {
            SQLCloseCursor(Stmt);
            return OK;
        }

        CHECK_STMT_RC(Stmt, fetched);

        printf("Fetched %d rows, expected 0\n", numrows);
        return FAIL;
    }
    CHECK_STMT_RC(Stmt, fetched);

    for(i = 0; i < numrows; ++i) {
        for(j = 0; j < NUM_PROC_FIELDS; ++j) {
            if(remaining & (1 << j)) {
                if(!strcmp(result[i].catalog, c_proc_expected[j].catalog)
                   && !sqlwcharcmp(result[i].procedure, CW(c_proc_expected[j].procedure), -1)
                   && !strcmp(result[i].column_name, c_proc_expected[j].column_name)
                   && result[i].column_type == c_proc_expected[j].column_type) {
                    is_num(strlen(c_proc_expected[j].catalog), result[i].catLen);
                    remaining &= ~(1 << j);
                    IS_OK(check_sorting(c_proc_expected_sorting, NUM_PROC_FIELDS, j, prev));
                    prev = j;
                    goto query_and_check_wn_found;
                }
            }
        }
        printf("QUERY %s %s %s %02X\n", catalog, procedure, column, mask);
        printf("RESULT%d %s ", i, result[i].catalog);
        printHex((char*)result[i].procedure, sizeof(result[i].procedure));
        printf(" %s\n", result[i].catalog);
        FAIL_IF(1, "Unexpected SQLProcedureColumns (W) result ^^\n");
query_and_check_wn_found:
        continue;
    }
    IS(!remaining);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    return OK;
}

static int query_and_check_NW(const char* const catalog, const char* const procedure, const char* const column, const Mask mask) {
    typedef struct {
        SQLWCHAR catalog[20];
        SQLCHAR procedure[50];
        SQLWCHAR column_name[50];
        SQLSMALLINT column_type;
        SQLLEN catLen;
        SQLLEN procLen;
        SQLLEN colLen;
    } ProcDataN;

    static ProcDataN result[NUM_PROC_FIELDS + 1];
    SQLULEN numrows;
    SQLRETURN fetched;
    Mask remaining = (mask & c_implemented_mask);
    size_t i, j;
    size_t prev = NUM_PROC_FIELDS;

    memset(result, 0, sizeof(result));

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, (SQLCHAR*)catalog, SQL_NTS, (SQLCHAR *)"", 0, (SQLCHAR*)procedure, SQL_NTS, (SQLCHAR*)column, SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_WCHAR, result[0].catalog, sizeof(result[0].catalog), iOdbc() ? &result[0].catLen : NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, result[0].procedure, sizeof(result[0].procedure), &result[0].procLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_WCHAR, result[0].column_name, sizeof(result[0].column_name), &result[0].colLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &result[0].column_type, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(result[0]), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)(NUM_PROC_FIELDS + 1), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &numrows, 0));
    fetched = SQLFetch(Stmt);
    if(!remaining) {
        if(fetched == SQL_NO_DATA) {
            SQLCloseCursor(Stmt);
            return OK;
        }

        CHECK_STMT_RC(Stmt, fetched);

        printf("Fetched %d rows, expected 0\n", numrows);
        return FAIL;
    }
    CHECK_STMT_RC(Stmt, fetched);

    for(i = 0; i < numrows; ++i) {
        for(j = 0; j < NUM_PROC_FIELDS; ++j) {
            if(remaining & (1 << j)) {
                if(!sqlwcharcmp(result[i].catalog, CW(c_proc_expected[j].catalog), -1)
                   && !strcmp(result[i].procedure, c_proc_expected[j].procedure)
                   && !sqlwcharcmp(result[i].column_name, CW(c_proc_expected[j].column_name), -1)
                   && result[i].column_type == c_proc_expected[j].column_type) {
                    is_num(strlen(c_proc_expected[j].procedure), result[i].procLen);
                    is_num(strlen(c_proc_expected[j].column_name) * sizeof(SQLWCHAR), result[i].colLen);
                    remaining &= ~(1 << j);
                    IS_OK(check_sorting(c_proc_expected_sorting, NUM_PROC_FIELDS, j, prev));
                    prev = j;
                    goto query_and_check_nw_found;
                }
            }
        }
        printf("QUERY %s %s %s %02X\n", catalog, procedure, column, mask);
        printf("RESULT%d ", i);
        printHex((char*)result[i].catalog, sizeof(result[i].catalog));
        printf(" %s ", result[i].procedure);
        printHex((char*)result[i].column_name, result[i].colLen);
        printf("\n");
        FAIL_IF(1, "Unexpected SQLProcedureColumns result ^^\n");
query_and_check_nw_found:
        continue;
    }
    IS(!remaining);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    return OK;
}

static int query_and_check_non_ascii_N(const char* const catalog, const char* const procedure, const char* const column, const Mask mask) {
    typedef struct {
        SQLCHAR catalog[50];
        SQLCHAR procedure[100];
        SQLCHAR column_name[100];
        SQLSMALLINT column_type;
        SQLLEN catLen;
        SQLLEN procLen;
        SQLLEN colLen;
    } ProcDataN;

    static ProcDataN result[NUM_PROC_FIELDS + 1];
    SQLULEN numrows;
    SQLRETURN fetched;
    Mask remaining = (mask & c_implemented_mask);
    size_t i, j;
    size_t prev = NUM_PROC_FIELDS;

    memset(result, 0, sizeof(result));

    CHECK_STMT_RC(Stmt, SQLProcedureColumns(Stmt, (SQLCHAR*)catalog, SQL_NTS, (SQLCHAR *)"", 0, (SQLCHAR*)procedure, SQL_NTS, (SQLCHAR*)column, SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, result[0].catalog, sizeof(result[0].catalog), &result[0].catLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, result[0].procedure, sizeof(result[0].procedure), &result[0].procLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, result[0].column_name, sizeof(result[0].column_name), &result[0].colLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &result[0].column_type, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(result[0]), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)(NUM_PROC_FIELDS + 1), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &numrows, 0));
    fetched = SQLFetch(Stmt);
    if(!remaining) {
        if(fetched == SQL_NO_DATA) {
            return OK;
        }

        CHECK_STMT_RC(Stmt, fetched);

        printf("Fetched %d rows, expected 0\n", numrows);
        return FAIL;
    }
    CHECK_STMT_RC(Stmt, fetched);

    for(i = 0; i < numrows; ++i) {
        for(j = 0; j < NUM_PROC_FIELDS; ++j) {
            if(remaining & (1 << j)) {
                if(!strcmp(result[i].catalog, c_proc_expected_non_ascii[j].catalog)
                   && !strcmp(result[i].procedure, c_proc_expected_non_ascii[j].procedure)
                   && !strcmp(result[i].column_name, c_proc_expected_non_ascii[j].column_name)
                   && result[i].column_type == c_proc_expected[j].column_type) {
                    is_num(strlen(c_proc_expected_non_ascii[j].catalog), result[i].catLen);
                    is_num(strlen(c_proc_expected_non_ascii[j].procedure), result[i].procLen);
                    is_num(strlen(c_proc_expected_non_ascii[j].column_name), result[i].colLen);
                    remaining &= ~(1 << j);
                    IS_OK(check_sorting(c_proc_expected_non_ascii_sorting, NUM_PROC_FIELDS, j, prev));
                    prev = j;
                    goto query_and_check_n_found;
                }
            }
        }
        printf("QUERY %s %s %s %02X\n", catalog, procedure, column, mask);
        printf("RESULT%d %s %s %s %d\n", i, result[i].catalog, result[i].procedure, result[i].column_name, result[i].column_type);
        FAIL_IF(1, "Unexpected SQLProcedureColumns result ^^\n");
        query_and_check_n_found:
        continue;
    }
    IS(!remaining);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    return OK;
}

static int query_and_check_non_ascii_W(const char* const catalog, const char* const procedure, const char* const column, const Mask mask) {
    typedef struct {
        SQLWCHAR catalog[50];
        SQLWCHAR procedure[100];
        SQLWCHAR column_name[100];
        SQLSMALLINT column_type;
        SQLLEN catLen;
        SQLLEN procLen;
        SQLLEN colLen;
    } ProcDataW;

    static ProcDataW result[NUM_PROC_FIELDS + 1];
    SQLWCHAR cat[50];
    SQLWCHAR proc[100];
    SQLWCHAR col[100];
    SQLWCHAR *pCat = NULL, *pProc = NULL, *pCol = NULL;
    SQLULEN numrows;
    SQLRETURN fetched;
    Mask remaining = (mask & c_implemented_mask);
    size_t i, j;
    size_t prev = NUM_PROC_FIELDS;

    memset(result, 0, sizeof(result));
    if(catalog) {
        pCat = CW(catalog);
    }
    if(procedure) {
        pProc = CW(procedure);
    }
    if(column) {
        pCol = CW(column);
    }

    CHECK_STMT_RC(Stmt, SQLProcedureColumnsW(Stmt, pCat, SQL_NTS, CW(""), 0, pProc, SQL_NTS, pCol, SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_WCHAR, result[0].catalog, sizeof(result[0].catalog), &result[0].catLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_WCHAR, result[0].procedure, sizeof(result[0].procedure), &result[0].procLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_WCHAR, result[0].column_name, sizeof(result[0].column_name), &result[0].colLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_SHORT, &result[0].column_type, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(result[0]), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)(NUM_PROC_FIELDS + 1), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttrW(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &numrows, 0));
    fetched = SQLFetch(Stmt);
    if(!remaining) {
        if(fetched == SQL_NO_DATA) {
            return OK;
        }

        CHECK_STMT_RC(Stmt, fetched);

        printf("Fetched %d rows, expected 0\n", numrows);
        return FAIL;
    }
    CHECK_STMT_RC(Stmt, fetched);

    for(i = 0; i < numrows; ++i) {
        for(j = 0; j < NUM_PROC_FIELDS; ++j) {
            if(remaining & (1 << j)) {
                if(!sqlwcharcmp(result[i].catalog, CW(c_proc_expected_non_ascii[j].catalog), -1)
                   && !sqlwcharcmp(result[i].procedure, CW(c_proc_expected_non_ascii[j].procedure), -1)
                   && !sqlwcharcmp(result[i].column_name, CW(c_proc_expected_non_ascii[j].column_name), -1)
                   && result[i].column_type == c_proc_expected[j].column_type) {
                    is_num(sqlwcharlen(CW(c_proc_expected_non_ascii[j].catalog)) * sizeof(SQLWCHAR), result[i].catLen);
                    is_num(sqlwcharlen(CW(c_proc_expected_non_ascii[j].procedure)) * sizeof(SQLWCHAR), result[i].procLen);
                    is_num(sqlwcharlen(CW(c_proc_expected_non_ascii[j].column_name)) * sizeof(SQLWCHAR), result[i].colLen);
                    remaining &= ~(1 << j);
                    IS_OK(check_sorting(c_proc_expected_non_ascii_sorting, NUM_PROC_FIELDS, j, prev));
                    prev = j;
                    goto query_and_check_w_found;
                }
            }
        }
        printf("QUERY %s %s %s %02X\n", catalog, procedure, column, mask);
        printf("RESULT%d ", i);
        printHex((char*)result[i].catalog, result[i].catLen);
        printf(" ");
        printHex((char*)result[i].procedure, result[i].procLen);
        printf(" ");
        printHex((char*)result[i].column_name, result[i].colLen);
        printf("\n");
        FAIL_IF(1, "Unexpected SQLProcedureColumns (W) result ^^\n");
        query_and_check_w_found:
        continue;
    }
    IS(!remaining);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    return OK;
}

#define NUM_FUNCS 3
#define BUF_SIZE 1024

ODBC_TEST(metadata_id_for_procedurecolumns) {
    static const char *const c_create_funcs[NUM_FUNCS] = {
        "CREATE FUNCTION test_get_name (test_input_number INT, test_input_width INT) RETURNS VARCHAR(256) AS BEGIN RETURN CONVERT(test_input_number, CHAR(256)); END",
        "CREATE PROCEDURE TEST_GET_PRECISION_AND_ROUND_UP (TEST_IN INT, TEST_OUT INT) AS BEGIN SET TEST_OUT = TEST_IN; END",
        "CREATE FUNCTION `test set and get precision` (`test precision` INT) RETURNS TABLE AS RETURN SELECT 0 AS `precision result`, `test precision`"
    };
    size_t i, j;

    const struct {
        ProcColWorkerFn fn;
        const char* mode;
    } functions[] = {
        {query_and_check_N, "N"},
        {query_and_check_W, "W"},
        {query_and_check_WN, "WN"},
        {query_and_check_NW, "NW"}
    };

    // Init env
    IS_OK(run_cleanup());
    for (i = 0; i < NUM_FUNCS; ++i) {
        OK_SIMPLE_STMT(Stmt, c_create_funcs[i]);
    }

    // The testing is below
    for (i = 0; i < LENGTHOF(functions); ++i) {
        // With id mode OFF query strings are treated as patterns
        CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_FALSE, SQL_IS_UINTEGER));
        for(j = 0; j < LENGTHOF(c_queries_with_id_off); ++j) {
            const QueryDesc* const q = c_queries_with_id_off + j;
            if(functions[i].fn(q->catalog, q->procedure, q->column, q->mask) != OK) {
                printf("Failed query {%s} {%s} {%s} with mode=%s id=OFF mask=%02X\n",
                       q->catalog, q->procedure, q->column, functions[i].mode, q->mask);
                return FAIL;
            }
        }

        // With id mode ON query strings are treated as identifiers
        CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, SQL_IS_UINTEGER));
        for(j = 0; j < LENGTHOF(c_queries_with_id_on); ++j) {
            const QueryDesc* const q = c_queries_with_id_on + j;
            if(functions[i].fn(q->catalog, q->procedure, q->column, q->mask) != OK) {
                printf("Failed query {%s} {%s} {%s} with mode=%s id=ON mask=%02X\n",
                       q->catalog, q->procedure, q->column, functions[i].mode, q->mask);
                return FAIL;
            }
        }
    }

    // With SQL_ATTR_METADATA_ID = TRUE NULL arguments are not acceptable
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, SQL_IS_UINTEGER));

    // Narrow string iface
    CHECK_STMT_ERR( Stmt,
                    SQLProcedureColumns(Stmt,
                                        NULL, SQL_NTS,
                                        (SQLCHAR *)"", SQL_NTS,
                                        (SQLCHAR*)"test_get_name", SQL_NTS,
                                        (SQLCHAR*)"test_input_number", SQL_NTS),
                    NULL_PTR_ERROR);
    CHECK_STMT_ERR( Stmt,
                    SQLProcedureColumns(Stmt,
                                        (SQLCHAR*)"odbc_test", SQL_NTS,
                                        NULL, SQL_NTS,
                                        (SQLCHAR*)"test_get_name", SQL_NTS,
                                        (SQLCHAR*)"test_input_number", SQL_NTS),
                    NULL_PTR_ERROR);
    CHECK_STMT_ERR( Stmt,
                    SQLProcedureColumns(Stmt,
                                        (SQLCHAR*)"odbc_test", SQL_NTS,
                                        (SQLCHAR *)"", SQL_NTS,
                                        NULL, SQL_NTS,
                                        (SQLCHAR*)"test_input_number", SQL_NTS),
                    NULL_PTR_ERROR);
    CHECK_STMT_ERR( Stmt,
                    SQLProcedureColumns(Stmt,
                                        (SQLCHAR*)"odbc_test", SQL_NTS,
                                        (SQLCHAR *)"", SQL_NTS,
                                        (SQLCHAR*)"test_get_name", SQL_NTS,
                                        NULL, SQL_NTS),
                    NULL_PTR_ERROR);

    // Wide string iface
    CHECK_STMT_ERR( Stmt,
                    SQLProcedureColumnsW(Stmt,
                                        NULL, SQL_NTS,
                                        CW(""), SQL_NTS,
                                        CW("test_get_name"), SQL_NTS,
                                        CW("test_input_number"), SQL_NTS),
                    NULL_PTR_ERROR);
    CHECK_STMT_ERR( Stmt,
                    SQLProcedureColumnsW(Stmt,
                                        CW("odbc_test"), SQL_NTS,
                                        NULL, SQL_NTS,
                                        CW("test_get_name"), SQL_NTS,
                                        CW("test_input_number"), SQL_NTS),
                    NULL_PTR_ERROR);
    CHECK_STMT_ERR( Stmt,
                    SQLProcedureColumnsW(Stmt,
                                        CW("odbc_test"), SQL_NTS,
                                        CW(""), SQL_NTS,
                                        NULL, SQL_NTS,
                                        CW("test_input_number"), SQL_NTS),
                    NULL_PTR_ERROR);
    CHECK_STMT_ERR( Stmt,
                    SQLProcedureColumnsW(Stmt,
                                        CW("odbc_test"), SQL_NTS,
                                        CW(""), SQL_NTS,
                                        CW("test_get_name"), SQL_NTS,
                                        NULL, SQL_NTS),
                    NULL_PTR_ERROR);

    return OK;
}

ODBC_TEST(procedurecolumns_non_ascii_N) {
    static const char *const c_create_funcs[NUM_FUNCS] = {
        "CREATE FUNCTION узнать_имя_васи (вася_пупкин INT, петя_васин INT) RETURNS VARCHAR(256) AS BEGIN RETURN CONVERT(вася_пупкин, CHAR(256)); END",
        "CREATE PROCEDURE НАЙТИ_ПЕТЮ_И_ОКРУГЛИТЬ (ПЕТЯ INT, НЕ_ПЕТЯ INT) AS BEGIN SET НЕ_ПЕТЯ = ПЕТЯ; END",
        "CREATE FUNCTION `взять и положить кого-то куда-то` (`кто-то там` INT) RETURNS TABLE AS RETURN SELECT 0 AS `не там`, `кто-то там`"
    };
    size_t i, j;

    const struct {
        ProcColWorkerFn fn;
        const char* mode;
    } functions[] = {
        {query_and_check_non_ascii_N, "N"},
        {query_and_check_non_ascii_W, "W"},
    };

    // Init env
    IS_OK(run_cleanup());
    for (i = 0; i < NUM_FUNCS; ++i) {
        OK_SIMPLE_STMT(Stmt, c_create_funcs[i]);
    }

    // The testing is below
    for (i = 0; i < LENGTHOF(functions); ++i) {
        // With id mode OFF query strings are treated as patterns
        CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_FALSE, SQL_IS_UINTEGER));
        for(j = 0; j < LENGTHOF(c_queries_non_ascii_with_id_off); ++j) {
            const QueryDesc* const q = c_queries_non_ascii_with_id_off + j;
            if(functions[i].fn(q->catalog, q->procedure, q->column, q->mask) != OK) {
                printf("Failed query {%s} {%s} {%s} with mode=%s id=OFF mask=%02X\n",
                       q->catalog, q->procedure, q->column, functions[i].mode, q->mask);
                return FAIL;
            }
        }

        // With id mode ON query strings are treated as identifiers
        CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, SQL_IS_UINTEGER));
        for(j = 0; j < LENGTHOF(c_queries_non_ascii_with_id_on); ++j) {
            const QueryDesc* const q = c_queries_non_ascii_with_id_on + j;
            if(functions[i].fn(q->catalog, q->procedure, q->column, q->mask) != OK) {
                printf("Failed query {%s} {%s} {%s} with mode=%s id=ON mask=%02X\n",
                       q->catalog, q->procedure, q->column, functions[i].mode, q->mask);
                return FAIL;
            }
        }
    }

    return OK;
}

ODBC_TEST(procedurecolumns_non_ascii_W) {
    static const char *const c_create_funcs[NUM_FUNCS] = {
        "CREATE FUNCTION узнать_имя_васи (вася_пупкин INT, петя_васин INT) RETURNS VARCHAR(256) AS BEGIN RETURN CONVERT(вася_пупкин, CHAR(256)); END",
        "CREATE PROCEDURE НАЙТИ_ПЕТЮ_И_ОКРУГЛИТЬ (ПЕТЯ INT, НЕ_ПЕТЯ INT) AS BEGIN SET НЕ_ПЕТЯ = ПЕТЯ; END",
        "CREATE FUNCTION `взять и положить кого-то куда-то` (`кто-то там` INT) RETURNS TABLE AS RETURN SELECT 0 AS `не там`, `кто-то там`"
    };
    size_t i, j;

    const struct {
        ProcColWorkerFn fn;
        const char* mode;
    } functions[] = {
        //{query_and_check_non_ascii_N, "N"},
        {query_and_check_non_ascii_W, "W"},
    };

    // Init env
    IS_OK(run_cleanup());
    for (i = 0; i < NUM_FUNCS; ++i) {
        OK_SIMPLE_STMTW(Stmt, CW(c_create_funcs[i]));
    }

    // The testing is below
    for (i = 0; i < LENGTHOF(functions); ++i) {
        // With id mode OFF query strings are treated as patterns
        CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_FALSE, SQL_IS_UINTEGER));
        for(j = 0; j < LENGTHOF(c_queries_non_ascii_with_id_off); ++j) {
            const QueryDesc* const q = c_queries_non_ascii_with_id_off + j;
            if(functions[i].fn(q->catalog, q->procedure, q->column, q->mask) != OK) {
                printf("Failed query {%s} {%s} {%s} with mode=%s id=OFF mask=%02X\n",
                       q->catalog, q->procedure, q->column, functions[i].mode, q->mask);
                return FAIL;
            }
        }

        // With id mode ON query strings are treated as identifiers
        CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, SQL_IS_UINTEGER));
        for(j = 0; j < LENGTHOF(c_queries_non_ascii_with_id_on); ++j) {
            const QueryDesc* const q = c_queries_non_ascii_with_id_on + j;
            if(functions[i].fn(q->catalog, q->procedure, q->column, q->mask) != OK) {
                printf("Failed query {%s} {%s} {%s} with mode=%s id=ON mask=%02X\n",
                       q->catalog, q->procedure, q->column, functions[i].mode, q->mask);
                return FAIL;
            }
        }
    }

    return OK;
}


MA_ODBC_TESTS my_tests[] =
        {
                {t_procedurecolumns3U, "t_procedurecolumns3U", NORMAL, UNICODE_DRIVER},
                {t_procedurecolumns3A, "t_procedurecolumns3A", NORMAL, ANSI_DRIVER},
                {t_procedurecolumns2U, "t_procedurecolumns2U", NORMAL, UNICODE_DRIVER},
                {t_procedurecolumns2A, "t_procedurecolumns2A", NORMAL, ANSI_DRIVER},
                {metadata_id_for_procedurecolumns, "metadata_id_for_procedurecolumns", NORMAL, ALL_DRIVERS},
                {procedurecolumns_non_ascii_N, "procedurecolumns_non_ascii_N", KNOWN_FAILURE, ALL_DRIVERS},	// TODO: PLAT-5560, PLAT-5561, PLAT-5562
                {procedurecolumns_non_ascii_W, "procedurecolumns_non_ascii_W", KNOWN_FAILURE, ALL_DRIVERS},	// TODO: PLAT-5560, PLAT-5561, PLAT-5562
                {NULL, NULL, NORMAL, ALL_DRIVERS}
        };


int main(int argc, char **argv) {
    int tests = sizeof(my_tests) / sizeof(MA_ODBC_TESTS) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}
