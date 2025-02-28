/*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
                2013, 2016 MariaDB Corporation AB
                2021 SingleStore, Inc.

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "tap.h"

ODBC_TEST(my_columns_null)
{
  SQLLEN rowCount= 0;
  /* initialize data */
  OK_SIMPLE_STMT(Stmt, "drop table if exists my_column_null");

  OK_SIMPLE_STMT(Stmt, "create table my_column_null(id int not null, name varchar(30))");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, SQL_NTS, NULL, SQL_NTS,
    (SQLCHAR *)"my_column_null", SQL_NTS,
    NULL, SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowCount));

  is_num(rowCount, 2);

  is_num(2, my_print_non_format_result(Stmt));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS my_column_null");

  return OK;
}


ODBC_TEST(my_drop_table)
{
  OK_SIMPLE_STMT(Stmt, "drop table if exists my_drop_table");
  OK_SIMPLE_STMT(Stmt, "create table my_drop_table(id int not null)");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0,
                            (SQLCHAR *)"my_drop_table", SQL_NTS, NULL, 0));

  is_num(1, my_print_non_format_result(Stmt));

  OK_SIMPLE_STMT(Stmt, "drop table my_drop_table");

  return OK;
}


#define TODBC_BIND_CHAR(n,buf) SQLBindCol(Stmt,n,SQL_C_CHAR,&buf,sizeof(buf),NULL);


ODBC_TEST(my_table_dbs)
{
  SQLCHAR    database[100];
  SQLRETURN  rc;
  SQLINTEGER nrows= 0 ;
  SQLLEN lenOrNull, rowCount= 0;
    
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS my_all_db_test1");
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS my_all_db_test2");
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS my_all_db_test3");
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS my_all_db_test4");

  /* This call caused problems when database names returned as '%' */
  CHECK_STMT_RC(Stmt, SQLTables(Stmt,(SQLCHAR*)SQL_ALL_CATALOGS, 1, NULL, 0, NULL, 0, NULL, 0));

  while (SQLFetch(Stmt) == SQL_SUCCESS)
  {
    ++nrows;
    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_CHAR, database,
                              sizeof(database), NULL));
    /* the table catalog in the results must not be '%' */
    FAIL_IF(database[0] == '%', "table catalog can't be '%'");
  }
  /* we should have got rows... */
  FAIL_IF(nrows == 0, "nrows should be > 0");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLTables(Stmt,(SQLCHAR *)SQL_ALL_CATALOGS, 1, "", 0, "", 0, NULL, 0));

  /* Added calls to SQLRowCount just to have tests of it with SQLTables. */
  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowCount));
  nrows = my_print_non_format_result(Stmt);

  is_num(rowCount, nrows);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLTables(Stmt,(SQLCHAR *)SQL_ALL_CATALOGS, SQL_NTS, "", 0, "", 0,
                  NULL, 0));

  is_num(nrows, my_print_non_format_result(Stmt));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLTables(Stmt,(SQLCHAR *)my_schema, (SQLSMALLINT)strlen(my_schema), NULL, 0, NULL, 0, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowCount));
  is_num(rowCount, my_print_non_format_result(Stmt));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLTables(Stmt,(SQLCHAR *)"information_schema", SQL_NTS, NULL, 0, NULL, 0, NULL, 0));

  FAIL_IF(my_print_non_format_result(Stmt) == 0, "0 tables returned for information_schema");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLTables(Stmt, (SQLCHAR *)"%", 1, "", 0, "", 0, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  memset(database,0,sizeof(database));
  rc = SQLGetData(Stmt,1,SQL_C_CHAR,database, (SQLLEN)sizeof(database),NULL);
  CHECK_STMT_RC(Stmt,rc);
  diag("catalog: %s", database);

  memset(database,0,sizeof(database));
  rc = SQLGetData(Stmt,2,SQL_C_CHAR,database, (SQLLEN)sizeof(database),&lenOrNull);
  CHECK_STMT_RC(Stmt,rc);
  diag("schema: %s", database);
  IS(lenOrNull == SQL_NULL_DATA);

  memset(database,0,sizeof(database));
  rc = SQLGetData(Stmt,3,SQL_C_CHAR,database, (SQLLEN)sizeof(database),&lenOrNull);
  CHECK_STMT_RC(Stmt,rc);
  diag("table: %s", database);

  memset(database,0,sizeof(database));
  rc = SQLGetData(Stmt,4,SQL_C_CHAR,database, (SQLLEN)sizeof(database),&lenOrNull);
  CHECK_STMT_RC(Stmt,rc);
  diag("type: %s", database);

  memset(database,0,sizeof(database));
  rc = SQLGetData(Stmt,5,SQL_C_CHAR, database, (SQLLEN)sizeof(database),&lenOrNull);
  CHECK_STMT_RC(Stmt,rc);
  diag("database remark: %s", database);

  SQLFreeStmt(Stmt,SQL_UNBIND);
  SQLFreeStmt(Stmt,SQL_CLOSE);

  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE my_all_db_test1");
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE my_all_db_test2");
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE my_all_db_test3");
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE my_all_db_test4");

  rc = SQLTables(Stmt, (SQLCHAR *)SQL_ALL_CATALOGS, 1, "", 0, "", 0, "", 0);
  CHECK_STMT_RC(Stmt,rc);

  nrows += 4;
  is_num(nrows, my_print_non_format_result(Stmt));
  rc = SQLFreeStmt(Stmt, SQL_CLOSE);
  CHECK_STMT_RC(Stmt,rc);

  rc = SQLTables(Stmt,(SQLCHAR *)SQL_ALL_CATALOGS, SQL_NTS,
                  "", 0, "", 0, "", 0);
  CHECK_STMT_RC(Stmt,rc);

  is_num(my_print_non_format_result(Stmt), nrows);
  rc = SQLFreeStmt(Stmt, SQL_CLOSE);
  CHECK_STMT_RC(Stmt,rc);

  rc = SQLTables(Stmt, (SQLCHAR *)"my_all_db_test", SQL_NTS,
                  "", 0, "", 0, "", 0);
  CHECK_STMT_RC(Stmt,rc);

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowCount));
  is_num(rowCount, 0);

  is_num(my_print_non_format_result(Stmt), 0);
  rc = SQLFreeStmt(Stmt, SQL_CLOSE);
  CHECK_STMT_RC(Stmt,rc);

  rc = SQLTables(Stmt, (SQLCHAR *)"my_all_db_test%", SQL_NTS,
                  "", 0, "", 0, NULL, 0);
  CHECK_STMT_RC(Stmt,rc);
  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowCount));
  is_num(my_print_non_format_result(Stmt), 0); /* MySQL's driver returns 4 rows for matching DB's here.
                                                  But I don't think it's in accordance with specs. And MSSQL driver
                                                  does not do that */
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* unknown table should be empty */
  rc = SQLTables(Stmt, (SQLCHAR *)"my_all_db_test%", SQL_NTS,
                  NULL, 0, (SQLCHAR *)"xyz", SQL_NTS, NULL, 0);
  CHECK_STMT_RC(Stmt,rc);

  is_num(my_print_non_format_result(Stmt), 0);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE my_all_db_test1");
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE my_all_db_test2");
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE my_all_db_test3");
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE my_all_db_test4");

  return OK;
}


ODBC_TEST(my_colpriv)
{
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS test_colprev1");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE test_colprev1(a INT)");

  // column_privileges table is not implemented on the engine side, so we expect an empty set here.
  EXPECT_STMT(Stmt, SQLColumnPrivileges(Stmt,
                                     NULL, SQL_NTS, NULL, SQL_NTS,
                                     (SQLCHAR *)"test_colprev1", SQL_NTS,
                                     (SQLCHAR *)/*NULL*/"%", SQL_NTS), SQL_SUCCESS_WITH_INFO);
  CHECK_SQLSTATE(Stmt, "01000");
  SQLLEN rowCount;
  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowCount));
  is_num(rowCount, 0);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE test_colprev1");

  return OK;
}


ODBC_TEST(t_sqlprocedures)
{
  OK_SIMPLE_STMT(Stmt, "DROP FUNCTION IF EXISTS t_sqlproc_func");
  OK_SIMPLE_STMT(Stmt,
         "CREATE FUNCTION t_sqlproc_func (a INT) RETURNS INT AS BEGIN RETURN SQRT(a); END");

  OK_SIMPLE_STMT(Stmt, "DROP PROCEDURE IF EXISTS t_sqlproc_proc");
  OK_SIMPLE_STMT(Stmt,
         "CREATE PROCEDURE t_sqlproc_proc() AS BEGIN"
         " ECHO SELECT COUNT(*) AS a FROM t_sqlproc;"
         "END;");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* Try without specifying a catalog. */
  CHECK_STMT_RC(Stmt, SQLProcedures(Stmt, NULL, 0, NULL, 0,
                               (SQLCHAR *)"t_sqlproc%", SQL_NTS));

  is_num(my_print_non_format_result(Stmt), 2);

  /* And try with specifying a catalog.  */
  CHECK_STMT_RC(Stmt, SQLProcedures(Stmt, (SQLCHAR *)my_schema, SQL_NTS, NULL, 0,
                               (SQLCHAR *)"t_sqlproc%", SQL_NTS));

  is_num(my_print_non_format_result(Stmt), 2);

  OK_SIMPLE_STMT(Stmt, "DROP PROCEDURE t_sqlproc_proc");
  OK_SIMPLE_STMT(Stmt, "DROP FUNCTION t_sqlproc_func");

  return OK;
}

#define MYSQL_NAME_LEN 192

ODBC_TEST(t_catalog)
{
    SQLRETURN rc;
    SQLCHAR      name[MYSQL_NAME_LEN+1];
    SQLUSMALLINT i;
    SQLSMALLINT  ncols, len;

    SQLCHAR colnames[19][20]= {
        "TABLE_CAT","TABLE_SCHEM","TABLE_NAME","COLUMN_NAME",
        "DATA_TYPE","TYPE_NAME","COLUMN_SIZE","BUFFER_LENGTH",
        "DECIMAL_DIGITS","NUM_PREC_RADIX","NULLABLE","REMARKS",
        "COLUMN_DEF","SQL_DATA_TYPE","SQL_DATETIME_SUB",
        "CHAR_OCTET_LENGTH","ORDINAL_POSITION","IS_NULLABLE"
    };
    SQLSMALLINT collengths[18]= {
        9,11,10,11,9,9,11,13,14,14,8,7,10,13,16,17,16,11
    };

    OK_SIMPLE_STMT(Stmt, "drop table if exists t_catalog");

    OK_SIMPLE_STMT(Stmt,"create table t_catalog(abc tinyint, bcdefghijklmno char(4), uifield int unsigned not null)");

    CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0,
                              (SQLCHAR *)"t_catalog", 9, NULL, 0));

    rc = SQLNumResultCols(Stmt, &ncols);
    CHECK_STMT_RC(Stmt,rc);

    diag("total columns: %d", ncols);
    IS(ncols == 18);
    IS(myrowcount(Stmt) == 3);

    SQLFreeStmt(Stmt, SQL_UNBIND);
    SQLFreeStmt(Stmt, SQL_CLOSE);

    rc = SQLColumns(Stmt, NULL, 0, NULL, 0,
                    (SQLCHAR *)"t_catalog", 9, NULL, 0);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLNumResultCols(Stmt,&ncols);
    CHECK_STMT_RC(Stmt,rc);

    for (i= 1; i <= (SQLUINTEGER) ncols; i++)
    {
        rc = SQLDescribeCol(Stmt, i, name, MYSQL_NAME_LEN+1, &len, NULL, NULL, NULL, NULL);
        CHECK_STMT_RC(Stmt,rc);

        diag("column %d: %s (%d)", i, name, len);
        is_num(len, collengths[i - 1]);
        IS_STR(name, colnames[i - 1], len);
    }
    SQLFreeStmt(Stmt,SQL_CLOSE);

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_catalog");

  return OK;
}


ODBC_TEST(tmysql_specialcols)
{
  OK_SIMPLE_STMT(Stmt,"drop table if exists tmysql_specialcols");
  OK_SIMPLE_STMT(Stmt,ServerNotOlderThan(Connection, 7, 3, 0) ? "create rowstore table tmysql_specialcols(col1 int primary key, col2 varchar(30), col3 int)" :
                                                               "create table tmysql_specialcols(col1 int primary key, col2 varchar(30), col3 int)");
    

  OK_SIMPLE_STMT(Stmt,"create index tmysql_ind1 on tmysql_specialcols(col1)");
    
  OK_SIMPLE_STMT(Stmt,"insert into tmysql_specialcols values(100,'venu',1)");

  OK_SIMPLE_STMT(Stmt,"insert into tmysql_specialcols values(200,'MySQL',2)");
    
  CHECK_DBC_RC(Connection, SQLTransact(NULL,Connection,SQL_COMMIT));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt,SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt,"select * from tmysql_specialcols");

  myrowcount(Stmt);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt,SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSpecialColumns(Stmt, SQL_BEST_ROWID, NULL,0, NULL,0,
                                   (SQLCHAR *)"tmysql_specialcols",SQL_NTS,
                                   SQL_SCOPE_SESSION, SQL_NULLABLE));

  myrowcount(Stmt);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt,SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt,"drop table tmysql_specialcols");

  CHECK_DBC_RC(Connection, SQLTransact(NULL,Connection,SQL_COMMIT));

  return OK;
}


/* To test SQLColumns misc case */
ODBC_TEST(t_columns)
{
  SQLSMALLINT   NumPrecRadix, DataType, Nullable, DecimalDigits;
  SQLLEN        cbColumnSize, cbDecimalDigits, cbNumPrecRadix,
                cbDataType, cbNullable;
  SQLINTEGER    cbDatabaseName;
  SQLUINTEGER   ColumnSize, i;
  SQLUINTEGER   ColumnCount= 7;
  SQLCHAR       ColumnName[MAX_NAME_LEN], DatabaseName[MAX_NAME_LEN];
  SQLINTEGER    Values[7][5][2]=
  {
    { {5,2},  {5,4}, {0,2},  {10,2},  {1,2}},
    { {SQL_WCHAR, 2},  {5,4},  {0,-1}, {10,-1}, {1,2}},
    { {SQL_WVARCHAR,2}, {20,4}, {0,-1}, {10,-1}, {0,2}},
    { {3,2},  {10,4}, {2,2},  {10,2},  {1,2}},
    { {-6,2},  {3,4}, {0,2},  {10,2},  {0,2}},
    { {4,2}, {10,4}, {0,2},  {10,2},  {0,2}},
    { {-6,2}, {3,4}, {0,2},  {10,2},  {0,2}}
  };

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_columns");

  OK_SIMPLE_STMT(Stmt,
         "CREATE TABLE t_columns (col0 SMALLINT,"
         "col1 CHAR(5), col2 VARCHAR(20) NOT NULL, col3 DECIMAL(10,2),"
         "col4 TINYINT NOT NULL, col5 INTEGER,"
         "col6 TINYINT NOT NULL, PRIMARY KEY(col5, col6)) CHARSET latin1");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_METADATA_ID,
                                (SQLPOINTER)SQL_FALSE, SQL_IS_UINTEGER));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  CHECK_DBC_RC(Connection, SQLGetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG,
                                 DatabaseName, MAX_NAME_LEN,
                                 &cbDatabaseName)); /* Current Catalog */

  for (i= 0; i < ColumnCount; i++)
  {
    sprintf((char *)ColumnName, "col%d", (int)i);
    diag("checking column `%s`", (char *)ColumnName);

    CHECK_STMT_RC(Stmt, SQLColumns(Stmt,
                              DatabaseName, (SQLSMALLINT)cbDatabaseName,
                              (SQLCHAR *)"", SQL_NTS,
                              (SQLCHAR *)"t_columns", SQL_NTS,
                              ColumnName, SQL_NTS));

    /* 5 -- Data type */
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_SSHORT, &DataType, 0,
                              &cbDataType));

    /* 7 -- Column Size */
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 7, SQL_C_ULONG, &ColumnSize, 0,
                              &cbColumnSize));

    /* 9 -- Decimal Digits */
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 9, SQL_C_SSHORT, &DecimalDigits, 0,
                              &cbDecimalDigits));

    /* 10 -- Num Prec Radix */
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 10, SQL_C_SSHORT, &NumPrecRadix, 0,
                              &cbNumPrecRadix));

    /* 11 -- Nullable */
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 11, SQL_C_SSHORT, &Nullable, 0,
                              &cbNullable));

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    is_num(DataType, GetDefaultCharType(Values[i][0][0], TRUE));
    is_num(cbDataType, Values[i][0][1]);

    is_num(ColumnSize,   Values[i][1][0]);
    is_num(cbColumnSize, Values[i][1][1]);

    is_num(DecimalDigits,   Values[i][2][0]);
    is_num(cbDecimalDigits, Values[i][2][1]);

    is_num(NumPrecRadix,   Values[i][3][0]);
    is_num(cbNumPrecRadix, Values[i][3][1]);

    is_num(Nullable,   Values[i][4][0]);
    is_num(cbNullable, Values[i][4][1]);

    FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  }

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_columns");

  return OK;
}


/* Test the bug SQLTables */
typedef struct t_table_bug
{
  SQLCHAR     szColName[MAX_NAME_LEN];
  SQLSMALLINT pcbColName;
  SQLSMALLINT pfSqlType;
  SQLUINTEGER pcbColDef;
  SQLSMALLINT pibScale;
  SQLSMALLINT pfNullable;
} t_describe_col;


t_describe_col t_tables_bug_data[10] =
{
  /* For "Unicode" connection */
  {"TABLE_CAT",   9, SQL_WVARCHAR, MYSQL_NAME_LEN, 0, SQL_NO_NULLS},
  {"TABLE_SCHEM",11, SQL_WVARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"TABLE_NAME", 10, SQL_WVARCHAR, MYSQL_NAME_LEN, 0, SQL_NO_NULLS},
  {"TABLE_TYPE", 10, SQL_WVARCHAR, MYSQL_NAME_LEN, 0, SQL_NO_NULLS},
  {"REMARKS",     7, SQL_WVARCHAR, MYSQL_NAME_LEN, 0, SQL_NO_NULLS},
  /* For "ANSI" connection */
  {"TABLE_CAT",   9, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NO_NULLS},
  {"TABLE_SCHEM",11, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NULLABLE},
  {"TABLE_NAME", 10, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NO_NULLS},
  {"TABLE_TYPE", 10, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NO_NULLS},
  {"REMARKS",     7, SQL_VARCHAR, MYSQL_NAME_LEN, 0, SQL_NO_NULLS}
};


ODBC_TEST(t_tables_bug)
{
  SQLSMALLINT i, ColumnCount, pcbColName, pfSqlType, pibScale, pfNullable;
  SQLULEN     pcbColDef;
  SQLCHAR     szColName[MAX_NAME_LEN];
  const int   RefArrOffset= is_unicode_driver() ? -1 : 4; /* 4 for "ANSI" connection, which is default atm, and -1 for "Unicode" */

  CHECK_STMT_RC(Stmt,  SQLTables(Stmt, NULL, 0, NULL, 0, NULL, 0,
                            (SQLCHAR *)"TABLE", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLNumResultCols(Stmt, &ColumnCount));
  is_num(ColumnCount, 5);

  for (i= 1; i <= ColumnCount; ++i)
  {
    CHECK_STMT_RC(Stmt, SQLDescribeCol(Stmt, (SQLUSMALLINT)i, szColName,
                                 MAX_NAME_LEN, &pcbColName, &pfSqlType,
                                 &pcbColDef, &pibScale, &pfNullable));

    fprintf(stdout, "# Column '%d':\n", i);
    fprintf(stdout, "#  Column Name   : %s\n", szColName);
    fprintf(stdout, "#  NameLengh     : %d\n", pcbColName);
    fprintf(stdout, "#  DataType      : %d\n", pfSqlType);
    fprintf(stdout, "#  ColumnSize    : %lu\n", (unsigned long)pcbColDef);
    fprintf(stdout, "#  DecimalDigits : %d\n", pibScale);
    fprintf(stdout, "#  Nullable      : %d\n", pfNullable);

    IS_STR(t_tables_bug_data[i + RefArrOffset].szColName, szColName, pcbColName);
    is_num(t_tables_bug_data[i + RefArrOffset].pcbColName, pcbColName);
    is_num(t_tables_bug_data[i + RefArrOffset].pfSqlType, pfSqlType);
    /* This depends on NAME_LEN in mysql_com.h */

    is_num(t_tables_bug_data[i + RefArrOffset].pibScale, pibScale);
    is_num(t_tables_bug_data[i + RefArrOffset].pfNullable, pfNullable);
  }

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}


ODBC_TEST(t_current_catalog_unicode)
{
  SQLWCHAR    db[255];
  SQLWCHAR    cur_db[255];
  SQLRETURN   rc;
  SQLINTEGER  len;

  rc = SQLFreeStmt(Stmt,SQL_CLOSE);
  CHECK_STMT_RC(Stmt,rc);

  rc = SQLGetConnectAttrW(Connection, SQL_ATTR_CURRENT_CATALOG, db, sizeof(db), &len);
  CHECK_DBC_RC(Connection,rc);

  is_num(len, strlen(my_schema) * sizeof(SQLWCHAR));
  IS_WSTR(db, wschema, 5);

  rc = SQLSetConnectAttrW(Connection, SQL_ATTR_CURRENT_CATALOG, db, SQL_NTS);
  CHECK_DBC_RC(Connection,rc);

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS test_odbc_current");

  latin_as_sqlwchar((char*)"test_odbc_current", cur_db);
  rc = SQLSetConnectAttrW(Connection, SQL_ATTR_CURRENT_CATALOG, cur_db, SQL_NTS);
  FAIL_IF(rc == SQL_SUCCESS, "Error expected");

  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE test_odbc_current");
  rc = SQLFreeStmt(Stmt,SQL_CLOSE);

  latin_as_sqlwchar((char*)"test_odbc_current", cur_db);
  rc = SQLSetConnectAttrW(Connection, SQL_ATTR_CURRENT_CATALOG, cur_db, SQL_NTS);
  CHECK_DBC_RC(Connection,rc);

  rc = SQLGetConnectAttrW(Connection, SQL_ATTR_CURRENT_CATALOG, db, 255, &len);
  CHECK_DBC_RC(Connection,rc);

  is_num(len, strlen("test_odbc_current") * sizeof(SQLWCHAR));
  IS_WSTR(db, cur_db, 18);

  /* reset for further tests */
  rc = SQLSetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, my_schema, SQL_NTS);
  CHECK_DBC_RC(Connection,rc);

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE test_odbc_current");

  return OK;
}


ODBC_TEST(t_current_catalog_ansi)
{
  SQLCHAR     cur_db[255], db[255];
  SQLRETURN   rc;
  SQLINTEGER len;

  rc = SQLFreeStmt(Stmt,SQL_CLOSE);
  CHECK_STMT_RC(Stmt,rc);

  rc = SQLGetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, db, sizeof(db), &len);
  CHECK_DBC_RC(Connection,rc);

  is_num(len, strlen(my_schema));
  IS_STR(db, my_schema, len);

  rc = SQLSetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, db, SQL_NTS);
  CHECK_DBC_RC(Connection,rc);

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS test_odbc_current");

  strcpy((char *)cur_db, "test_odbc_current");
  rc = SQLSetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, cur_db, SQL_NTS);
  FAIL_IF(rc == SQL_SUCCESS, "Error expected");

  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE test_odbc_current");
  rc = SQLFreeStmt(Stmt,SQL_CLOSE);

  strcpy((char *)cur_db, "test_odbc_current");
  rc = SQLSetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, cur_db, SQL_NTS);
  CHECK_DBC_RC(Connection,rc);

  rc = SQLGetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, db, 255, &len);
  CHECK_DBC_RC(Connection,rc);

  is_num(len, 17);
  IS_STR(db, cur_db, 18);



  /* reset for further tests */
  rc = SQLSetConnectAttr(Connection, SQL_ATTR_CURRENT_CATALOG, (SQLCHAR *)my_schema, SQL_NTS);
  CHECK_DBC_RC(Connection,rc);

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE test_odbc_current");

  return OK;
}


ODBC_TEST(tmysql_showkeys)
{
  SQLRETURN rc;

  OK_SIMPLE_STMT(Stmt,"drop table if exists tmysql_spk");

  OK_SIMPLE_STMT(Stmt, ServerNotOlderThan(Connection, 7, 3, 0) ? "create rowstore table tmysql_spk(col1 int primary key)" :
                                                               "create table tmysql_spk(col1 int primary key)");
   
  CHECK_DBC_RC(Connection, SQLTransact(NULL,Connection,SQL_COMMIT));
    
  rc = SQLFreeStmt(Stmt,SQL_CLOSE);
  CHECK_STMT_RC(Stmt,rc);

  OK_SIMPLE_STMT(Stmt,"SHOW KEYS FROM tmysql_spk");
    

  IS(1 == myrowcount(Stmt));

  rc = SQLFreeStmt(Stmt,SQL_CLOSE);
  CHECK_STMT_RC(Stmt,rc);

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS tmysql_spk");

  return OK;
}


ODBC_TEST(t_sqltables)
{
  SQLLEN      rowCount, LenInd;
  SQLHDBC     hdbc1;
  SQLHSTMT    Stmt1;
  int         AllTablesCount= 0, Rows;
  SQLCHAR     Buffer[64];

  if (!SQL_SUCCEEDED(SQLExecDirect(Stmt, "DROP SCHEMA IF EXISTS mariadbodbc_sqltables", SQL_NTS)))
  {
    odbc_print_error(SQL_HANDLE_STMT, Stmt);
  }

  if (!SQL_SUCCEEDED(SQLExecDirect(Stmt, "CREATE SCHEMA mariadbodbc_sqltables", SQL_NTS)))
  {
    odbc_print_error(SQL_HANDLE_STMT, Stmt);
    skip("Test user has no rights to drop/create databases");
  }

  IS(AllocEnvConn(&Env, &hdbc1));
  Stmt1= DoConnect(hdbc1, FALSE, NULL, NULL, NULL, 0, "mariadbodbc_sqltables", 0, NULL, NULL);
  FAIL_IF(Stmt1 == NULL, "");

  OK_SIMPLE_STMT(Stmt1, "CREATE TABLE t1 (a int)");
  OK_SIMPLE_STMT(Stmt1, "CREATE TABLE t2 (a int)");
  OK_SIMPLE_STMT(Stmt1, "CREATE TABLE t3 (a int)");

  CHECK_STMT_RC(Stmt1, SQLTables(Stmt1, NULL, 0, NULL, 0, NULL, 0, NULL, 0));

  AllTablesCount= myrowcount(Stmt1);
  FAIL_IF(AllTablesCount != 3, "There should be 3 tables"); /* 3 tables in current db */

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  CHECK_STMT_RC(Stmt1, SQLTables(Stmt1, (SQLCHAR *)"%", SQL_NTS, NULL, 0, NULL, 0, NULL, 0));

  Rows= 0;
  while (SQLFetch(Stmt1) != SQL_NO_DATA_FOUND)
  {
    ++Rows;
    CHECK_STMT_RC(Stmt1, SQLGetData(Stmt1, 3, SQL_C_CHAR, Buffer, sizeof(Buffer), &LenInd));
    FAIL_IF(LenInd == SQL_NULL_DATA, "Table Name should not be NULL")
  }
  FAIL_IF(Rows <= AllTablesCount, "% catalog should give more results than NULL catalog");

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  CHECK_STMT_RC(Stmt1, SQLTables(Stmt1, NULL, 0, NULL, 0, NULL, 0,
                                (SQLCHAR *)"system table", SQL_NTS));

  is_num(myrowcount(Stmt1), 0);

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  CHECK_STMT_RC(Stmt1, SQLTables(Stmt1, NULL, 0, NULL, 0, NULL, 0,
                               (SQLCHAR *)"TABLE", SQL_NTS));
  Rows= my_print_non_format_result_ex(Stmt1, FALSE);
  CHECK_STMT_RC(Stmt1, SQLRowCount(Stmt1, &rowCount));

  is_num(Rows, rowCount);
  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  CHECK_STMT_RC(Stmt1, SQLTables(Stmt1, (SQLCHAR *)"TEST", SQL_NTS,
                 (SQLCHAR *)"TEST", SQL_NTS, NULL, 0,
                 (SQLCHAR *)"TABLE", SQL_NTS));

  is_num(my_print_non_format_result(Stmt1), 0);

  CHECK_STMT_RC(Stmt1, SQLTables(Stmt1, NULL, 0, (SQLCHAR *)"%", SQL_NTS, NULL, 0, NULL, 0));

  diag("all schemas %d", myrowcount(Stmt1));

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  /* List of table types - table, view, system view */
  CHECK_STMT_RC(Stmt1, SQLTables(Stmt1, "", 0, "", 0, "", 0, (SQLCHAR *)"%", SQL_NTS));

  is_num(my_print_non_format_result(Stmt1), 3);
  /* my_print_non_format_result closes cursor by default */

  OK_SIMPLE_STMT(Stmt, "DROP SCHEMA mariadbodbc_sqltables");

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_DROP));
  CHECK_DBC_RC(hdbc1, SQLDisconnect(hdbc1));
  CHECK_DBC_RC(hdbc1, SQLFreeConnect(hdbc1));

  return OK;
}


ODBC_TEST(my_information_schema)
{
  SQLCHAR   conn[512], conn_out[512];
  SQLHDBC Connection1;
  SQLHSTMT Stmt1;
  SQLSMALLINT conn_out_len;
  SQLRETURN rc;

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS istest__");
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS istest_1");
  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS istest_2");

  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE istest__");
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE istest_1");
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE istest_2");

  OK_SIMPLE_STMT(Stmt, "CREATE TABLE istest__.istab_(a INT,b INT,c INT, d INT)");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE istest_1.istab1(a INT,b INT,c INT, d INT)");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE istest_2.istab2(a INT,b INT,c INT, d INT)");

  /* We need to have istest__ as the default DB */
  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s;DATABASE=istest__;OPTION=0", my_dsn, my_uid, my_pwd);
  
  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &Connection1));

  CHECK_DBC_RC(Connection1, SQLDriverConnect(Connection1, NULL, conn, sizeof(conn), conn_out,
                                 sizeof(conn_out), &conn_out_len,
                                 SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(Connection1, SQLAllocStmt(Connection1, &Stmt1));

  rc = SQLTables(Stmt1, "istest__", SQL_NTS, NULL, 0, "istab%", SQL_NTS, NULL, 0);
  CHECK_STMT_RC(Stmt1,rc);

  /* all tables from all databases should be displayed */
  is_num(my_print_non_format_result(Stmt1), 3);
  rc = SQLFreeStmt(Stmt1, SQL_CLOSE);

  rc = SQLTables(Stmt1, "istest\\_\\_", SQL_NTS, NULL, 0, "istab%", SQL_NTS, NULL, 0);
  CHECK_STMT_RC(Stmt1,rc);

  is_num(my_print_non_format_result(Stmt1), 1);
  rc = SQLFreeStmt(Stmt1, SQL_CLOSE);
  CHECK_STMT_RC(Stmt1,rc);

  CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
  CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));

  return OK;
}

/**
 Tests the non-error code paths in catalog.c that return an empty set to
 make sure the resulting empty result sets at least indicate the right
 number of columns.
*/
ODBC_TEST(empty_set)
{
  SQLSMALLINT columns;

  /* SQLTables(): no known table types. */
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"UNKNOWN", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLNumResultCols(Stmt, &columns));
  is_num(columns, 5);
  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* SQLTables(): no tables found. */
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"no_such_table", SQL_NTS, NULL, SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLNumResultCols(Stmt, &columns));
  is_num(columns, 5);
  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* SQLTables(): empty catalog with existing table */
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_sqltables_empty");
  OK_SIMPLE_STMT(Stmt, "create table t_sqltables_empty (x int)");
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, SQL_NTS, NULL, 0,
			   (SQLCHAR *) "t_sqltables_empty", SQL_NTS,
			   NULL, SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLNumResultCols(Stmt, &columns));
  is_num(columns, 5);
  FAIL_IF(SQLFetch(Stmt) == SQL_NO_DATA_FOUND, "expected data");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_sqltables_empty");

  return OK;
}


/**
 Bug #23031: SQLTables returns inaccurate catalog information on views
*/
ODBC_TEST(t_bug23031)
{
  SQLCHAR buff[255];

  OK_SIMPLE_STMT(Stmt, "DROP VIEW IF EXISTS t_bug23031_v");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug23031_t");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_bug23031_t (a INT) COMMENT 'Whee!'");
  OK_SIMPLE_STMT(Stmt, "CREATE VIEW t_bug23031_v AS SELECT * FROM t_bug23031_t");

  /* Get both the table and view. */
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"t_bug23031%", SQL_NTS, NULL, SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, buff, 3), "t_bug23031_t", 12);
  IS_STR(my_fetch_str(Stmt, buff, 4), "TABLE", 10);
  IS_STR(my_fetch_str(Stmt, buff, 5), "Whee!", 5);

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, buff, 3), "t_bug23031_v", 12);
  IS_STR(my_fetch_str(Stmt, buff, 4), "VIEW", 4);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* Get just the table. */
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"t_bug23031%", SQL_NTS,
                           (SQLCHAR *)"TABLE", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, buff, 3), "t_bug23031_t", 12);
  IS_STR(my_fetch_str(Stmt, buff, 4), "TABLE", 10);
  IS_STR(my_fetch_str(Stmt, buff, 5), "Whee!", 5);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* Get just the view. */
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, SQL_NTS, NULL, SQL_NTS,
                           (SQLCHAR *)"t_bug23031%", SQL_NTS,
                           (SQLCHAR *)"VIEW", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, buff, 3), "t_bug23031_v", 12);
  IS_STR(my_fetch_str(Stmt, buff, 4), "VIEW", 4);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP VIEW IF EXISTS t_bug23031_v");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug23031_t");

  return OK;
}


/**
 Bug #15713: null pointer when use the table qualifier in SQLColumns()
*/
ODBC_TEST(bug15713)
{
  SQLHDBC Connection1;
  SQLHSTMT Stmt1;
  SQLCHAR   conn[512], conn_out[512];
  SQLSMALLINT conn_out_len;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug15713");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_bug15713 (a INT)");

  /* The connection strings must not include DATABASE. */
  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s", my_dsn, my_uid, my_pwd);
 
  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &Connection1));

  CHECK_DBC_RC(Connection1, SQLDriverConnect(Connection1, NULL, conn, sizeof(conn), conn_out,
                                 sizeof(conn_out), &conn_out_len,
                                 SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(Connection1, SQLAllocStmt(Connection1, &Stmt1));

  CHECK_STMT_RC(Stmt1, SQLColumns(Stmt1, (SQLCHAR *)my_schema, SQL_NTS,
                             NULL, 0, (SQLCHAR *)"t_bug15713", SQL_NTS,
                             NULL, 0));

  CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));
  IS_STR(my_fetch_str(Stmt1, conn, 1), my_schema, strlen(my_schema));
  IS_STR(my_fetch_str(Stmt1, conn, 3), "t_bug15713", 10);
  IS_STR(my_fetch_str(Stmt1, conn, 4), "a", 1);

  FAIL_IF(SQLFetch(Stmt1) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_DROP));
  CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
  CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug15713");
  return OK;
}


/**
 Bug #28316: Fatal crash with Crystal Reports and MySQL Server
*/
ODBC_TEST(t_bug28316)
{
  CHECK_STMT_RC(Stmt, SQLProcedures(Stmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS));

  return OK;
}


/**
 Bug #8860: Generic SQLColumns not supported?
*/
ODBC_TEST(bug8860)
{
  SQLCHAR buff[512];

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug8860");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS `t_bug8860_a'b`");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_bug8860 (a INT)");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE `t_bug8860_a'b` (b INT)");

  /*
   Specifying nothing gets us columns from all of the tables in the
   current database.
  */
  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0));

  /* We should have at least two rows. There may be more. */
  FAIL_IF(myrowcount(Stmt) < 2, "expected min. 2 rows");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  if (!using_dm(Connection))
  {
    /* Specifying "" as the table name gets us nothing. */
    /* But iODBC, for one, will convert our "" into a NULL. */
    CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0, (SQLCHAR *)"", SQL_NTS,
                              NULL, 0));

    is_num(myrowcount(Stmt), 0);
  }

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* Get the info from just one table.  */
  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0,
                            (SQLCHAR *)"t_bug8860", SQL_NTS,
                            NULL, 0));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  IS_STR(my_fetch_str(Stmt, buff, 3), "t_bug8860", 9);
  IS_STR(my_fetch_str(Stmt, buff, 4), "a", 1);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* Get the info from just one table with a funny name.  */
  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0,
                            (SQLCHAR *)"t_bug8860_a\\'b", SQL_NTS,
                            NULL, 0));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  IS_STR(my_fetch_str(Stmt, buff, 3), "t_bug8860_a'b", 13);
  IS_STR(my_fetch_str(Stmt, buff, 4), "b", 1);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug8860");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS `t_bug8860_a'b`");
  return OK;
}

/**
 Bug #26934: SQLTables behavior has changed
*/
ODBC_TEST(t_bug26934)
{
  SQLHENV    Env1;
  SQLHDBC    Connection1;
  SQLHSTMT   Stmt1;

  ODBC_Connect(&Env1, &Connection1, &Stmt1);

  killConnection(Stmt1, Stmt);
  
  EXPECT_STMT(Stmt1, SQLTables(Stmt1, (SQLCHAR *)"%", 1, NULL, SQL_NTS,
                                NULL, SQL_NTS, NULL, SQL_NTS), SQL_ERROR);
  CHECK_SQLSTATE(Stmt1, "08S01");

  ODBC_Disconnect(Env1, Connection1, Stmt1);

  return OK;
}


/**
 Bug #29888: Crystal wizard throws error on including tables
*/
ODBC_TEST(t_bug29888)
{
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug29888");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_bug29888 (a INT, b INT)");

  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, my_schema, SQL_NTS, NULL, SQL_NTS,
                            (SQLCHAR *)"t_bug29888", SQL_NTS,
                            (SQLCHAR *)"%", SQL_NTS));

  is_num(myrowcount(Stmt), 2);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug29888");

  return OK;
}


/**
  Bug #14407: SQLColumns gives wrong information of not nulls
*/
ODBC_TEST(t_bug14407)
{
  SQLCHAR col[10];
  SQLSMALLINT nullable;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug14407");
  OK_SIMPLE_STMT(Stmt,
         "CREATE TABLE t_bug14407(a INT NOT NULL AUTO_INCREMENT PRIMARY KEY)");

  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0,
                            (SQLCHAR *)"t_bug14407", SQL_NTS, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, col, 4), "a", 1);
  is_num(my_fetch_int(Stmt, 11), SQL_NO_NULLS);
  IS_STR(my_fetch_str(Stmt, col, 18), "NO", 3);

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /**
    Bug #26108  MyODBC ADO field attributes reporting adFldMayBeNull for
    not null columns
  */
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM t_bug14407");

  CHECK_STMT_RC(Stmt, SQLDescribeCol(Stmt, 1, col, sizeof(col), NULL, NULL, NULL,
                                NULL, &nullable));
  is_num(nullable, SQL_NO_NULLS);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bug14407");
  return OK;
}

/*
  Bug #32864 MyODBC /Crystal Reports truncated table names,
             lost fields when names > 21chars
  Fails if built with c/c and probably >=5.5 (with everything
  where SYSTEM_CHARSET_MBMAXLEN != mbmaxlen of UTF8_CHARSET_NUMBER charset)
*/
ODBC_TEST(t_bug32864)
{
  SQLLEN dispsize= 0;
  SQLULEN colsize= 0;
  SQLCHAR dummy[20];

  CHECK_STMT_RC(Stmt, SQLTables(Stmt, (SQLCHAR *)"%", SQL_NTS, NULL, 0, NULL, 0,
                           NULL, 0));
  CHECK_STMT_RC(Stmt, SQLColAttribute(Stmt, 3, SQL_COLUMN_DISPLAY_SIZE, NULL, 0,
                                 NULL, &dispsize));
  if (ServerNotOlderThan(Connection, 8, 5, 0))
  {
    is_num(dispsize, 256);
  }
  else
  {
    is_num(dispsize, 64);
  }
  CHECK_STMT_RC(Stmt, SQLDescribeCol(Stmt, 3, dummy, sizeof(dummy), NULL, NULL,
                                &colsize, NULL, NULL));

  if (ServerNotOlderThan(Connection, 8, 5, 0))
  {
    is_num(colsize, 256);
  }
  else
  {
    is_num(colsize, 64);
  }

  return OK;
}


/*
  Bug #32989 - Crystal Reports fails if a field has a single quote
*/
ODBC_TEST(t_bug32989)
{
  SQLCHAR name[20];
  SQLLEN len;

  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug32989");
  OK_SIMPLE_STMT(Stmt, "create table t_bug32989 (`doesn't work` int)");

  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, (SQLCHAR *)my_schema, SQL_NTS, NULL, 0,
                            (SQLCHAR *)"t_bug32989", SQL_NTS, NULL, 0));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 4, SQL_C_CHAR, name, 20, &len));
  is_num(len, 12);
  IS_STR(name, "doesn't work", 13);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug32989");
  return OK;
}


/**
 Bug #33298: ADO returns wrong parameter count in the query (always 0)
*/
ODBC_TEST(t_bug33298)
{
  SQLRETURN rc= 0;
  FAIL_IF(SQLProcedureColumns(Stmt, NULL, 0, NULL, 0,
                                         NULL, 0, NULL, 0)
                                         != SQL_SUCCESS, "Error");
  rc= SQLFetch(Stmt);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  if (rc == SQL_ERROR)
    return FAIL;

  return OK;
}

/**
 Bug #12805: ADO failed to retrieve the length of LONGBLOB columns
*/
ODBC_TEST(t_bug12805)
{
  SQLHENV     Env1;
  SQLHDBC     Connection1;
  SQLHSTMT    Stmt1;
  SQLCHAR     dummy[10];
  SQLULEN     length;  
  SQLUINTEGER len2;
  int         saved_options= my_options;

  diag("Option not supported yet");
  return SKIP;

  my_options = 1 << 27;

  ODBC_Connect(&Env1, &Connection1, &Stmt1);

  OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS bug12805");
  OK_SIMPLE_STMT(Stmt1, "CREATE TABLE bug12805("\
                 "id INT PRIMARY KEY auto_increment,"\
                 "longdata LONGBLOB NULL)");

  CHECK_STMT_RC(Stmt1, SQLColumns(Stmt1, NULL, 0, NULL, 0,
                             (SQLCHAR *)"bug12805", SQL_NTS,
                             (SQLCHAR *)"longdata", SQL_NTS));

  CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));
  CHECK_STMT_RC(Stmt1, SQLGetData(Stmt1, 7, SQL_C_ULONG, &len2,
                             0, NULL));
  is_num(len2, 1073741823);
  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  length= 0;
  OK_SIMPLE_STMT(Stmt1, "SELECT * FROM bug12805");
  CHECK_STMT_RC(Stmt1, SQLDescribeCol(Stmt1, 2, dummy, sizeof(dummy) - 1, NULL,
                                 NULL, &length, NULL, NULL));
  is_num(length, 1073741823);

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));
  ODBC_Disconnect(Env1, Connection1, Stmt1);

  /* Check without the 32-bit signed flag */
  CHECK_STMT_RC(Stmt, SQLColumns(Stmt, NULL, 0, NULL, 0,
                            (SQLCHAR *)"bug12805", SQL_NTS,
                            (SQLCHAR *)"longdata", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 7, SQL_C_ULONG, &len2,
                            0, NULL));
  is_num(len2, 1073741823);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  length= 0;
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM bug12805");
  CHECK_STMT_RC(Stmt, SQLDescribeCol(Stmt, 2, dummy, sizeof(dummy), NULL, NULL,
                                 &length, NULL, NULL));
  is_num(length, 1073741823);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE bug12805");

  my_options= saved_options;
  return OK;
}


/**
 Bug #30770: Access Violation in myodbc3.dll
*/
ODBC_TEST(t_bug30770)
{
  SQLCHAR    buff[512];
  SQLHENV    Env1;
  SQLHDBC    Connection1;
  SQLHSTMT   Stmt1;
  SQLCHAR    conn[MAX_NAME_LEN*2];

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS bug30770");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE bug30770 (a INT)");

  /* Connect with no default daabase */
  sprintf((char *)conn, "DRIVER=%s;SERVER=%s;" \
                        "UID=%s;PASSWORD=%s;DATABASE=%s;PORT=%u;%s", my_drivername, my_servername,
                        my_uid, my_pwd, my_schema, my_port, add_connstr);
  
  is_num(mydrvconnect(&Env1, &Connection1, &Stmt1, conn), OK);

  /* Get the info from just one table.  */
  CHECK_STMT_RC(Stmt1, SQLColumns(Stmt1, NULL, SQL_NTS, NULL, SQL_NTS,
                             (SQLCHAR *)"bug30770", SQL_NTS, NULL, 0));

  CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));

  IS_STR(my_fetch_str(Stmt1, buff, 3), "bug30770", 9);
  IS_STR(my_fetch_str(Stmt1, buff, 4), "a", 1);

  FAIL_IF(SQLFetch(Stmt1) != SQL_NO_DATA_FOUND, "expected no data");
  ODBC_Disconnect(Env1, Connection1, Stmt1);

  OK_SIMPLE_STMT(Stmt, "DROP TABLE bug30770");
  return OK;
}


/**
 Bug #36275: SQLTables buffer overrun
*/
ODBC_TEST(t_bug36275)
{
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, 0, NULL, 0, NULL, 0,
                           (SQLCHAR *)
/* Just a really long literal to blow out the buffer. */
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789"
"0123456789012345678901234567890123456789012345678901234567890123456789",
                           SQL_NTS));

  return OK;
}


/*
  Bug #39957 - NULL catalog did not return correct catalog in result
*/
ODBC_TEST(t_bug39957)
{
  SQLCHAR buf[50];
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug39957");
  OK_SIMPLE_STMT(Stmt, "create table t_bug39957 (x int)");
  CHECK_STMT_RC(Stmt, SQLTables(Stmt, NULL, 0, NULL, 0,
			   (SQLCHAR *)"t_bug39957", SQL_NTS, NULL, 0));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, buf, 3), "t_bug39957", 11);
  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "expected no data");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "drop table if exists t_bug39957");

  return OK;
}


MA_ODBC_TESTS my_tests[]=
{
  {my_columns_null, "my_columns_null", NORMAL, ALL_DRIVERS},
  {my_drop_table, "my_drop_table", NORMAL, ALL_DRIVERS},
  {my_table_dbs, "my_table_dbs", NORMAL, ALL_DRIVERS},
  {my_colpriv, "my_colpriv", NORMAL, ALL_DRIVERS},
  {t_sqlprocedures, "t_sqlprocedures", NORMAL, ALL_DRIVERS},
  {t_catalog, "t_catalog", NORMAL, ALL_DRIVERS},
  {tmysql_specialcols, "tmysql_specialcols", NORMAL, ALL_DRIVERS},
  {t_columns, "t_columns", NORMAL, ANSI_DRIVER},
  {t_tables_bug,"t_tables_bug", NORMAL, ANSI_DRIVER},
  {t_current_catalog_unicode, "t_current_catalog_unicode", NORMAL, ALL_DRIVERS},
  {t_current_catalog_ansi, "t_current_catalog_ansi", NORMAL, ALL_DRIVERS},
  {tmysql_showkeys, "tmysql_showkeys", CSPS_OK | SSPS_FAIL},
  {t_sqltables, "t_sqltables", NORMAL, ALL_DRIVERS},
  {my_information_schema, "my_information_schema", NORMAL, ALL_DRIVERS},
  {empty_set, "empty_set", NORMAL, ALL_DRIVERS},
  {t_bug23031, "t_bug23031", NORMAL, ALL_DRIVERS},
  {bug15713, "bug15713", NORMAL, ALL_DRIVERS},
  {t_bug28316, "t_bug28316", NORMAL, ALL_DRIVERS},
  {bug8860, "bug8860", NORMAL, ALL_DRIVERS},
// TODO: PLAT-6326
#ifndef IS_ON_S2MS
  {t_bug26934, "t_bug26934", NORMAL, ALL_DRIVERS},
#endif
  {t_bug29888, "t_bug29888", NORMAL, ALL_DRIVERS},
  {t_bug14407, "t_bug14407", NORMAL, ALL_DRIVERS},
  {t_bug32864, "t_bug32864", NORMAL, ALL_DRIVERS},
  {t_bug32989, "t_bug32989", NORMAL, ALL_DRIVERS},
  {t_bug33298, "t_bug33298", NORMAL, ALL_DRIVERS},
  {t_bug12805, "t_bug12805", NORMAL, ALL_DRIVERS},
  {t_bug30770,"t_bug30770", NORMAL, ALL_DRIVERS},
  {t_bug36275, "t_bug36275", NORMAL, ALL_DRIVERS},
  {t_bug39957, "t_bug39957", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
