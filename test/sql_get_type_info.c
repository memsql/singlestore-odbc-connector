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

#include "types_info.h"

#define BUFFER_LEN 256

void get_conn_string(char* c, char* additional_params) {
    sprintf((char *) c, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;NO_SSPS=%d;%s",
            my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, NoSsps, additional_params);
}

int check_stmt_correct_type_info(const MADB_TypeInfo* const ExpTypeInfo, MADB_TypeInfo* const RecTypeInfo, SQLLEN cpSize) {
    FAIL_IF_NE_STR(ExpTypeInfo->TypeName, RecTypeInfo->TypeName, "Wrong TYPE_INFO returned");
    IS(strlen(RecTypeInfo->TypeName));

    FAIL_IF_NE_INT(ExpTypeInfo->DataType, RecTypeInfo->DataType, "Wrong DATA_TYPE returned");
    IS(RecTypeInfo->DataType);

    FAIL_IF_NE_INT(RecTypeInfo->ColumnSize, ExpTypeInfo->ColumnSize,  "Wrong COLUMN_SIZE returned");
    IS(RecTypeInfo->ColumnSize);

    FAIL_IF_NE_STR(ExpTypeInfo->LiteralPrefix, RecTypeInfo->LiteralPrefix, "Wrong LITERAL_PREFIX returned");
    FAIL_IF_NE_STR(ExpTypeInfo->LiteralSuffix, RecTypeInfo->LiteralSuffix, "Wrong LITERAL_SUFFIX returned");
    IS((!strlen(RecTypeInfo->LiteralPrefix) && !strlen(RecTypeInfo->LiteralSuffix))
        || !strcmp(RecTypeInfo->LiteralPrefix, RecTypeInfo->LiteralSuffix));

    if (cpSize > 0) {
        FAIL_IF_NE_STR(ExpTypeInfo->CreateParams, RecTypeInfo->CreateParams, "Wrong CREATE_PARAMS returned");
        IS(strlen(RecTypeInfo->CreateParams));
    } else {
        IS(!strlen(RecTypeInfo->CreateParams));
    }

    FAIL_IF_NE_INT(ExpTypeInfo->Nullable, RecTypeInfo->Nullable, "Wrong NULLABLE returned");
    IS(RecTypeInfo->Nullable == SQL_NO_NULLS || RecTypeInfo->Nullable == SQL_NULLABLE);

    FAIL_IF_NE_INT(ExpTypeInfo->CaseSensitive, RecTypeInfo->CaseSensitive, "Wrong CASE_SENSITIVE returned");
    IS(RecTypeInfo->CaseSensitive == SQL_FALSE || RecTypeInfo->CaseSensitive == SQL_TRUE);

    FAIL_IF_NE_INT(ExpTypeInfo->Searchable, RecTypeInfo->Searchable, "Wrong SEARCHABLE returned");
    IS(RecTypeInfo->Searchable == SQL_PRED_NONE
        || RecTypeInfo->Searchable == SQL_PRED_CHAR
        || RecTypeInfo->Searchable == SQL_PRED_BASIC
        || RecTypeInfo->Searchable == SQL_SEARCHABLE);

    FAIL_IF_NE_INT(ExpTypeInfo->Unsigned, RecTypeInfo->Unsigned, "Wrong UNSIGNED_ATTRIBUTE returned");
    IS(RecTypeInfo->Unsigned == SQL_FALSE || RecTypeInfo->Unsigned == SQL_TRUE);

    FAIL_IF_NE_INT(ExpTypeInfo->FixedPrecScale, RecTypeInfo->FixedPrecScale, "Wrong FIXED_PREC_SCALE returned");
    IS(RecTypeInfo->FixedPrecScale == SQL_FALSE || RecTypeInfo->FixedPrecScale == SQL_TRUE);

    FAIL_IF_NE_INT(ExpTypeInfo->AutoUniqueValue, RecTypeInfo->AutoUniqueValue, "Wrong AUTO_UNIQUE_VALUE returned");
    IS(RecTypeInfo->AutoUniqueValue == SQL_FALSE || RecTypeInfo->AutoUniqueValue == SQL_TRUE);

    FAIL_IF_NE_STR(ExpTypeInfo->LocalTypeName, RecTypeInfo->LocalTypeName, "Wrong LOCAL_TYPE_NAME returned");
    IS(!strcmp(RecTypeInfo->LocalTypeName, RecTypeInfo->TypeName));

    FAIL_IF_NE_INT(ExpTypeInfo->MinimumScale, RecTypeInfo->MinimumScale, "Wrong MINIMUM_SCALE returned");
    FAIL_IF_NE_INT(ExpTypeInfo->MaximumScale, RecTypeInfo->MaximumScale, "Wrong MAXIMUM_SCALE returned");
    IS(RecTypeInfo->MinimumScale == -RecTypeInfo->MaximumScale);
    IS(!RecTypeInfo->MinimumScale || strstr(ExpTypeInfo->CreateParams, "scale"));

    FAIL_IF_NE_INT(ExpTypeInfo->SqlDataType, RecTypeInfo->SqlDataType, "Wrong SQL_DATA_TYPE returned");
    IS(RecTypeInfo->SqlDataType == RecTypeInfo->DataType
        || RecTypeInfo->SqlDataType == SQL_DATETIME);

    if (ExpTypeInfo->SqlDateTimeSub != RecTypeInfo->SqlDateTimeSub)
      printf("boom");
    FAIL_IF_NE_INT(ExpTypeInfo->SqlDateTimeSub, RecTypeInfo->SqlDateTimeSub, "Wrong SQL_DATETIME_SUB returned");

    FAIL_IF_NE_INT(RecTypeInfo->NumPrecRadix, ExpTypeInfo->NumPrecRadix, "Wrong NUM_PREC_RADIX returned");

    FAIL_IF_NE_INT(RecTypeInfo->IntervalPrecision, ExpTypeInfo->IntervalPrecision, "Wrong INTERVAL_PRECISION returned");
    IS(!RecTypeInfo->IntervalPrecision);

    // Reset the CreateParams string
    RecTypeInfo->CreateParams[0] = 0;

    return OK;
}

#define INIT_TYPE_INFO(ti) \
do {                             \
    (ti)->TypeName = my_alloca(sizeof(SQLCHAR) * BUFFER_LEN);\
    *(ti)->TypeName = 0;\
    (ti)->LiteralPrefix = my_alloca(sizeof(SQLCHAR) * BUFFER_LEN);\
    *(ti)->LiteralPrefix = 0;\
    (ti)->LiteralSuffix = my_alloca(sizeof(SQLCHAR) * BUFFER_LEN);\
    *(ti)->LiteralSuffix = 0;\
    (ti)->CreateParams = my_alloca(sizeof(SQLCHAR) * BUFFER_LEN);\
    *(ti)->CreateParams = 0;\
    (ti)->LocalTypeName = my_alloca(sizeof(SQLCHAR) * BUFFER_LEN);\
    *(ti)->LocalTypeName = 0;\
} while(0)

SQLRETURN BindColumnWithDescriptor(SQLHANDLE Stmt1, SQLUSMALLINT colNumber, SQLSMALLINT targetType, SQLPOINTER *targetValue, SQLLEN *cpSize) {
    unsigned long length = 0;
    int rc = SQLColAttribute(Stmt1, colNumber, SQL_DESC_LENGTH, NULL, 0, NULL, (SQLPOINTER) &length);
    if (!SQL_SUCCEEDED(rc)) { return rc; }
    return SQLBindCol(Stmt1, colNumber, targetType, targetValue, length, cpSize);
}


int bind_type_info(SQLHANDLE Stmt1, MADB_TypeInfo *TypeInfo, SQLLEN *cpSize) {
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 1, SQL_C_CHAR,(SQLPOINTER) TypeInfo->TypeName, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 2, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->DataType, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 3, SQL_C_LONG,(SQLPOINTER) &TypeInfo->ColumnSize, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 4, SQL_C_CHAR,(SQLPOINTER) TypeInfo->LiteralPrefix, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 5, SQL_C_CHAR,(SQLPOINTER) TypeInfo->LiteralSuffix, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 6, SQL_C_CHAR,(SQLPOINTER) TypeInfo->CreateParams, cpSize));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 7, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->Nullable, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 8, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->CaseSensitive, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 9, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->Searchable, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 10, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->Unsigned, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 11, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->FixedPrecScale, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 12, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->AutoUniqueValue, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 13, SQL_C_CHAR,(SQLPOINTER) TypeInfo->LocalTypeName, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 14, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->MinimumScale, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 15, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->MaximumScale, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 16, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->SqlDataType, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 17, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->SqlDateTimeSub, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 18, SQL_C_LONG,(SQLPOINTER) &TypeInfo->NumPrecRadix, NULL));
    CHECK_STMT_RC(Stmt1, BindColumnWithDescriptor(Stmt1, 19, SQL_C_SHORT,(SQLPOINTER) &TypeInfo->IntervalPrecision, NULL));
    return OK;
}

int compare( const void* a, const void* b)
{
    MADB_TypeInfo ti_a = * ( (MADB_TypeInfo*) a );
    MADB_TypeInfo ti_b = * ( (MADB_TypeInfo*) b );

    if (ti_a.DataType == ti_b.DataType)
    {
      return strcmp(ti_a.TypeName, ti_b.TypeName);
    } else return ti_a.DataType - ti_b.DataType;
}

int run_sql_get_type_info(SQLHANDLE Stmt1, SQLSMALLINT DataType, MADB_TypeInfo *CorrectTypeInfos) {
    SQLLEN cpSize;
    MADB_TypeInfo recTypeInfo;
    MADB_TypeInfo ExpTypeInfo[TYPES_COUNT];
    INIT_TYPE_INFO(&recTypeInfo);
    SQLSMALLINT numOfRowsFetched = 0, numResultCols = 0, expTypeInfoCount = 0;
    int rc, i;

    for (i = 0; i < TYPES_COUNT; i++)
    {
        if (DataType == SQL_ALL_TYPES || CorrectTypeInfos[i].DataType == DataType || CorrectTypeInfos[i].DataTypeAlias == DataType)
        {
            ExpTypeInfo[expTypeInfoCount++] = CorrectTypeInfos[i];
        }
    }

    rc = SQLGetTypeInfo(Stmt1, DataType);
#ifdef _WIN32
    /* unsupported field */
    if(rc == SQL_ERROR && (!expTypeInfoCount || ExpTypeInfo[0].DataTypeAlias)) {
        SQLCHAR SQLState[6];
        SQLINTEGER NativeError;
        SQLCHAR SQLMessage[SQL_MAX_MESSAGE_LENGTH];
        SQLSMALLINT TextLength;

        const SQLRETURN result = SQLGetDiagRec(SQL_HANDLE_STMT, Stmt1, 1, SQLState, &NativeError, SQLMessage, SQL_MAX_MESSAGE_LENGTH, &TextLength);
        if(result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO) {
          const char* const ExpectedError = "SQL data type out of range";
          const SQLSMALLINT ExpectedLength = strlen(ExpectedError);
          if(!strcmp(SQLState, "S1004")
             && NativeError == 0
             && ExpectedLength <= TextLength
             && !strcmp(SQLMessage + (TextLength - ExpectedLength), ExpectedError)) {
            diag("Windows DM error on field %d", DataType);
            return OK;
          } else {
            fprintf(stdout, "[%s] (%d) %s\n", SQLState, NativeError, SQLMessage);
          }
        } else {
          fprintf(stdout, "Error %d getting diagnostic records with SQLGetDiagRec()\n", result);
        }
    }
#endif // _WIN32
    CHECK_STMT_RC(Stmt1, rc);

    if ((rc = bind_type_info(Stmt1, &recTypeInfo, &cpSize)) != OK) {
        return rc;
    }
    qsort(ExpTypeInfo, expTypeInfoCount, sizeof(MADB_TypeInfo), compare);
    CHECK_STMT_RC(Stmt1, SQLNumResultCols(Stmt1, &numResultCols));
    FAIL_IF_NE_INT(TYPE_INFO_FIELDS_COUNT, numResultCols, "Wrong number of result columns returned");
    while (SQL_SUCCEEDED(SQLFetch(Stmt1))) {
        if(numOfRowsFetched >= expTypeInfoCount) {
            fprintf(stdout, "Unexpected row %s for DataType %d (File: %s:%d)\n", recTypeInfo.TypeName, DataType, __FILE__, __LINE__);
        } else if ((rc = check_stmt_correct_type_info(&ExpTypeInfo[numOfRowsFetched], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }
        numOfRowsFetched++;
    }
    diag("Fetched %d rows", numOfRowsFetched);
    FAIL_IF_NE_INT(numOfRowsFetched, expTypeInfoCount, "Wrong number of rows fetched");

    return OK;
}

#define CURSORS_CNT 3
const SQLPOINTER cursorTypes[CURSORS_CNT] = {(SQLPOINTER)SQL_CURSOR_DYNAMIC, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                                             (SQLPOINTER)SQL_CURSOR_STATIC,};

ODBC_TEST(t_sqlgettypeinfo2) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1, DynCursorConnection;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512], dynCursorConn[600]; /* dynCursorConn should be larger than conn to make sure sprintf does not corrupt memory */

    get_conn_string((char *) conn, "");
    sprintf((char *) dynCursorConn, "%s;OPTION=67108898", conn);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC2, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &DynCursorConnection));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(DynCursorConnection,
                 SQLDriverConnect(DynCursorConnection, NULL, dynCursorConn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    int i, j, rc;
    for(i = 0; i < CURSORS_CNT; i++) {
        if (i == 0) { // DynamicCursor
            CHECK_DBC_RC(DynCursorConnection, SQLAllocHandle(SQL_HANDLE_STMT, DynCursorConnection, &Stmt1));
        } else {
            CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));
        }
        CHECK_STMT_RC(Stmt1, SQLSetStmtAttr(Stmt1, SQL_ATTR_CURSOR_TYPE, cursorTypes[i], 0));

        for (j = 0; j < SQL_DATA_TYPES_COUNT; j++)
        {
            if((rc = run_sql_get_type_info(Stmt1, sqlDataTypes[j], TypeInfoV2)) != OK)
            {
                diag("Error running SQLGetTypeInfo for cursor type %d and SQL data type %d", cursorTypes[i], sqlDataTypes[j]);
                return rc;
            }
            CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));
        }

        CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    }
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_DBC_RC(DynCursorConnection, SQLDisconnect(DynCursorConnection));
    CHECK_DBC_RC(DynCursorConnection, SQLFreeHandle(SQL_HANDLE_DBC, DynCursorConnection));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));
    return OK;
}

ODBC_TEST(t_sqlgettypeinfo3) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1, DynCursorConnection;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512], dynCursorConn[600]; /* dynCursorConn should be larger than conn to make sure sprintf does not corrupt memory */

    get_conn_string((char *) conn, "");
    sprintf((char *) dynCursorConn, "%s;OPTION=67108898", conn);

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &DynCursorConnection));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(DynCursorConnection,
                 SQLDriverConnect(DynCursorConnection, NULL, dynCursorConn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    int i, j, rc;
    for(i = 0; i < CURSORS_CNT; i++) {
        if (i == 0) { // DynamicCursor
            CHECK_DBC_RC(DynCursorConnection, SQLAllocHandle(SQL_HANDLE_STMT, DynCursorConnection, &Stmt1));
        } else {
            CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));
        }
        CHECK_STMT_RC(Stmt1, SQLSetStmtAttr(Stmt1, SQL_ATTR_CURSOR_TYPE, cursorTypes[i], 0));

        for (j = 0; j < SQL_DATA_TYPES_COUNT; j++)
        {
            if((rc = run_sql_get_type_info(Stmt1, sqlDataTypes[j], TypeInfoV3)) != OK)
            {
                diag("Error running SQLGetTypeInfo for cursor type %d and SQL data type %d", cursorTypes[i], sqlDataTypes[j]);
                return rc;
            }
            CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));
        }

        CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    }
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_DBC_RC(DynCursorConnection, SQLDisconnect(DynCursorConnection));
    CHECK_DBC_RC(DynCursorConnection, SQLFreeHandle(SQL_HANDLE_DBC, DynCursorConnection));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));
    return OK;
}

ODBC_TEST(t_sqlgettypeinfo_sequential) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    get_conn_string((char *) conn, "");

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    IS_OK(run_sql_get_type_info(Stmt1, SQL_ALL_TYPES, TypeInfoV3));
    IS_OK(run_sql_get_type_info(Stmt1, SQL_ALL_TYPES, TypeInfoV3));

    SQLCHAR *sql = "SELECT 1";
    CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));
    CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_UNBIND));
    CHECK_STMT_RC(Stmt1, SQLExecDirect(Stmt1, sql, strlen(sql)));
    CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));

    CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));
    IS_OK(run_sql_get_type_info(Stmt1, SQL_ALL_TYPES, TypeInfoV3));


    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));
    return OK;
}

ODBC_TEST(t_sqlgettypeinfo_no_bind) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    get_conn_string((char *) conn, "");

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    CHECK_STMT_RC(Stmt1, SQLGetTypeInfo(Stmt1,SQL_ALL_TYPES));
    int numOfRowsFetched = 0;
    while(SQL_SUCCEEDED(SQLFetch(Stmt1))) ++numOfRowsFetched;
    diag("Fetched %d rows", numOfRowsFetched);
    FAIL_IF_NE_INT(TYPES_COUNT, numOfRowsFetched, "Wrong number of rows fetched");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));
    return OK;
}

ODBC_TEST(t_sqlgettypeinfo_fetchscroll) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1, DynCursorConnection;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512], dynCursorConn[600]; /* dynCursorConn should be larger than conn to make sure sprintf does not corrupt memory */
    SQLLEN cpSize;
    MADB_TypeInfo recTypeInfo, *expTypeInfo = TypeInfoV3;
    int rc, i;
    const int cursorsCnt = 2;
    const SQLPOINTER cursors[3] = {(SQLPOINTER)SQL_CURSOR_DYNAMIC, (SQLPOINTER)SQL_CURSOR_STATIC,};

    INIT_TYPE_INFO(&recTypeInfo);

    qsort(expTypeInfo, TYPES_COUNT, sizeof(MADB_TypeInfo), compare);

    get_conn_string((char *) conn, "");
    sprintf((char *) dynCursorConn, "%sOPTION=67108898", conn);
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &DynCursorConnection));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(DynCursorConnection,
                 SQLDriverConnect(DynCursorConnection, NULL, dynCursorConn, (SQLSMALLINT) strlen((const char *) dynCursorConn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));

    for(i = 0; i < cursorsCnt; i++) {
        if (i == 0) { // DynamicCursor
            CHECK_DBC_RC(DynCursorConnection, SQLAllocHandle(SQL_HANDLE_STMT, DynCursorConnection, &Stmt1));
        } else {
            CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));
        }
        CHECK_STMT_RC(Stmt1, SQLSetStmtAttr(Stmt1, SQL_ATTR_CURSOR_TYPE, cursors[i], 0));

        CHECK_STMT_RC(Stmt1, SQLGetTypeInfo(Stmt1, SQL_ALL_TYPES));
        if ((rc = bind_type_info(Stmt1, &recTypeInfo, &cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[0], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFetchScroll(Stmt1, SQL_FETCH_LAST, 0));
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[TYPES_COUNT - 1], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFetchScroll(Stmt1, SQL_FETCH_PRIOR, -1));
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[TYPES_COUNT - 2], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFetchScroll(Stmt1, SQL_FETCH_FIRST, 0));
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[0], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFetchScroll(Stmt1, SQL_FETCH_NEXT, -1));
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[1], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFetchScroll(Stmt1, SQL_FETCH_RELATIVE, 3));
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[4], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFetchScroll(Stmt1, SQL_FETCH_ABSOLUTE, -5));
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[TYPES_COUNT - 5], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[TYPES_COUNT - 4], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }

        CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    }
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_DBC_RC(DynCursorConnection, SQLDisconnect(DynCursorConnection));
    CHECK_DBC_RC(DynCursorConnection, SQLFreeHandle(SQL_HANDLE_DBC, DynCursorConnection));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

ODBC_TEST(t_sqlgettypeinfo_getdata) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];
    SQLSMALLINT dataType;
    MADB_TypeInfo *expTypeInfo = TypeInfoV3;
    qsort(expTypeInfo, TYPES_COUNT, sizeof(MADB_TypeInfo), compare);

    get_conn_string((char *) conn, "NO_CACHE=0");

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    CHECK_STMT_RC(Stmt1, SQLGetTypeInfo(Stmt1, SQL_ALL_TYPES));

    SQLCHAR typeName[BUFFER_LEN];
    CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));

    // VARCHAR
    CHECK_STMT_RC(Stmt1, SQLGetData(Stmt1, 1, SQL_C_CHAR, typeName, BUFFER_LEN, 0));
    FAIL_IF_NE_STR(expTypeInfo[0].TypeName, typeName, "Invalid TYPE_NAME returned");

    // INTEGER
    CHECK_STMT_RC(Stmt1, SQLSetPos(Stmt1, 3, SQL_POSITION, SQL_LOCK_NO_CHANGE));
    CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));
    CHECK_STMT_RC(Stmt1, SQLGetData(Stmt1, 2, SQL_SMALLINT, &dataType, BUFFER_LEN, 0));
    FAIL_IF_NE_INT(expTypeInfo[2].DataType, dataType, "Invalid DATA_TYPE returned");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));
    return OK;
}

ODBC_TEST(t_sqlgettypeinfo_closecursor) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];
    MADB_TypeInfo *expTypeInfo = TypeInfoV3;
    qsort(expTypeInfo, TYPES_COUNT, sizeof(MADB_TypeInfo), compare);

    get_conn_string((char *) conn, "");

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    CHECK_STMT_RC(Stmt1, SQLGetTypeInfo(Stmt1, SQL_ALL_TYPES));

    CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));

    CHECK_STMT_RC(Stmt1, SQLCloseCursor(Stmt1));
    FAIL_IF(SQL_SUCCEEDED(SQLFetch(Stmt1)), "SQLFetch should fail after SQLCloseCursor");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));
    return OK;
}

ODBC_TEST(t_sqlgettypeinfo_colattribute) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];
    SQLLEN unsgnd;
    SQLCHAR typeName[BUFFER_LEN];

    get_conn_string((char *) conn, "");

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    CHECK_STMT_RC(Stmt1, SQLGetTypeInfo(Stmt1, SQL_ALL_TYPES));

    CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));

    CHECK_STMT_RC(Stmt1, SQLColAttribute(Stmt1, 1, SQL_DESC_UNSIGNED, NULL, 0, NULL, &unsgnd));
    FAIL_IF_NE_INT(0, unsgnd, "Invalid SQL_DESC_UNSIGNED");

    CHECK_STMT_RC(Stmt1, SQLColAttribute(Stmt1, 1, SQL_DESC_TYPE_NAME, typeName, BUFFER_LEN, NULL, NULL));
    FAIL_IF_NE_STR("longtext", typeName, "Invalid SQL_DESC_TYPE_NAME");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));
    return OK;
}

ODBC_TEST(t_sqlgettypeinfo_describecol) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLCHAR conn[512];

    SQLCHAR ColumnName[BUFFER_LEN];
    SQLSMALLINT NameLength;
    SQLSMALLINT DataType;
    SQLULEN ColumnSize;
    SQLSMALLINT DecimalDigits;
    SQLSMALLINT Nullable;

    get_conn_string((char *) conn, "");

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    CHECK_STMT_RC(Stmt1, SQLGetTypeInfo(Stmt1, SQL_ALL_TYPES));

    int i;
    for(i = 0; i < TYPE_INFO_FIELDS_COUNT;i++) {
        CHECK_STMT_RC(Stmt1, SQLDescribeCol(Stmt1, i + 1, ColumnName, BUFFER_LEN, &NameLength, &DataType, &ColumnSize,
                                           &DecimalDigits, &Nullable));
        FAIL_IF_NE_STR(fieldNames[i], ColumnName, "Invalid column name returned");
        FAIL_IF_NE_INT(fieldTypes[i], DataType, "Invalid data type returned");
        FAIL_IF_NE_INT(fieldSizes[i], ColumnSize, "Invalid column size returned");
        FAIL_IF_NE_INT(0, DecimalDigits, "Invalid decimal digits returned");
        FAIL_IF_NE_INT(DataType == SQL_VARCHAR, Nullable, "Invalid nullable returned");
    }

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));
    return OK;
}

ODBC_TEST(t_sqlgettypeinfo_stmtattributes) {
    SQLHANDLE henv1;
    SQLHANDLE Connection1;
    SQLHANDLE Stmt1;
    SQLHANDLE ard;
    SQLCHAR conn[512];
    int rc, numOfRowsFetched;
    SQLLEN cpSize;
    MADB_TypeInfo recTypeInfo, *expTypeInfo = TypeInfoV3;
    INIT_TYPE_INFO(&recTypeInfo);
    qsort(expTypeInfo, TYPES_COUNT, sizeof(MADB_TypeInfo), compare);

    get_conn_string((char *) conn, "");

    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
    CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                      (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER));
    CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
    CHECK_DBC_RC(Connection1,
                 SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT) strlen((const char *) conn), NULL, 0,
                                  NULL, SQL_DRIVER_NOPROMPT));
    CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

    CHECK_STMT_RC(Stmt1, SQLGetTypeInfo(Stmt1, SQL_ALL_TYPES));

    CHECK_STMT_RC(Stmt1, SQLGetStmtAttr(Stmt1, SQL_ATTR_APP_ROW_DESC, &ard, SQL_IS_POINTER, NULL));
    CHECK_DESC_RC(ard, SQLSetDescField (ard, 3, SQL_DESC_PRECISION, (void*)10, 0));
    CHECK_STMT_RC(ard, SQLSetDescField (ard, 3, SQL_DESC_SCALE, (void*) 3, 0));
    CHECK_STMT_RC(ard, SQLSetDescField (ard, 1, SQL_DESC_LENGTH, (void*) 50, 0));

    // We need to bind after SQLSetDescField, because it unbinds updated fields because of bug41018
    // Unable to find info on this bug
    if ((rc = bind_type_info(Stmt1, &recTypeInfo, &cpSize)) != OK) {
        return rc;
    }
    numOfRowsFetched = 0;
    while (SQL_SUCCEEDED(SQLFetch(Stmt1))) {
        if ((rc = check_stmt_correct_type_info(&expTypeInfo[numOfRowsFetched], &recTypeInfo, cpSize)) != OK) {
            return rc;
        }
        numOfRowsFetched++;
    }
    FAIL_IF_NE_INT(TYPES_COUNT, numOfRowsFetched, "Wrong number of rows fetched");

    CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
    CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
    CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
    CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

    return OK;
}

MA_ODBC_TESTS my_tests[] =
{
    {t_sqlgettypeinfo2, "t_sqlgettypeinfo2", NORMAL, ALL_DRIVERS},
    {t_sqlgettypeinfo3, "t_sqlgettypeinfo3", NORMAL, ALL_DRIVERS},
    {t_sqlgettypeinfo_sequential, "t_sqlgettypeinfo_sequential", NORMAL, ALL_DRIVERS},
    {t_sqlgettypeinfo_no_bind, "t_sqlgettypeinfo_no_bind", NORMAL, ALL_DRIVERS},
    {t_sqlgettypeinfo_fetchscroll, "t_sqlgettypeinfo_fetchscroll", NORMAL, ALL_DRIVERS},
    {t_sqlgettypeinfo_getdata, "t_sqlgettypeinfo_getdata", NORMAL, ALL_DRIVERS},
    {t_sqlgettypeinfo_closecursor, "t_sqlgettypeinfo_closecursor", NORMAL, ALL_DRIVERS},
    {t_sqlgettypeinfo_colattribute, "t_sqlgettypeinfo_colattribute", NORMAL, ALL_DRIVERS},
    {t_sqlgettypeinfo_describecol, "t_sqlgettypeinfo_describecol", NORMAL, ANSI_DRIVER},
    {t_sqlgettypeinfo_stmtattributes, "t_sqlgettypeinfo_stmtattributes", NORMAL, ALL_DRIVERS},
    {NULL, NULL, NORMAL, ALL_DRIVERS}
};


int main(int argc, char **argv) {
    int tests = sizeof(my_tests) / sizeof(MA_ODBC_TESTS) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}
