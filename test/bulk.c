/*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
                2013, 2019 MariaDB Corporation AB
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

#define MAX_INSERT_COUNT 800

ODBC_TEST(t_bulk_insert_nts)
{
  char a[2][30]= {"Name 1", "Name 23"};
  SQLLEN indicator[2]= {SQL_NTS, SQL_NTS};

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bulk_insert");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_bulk_insert (a VARCHAR(20))");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)2, 0));

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, &a[0], 30, indicator));

  OK_SIMPLE_STMT(Stmt, "SELECT a FROM t_bulk_insert LIMIT 1");

  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)1, 0));

  OK_SIMPLE_STMT(Stmt, "SELECT * FROM t_bulk_insert");
  is_num(myrowcount(Stmt), 2);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bulk_insert");

  return OK;
}

/* TODO: As of now this test is useless */
ODBC_TEST(t_bulk_insert_test)
{
  char a[2][30]= {"Name 1", "Name 23"};
  double b[2]= {5.78, 6.78};
  double d[2]= {1.23, 3.45};
  SQLLEN indicator[2]= {SQL_NTS, SQL_NTS};
  SQLLEN b_indicator[2]= {0,0};
  SQLLEN d_indicator[2]= {0,0};

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bulk_insert");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_bulk_insert (a VARCHAR(20), b bigint(20), c decimal(15,2))");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)2, 0));

  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR*)"INSERT INTO t_bulk_insert VALUES (?,?,?)", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, &a[0], 30, indicator));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_DOUBLE, &b[0], 8, b_indicator));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_DOUBLE, &d[0], 8, d_indicator));

  OK_SIMPLE_STMT(Stmt, "SELECT a,b,c FROM t_bulk_insert LIMIT 1");

//  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  CHECK_DBC_RC(Connection, SQLEndTran(SQL_HANDLE_DBC, Connection, 0));

  SQLFreeHandle(SQL_HANDLE_STMT, Stmt);

  CHECK_DBC_RC(Connection, SQLAllocHandle(SQL_HANDLE_STMT, Connection, &Stmt));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));
  //SQLFreeHandle(SQL_HANDLE_STMT, Stmt);
  CHECK_DBC_RC(Stmt, SQLEndTran(SQL_HANDLE_DBC, Connection, 0));

  //CHECK_DBC_RC(Connection, SQLDisconnect(Connection));

  return OK;
}

ODBC_TEST(t_bulk_insert)
{
  SQLINTEGER i, id[MAX_INSERT_COUNT+1];
  SQLCHAR    name[MAX_INSERT_COUNT][40],
    txt[MAX_INSERT_COUNT][60],
    ltxt[MAX_INSERT_COUNT][70];
  SQLDOUBLE  dt, dbl[MAX_INSERT_COUNT];

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bulk_insert");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_bulk_insert (id INT, v VARCHAR(100),"
                       "txt TEXT, ft FLOAT(10), ltxt LONG VARCHAR)");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  dt= 0.23456;

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)MAX_INSERT_COUNT, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CONCURRENCY,
                                     (SQLPOINTER)SQL_CONCUR_ROWVER, 0));

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, id, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, name, sizeof(name[0]), NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, txt, sizeof(txt[0]), NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_DOUBLE, dbl, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_CHAR, ltxt, sizeof(ltxt[0]), NULL));

  OK_SIMPLE_STMT(Stmt, "SELECT id, v, txt, ft, ltxt FROM t_bulk_insert");

  FAIL_IF(SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0)!= SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  for (i= 0; i < MAX_INSERT_COUNT; i++)
  {
    id[i]= i;
    dbl[i]= i + dt;
    sprintf((char *)name[i], "Varchar%d", i);
    sprintf((char *)txt[i],  "Text%d", i);
    sprintf((char *)ltxt[i], "LongText, id row:%d", i);
  }

  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));
  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)1, 0));

  OK_SIMPLE_STMT(Stmt, "SELECT * FROM t_bulk_insert");
  is_num(myrowcount(Stmt), MAX_INSERT_COUNT * 2);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bulk_insert");

  return OK;
}


ODBC_TEST(t_mul_pkdel)
{
  SQLINTEGER nData;
  SQLLEN nlen;
  SQLULEN pcrow;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_mul_pkdel");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_mul_pkdel (a INT NOT NULL, b INT,"
                       "c VARCHAR(30) NOT NULL, PRIMARY KEY(a, c))");
  OK_SIMPLE_STMT(Stmt, "INSERT INTO t_mul_pkdel VALUES (100,10,'MySQL1'),"
                       "(200,20,'MySQL2'),(300,20,'MySQL3'),(400,20,'MySQL4')");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetCursorName(Stmt, (SQLCHAR *)"venu", SQL_NTS));

  OK_SIMPLE_STMT(Stmt, "SELECT a, c FROM t_mul_pkdel");

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &nData, 0, NULL));

  CHECK_STMT_RC(Stmt, SQLExtendedFetch(Stmt, SQL_FETCH_NEXT, 1, &pcrow, NULL));
  CHECK_STMT_RC(Stmt, SQLSetPos(Stmt, 1, SQL_POSITION, SQL_LOCK_NO_CHANGE));

  CHECK_STMT_RC(Stmt, SQLSetPos(Stmt, 1, SQL_DELETE, SQL_LOCK_NO_CHANGE));
  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &nlen));
  is_num(nlen, 1);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT * FROM t_mul_pkdel");

  is_num(myrowcount(Stmt), 3);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_mul_pkdel");

  return OK;
}


/**
  Bug #24306: SQLBulkOperations always uses indicator varables' values from
  the first record
*/
ODBC_TEST(t_bulk_insert_indicator)
{
  SQLINTEGER id[4], nData;
  SQLLEN     indicator[4], nLen;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS my_bulk");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE my_bulk (id int default 5)");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  CHECK_STMT_RC(Stmt,
                SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)3, 0));

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, id, 0, indicator));

  OK_SIMPLE_STMT(Stmt, "SELECT id FROM my_bulk");

  FAIL_IF(SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0)!=SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  id[0]= 1; indicator[0]= SQL_COLUMN_IGNORE;
  id[1]= 2; indicator[1]= SQL_NULL_DATA;
  id[2]= 3; indicator[2]= 0;

  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)1, 0));

  OK_SIMPLE_STMT(Stmt, "SELECT id FROM my_bulk ORDER BY IFNULL(id, 0)");

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &nData, 0, &nLen));

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(nLen, SQL_NULL_DATA);

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 3);
  IS(nLen != SQL_NULL_DATA);

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 5);
  IS(nLen != SQL_NULL_DATA);

  FAIL_IF(SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0)!=SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS my_bulk");

  return OK;
}


/**
  Simple structure for a row (just one element) plus an indicator column.
*/
typedef struct {
  SQLINTEGER val;
  SQLLEN     ind;
} row;

/**
  This is related to the fix for Bug #24306 -- handling of row-wise binding,
  plus handling of SQL_ATTR_ROW_BIND_OFFSET_PTR, within the context of
  SQLBulkOperations(Stmt, SQL_ADD).
*/
ODBC_TEST(t_bulk_insert_rows)
{
  row        rows[3];
  SQLINTEGER nData;
  SQLLEN     nLen, offset;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS my_bulk");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE my_bulk (id int default 5)");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)3, 0));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_TYPE,
                                     (SQLPOINTER)sizeof(row), 0));

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &rows[0].val, 0,
                                 &rows[0].ind));

  OK_SIMPLE_STMT(Stmt, "SELECT id FROM my_bulk");

  FAIL_IF(SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0)!=SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  rows[0].val= 1; rows[0].ind= SQL_COLUMN_IGNORE;
  rows[1].val= 2; rows[1].ind= SQL_NULL_DATA;
  rows[2].val= 3; rows[2].ind= 0;

  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  /* Now re-insert the last row using SQL_ATTR_ROW_BIND_OFFSET_PTR */
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)1, 0));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_OFFSET_PTR,
                                     (SQLPOINTER)&offset, 0));

  offset= 2 * sizeof(row);

  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  /* Remove SQL_ATTR_ROW_BIND_OFFSET_PTR */
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_OFFSET_PTR,
                                     NULL, 0));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "SELECT id FROM my_bulk ORDER BY IFNULL(id, 0)");

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &nData, 0, &nLen));

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(nLen, SQL_NULL_DATA);

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 3);
  IS(nLen != SQL_NULL_DATA);

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 3);
  IS(nLen != SQL_NULL_DATA);

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(nData, 5);
  IS(nLen != SQL_NULL_DATA);

  FAIL_IF(SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0)!=SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS my_bulk");

  return OK;
}

#define MAODBC_ROWS 2
ODBC_TEST(t_odbc90)
{
  SQLHANDLE  henv1;
  SQLHANDLE  Connection1;
  SQLHANDLE  Stmt1;
  SQLCHAR    conn[512], sval[MAODBC_ROWS][32]={"Record 1", "Record 21"};
  SQLLEN     ind1[MAODBC_ROWS]= {SQL_COLUMN_IGNORE, 0}, ind2[MAODBC_ROWS]= {sizeof(int), sizeof(int)},
    ind3[MAODBC_ROWS]= {8, 9}, ind4[MAODBC_ROWS]={SQL_COLUMN_IGNORE, SQL_COLUMN_IGNORE};
  SQLINTEGER nval[MAODBC_ROWS]={100, 500}, id[MAODBC_ROWS]={2, 7};

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS odbc90");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE odbc90 (id int not null primary key auto_increment, \
                          nval int not null, sval varchar(32) not null, ts timestamp not null default now)");

  /* odbc 3 */
  /* This cursor closing is required, otherwise DM(on Windows) freaks out */
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  CHECK_STMT_RC(Stmt,
                SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)MAODBC_ROWS, 0));

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, id, 0, ind1));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, nval, 0, ind2));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, sval, sizeof(sval[0]), ind3));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, NULL, 0, ind4));

  OK_SIMPLE_STMT(Stmt, "SELECT id, nval, sval, ts FROM odbc90");

  FAIL_IF(SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0)!=SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE,
                                     (SQLPOINTER)1, 0));

  OK_SIMPLE_STMT(Stmt, "SELECT id, nval, sval, ts FROM odbc90 ORDER BY id");

  ind4[0]= 0;
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, id, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, nval, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, sval[0], sizeof(sval[0]), NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_CHAR, sval[1], sizeof(sval[0]), ind4));

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(id[0], 1);
  is_num(nval[0], 100);
  IS_STR(sval[0], "Record 1", ind3[0] + 1);
  is_num(ind4[0], 19);

  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0));
  is_num(id[0], 7);
  is_num(nval[0], 500);
  IS_STR(sval[0], "Record 21", ind3[1] + 1);
  is_num(ind4[0], 19);

  FAIL_IF(SQLFetchScroll(Stmt, SQL_FETCH_NEXT, 0)!=SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* odbc 2. Not sure if it's really needed internaly one call is mapped to another as well. But won't hurt. */
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS odbc90");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE odbc90 (id int not null primary key auto_increment, \
                                            nval int not null, sval varchar(32) not null, ts timestamp not null default now)");
  id[0]= 2;
  ind4[0]= SQL_COLUMN_IGNORE;
  strcpy((char*)(sval[0]), "Record 1");
  strcpy((char*)(sval[1]), "Record 21");
  nval[0]= 100;

  sprintf((char *)conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s",
          my_drivername, my_servername, my_uid, "XXXXXXXX", my_schema, ma_strport, add_connstr);
  diag("Starting new connection: %s", conn);
  sprintf((char*)conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;DATABASE=%s;%s;%s;",
          my_drivername, my_servername, my_uid, my_pwd, my_schema, ma_strport, add_connstr);
  CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv1));
  CHECK_ENV_RC(henv1, SQLSetEnvAttr(henv1, SQL_ATTR_ODBC_VERSION,
                                    (SQLPOINTER)SQL_OV_ODBC2, SQL_IS_INTEGER));
  CHECK_ENV_RC(henv1, SQLAllocHandle(SQL_HANDLE_DBC, henv1, &Connection1));
  CHECK_DBC_RC(Connection1, SQLDriverConnect(Connection1, NULL, conn, (SQLSMALLINT)strlen((const char*)conn), NULL, 0,
                                             NULL, SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(Connection1, SQLAllocHandle(SQL_HANDLE_STMT, Connection1, &Stmt1));

  /* This cursor closing is required, otherwise DM(on Windows) freaks out */
  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));
  CHECK_STMT_RC(Stmt1, SQLSetStmtAttr(Stmt1, SQL_ATTR_CURSOR_TYPE,
                                      (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  CHECK_STMT_RC(Stmt1,
                SQLSetStmtAttr(Stmt1, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)MAODBC_ROWS, 0));

  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 1, SQL_C_LONG, id, 0, ind1));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 2, SQL_C_LONG, nval, 0, ind2));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 3, SQL_C_CHAR, sval, sizeof(sval[0]), ind3));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 4, SQL_C_CHAR, NULL, 0, ind4));

  OK_SIMPLE_STMT(Stmt1, "SELECT id, nval, sval, ts FROM odbc90");

  FAIL_IF(SQLExtendedFetch(Stmt1, SQL_FETCH_FIRST, 2, NULL, NULL)!=SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  CHECK_STMT_RC(Stmt1, SQLSetPos(Stmt1, 0, SQL_ADD, SQL_LOCK_NO_CHANGE));

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_UNBIND));
  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  CHECK_STMT_RC(Stmt1, SQLSetStmtAttr(Stmt1, SQL_ATTR_ROW_ARRAY_SIZE,
                                      (SQLPOINTER)1, 0));

  OK_SIMPLE_STMT(Stmt1, "SELECT id, nval, sval, ts FROM odbc90 ORDER BY id");

  ind4[0]= 0;
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 1, SQL_C_LONG, id, 0, NULL));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 2, SQL_C_LONG, nval, 0, NULL));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 3, SQL_C_CHAR, sval[0], sizeof(sval[0]), NULL));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 4, SQL_C_CHAR, sval[1], sizeof(sval[0]), ind4));

  CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));
  is_num(id[0], 1);
  is_num(nval[0], 100);
  IS_STR(sval[0], "Record 1", ind3[0] + 1);
  is_num(ind4[0], 19);

  CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));
  is_num(id[0], 7);
  is_num(nval[0], 500);
  IS_STR(sval[0], "Record 21", ind3[1] + 1);
  is_num(ind4[0], 19);

  FAIL_IF(SQLFetch(Stmt1)!=SQL_NO_DATA_FOUND, "SQL_NO_DATA_FOUND expected");

  CHECK_STMT_RC(Stmt1, SQLFreeHandle(SQL_HANDLE_STMT, Stmt1));
  CHECK_DBC_RC(Connection1, SQLDisconnect(Connection1));
  CHECK_DBC_RC(Connection1, SQLFreeHandle(SQL_HANDLE_DBC, Connection1));
  CHECK_ENV_RC(henv1, SQLFreeHandle(SQL_HANDLE_ENV, henv1));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE odbc90");

  return OK;
}
#undef MAODBC_ROWS

/* This is test of DELETE statemten with param array. The primary goal is to test MariaDB bulk operation,
   thus putting it into "bulk". ODBC-117 is corresponding bug report */
ODBC_TEST(t_bulk_delete)
{
  SQLINTEGER a[3]= {1, 3, 2};
  SQLCHAR val[7], insert_val[][7]= {"first", "third", "second"};
  SQLLEN val_indicator[]= {SQL_NTS, SQL_NTS, SQL_NTS}, id_ind[]= {4, 4, 4};

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bulk_delete");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_bulk_delete (id INT unsigned not null primary key, val VARCHAR(20) not null)");
  /*OK_SIMPLE_STMT(Stmt, "INSERT INTO t_bulk_delete VALUES(1, 'first'), (2, 'second'), (3, 'third')");*/

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE,
                                     (SQLPOINTER)3, 0));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, a, 0, id_ind));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, insert_val, sizeof(insert_val[0]), val_indicator));

  /* Needed for crazy iODBC on OS X */
  is_num(iOdbcSetParamBufferSize(Stmt, 2, sizeof(insert_val[0])), OK);

  OK_SIMPLE_STMT(Stmt, "INSERT INTO t_bulk_delete VALUES(?, ?)");

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE,
                                     (SQLPOINTER)2, 0));
  OK_SIMPLE_STMT(Stmt, "DELETE FROM t_bulk_delete WHERE id=?");

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)1, 0));
  OK_SIMPLE_STMT(Stmt, "SELECT id, val FROM t_bulk_delete");

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), 2);
  IS_STR(my_fetch_str(Stmt, val, 2), "second", 7);

  EXPECT_STMT(Stmt, SQLFetch(Stmt), SQL_NO_DATA);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_bulk_delete");

  return OK;
}

#define MAODBC_ROWS 3
ODBC_TEST(t_odbc149)
{
  SQL_TIMESTAMP_STRUCT  Val[MAODBC_ROWS];
  SQLINTEGER id[]= {1, 2, 3}, idBuf, row= 0;
  /* Garbage in Len */
  SQLLEN tsLen[]= {0x01800000, SQL_NULL_DATA, 0x01800000}, bLen[]= {5, 3, 6}, bBufLen;
  SQLCHAR c[][8]= {"str1", "abcd", "xxxxxxy"}, cBuf[16];
  SQLCHAR b[][8]= {"\x1f\x1f\x1f\x00\x1f", "\x00\x44\x88", "\xcd\xcd\xcd\xcd\xcd\xcd"}, bBuf[256];
  SQLLEN  cInd[]= {SQL_NTS, SQL_NTS, SQL_NTS}, wLen;
  SQLWCHAR w[][8]= {{'x', 'x', 'x', 0}, {'y', 'y', 0}, {'z', 'z', 'z', 'a', 0}}, wBuf[16];

  //TODO https://memsql.atlassian.net/jira/software/c/projects/PLAT/issues/PLAT-5346
  if (iOdbc() && is_unicode_driver()) {
    return OK;
  }

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS odbc149");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE odbc149 (id int not null, ts timestamp, c varchar(16), b tinyblob, w varchar(16))");

  /* This cursor closing is required, otherwise DM(not on Windows) freaks out */
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR*)"INSERT INTO odbc149(id, ts, c, b, w) values(?, ?, ?, ?, ?)", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)MAODBC_ROWS, 0));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, id, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIMESTAMP, 0, 0, Val, tsLen[0]/* Garbage as buffer len */, tsLen));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 0, 0, c, sizeof(c[0]), cInd));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 4, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, 0, 0, b, sizeof(b[0]), bLen));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WCHAR, 0, 0, w, sizeof(w[0]), cInd));

  is_num(iOdbcSetParamBufferSize(Stmt, 3, sizeof(c[0])), OK);
  is_num(iOdbcSetParamBufferSize(Stmt, 4, sizeof(b[0])), OK);
  is_num(iOdbcSetParamBufferSize(Stmt, 5, sizeof(w[0])), OK);

  memset(Val, 0, sizeof(Val));
  Val[0].year= 2017;
  Val[0].month= 6;
  Val[0].day= 22;
  Val[2].year= 2018;
  Val[2].month= 7;
  Val[2].day= 27;

  CHECK_STMT_RC(Stmt, SQLExecute(Stmt));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)1, 0));
  OK_SIMPLE_STMT(Stmt, "SELECT id, ts, c, b, w FROM odbc149 ORDER BY id")
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &idBuf, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_TIMESTAMP, &Val[1], 0, tsLen));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_CHAR, cBuf, sizeof(cBuf), cInd));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_BINARY, bBuf, sizeof(bBuf), &bBufLen));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_WCHAR, wBuf, sizeof(wBuf), &wLen));

  for (row= 0; row < MAODBC_ROWS; ++row)
  {
    memset(&Val[1], 0, sizeof(SQL_TIMESTAMP_STRUCT));
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    if (row != 1)
    {
      is_num(tsLen[0], sizeof(SQL_TIMESTAMP_STRUCT));
      is_num(Val[1].year, Val[row].year);
      is_num(Val[1].month, Val[row].month);
      is_num(Val[1].day, Val[row].day);
    }
    else
    {
      IS(*tsLen == SQL_NULL_DATA);
    }

    is_num(idBuf, id[row]);
    IS_STR(cBuf, c[row], strlen((const char*)(c[row])) + 1);
    is_num(bBufLen, bLen[row]);
    memcmp(bBuf, b[row], bBufLen);
    //TODO https://memsql.atlassian.net/jira/software/c/projects/PLAT/issues/PLAT-5346
    if (!iOdbc() && !is_ansi_driver()) {
      IS_WSTR(wBuf, w[row], wLen/sizeof(SQLWCHAR));
    }
  }

  EXPECT_STMT(Stmt, SQLFetch(Stmt), SQL_NO_DATA);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE odbc149");

  return OK;
}
#undef MAODBC_ROWS

#define NUM_ROWS 3
#define BUFF_SIZE 10
#define NUM_CURSOR_TYPES 3
ODBC_TEST(bulk_insert_all_datatypes)
{
  SQLULEN  cursorTypes[NUM_CURSOR_TYPES] = {SQL_CURSOR_FORWARD_ONLY, SQL_CURSOR_STATIC, SQL_CURSOR_DYNAMIC};
  int currentCursorIndex;
  for (currentCursorIndex = 0; currentCursorIndex < NUM_CURSOR_TYPES; currentCursorIndex++)
  {
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                       (SQLPOINTER)cursorTypes[currentCursorIndex], 0));

    typedef struct Row {
      SQLSMALLINT id;

      SQLCHAR c_char[BUFF_SIZE];
      SQLLEN c_char_len;

      SQLWCHAR c_wchar[BUFF_SIZE];
      SQLLEN c_wchar_len;

      SQLSMALLINT c_sshort;
      SQLUSMALLINT c_ushort;
      SQLINTEGER c_slong;
      SQLUINTEGER c_ulong;
      SQLREAL c_float;
      SQLDOUBLE c_double;
      SQLCHAR c_bit;
      SQLSCHAR c_stinyint;
      SQLCHAR c_utinyint;
      SQLBIGINT c_sbigint;
      SQLUBIGINT c_ubigint;

      SQLCHAR c_binary[BUFF_SIZE];
      SQLLEN c_binary_len;

      SQL_DATE_STRUCT c_type_date;
      SQL_TIME_STRUCT c_type_time;
      SQL_TIMESTAMP_STRUCT c_type_timestamp;
      SQLLEN c_timestamp_len;
      SQL_NUMERIC_STRUCT c_numeric;
    } Row;

    int i;
    SQLHDESC ard;
    SQLULEN rowsFetched;
    SQLUSMALLINT rowStatusArray[NUM_ROWS];
    Row rowsSelect[NUM_ROWS];
    Row rowsInsert[NUM_ROWS] = {
      {0, "asd", 3, {1, 2, 3, 0}, 3, -1, 1, 100, 100, 0.3, 0.3333, 1, 10, 10, 100, 100, "abc", 3, {2001, 11, 1}, {5, 23, 59}, {2001, 11, 1, 5, 23, 59, 123456001}},
      {1, "bcd", 3, {2, 3, 4, 0}, 3, 2, 2, 10, 10, 0.5, 0.555, 1, 20, 20, 200, 200, "sdf", 3, {2005, 10, 6}, {7, 13, 29}, {2005, 10, 6, 7, 13, 29, 2345001}},
      {2, "", 0, {0}, 0, 3, 3, -20, 20, -10.2, -100.222, 0, -30, 30, -300, 300, "", 0, {2002, 12, 1}, {0, 0, 0}, {2002, 12, 1, 0, 0, 0, 0}},
    };

    // Init rowsInsert[0].c_numeric with 123.45
    rowsInsert[0].c_numeric.sign = 1;
    memset (rowsInsert[0].c_numeric.val, 0, 16);
    rowsInsert[0].c_numeric.val[0] = 0x39;
    rowsInsert[0].c_numeric.val[1] = 0x30;
    rowsInsert[0].c_numeric.precision = 5;
    rowsInsert[0].c_numeric.scale = 2;

    // Init rowsInsert[1].c_numeric with -123.45
    rowsInsert[1].c_numeric = rowsInsert[0].c_numeric;
    rowsInsert[1].c_numeric.sign = 0;

    // Init rowsInsert[2].c_numeric with 0
    rowsInsert[2].c_numeric.sign = 1;
    memset (rowsInsert[2].c_numeric.val, 0, 16);
    rowsInsert[2].c_numeric.precision = 1;
    rowsInsert[2].c_numeric.scale = 0;

    // Prepare a table
    OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS bulk_insert_all_datatypes");
    OK_SIMPLE_STMT(Stmt, "CREATE TABLE bulk_insert_all_datatypes(id INT, c_char CHAR(10), c_wchar VARCHAR(10), "
                         "c_sshort SMALLINT, c_ushort SMALLINT UNSIGNED, c_slong INT, c_ulong INT UNSIGNED, c_float FLOAT, "
                         "c_double DOUBLE, c_bit BIT(64), c_stinyint TINYINT, c_utinyint TINYINT UNSIGNED, "
                         "c_sbigint BIGINT, c_ubigint BIGINT UNSIGNED, c_binary BINARY(10), c_type_date DATE, "
                         "c_type_time TIME, c_type_timestamp TIMESTAMP(6), c_numeric DECIMAL(5, 2))");

    OK_SIMPLE_STMT(Stmt, "SELECT * FROM bulk_insert_all_datatypes");

    // Prepare statement attributes that are needed for row-wise binding
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER)sizeof(Row), 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER)NUM_ROWS, 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_STATUS_PTR, rowStatusArray, 0));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_ROWS_FETCHED_PTR, &rowsFetched, 0));

    // Precision and scale Ard should be provided via descriptor API: SQLSetDescField.
    // SQLSetDescField for all required fields must be called before the SQLBindCol, otherwise the column will be unbound.
    CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_APP_ROW_DESC, &ard, SQL_IS_POINTER, NULL));
    CHECK_STMT_RC(ard, SQLSetDescField(ard, 19, SQL_DESC_PRECISION, (SQLPOINTER)5, 0));
    CHECK_STMT_RC(ard, SQLSetDescField(ard, 19, SQL_DESC_SCALE, (SQLPOINTER)2, 0));

    // Bind columns
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_SSHORT, &rowsInsert[0].id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, rowsInsert[0].c_char, BUFF_SIZE, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_WCHAR, rowsInsert[0].c_wchar, BUFF_SIZE*sizeof(SQLWCHAR), &rowsSelect[0].c_wchar_len));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_SSHORT, &rowsInsert[0].c_sshort, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_USHORT, &rowsInsert[0].c_ushort, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 6, SQL_C_SLONG, &rowsInsert[0].c_slong, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 7, SQL_C_ULONG, &rowsInsert[0].c_ulong, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 8, SQL_C_FLOAT, &rowsInsert[0].c_float, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 9, SQL_C_DOUBLE, &rowsInsert[0].c_double, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 10, SQL_C_BIT, &rowsInsert[0].c_bit, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 11, SQL_C_STINYINT, &rowsInsert[0].c_stinyint, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 12, SQL_C_UTINYINT, &rowsInsert[0].c_utinyint, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 13, SQL_C_SBIGINT, &rowsInsert[0].c_sbigint, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 14, SQL_C_UBIGINT, &rowsInsert[0].c_ubigint, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 15, SQL_C_BINARY, &rowsInsert[0].c_binary, BUFF_SIZE, &rowsInsert[0].c_binary_len));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 16, SQL_C_TYPE_DATE, &rowsInsert[0].c_type_date, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 17, SQL_C_TYPE_TIME, &rowsInsert[0].c_type_time, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 18, SQL_C_TYPE_TIMESTAMP, &rowsInsert[0].c_type_timestamp, 0, &rowsInsert[0].c_timestamp_len));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 19, SQL_C_NUMERIC, &rowsInsert[0].c_numeric, 0, NULL));

    // Perform bulk insert. It should insert NUM_ROWS new rows to the table
    CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));
    /*
    is_num(rowsFetched, NUM_ROWS);
    for (i = 0; i < NUM_ROWS; i++) {
      is_num(rowStatusArray[i], SQL_ROW_SUCCESS); // TODO PLAT-5555
    }*/

    // Free the statement
    SQLFreeStmt(Stmt, SQL_CLOSE);
    SQLFreeStmt(Stmt, SQL_UNBIND);

    // check that table contains correct content
    OK_SIMPLE_STMT(Stmt, "SELECT * FROM bulk_insert_all_datatypes ORDER BY id");

    CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_APP_ROW_DESC, &ard, SQL_IS_POINTER, NULL));
    CHECK_STMT_RC(ard, SQLSetDescField(ard, 19, SQL_DESC_PRECISION, (SQLPOINTER)5, 0));
    CHECK_STMT_RC(ard, SQLSetDescField(ard, 19, SQL_DESC_SCALE, (SQLPOINTER)2, 0));

    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_SSHORT, &rowsSelect[0].id, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, &rowsSelect[0].c_char, BUFF_SIZE, &rowsSelect[0].c_char_len));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_WCHAR, rowsSelect[0].c_wchar, BUFF_SIZE*sizeof(SQLWCHAR), &rowsSelect[0].c_wchar_len));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_SSHORT, &rowsSelect[0].c_sshort, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_USHORT, &rowsSelect[0].c_ushort, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 6, SQL_C_SLONG, &rowsSelect[0].c_slong, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 7, SQL_C_ULONG, &rowsSelect[0].c_ulong, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 8, SQL_C_FLOAT, &rowsSelect[0].c_float, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 9, SQL_C_DOUBLE, &rowsSelect[0].c_double, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 10, SQL_C_BIT, &rowsSelect[0].c_bit, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 11, SQL_C_STINYINT, &rowsSelect[0].c_stinyint, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 12, SQL_C_UTINYINT, &rowsSelect[0].c_utinyint, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 13, SQL_C_SBIGINT, &rowsSelect[0].c_sbigint, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 14, SQL_C_UBIGINT, &rowsSelect[0].c_ubigint, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 15, SQL_C_BINARY, &rowsSelect[0].c_binary, BUFF_SIZE, &rowsSelect[0].c_binary_len));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 16, SQL_C_TYPE_DATE, &rowsSelect[0].c_type_date, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 17, SQL_C_TYPE_TIME, &rowsSelect[0].c_type_time, 0, NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 18, SQL_C_TYPE_TIMESTAMP, &rowsSelect[0].c_type_timestamp, 0, &rowsSelect[0].c_timestamp_len));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 19, SQL_C_NUMERIC, &rowsSelect[0].c_numeric, 0, NULL));

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(rowsFetched, NUM_ROWS);
    for (i = 0; i < NUM_ROWS; i++) {
      is_num(rowStatusArray[i], SQL_ROW_SUCCESS);
    }

    for (i = 0; i < NUM_ROWS; i++)
    {
      printf("Checking row #%d\n", i);

      is_num(rowsSelect[i].id, rowsInsert[i].id);

      is_num(rowsSelect[i].c_char_len, rowsInsert[i].c_char_len);
      IS_STR(rowsSelect[i].c_char, rowsInsert[i].c_char, rowsSelect[i].c_char_len+1);

      is_num(rowsSelect[i].c_wchar_len, rowsInsert[i].c_wchar_len*sizeof(SQLWCHAR));
      if (!iOdbc() && !is_ansi_driver()) {
        // Ansi iODBC driver for some reason break the WCHAR inside of `rowsInsert` and this check fails
        IS_STR(rowsSelect[i].c_wchar, rowsInsert[i].c_wchar, rowsSelect[i].c_wchar_len+1);
      }

      is_num(rowsSelect[i].c_sshort, rowsInsert[i].c_sshort);
      is_num(rowsSelect[i].c_ushort, rowsInsert[i].c_ushort);
      is_num(rowsSelect[i].c_slong, rowsInsert[i].c_slong);
      is_num(rowsSelect[i].c_ulong, rowsInsert[i].c_ulong);
      is_double(rowsSelect[i].c_float, rowsInsert[i].c_float);
      is_num(rowsSelect[i].c_double, rowsInsert[i].c_double);
      is_num(rowsSelect[i].c_bit, rowsInsert[i].c_bit);
      is_num(rowsSelect[i].c_stinyint, rowsInsert[i].c_stinyint);
      is_num(rowsSelect[i].c_utinyint, rowsInsert[i].c_utinyint);
      is_num(rowsSelect[i].c_sbigint, rowsInsert[i].c_sbigint);
      is_num(rowsSelect[i].c_ubigint, rowsInsert[i].c_ubigint);

      // In the table, we created BINARY(10) column
      // SingleStore adds zero bytes to the result
      is_num(rowsSelect[i].c_binary_len, 10);
      IS_STR(rowsSelect[i].c_binary, rowsInsert[i].c_binary, rowsInsert[i].c_binary_len+1);

      is_num(rowsSelect[i].c_type_date.year, rowsInsert[i].c_type_date.year);
      is_num(rowsSelect[i].c_type_date.month, rowsInsert[i].c_type_date.month);
      is_num(rowsSelect[i].c_type_date.day, rowsInsert[i].c_type_date.day);

      is_num(rowsSelect[i].c_type_time.hour, rowsInsert[i].c_type_time.hour);
      is_num(rowsSelect[i].c_type_time.minute, rowsInsert[i].c_type_time.minute);
      is_num(rowsSelect[i].c_type_time.second, rowsInsert[i].c_type_time.second);

      is_num(rowsSelect[i].c_type_timestamp.year, rowsInsert[i].c_type_timestamp.year);
      is_num(rowsSelect[i].c_type_timestamp.month, rowsInsert[i].c_type_timestamp.month);
      is_num(rowsSelect[i].c_type_timestamp.day, rowsInsert[i].c_type_timestamp.day);
      is_num(rowsSelect[i].c_type_timestamp.hour, rowsInsert[i].c_type_timestamp.hour);
      is_num(rowsSelect[i].c_type_timestamp.minute, rowsInsert[i].c_type_timestamp.minute);
      is_num(rowsSelect[i].c_type_timestamp.second, rowsInsert[i].c_type_timestamp.second);
      // Precision is handled incorrectly for SSPS. It is caused by the engine bug DB-47380
      // TODO enable this test for SSPS when 7.5 will be released and added for testing (it should contain fix)
      if (NoSsps)
      {
        // S2 support saves only 6 numbers after comma, when ODBC structure allows to provide 9 numbers
        // We lose last 3 numbers, so we are not comparing them here
        is_num(rowsSelect[i].c_type_timestamp.fraction, rowsInsert[i].c_type_timestamp.fraction/1000*1000);
      }

      is_num(rowsSelect[i].c_numeric.precision, rowsInsert[i].c_numeric.precision);
      is_num(rowsSelect[i].c_numeric.scale, rowsInsert[i].c_numeric.scale);
      is_num(rowsSelect[i].c_numeric.sign, rowsInsert[i].c_numeric.sign);
      IS_STR(rowsSelect[i].c_numeric.val, rowsInsert[i].c_numeric.val, SQL_MAX_NUMERIC_LEN);
    }

    FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA, "No data expected");
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  }

  return OK;
}
#undef NUM_ROWS
#undef BUFF_SIZE

ODBC_TEST(unsupported_bulk_operations)
{
  OK_SIMPLE_STMT(Stmt, "SELECT 1");

  EXPECT_STMT(Stmt, SQLBulkOperations(Stmt, SQL_UPDATE_BY_BOOKMARK), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "HYC00");
  EXPECT_STMT(Stmt, SQLBulkOperations(Stmt, SQL_DELETE_BY_BOOKMARK), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "HYC00");
  EXPECT_STMT(Stmt, SQLBulkOperations(Stmt, SQL_FETCH_BY_BOOKMARK), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "HYC00");

  return OK;
}

#define BUFF_SIZE 10
ODBC_TEST(bulk_load_data)
{
  SQLCHAR col1InsPart1[5] = "abcd";
  SQLCHAR col1InsPart2[3] = "ef";
  SQLCHAR col2Ins [4] = "abc";
  // We are returning "N" for SQL_NEED_LONG_DATA_LEN
  // This means that we can place any nonnegative value to the SQL_LEN_DATA_AT_EXEC macro
  // and the value will be ignored
  SQLLEN colInsLen = SQL_LEN_DATA_AT_EXEC(0);
  int colId1;
  int colId2;
  int i;

  SQLCHAR col1Sel[BUFF_SIZE];
  SQLCHAR col2Sel[BUFF_SIZE];
  SQLLEN col1SelLen;
  SQLLEN col2SelLen;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS bulk_load_data");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE bulk_load_data(text1 CHAR(10), text2 CHAR(10))");

  OK_SIMPLE_STMT(Stmt, "SELECT * FROM bulk_load_data");

  // bind columns
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, &colId1, 0, &colInsLen));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, &colId2, 0, &colInsLen));

  // insert one row
  EXPECT_STMT(Stmt,SQLBulkOperations(Stmt, SQL_ADD), SQL_NEED_DATA);

  // add data at execution parameters
  SQLPOINTER colId;
  for (i = 0; i < 2; i++) {
    EXPECT_STMT(Stmt, SQLParamData(Stmt, &colId), iOdbc() ? SQL_ERROR : SQL_NEED_DATA);
    if (colId == &colId1)
    {
      CHECK_STMT_RC(Stmt, SQLPutData(Stmt, col1InsPart1, SQL_NTS));
      CHECK_STMT_RC(Stmt, SQLPutData(Stmt, col1InsPart2, SQL_NTS));
    } else if (colId == &colId2)
    {
      CHECK_STMT_RC(Stmt, SQLPutData(Stmt, col2Ins, SQL_NTS));
    } else
    {
      return FAIL;
    }
  }
  CHECK_STMT_RC(Stmt, SQLParamData(Stmt, &colId));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  // check that row is inserted correctly
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM bulk_load_data");
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_CHAR, col1Sel, BUFF_SIZE, &col1SelLen));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_CHAR, col2Sel, BUFF_SIZE, &col2SelLen));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  // check first column
  is_num(col1SelLen, strlen((char *)col1InsPart1) + strlen((char *)col1InsPart2));
  IS_STR(col1Sel, col1InsPart1, strlen((char *)col1InsPart1));
  IS_STR(col1Sel+strlen((char *)col1InsPart1), col1InsPart2, strlen((char *)col1InsPart2)+1);

  // check second column
  is_num(col2SelLen, strlen((char *)col2Ins));
  IS_STR(col2Sel, col2Ins, strlen((char *)col2Ins));
  return OK;
}
#undef BUFF_SIZE

ODBC_TEST(bulk_ignore_column)
{
  SQLINTEGER a, b, c;
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS bulk_ignore_column");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE bulk_ignore_column(a INT DEFAULT 7, b INT DEFAULT 8, c INT default -1)");

  // insert one row with all default columns
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM bulk_ignore_column");
  SQLLEN len = SQL_COLUMN_IGNORE;
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, NULL, 0, &len));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, NULL, 0, &len));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_LONG, NULL, 0, &len));
  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));

  // check the content of the table
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM bulk_ignore_column");
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &a, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, &b, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_LONG, &c, 0, NULL));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(a, 7);
  is_num(b, 8);
  is_num(c, -1);
  EXPECT_STMT(Stmt, SQLFetch(Stmt), SQL_NO_DATA);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));

  // truncate the table
  OK_SIMPLE_STMT(Stmt, "TRUNCATE bulk_ignore_column");

  // insert one row with one default column and two specified in the bound columns
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM bulk_ignore_column");
  b = 4;
  c = 5;
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, NULL, 0, &len));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, &b, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_LONG, &c, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));

  // check the content of the table
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM bulk_ignore_column");
  a = 0;
  b = 0;
  c = 0;
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &a, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, &b, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_LONG, &c, 0, NULL));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(a, 7);
  is_num(b, 4);
  is_num(c, 5);
  EXPECT_STMT(Stmt, SQLFetch(Stmt), SQL_NO_DATA);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_UNBIND));

  return OK;
}

MA_ODBC_TESTS my_tests[]=
{
  {t_bulk_insert_nts, "t_bulk_insert_nts", NORMAL, ALL_DRIVERS},
  {t_bulk_insert_test, "t_bulk_insert_test", NORMAL, ALL_DRIVERS},
  {t_bulk_insert, "t_bulk_insert", NORMAL, ALL_DRIVERS},
  {t_mul_pkdel, "t_mul_pkdel", NORMAL, ALL_DRIVERS},
  {t_bulk_insert_indicator, "t_bulk_insert_indicator", NORMAL, ALL_DRIVERS},
  {t_bulk_insert_rows, "t_bulk_insert_rows", NORMAL, ALL_DRIVERS},
  {t_odbc90, "odbc90_insert_with_ts_col", NORMAL, ALL_DRIVERS},
  {t_bulk_delete, "t_bulk_delete", NORMAL, ALL_DRIVERS},
  {t_odbc149, "odbc149_ts_col_insert" , NORMAL, ALL_DRIVERS},
  {bulk_insert_all_datatypes, "bulk_insert_all_datatypes" , NORMAL, ALL_DRIVERS},
  {unsupported_bulk_operations, "unsupported_bulk_operations" , NORMAL, ALL_DRIVERS},
  {bulk_load_data, "bulk_load_data" , CSPS_OK | SSPS_FAIL, ALL_DRIVERS},
  {bulk_ignore_column, "bulk_ignore_column" , NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
