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


ODBC_TEST(client_side_show)
{
    OK_SIMPLE_STMT(Stmt, "SHOW USERS");
    OK_SIMPLE_STMT(Stmt, "SHOW DATABASES");
    OK_SIMPLE_STMT(Stmt, "SHOW TABLES");

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_prepare)
{
    SQLWCHAR aParam[20] = {0x441, 0x442, 0x43E}; // сто
    SQLLEN aParamLen = sqlwcharlen(aParam) * sizeof(SQLWCHAR);
    SQLCHAR bParam[10] = "hundred";
    SQLLEN bParamLen = strlen(bParam);
    SQL_NUMERIC_STRUCT cParam = {0, 0, 1, "\xff\xa0"}; // With scale=3 should be equal to 41.215
    SQLLEN cParamLen;
    SQLINTEGER dParam = -193;
    SQLLEN dParamLen;
    SQLUBIGINT eParam = 18446744073709551615ULL;
    SQLLEN eParamLen;
    SQL_TIMESTAMP_STRUCT fParam = {2020, 11, 20, 19, 5, 31};
    SQLLEN fParamLen;
    SQLCHAR gParam[10] = "\x64\x65";
    SQLLEN gParamLen = strlen(gParam);
    SQLCHAR hParam[20] = "\x61\x62\x63";
    SQLLEN hParamLen = strlen(hParam);
    char iParam = 120;
    SQLLEN iParamLen;
    SQL_TIME_STRUCT jParam = {19, 43, 1};
    SQLLEN jParamLen;
    SQL_DATE_STRUCT kParam = {2020, 11, 20};
    SQLLEN kParamLen;
    SQLDOUBLE lParam = -123456789.123456789123456789;
    SQLLEN lParamLen;
    SQLINTEGER nParam = -16;
    SQLLEN nParamInd = SQL_NULL_DATA;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS csps");
    char *createTableQuery = "CREATE TABLE csps(a char(10), b varchar(20), c numeric(10,3), d int, e bigint unsigned, "
                             "f datetime(6), g binary(5), h bit(64), i tinyint, j time, k date, l double, n int)";

    OK_SIMPLE_STMT(Stmt, createTableQuery);
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO csps VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                                   SQL_NTS));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, sizeof(aParam) / sizeof(SQLWCHAR),
                                   0, aParam, sizeof(aParam), &aParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, bParam, sizeof(bParam),
                                   &bParamLen));

    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_NUMERIC, SQL_NUMERIC, 10, 3, &cParam, sizeof(cParam),
                                   &cParamLen));

    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dParam, sizeof(dParam),
                                   &dParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 5, SQL_PARAM_INPUT, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &eParam, sizeof(eParam),
                                   &eParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 6, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, sizeof(fParam),
                                   6, &fParam, sizeof(fParam), &fParamLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 7, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, sizeof(gParam), 0, gParam,
                                         sizeof(gParam), &gParamLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 8, SQL_PARAM_INPUT, SQL_C_BIT, SQL_BIT, sizeof(hParam), 0, hParam,
                                         sizeof(hParam), &hParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 9, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &iParam, sizeof(iParam),
                                   &iParamLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 10, SQL_PARAM_INPUT, SQL_C_TYPE_TIME, SQL_TYPE_TIME, sizeof(jParam), 0,
                                         &jParam, sizeof(jParam), &jParamLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 11, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_TYPE_DATE, sizeof(kParam), 0,
                                         &kParam, sizeof(kParam), &kParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 12, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 20, &lParam, sizeof(lParam),
                                   &lParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 13, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &nParam, sizeof(nParam),
                                   &nParamInd));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    SQLWCHAR aCol[20];
    memset(aCol, 0, sizeof(aCol));
    SQLLEN aColLen;
    SQLCHAR bCol[20];
    SQLLEN bColLen;
    SQL_NUMERIC_STRUCT cCol;
    SQLLEN cColLen;
    SQLINTEGER dCol;
    SQLLEN dColLen;
    SQLUBIGINT eCol;
    SQLLEN eColLen;
    SQL_TIMESTAMP_STRUCT fCol;
    SQLLEN fColLen;
    SQLCHAR gCol[10];
    SQLLEN gColLen;
    SQLCHAR hCol[10];
    SQLLEN hColLen;
    char iCol;
    SQLLEN iColLen;
    SQL_TIME_STRUCT jCol;
    SQLLEN jColLen;
    SQL_DATE_STRUCT kCol;
    SQLLEN kColLen;
    SQLDOUBLE lCol;
    SQLLEN lColLen;
    SQLINTEGER nCol;
    SQLLEN nColInd;
    SQLHDESC ard;

    OK_SIMPLE_STMT(Stmt, "SELECT * FROM csps");
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_WCHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_CHAR, bCol, sizeof(bCol), &bColLen));

    // Precision and scale Ard should be provided via descriptor API: SQLSetDescField.
    // SQLSetDescField for all required fields must be called before the SQLBindCol, otherwise the column will be unbound.
    // I don't fully understand such behavior, but this is how MySQL driver works too.
    CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_APP_ROW_DESC, &ard, SQL_IS_POINTER, NULL));
    CHECK_STMT_RC(ard, SQLSetDescField(ard, 3, SQL_DESC_PRECISION, (void *) 10, 0));
    CHECK_STMT_RC(ard, SQLSetDescField(ard, 3, SQL_DESC_SCALE, (void *) 3, 0));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_NUMERIC, &cCol, sizeof(cCol), &cColLen));

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_SLONG, &dCol, sizeof(dCol), &dColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_UBIGINT, &eCol, sizeof(eCol), &eColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 6, SQL_C_TYPE_TIMESTAMP, &fCol, sizeof(fCol), &fColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 7, SQL_C_BINARY, gCol, sizeof(gCol), &gColLen));

    // FIGURE OUT WHY BIT 0 is returned if we inserted 1.
    // Seems like the engine sends "" for some reason. Looked at the wireshark traces and there's indeed nothing in the
    // BIT field, so don't compare it for now.
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 8, SQL_C_BIT, hCol, sizeof(hCol), &hColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 9, SQL_C_TINYINT, &iCol, sizeof(iCol), &iColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 10, SQL_C_TYPE_TIME, &jCol, sizeof(jCol), &jColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 11, SQL_C_TYPE_DATE, &kCol, sizeof(kCol), &kColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 12, SQL_C_DOUBLE, &lCol, sizeof(lCol), &lColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 13, SQL_C_LONG, &nCol, sizeof(nCol), &nColInd));
    if (SQL_SUCCEEDED(SQLFetch(Stmt)))
    {
        if (!(iOdbc() && is_ansi_driver())) {
          // iODBC Ansi DM works bad with WCHARs
          IS_WSTR(aCol, aParam, aColLen);
        }
        IS_STR(bCol, bParam, bColLen);
        IS_STR(cCol.val, cParam.val, strlen(cCol.val));
        is_num(dCol, dParam);
        is_num(eCol, eParam);
        FAIL_IF(fParam.year != fCol.year || fParam.month != fCol.month || fParam.day != fCol.day ||
                fParam.hour != fCol.hour || fParam.minute != fCol.minute || fParam.second != fCol.second,
                "wrong timestamp value returned");
        IS_STR(gCol, gParam, gColLen);
        is_num(iCol, iParam);
        FAIL_IF(jParam.hour != jCol.hour || jParam.minute != jCol.minute || jParam.second != jCol.second,
                "wrong time value fetched");
        FAIL_IF(kParam.year != kCol.year || kParam.month != kCol.month || kCol.day != kCol.day,
                "wrong date value fetched");
        FAIL_IF(lParam - lCol >= 1e9 || lCol - lParam >= 1e9, "wrong double value fetched");
        FAIL_IF(nColInd != SQL_NULL_DATA, "column should be NULL");
    }

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS csps");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_static_cursor)
{
    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_cursor(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_STATIC, 0));
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_cursor VALUES(?, ?)", SQL_NTS));

    for (i = 0; i < 5; ++i)
    {
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                             sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                             sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    }

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_cursor ORDER BY a");
    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_ABSOLUTE, 3));
    is_num(bCol, bParam[2]);
    IS_STR(aCol, aParam[2], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_PREV, 3));
    is_num(bCol, bParam[1]);
    IS_STR(aCol, aParam[1], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_FIRST, 3));
    is_num(bCol, bParam[0]);
    IS_STR(aCol, aParam[0], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 4));
    is_num(bCol, bParam[1]);
    IS_STR(aCol, aParam[1], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_LAST, 3));
    is_num(bCol, bParam[4]);
    IS_STR(aCol, aParam[4], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_RELATIVE, -1));
    is_num(bCol, bParam[3]);
    IS_STR(aCol, aParam[3], aColLen);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_dynamic_cursor)
{
    // Allow the use of dynamic cursor in the DSN options.
    SQLHANDLE henv, hdbc, hstmt;
    SQLCHAR DSNString[256];

    sprintf((char *)DSNString, "DRIVER=%s;SERVER=%s;DATABASE=%s;UID=%s;PASSWORD=%s;PORT=%d;NO_SSPS=%d;OPTIONS=%lu;%s",
            my_drivername, my_servername, my_schema, my_uid, my_pwd, my_port, NoSsps, my_options|32, add_connstr);

    mydrvconnect(&henv, &hdbc, &hstmt, DSNString);

    CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_DYNAMIC, 0));

    // INVESTIGATE WHAT'S UP WITH THIS CONCURRENCY ATTRIBUTE.
    // SEEMS LIKE IT RETURNS SUCCESS_WITH_INFO AND DOES NOT CHANGE THE ATTRIBUTE.
    // THE ONLY VALUE IT USES IS SQL_CONCUR_READ_ONLY.
    // SETTING THIS THING IS BASICALLY POINTLESS BUT I'M KEEPING THE STATEMENT TO NOT FORGET.
    CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CONCURRENCY, (SQLPOINTER) SQL_CONCUR_ROWVER, 0));
    CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 1, 0));

    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS cs_cursor");
    OK_SIMPLE_STMT(hstmt, "CREATE TABLE cs_cursor(a char(10), b int)");
    CHECK_STMT_RC(hstmt, SQLPrepare(hstmt, (SQLCHAR *) "INSERT INTO cs_cursor VALUES(?, ?)", SQL_NTS));

    for (i = 0; i < 5; ++i)
    {
        CHECK_STMT_RC(hstmt, SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                              sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(hstmt,
                      SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                       sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(hstmt, SQLExecute(hstmt));
    }

    OK_SIMPLE_STMT(hstmt, "SELECT a, b FROM cs_cursor ORDER BY a");
    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;

    CHECK_STMT_RC(hstmt, SQLBindCol(hstmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(hstmt, SQLBindCol(hstmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    CHECK_STMT_RC(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 3));
    is_num(bCol, bParam[2]);
    IS_STR(aCol, aParam[2], aColLen);

    CHECK_STMT_RC(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_PREV, 3));
    is_num(bCol, bParam[1]);
    IS_STR(aCol, aParam[1], aColLen);

    CHECK_STMT_RC(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_FIRST, 3));
    is_num(bCol, bParam[0]);
    IS_STR(aCol, aParam[0], aColLen);

    CHECK_STMT_RC(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_NEXT, 4));
    is_num(bCol, bParam[1]);
    IS_STR(aCol, aParam[1], aColLen);

    CHECK_STMT_RC(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_LAST, 3));
    is_num(bCol, bParam[4]);
    IS_STR(aCol, aParam[4], aColLen);

    CHECK_STMT_RC(hstmt, SQLFetchScroll(hstmt, SQL_FETCH_RELATIVE, -1));
    is_num(bCol, bParam[3]);
    IS_STR(aCol, aParam[3], aColLen);

    ODBC_Disconnect(henv, hdbc, hstmt);

    return OK;
}


ODBC_TEST(client_side_forward_only_cursor)
{
    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_cursor(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_FORWARD_ONLY, 0));
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_cursor VALUES(?, ?)", SQL_NTS));

    for (i = 0; i < 5; ++i)
    {
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                             sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                             sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    }

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_cursor ORDER BY a");
    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    FAIL_IF(SQL_SUCCEEDED(SQLFetchScroll(Stmt, SQL_FETCH_ABSOLUTE, 3)),
            "only SQL_FETCH_NEXT is allowed for a FORWARD_ONLY cursor");

    int currentRow = 0;
    while (SQL_SUCCEEDED(SQLFetch(Stmt)))
    {
        is_num(bCol, bParam[currentRow]);
        IS_STR(aCol, aParam[currentRow], aColLen);
        currentRow++;
    }
    is_num(currentRow, 5);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_multiparam)
{
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_cursor(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 5, 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER) SQL_CURSOR_STATIC, 0));
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_cursor VALUES(?, ?)", SQL_NTS));

    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[0],
                                         sizeof(aParam[0]), len));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[0],
                                         sizeof(bParam[0]), NULL));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

    SQLLEN numOfRows;
    // The following select will return result only for the last set of parameters
    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_cursor ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &numOfRows));
    is_num(numOfRows, 5);

    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_ABSOLUTE, 3));
    is_num(bCol, bParam[2]);
    IS_STR(aCol, aParam[2], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_PREV, 3));
    is_num(bCol, bParam[1]);
    IS_STR(aCol, aParam[1], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_FIRST, 3));
    is_num(bCol, bParam[0]);
    IS_STR(aCol, aParam[0], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 4));
    is_num(bCol, bParam[1]);
    IS_STR(aCol, aParam[1], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_LAST, 3));
    is_num(bCol, bParam[4]);
    IS_STR(aCol, aParam[4], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_RELATIVE, -1));
    is_num(bCol, bParam[3]);
    IS_STR(aCol, aParam[3], aColLen);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_multirow)
{
    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_STATIC, 0));

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_cursor(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_cursor VALUES(?, ?)", SQL_NTS));

    for (i = 0; i < 5; ++i)
    {
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                             sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                             sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    }

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_cursor ORDER BY a");

    // Do a row-wise binding.
    struct {
        SQLCHAR aCol[10];
        SQLINTEGER bCol;
        SQLLEN aColLen;
    } rows[5];
    size_t row_size = sizeof(rows[0]);

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) row_size, 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 2, 0));

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, &rows[0].aCol, sizeof(rows[0].aCol), &rows[0].aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &rows[0].bCol, 0, NULL));

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_ABSOLUTE, 3));
    is_num(rows[0].bCol, bParam[2]);
    is_num(rows[1].bCol, bParam[3]);
    IS_STR(rows[0].aCol, aParam[2], rows[0].aColLen);
    IS_STR(rows[1].aCol, aParam[3], rows[1].aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_PREV, 3));
    is_num(rows[0].bCol, bParam[0]);
    is_num(rows[1].bCol, bParam[1]);
    IS_STR(rows[0].aCol, aParam[0], rows[0].aColLen);
    IS_STR(rows[1].aCol, aParam[1], rows[1].aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_LAST, 3));
    is_num(rows[0].bCol, bParam[3]);
    is_num(rows[1].bCol, bParam[4]);
    IS_STR(rows[0].aCol, aParam[3], rows[0].aColLen);
    IS_STR(rows[1].aCol, aParam[4], rows[1].aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_RELATIVE, -1));
    is_num(rows[0].bCol, bParam[2]);
    is_num(rows[1].bCol, bParam[3]);
    IS_STR(rows[0].aCol, aParam[2], rows[0].aColLen);
    IS_STR(rows[1].aCol, aParam[3], rows[1].aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_FIRST, 3));
    is_num(rows[0].bCol, bParam[0]);
    is_num(rows[1].bCol, bParam[1]);
    IS_STR(rows[0].aCol, aParam[0], rows[0].aColLen);
    IS_STR(rows[1].aCol, aParam[1], rows[1].aColLen);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 4));
    is_num(rows[0].bCol, bParam[2]);
    is_num(rows[1].bCol, bParam[3]);
    IS_STR(rows[0].aCol, aParam[2], rows[0].aColLen);
    IS_STR(rows[1].aCol, aParam[3], rows[1].aColLen);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_multirow_columnwise)
{
    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_STATIC, 0));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_cursor(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_cursor VALUES(?, ?)", SQL_NTS));

    for (i = 0; i < 5; ++i)
    {
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                             sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                             sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    }

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_cursor ORDER BY a");

    // Do a column-wise binding.
    SQLCHAR aCol[5][10];
    SQLLEN aColLen[5];
    SQLINTEGER bCol[5];

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) SQL_BIND_BY_COLUMN, 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 2, 0));

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol[0]), aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, bCol, 0, NULL));

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_ABSOLUTE, 3));
    is_num(bCol[0], bParam[2]);
    is_num(bCol[1], bParam[3]);
    IS_STR(aCol[0], aParam[2], aColLen[0]);
    IS_STR(aCol[1], aParam[3], aColLen[1]);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_PREV, 3));
    is_num(bCol[0], bParam[0]);
    is_num(bCol[1], bParam[1]);
    IS_STR(aCol[0], aParam[0], aColLen[0]);
    IS_STR(aCol[1], aParam[1], aColLen[1]);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_LAST, 3));
    is_num(bCol[0], bParam[3]);
    is_num(bCol[1], bParam[4]);
    IS_STR(aCol[0], aParam[3], aColLen[0]);
    IS_STR(aCol[1], aParam[4], aColLen[1]);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_RELATIVE, -1));
    is_num(bCol[0], bParam[2]);
    is_num(bCol[1], bParam[3]);
    IS_STR(aCol[0], aParam[2], aColLen[0]);
    IS_STR(aCol[1], aParam[3], aColLen[1]);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_FIRST, 3));
    is_num(bCol[0], bParam[0]);
    is_num(bCol[1], bParam[1]);
    IS_STR(aCol[0], aParam[0], aColLen[0]);
    IS_STR(aCol[1], aParam[1], aColLen[1]);

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 4));
    is_num(bCol[0], bParam[2]);
    is_num(bCol[1], bParam[3]);
    IS_STR(aCol[0], aParam[2], aColLen[0]);
    IS_STR(aCol[1], aParam[3], aColLen[1]);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_cursor");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_get_data)
{
    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_getdata");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_getdata(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_getdata VALUES(?, ?)", SQL_NTS));

    for (i = 0; i < 5; ++i)
    {
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                             sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                             sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    }

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_RESET_PARAMS));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    // Running these SELECTs multiple times just to verify that we release the results properly.
    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_getdata ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "SELECT a, b FROM cs_getdata ORDER BY a", SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;

    int currentRow = 0;
    while (SQL_SUCCEEDED(SQLFetch(Stmt)))
    {
        CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, sizeof(aCol), &aColLen));
        CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 2, SQL_C_LONG, &bCol, sizeof(bCol), &bColLen));
        is_num(bCol, bParam[currentRow]);
        IS_STR(aCol, aParam[currentRow], aColLen);
        currentRow++;
    }
    is_num(currentRow, 5);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_getdata");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_get_data_buffered)
{
    SQLCHAR aParam[15] = "0123456789";
    SQLLEN len = SQL_NTS;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_getdata");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_getdata(a char(20))");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_getdata VALUES(?)", SQL_NTS));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam, sizeof(aParam),
                                   &len));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_RESET_PARAMS));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt, "SELECT a FROM cs_getdata");

    SQLCHAR aCol[15];
    SQLLEN aColLen;

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, 2, &aColLen));
    is_num(aColLen, 10);
    IS_STR(aCol, "0", aColLen);

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, 3, &aColLen));
    is_num(aColLen, 9);
    IS_STR(aCol, "12", aColLen);

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, 1, &aColLen));
    is_num(aColLen, 7);
    IS_STR(aCol, "\0", aColLen);

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, 0, &aColLen));
    is_num(aColLen, 7);
    IS_STR(aCol, "\0", aColLen);

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, 6, &aColLen));
    is_num(aColLen, 7);
    IS_STR(aCol, "34567", aColLen);

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, 10, &aColLen));
    is_num(aColLen, 2);
    IS_STR(aCol, "89", aColLen);

    FAIL_IF(SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, 3, &aColLen) != SQL_NO_DATA, "SQL_NO_DATA should have been returned");

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_getdata");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_get_data_many_types)
{
    SQLWCHAR aParam[20] = {0x441, 0x442, 0x43E}; // сто
    SQLLEN aParamLen = sqlwcharlen(aParam) * sizeof(SQLWCHAR);
    SQLCHAR bParam[10] = "hundred";
    SQLLEN bParamLen = strlen(bParam);
    SQL_NUMERIC_STRUCT cParam = {0, 0, 1, "\xff\xa0"}; // With scale=3 should be equal to 41.215
    SQLLEN cParamLen;
    SQLINTEGER dParam = -193;
    SQLLEN dParamLen;
    SQLUBIGINT eParam = 18446744073709551615ULL;
    SQLLEN eParamLen;
    SQL_TIMESTAMP_STRUCT fParam = {2020, 11, 20, 19, 5, 31, 123456000};
    SQLLEN fParamLen;
    SQLCHAR gParam[10] = "\x64\x65";
    SQLLEN gParamLen = strlen(gParam);
    SQLCHAR hParam[20] = "\x61\x62\x63";
    SQLLEN hParamLen = strlen(hParam);
    char iParam = 120;
    SQLLEN iParamLen;
    SQL_TIME_STRUCT jParam = {19, 43, 1};
    SQLLEN jParamLen;
    SQL_DATE_STRUCT kParam = {2020, 11, 20};
    SQLLEN kParamLen;
    SQLDOUBLE lParam = -123456789.123456789123456789;
    SQLLEN lParamLen;
    SQLINTEGER nParam = -16;
    SQLLEN nParamInd = SQL_NULL_DATA;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_getdata");
    char *createTableQuery = "CREATE TABLE cs_getdata(a char(10), b varchar(20), c numeric(10,3), d int, e bigint unsigned, "
                             "f datetime(6), g binary(5), h bit(64), i tinyint, j time, k date, l double, n int)";

    OK_SIMPLE_STMT(Stmt, createTableQuery);
    CHECK_STMT_RC(Stmt,
                  SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_getdata VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                             SQL_NTS));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, sizeof(aParam) / sizeof(SQLWCHAR),
                                   0, aParam, sizeof(aParam), &aParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, bParam, sizeof(bParam),
                                   &bParamLen));

    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_NUMERIC, SQL_NUMERIC, 10, 3, &cParam, sizeof(cParam),
                                   &cParamLen));

    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dParam, sizeof(dParam),
                                   &dParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 5, SQL_PARAM_INPUT, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &eParam, sizeof(eParam),
                                   &eParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 6, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, sizeof(fParam),
                                   6, &fParam, sizeof(fParam), &fParamLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 7, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, sizeof(gParam), 0, gParam,
                                         sizeof(gParam), &gParamLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 8, SQL_PARAM_INPUT, SQL_C_BIT, SQL_BIT, sizeof(hParam), 0, hParam,
                                         sizeof(hParam), &hParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 9, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &iParam, sizeof(iParam),
                                   &iParamLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 10, SQL_PARAM_INPUT, SQL_C_TYPE_TIME, SQL_TYPE_TIME, sizeof(jParam), 0,
                                         &jParam, sizeof(jParam), &jParamLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 11, SQL_PARAM_INPUT, SQL_C_TYPE_DATE, SQL_TYPE_DATE, sizeof(kParam), 0,
                                         &kParam, sizeof(kParam), &kParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 12, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 20, &lParam, sizeof(lParam),
                                   &lParamLen));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 13, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &nParam, sizeof(nParam),
                                   &nParamInd));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    SQLWCHAR aCol[20];
    memset(aCol, 0, sizeof(aCol));
    SQLLEN aColLen;
    SQLCHAR bCol[20];
    SQLLEN bColLen;
    SQL_NUMERIC_STRUCT cCol;
    SQLLEN cColLen;
    SQLINTEGER dCol;
    SQLLEN dColLen;
    SQLUBIGINT eCol;
    SQLLEN eColLen;
    SQL_TIMESTAMP_STRUCT fCol;
    SQLLEN fColLen;
    SQLCHAR gCol[10];
    SQLLEN gColLen;
    SQLCHAR hCol[10];
    SQLLEN hColLen;
    char iCol;
    SQLLEN iColLen;
    SQL_TIME_STRUCT jCol;
    SQLLEN jColLen;
    SQL_DATE_STRUCT kCol;
    SQLLEN kColLen;
    SQLDOUBLE lCol;
    SQLLEN lColLen;
    SQLINTEGER nCol;
    SQLLEN nColInd;
    SQLHDESC ard;

    OK_SIMPLE_STMT(Stmt, "SELECT * FROM cs_getdata");

    CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_APP_ROW_DESC, &ard, SQL_IS_POINTER, NULL));
    CHECK_DESC_RC(ard, SQLSetDescField(ard, 3, SQL_DESC_PRECISION, (void *) 10, 0));
    CHECK_DESC_RC(ard, SQLSetDescField(ard, 3, SQL_DESC_SCALE, (void *) 3, 0));
    CHECK_DESC_RC(ard, SQLSetDescField(ard, 3, SQL_DESC_CONCISE_TYPE, (void *) SQL_C_NUMERIC, 0));

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_WCHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 2, SQL_C_CHAR, bCol, sizeof(bCol), &bColLen));
    //TODO https://memsql.atlassian.net/jira/software/c/projects/PLAT/issues/PLAT-5344
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 3, iOdbc() ? SQL_C_NUMERIC : SQL_ARD_TYPE, &cCol, sizeof(cCol), &cColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 4, SQL_C_LONG, &dCol, sizeof(dCol), &dColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 5, SQL_C_UBIGINT, &eCol, sizeof(eCol), &eColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 6, SQL_C_TIMESTAMP, &fCol, sizeof(fCol), &fColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 7, SQL_C_BINARY, &gCol, sizeof(gCol), &gColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 8, SQL_C_BIT, &hCol, sizeof(hCol), &hColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 9, SQL_C_TINYINT, &iCol, sizeof(iCol), &iColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 10, SQL_C_TIME, &jCol, sizeof(jCol), &jColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 11, SQL_C_DATE, &kCol, sizeof(kCol), &kColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 12, SQL_C_DOUBLE, &lCol, sizeof(lCol), &lColLen));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 13, SQL_C_LONG, &nCol, sizeof(nCol), &nColInd));

    if (!(iOdbc() && is_ansi_driver())) {
      // iODBC Ansi DM works bad with WCHARs
      IS_WSTR(aCol, aParam, aColLen);
    }
    IS_STR(bCol, bParam, bColLen);
    IS_STR(cCol.val, cParam.val, strlen(cCol.val));
    is_num(dCol, dParam);
    is_num(eCol, eParam);
    FAIL_IF(fParam.year != fCol.year || fParam.month != fCol.month || fParam.day != fCol.day ||
            fParam.hour != fCol.hour || fParam.minute != fCol.minute || fParam.second != fCol.second ||
            fParam.fraction != fCol.fraction,"wrong timestamp value returned");
    IS_STR(gCol, gParam, gColLen);
    is_num(iCol, iParam);
    FAIL_IF(jParam.hour != jCol.hour || jParam.minute != jCol.minute || jParam.second != jCol.second,
            "wrong time value fetched");
    FAIL_IF(kParam.year != kCol.year || kParam.month != kCol.month || kCol.day != kCol.day, "wrong date value fetched");
    FAIL_IF(lParam - lCol >= 1e9 || lCol - lParam >= 1e9, "wrong double value fetched");
    FAIL_IF(nColInd != SQL_NULL_DATA, "column should be NULL");

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_getdata");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_put_data)
{
    SQLCHAR aParam[15] = "0123456789";
    SQLLEN len = SQL_DATA_AT_EXEC;
    SQLPOINTER paramData;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_putdata");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_putdata(a char(20))");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_putdata VALUES(?)", SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, (SQLPOINTER) 1,
                                         sizeof(aParam), &len));

    FAIL_IF(SQLExecute(Stmt) != SQL_NEED_DATA, "SQL_NEED_DATA expected");
    FAIL_IF(SQLParamData(Stmt, &paramData) != SQL_NEED_DATA, "SQL_NEED_DATA expected");

    CHECK_STMT_RC(Stmt, SQLPutData(Stmt, aParam, strlen(aParam)));
    CHECK_STMT_RC(Stmt, SQLPutData(Stmt, aParam, strlen(aParam)));
    CHECK_STMT_RC(Stmt, SQLParamData(Stmt, &paramData));

    len = SQL_NTS;
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam, sizeof(aParam),
                                   &len));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

    OK_SIMPLE_STMT(Stmt, "SELECT a FROM cs_putdata ORDER BY a DESC");
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    SQLCHAR aCol[25];
    SQLLEN aColLen;
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, aCol, sizeof(aCol), &aColLen));
    IS_STR(aCol, aParam, 10);
    IS_STR(aCol + 10, aParam, 10);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_putdata");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_put_data_non_char)
{
    SQLINTEGER aParam = 123;
    SQLLEN len = SQL_DATA_AT_EXEC;
    SQLPOINTER paramData;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_putdata");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_putdata(a int)");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_putdata VALUES(?)", SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, sizeof(aParam), 0, &aParam,
                                         sizeof(aParam), &len));

    FAIL_IF(SQLExecute(Stmt) != SQL_NEED_DATA, "SQL_NEED_DATA expected");
    FAIL_IF(SQLParamData(Stmt, &paramData) != SQL_NEED_DATA, "SQL_NEED_DATA expected");

    CHECK_STMT_RC(Stmt, SQLPutData(Stmt, &aParam, sizeof(aParam)));
    //FAIL_IF(SQL_SUCCEEDED(SQLPutData(Stmt, &aParam, sizeof(aParam))),
    //        "second call to SQLPutData should raise an error");
    CHECK_STMT_RC(Stmt, SQLParamData(Stmt, &paramData));

    OK_SIMPLE_STMT(Stmt, "SELECT a FROM cs_putdata");
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    SQLINTEGER aCol;
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_LONG, &aCol, sizeof(aCol), NULL));
    is_num(aCol, aParam);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_putdata");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_put_data_multiple) {
    SQLCHAR aParam[3][5] = {"ab", "bc", "cd"};
    SQLLEN len[3] = {SQL_DATA_AT_EXEC, SQL_DATA_AT_EXEC, SQL_DATA_AT_EXEC};
    SQLPOINTER paramData;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_put_data");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_put_data(a char(10))");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_put_data VALUES(?)", SQL_NTS));

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) 3, 0));

    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[0],
                                         sizeof(aParam[0]), len));
    FAIL_IF(SQLExecute(Stmt) != SQL_NEED_DATA, "SQL_NEED_DATA expected");
    FAIL_IF(SQLParamData(Stmt, &paramData) != SQL_NEED_DATA, "SQL_NEED_DATA expected");

    FAIL_IF(SQLPutData(Stmt, aParam[0], strlen(aParam[0])) != SQL_ERROR,
            "SQL_ERROR is expected on a paramset size > 1");

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_put_data");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_bulk_add) {
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_bulk_add");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_bulk_add(a char(10), b int)");

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 5, 0));

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, aParam, sizeof(aParam[0]), len));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, bParam, sizeof(bParam[0]), NULL));

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_bulk_add");
    CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 1, 0));
    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_bulk_add ORDER BY a");

    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    int currentRow = 0;
    while (SQL_SUCCEEDED(SQLFetch(Stmt))) {
        is_num(bCol, bParam[currentRow]);
        IS_STR(aCol, aParam[currentRow], aColLen);
        currentRow++;
    }
    is_num(currentRow, 5);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_bulk_add");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_set_pos_del) {
    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_STATIC, 0));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_set_pos");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_set_pos(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_set_pos VALUES(?, ?)", SQL_NTS));

    for (i = 0; i < 5; ++i) {
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                             sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                             sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    }

    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;
    SQLLEN affectedRows;

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_set_pos ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &affectedRows));
    is_num(affectedRows, 5);

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_ABSOLUTE, 3));
    is_num(bCol, 30);
    IS_STR(aCol, aParam[2], aColLen);

    CHECK_STMT_RC(Stmt, SQLSetPos(Stmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE));
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &affectedRows));
    is_num(affectedRows, 1);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_set_pos ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &affectedRows));
    is_num(affectedRows, 4);

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    CHECK_STMT_RC(Stmt, SQLSetPos(Stmt, 2, SQL_POSITION, SQL_LOCK_NO_CHANGE));

    // Passing 0 means delete every row in the rowset. Since the rowset size if 1, it'll delete a single row.
    CHECK_STMT_RC(Stmt, SQLSetPos(Stmt, 0, SQL_DELETE, SQL_LOCK_NO_CHANGE));
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &affectedRows));
    is_num(affectedRows, 1);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_set_pos ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &affectedRows));
    is_num(affectedRows, 3);

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(bCol, bParam[0]);
    IS_STR(aCol, aParam[0], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(bCol, bParam[3]);
    IS_STR(aCol, aParam[3], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(bCol, bParam[4]);
    IS_STR(aCol, aParam[4], aColLen);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_set_pos");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_set_pos_del_multiple_rows)
{
    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_set_pos");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_set_pos(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_set_pos VALUES(?, ?)", SQL_NTS));

    for (i = 0; i < 5; ++i)
    {
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                             sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                             sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    }

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;
    SQLLEN affectedRows;

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_set_pos ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &affectedRows));
    is_num(affectedRows, 5);

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 3, 0));
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    CHECK_STMT_RC(Stmt, SQLSetPos(Stmt, 0, SQL_DELETE, SQL_LOCK_NO_CHANGE));
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &affectedRows));
    is_num(affectedRows, 3);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt, "SELECT a, b FROM cs_set_pos ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &affectedRows));
    is_num(affectedRows, 2);

    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) 1, 0));

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(bCol, bParam[3]);
    IS_STR(aCol, aParam[3], aColLen);

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(bCol, bParam[4]);
    IS_STR(aCol, aParam[4], aColLen);

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_set_pos");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_set_pos_putdata)
{
    SQLCHAR aParam[15] = "0123456789";
    SQLLEN len = strlen(aParam);
    SQLPOINTER paramData;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_putdata");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_putdata(a char(30))");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_putdata VALUES(?)", SQL_NTS));
    CHECK_STMT_RC(Stmt,
                  SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam, sizeof(aParam),
                                   &len));

    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    aParam[0] = '9';
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

    SQLCHAR aCol[15];
    SQLLEN aColLen;

    OK_SIMPLE_STMT(Stmt, "SELECT a FROM cs_putdata ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_LAST, 1));
    IS_STR(aCol, aParam, aColLen);

    aColLen = SQL_DATA_AT_EXEC;

    FAIL_IF(SQLSetPos(Stmt, 1, SQL_UPDATE, SQL_LOCK_NO_CHANGE) != SQL_NEED_DATA, "SQL_NEED_DATA expected");
    FAIL_IF(SQLParamData(Stmt, &paramData) != SQL_NEED_DATA, "SQL_NEED_DATA expected");

    CHECK_STMT_RC(Stmt, SQLPutData(Stmt, aCol, strlen(aCol)));
    CHECK_STMT_RC(Stmt, SQLPutData(Stmt, aCol, strlen(aCol)));
    CHECK_STMT_RC(Stmt, SQLParamData(Stmt, &paramData));

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt, "SELECT a FROM cs_putdata ORDER BY a");
    CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_LAST, 1));
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, &aCol, 11, &aColLen));
    IS_STR(aCol, aParam, 10);
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, &aCol, 11, &aColLen));
    IS_STR(aCol, aParam, 10);
    FAIL_IF(SQLGetData(Stmt, 1, SQL_C_CHAR, &aCol, 10, &aColLen) != SQL_NO_DATA, "SQL_NO_DATA expected");

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_set_pos");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_multistatements)
{
    int i;
    SQLCHAR aParam[5][3] = {"ab", "bc", "cd", "de", "ef"};
    SQLLEN len[5] = {SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS, SQL_NTS};
    SQLINTEGER bParam[5] = {10, 20, 30, 40, 50};
    SQLLEN rows_selected;

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_multistatements");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_multistatements(a char(10), b int)");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt,
                                   (SQLCHAR *) "INSERT INTO cs_multistatements VALUES(?, 10); INSERT INTO cs_multistatements VALUES('ten', ?)",
                                   SQL_NTS));

    for (i = 0; i < 5; ++i)
    {
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[i],
                                             sizeof(aParam[i]), &len[i]));
        CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[i],
                                             sizeof(bParam[i]), NULL));
        CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    }
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));

    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[0],
                                         sizeof(aParam[0]), &len[0]));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, SQL_NTS, 0, &bParam[0],
                                         sizeof(bParam[0]), NULL));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, SQL_NTS, 0, aParam[4],
                                         sizeof(aParam[4]), &len[4]));

    // Running these SELECTs multiple times just to verify that we release the results properly.
    OK_SIMPLE_STMT(Stmt,
                   "SELECT a, b FROM cs_multistatements WHERE a=? AND b=? ORDER BY a; SELECT a FROM cs_multistatements WHERE a=?");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt,
                                   (SQLCHAR *) "SELECT a, b FROM cs_multistatements WHERE a=? AND b=? ORDER BY a; SELECT a FROM cs_multistatements WHERE a=?",
                                   SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

    SQLCHAR aCol[10];
    SQLLEN aColLen;
    SQLINTEGER bCol;
    SQLLEN bColLen;

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_CHAR, aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_INTEGER, &bCol, sizeof(bCol), &bColLen));

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(bCol, bParam[0]);
    IS_STR(aCol, aParam[0], aColLen);
    FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA, "SQL_NO_DATA expected");

    CHECK_STMT_RC(Stmt, SQLMoreResults(Stmt));
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    IS_STR(aCol, aParam[4], aColLen);
    FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA, "SQL_NO_DATA expected");

    FAIL_IF(SQLMoreResults(Stmt) != SQL_NO_DATA, "SQL_NO_DATA expected");

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    return OK;
}


ODBC_TEST(client_side_ipd)
{
    int i;
    SQLCHAR aParam[3] = "1";
    SQLLEN alen = SQL_NTS;
    SQL_TIMESTAMP_STRUCT bParam = {0};
    memset(&bParam, 0, sizeof(bParam));
    bParam.hour = 17;
    bParam.minute = 26;
    bParam.second = 35;
    SQLLEN bLen = sizeof(SQL_TIMESTAMP_STRUCT);
    SQL_TIMESTAMP_STRUCT cParam = {2021, 1, 13};
    SQLLEN cLen = sizeof(SQL_TIMESTAMP_STRUCT);
    SQL_TIMESTAMP_STRUCT dParam = {2021, 1, 13, 17, 26, 35, 123456000};
    SQLLEN dLen = sizeof(SQL_TIMESTAMP_STRUCT);

    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_ipd");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE cs_ipd(a BIT(1), b time, c date, d datetime(6))");
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *) "INSERT INTO cs_ipd VALUES(?, ?, ?, ?)", SQL_NTS));

    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_CHAR, SQL_BIT, SQL_NTS, 0, aParam, sizeof(aParam), &alen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIME, SQL_NTS, 0, &bParam, sizeof(bParam), &bLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_TYPE_TIMESTAMP, SQL_TYPE_DATE, SQL_NTS, 0, &cParam, sizeof(cParam), &cLen));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 4, SQL_PARAM_INPUT, SQL_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_NTS, 0, &dParam, sizeof(dParam), &dLen));

    CHECK_STMT_RC(Stmt, SQLExecute(Stmt));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt,"SELECT a, b, c, d  FROM cs_ipd");

    SQLCHAR aCol;
    SQLLEN aColLen;
    SQL_TIME_STRUCT bCol;
    SQLLEN bColLen;
    SQL_DATE_STRUCT cCol;
    SQLLEN cColLen;
    SQL_TIMESTAMP_STRUCT dCol;
    SQLLEN dColLen;

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_BIT, &aCol, sizeof(aCol), &aColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_TYPE_TIME, &bCol, sizeof(bCol), &bColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_TYPE_DATE, &cCol, sizeof(cCol), &cColLen));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_TYPE_TIMESTAMP, &dCol, sizeof(dCol), &dColLen));

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(1, aCol);
    FAIL_IF(bCol.hour != bParam.hour || bCol.minute != bParam.minute || bCol.second != bParam.second, "wrong TIME returned");
    FAIL_IF(cCol.year != cParam.year || cCol.month != cParam.month || cCol.day != cParam.day, "wrong DATE returned");
    FAIL_IF(dCol.year != dParam.year || dCol.month != dParam.month || dCol.day != dParam.day ||
            dCol.hour != dParam.hour || dCol.minute != dParam.minute || dCol.second != dParam.second ||
            dCol.fraction != dParam.fraction, "wrong TIMESTAMP returned");

    FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA, "SQL_NO_DATA expected");

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS cs_ipd");

    return OK;
}


MA_ODBC_TESTS my_tests[] =
{
    {client_side_show,                      "client_side_show", CSPS_OK | SSPS_FAIL, ALL_DRIVERS},
    {client_side_prepare,                   "client_side_prepare", NORMAL, ALL_DRIVERS},
    {client_side_static_cursor,             "client_side_static_cursor", NORMAL, ALL_DRIVERS},
    {client_side_dynamic_cursor,            "client_side_dynamic_cursor", NORMAL, ALL_DRIVERS},
    {client_side_forward_only_cursor,       "client_side_forward_only_cursor", NORMAL, ALL_DRIVERS},
    {client_side_multiparam,                "client_side_multiparam", NORMAL, ALL_DRIVERS},
    {client_side_multirow,                  "client_side_multirow", NORMAL, ALL_DRIVERS},
    {client_side_multirow_columnwise,       "client_side_multirow_columnwise", NORMAL, ALL_DRIVERS},
    {client_side_get_data,                  "client_side_get_data", NORMAL, ALL_DRIVERS},
    {client_side_get_data_buffered,         "client_side_get_data_buffered", NORMAL, ALL_DRIVERS},
    {client_side_get_data_many_types,       "client_side_get_data_many_types", NORMAL, ALL_DRIVERS},
    {client_side_put_data,                  "client_side_put_data", CSPS_OK | SSPS_FAIL, ALL_DRIVERS},
    {client_side_put_data_non_char,         "client_side_put_data_non_char", CSPS_OK | SSPS_FAIL, ALL_DRIVERS},
    {client_side_put_data_multiple,         "client_side_put_data_multiple", NORMAL, ALL_DRIVERS}, // not yet supported
    {client_side_bulk_add,                  "client_side_bulk_add", NORMAL, ALL_DRIVERS},
    {client_side_set_pos_del,               "client_side_set_pos_del", NORMAL, ALL_DRIVERS},
    {client_side_set_pos_del_multiple_rows, "client_side_set_pos_del_multiple_rows", NORMAL, ALL_DRIVERS},
    {client_side_multistatements,           "client_side_multistatements", NORMAL, ALL_DRIVERS},
    {client_side_ipd, "client_side_ipd", NORMAL, ALL_DRIVERS},
    {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
    int tests = sizeof(my_tests) / sizeof(MA_ODBC_TESTS) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}
