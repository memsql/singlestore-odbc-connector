/*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
                2013, 2017 MariaDB Corporation AB
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
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_WCHAR, rowsInsert[0].c_wchar, BUFF_SIZE*sizeof(SQLWCHAR), iOdbc() ? &rowsSelect[0].c_wchar_len : NULL));
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
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 18, SQL_C_TYPE_TIMESTAMP, &rowsInsert[0].c_type_timestamp, 0, iOdbc() ? &rowsInsert[0].c_timestamp_len : NULL));
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 19, SQL_C_NUMERIC, &rowsInsert[0].c_numeric, 0, NULL));

    // Perform bulk insert. It should insert NUM_ROWS new rows to the table
    CHECK_STMT_RC(Stmt, SQLBulkOperations(Stmt, SQL_ADD));
    /*
    is_num(rowsFetched, NUM_ROWS);
    for (i = 0; i < NUM_ROWS; i++) {
      is_num(rowStatusArray[i], SQL_ROW_SUCCESS);  // TODO PLAT-5555
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
    CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 18, SQL_C_TYPE_TIMESTAMP, &rowsSelect[0].c_type_timestamp, 0, iOdbc() ? &rowsSelect[0].c_timestamp_len : NULL));
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
      if (!(iOdbc() && is_ansi_driver())) {
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


MA_ODBC_TESTS my_tests[]=
{
  {bulk_insert_all_datatypes, "bulk_insert_all_datatypes",      NORMAL, ALL_DRIVERS},
  {NULL, NULL, 0, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
