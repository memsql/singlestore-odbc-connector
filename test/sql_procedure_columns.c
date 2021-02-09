#include "tap.h"

#define SQL_COLUMNS_BUFFER_LEN 256

#define CREATE_ROUTINE_TEMPLATE \
"CREATE %s test_procedure_columns(a TINYINT , b SMALLINT NOT NULL, c MEDIUMINT UNSIGNED , d INT NOT NULL, e BIGINT UNSIGNED , f DOUBLE NOT NULL, g FLOAT ,"\
"h DECIMAL(10, 5) NOT NULL, i DATE, j TIME NOT NULL , k DATETIME, l DATETIME(6) NOT NULL , m TIMESTAMP, n TIMESTAMP(6) NOT NULL , o YEAR,"\
"p CHAR(11) NOT NULL, q BINARY, r VARCHAR(13) NOT NULL , s VARBINARY(17), t LONGTEXT NOT NULL , u MEDIUMTEXT, v TEXT NOT NULL , w TINYTEXT,"\
"x LONGBLOB NOT NULL, y MEDIUMBLOB, z BLOB NOT NULL, aa TINYBLOB, ab BIT(1) NOT NULL,"\
"ac JSON DEFAULT '{}', ad GEOGRAPHY NOT NULL DEFAULT 'POINT(1, 1)', ae GEOGRAPHYPOINT DEFAULT 'POINT(1, 1)', af ENUM('e') NOT NULL DEFAULT 'e', ag SET('s') DEFAULT 's') "\
"RETURNS INT AS BEGIN RETURN 0; END"

#define DROP_ROUTINE_TEMPLATE "DROP %s IF EXISTS test_procedure_columns"

int run_sql_procedurecolumns_routine_type(SQLHANDLE Stmt, const SQLSMALLINT *ExpDataType, const SQLSMALLINT *ExpSqlDataType, const char* RoutineType) {
    const int ExpNumOfRowsFetched = 33;
    SQLCHAR *ExpTableCat = my_schema;
    SQLCHAR *ExpRoutineName = "test_procedure_columns";
    char createStmtStr[1024], dropStmtStr[128];
    unsigned long crLength = strlen(CREATE_ROUTINE_TEMPLATE) + strlen(RoutineType) + 1;
    unsigned long drLength = strlen(DROP_ROUTINE_TEMPLATE) + strlen(RoutineType) + 1;
    _snprintf(createStmtStr, crLength, CREATE_ROUTINE_TEMPLATE, RoutineType);
    _snprintf(dropStmtStr, drLength, DROP_ROUTINE_TEMPLATE, RoutineType);

    char *ExpTypeName[33] = {"tinyint", "smallint", "mediumint unsigned", "int", "bigint unsigned", "double", "float",
                             "newdecimal", "date", "time", "datetime", "datetime", "timestamp", "timestamp", "year",
                             "char", "binary", "varchar", "varbinary", "longtext", "mediumtext", "text", "tinytext",
                             "longblob", "mediumblob", "blob", "tinyblob", "bit",
                             "json", "geography", "geographypoint", "enum", "set"};
    SQLINTEGER ExpColSize[33] = {3, 5, 8, 10, 20, 50, 50, 10, 10, 8, 19, 26, 19, 26, 4, 33, 1, 39, 17,
                                 2147483647, 50331645, 196605, 765, 2147483647, 16777215, 65535, 255, -1, -1, -1, -1, -1,
                                 -1};
    SQLSMALLINT ExpDecimalDigits[33] = {0, 0, 0, 0, 0, 31, 31, 5, 0, 0, 0, 6, 0, 6,
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

    OK_SIMPLE_STMT(Stmt, dropStmtStr);
    OK_SIMPLE_STMT(Stmt, createStmtStr);

    CHECK_HANDLE_RC(SQL_HANDLE_STMT, Stmt, SQLProcedureColumns(Stmt, ExpTableCat, SQL_NTS, NULL, 0,
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
        FAIL_IF(dataType != ExpDataType[numOfRowsFetched], "Wrong DATA_TYPE returned!");
        if (_stricmp(typeName, ExpTypeName[numOfRowsFetched]) != 0) {
            printf("RealName: %s, ExpectedName: %s", typeName, ExpTypeName[numOfRowsFetched]);
        }
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
    OK_SIMPLE_STMT(Stmt, dropStmtStr);

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
                                   SQL_FLOAT,
                                   SQL_DECIMAL, SQL_TYPE_DATE, SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_SMALLINT,
                                   SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                   SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BIT,
                                   SQL_WLONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_WVARCHAR, SQL_WVARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_FLOAT,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                      SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BIT,
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
                                   SQL_FLOAT,
                                   SQL_DECIMAL, SQL_TYPE_DATE, SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP,
                                   SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_SMALLINT,
                                   SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                   SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BIT,
                                   SQL_LONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_VARCHAR, SQL_VARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_FLOAT,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                      SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BIT,
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
                 SQLDriverConnectA(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
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
                                   SQL_FLOAT,
                                   SQL_DECIMAL, SQL_DATE, SQL_TIME, SQL_TIMESTAMP, SQL_TIMESTAMP,
                                   SQL_TIMESTAMP, SQL_TIMESTAMP, SQL_SMALLINT,
                                   SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                   SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BIT,
                                   SQL_WLONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_WVARCHAR, SQL_WVARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_FLOAT,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_WCHAR, SQL_BINARY, SQL_WVARCHAR, SQL_VARBINARY, SQL_WLONGVARCHAR,
                                      SQL_WLONGVARCHAR, SQL_WLONGVARCHAR, SQL_WLONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BIT,
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
                                   SQL_FLOAT,
                                   SQL_DECIMAL, SQL_DATE, SQL_TIME, SQL_TIMESTAMP, SQL_TIMESTAMP,
                                   SQL_TIMESTAMP, SQL_TIMESTAMP, SQL_SMALLINT,
                                   SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                   SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                   SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BIT,
                                   SQL_LONGVARCHAR, SQL_VARBINARY, SQL_VARBINARY, SQL_VARCHAR, SQL_VARCHAR};
    SQLSMALLINT ExpSqlDataType[33] = {SQL_TINYINT, SQL_SMALLINT, SQL_INTEGER, SQL_INTEGER, SQL_BIGINT, SQL_DOUBLE,
                                      SQL_FLOAT,
                                      SQL_DECIMAL, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME, SQL_DATETIME,
                                      SQL_DATETIME, SQL_DATETIME, SQL_SMALLINT,
                                      SQL_CHAR, SQL_BINARY, SQL_VARCHAR, SQL_VARBINARY, SQL_LONGVARCHAR,
                                      SQL_LONGVARCHAR, SQL_LONGVARCHAR, SQL_LONGVARCHAR,
                                      SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_LONGVARBINARY, SQL_BIT,
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


MA_ODBC_TESTS my_tests[] =
        {
                {t_procedurecolumns3U, "t_procedurecolumns3U", NORMAL, UNICODE_DRIVER},
                {t_procedurecolumns3A, "t_procedurecolumns3A", NORMAL, ANSI_DRIVER},
                {t_procedurecolumns2U, "t_procedurecolumns2U", NORMAL, UNICODE_DRIVER},
                {t_procedurecolumns2A, "t_procedurecolumns2A", NORMAL, ANSI_DRIVER},
                {NULL, NULL, NORMAL, ALL_DRIVERS}
        };


int main(int argc, char **argv) {
    int tests = sizeof(my_tests) / sizeof(MA_ODBC_TESTS) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}