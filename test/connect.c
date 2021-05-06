/*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
                2016, 2019 MariaDB Corporation AB
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

ODBC_TEST(basic_connect) {
  HSTMT hdbc;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));

  CHECK_DBC_RC(hdbc, SQLConnect(hdbc, my_dsn, SQL_NTS, my_uid, SQL_NTS, my_pwd, SQL_NTS));

  OK_SIMPLE_STMT(Stmt, "DROP USER IF EXISTS basic_connect");
  OK_SIMPLE_STMT(Stmt, "CREATE USER basic_connect@'%' IDENTIFIED BY 's3cureP@ss'")
  if (!SQL_SUCCEEDED(SQLExecDirect(Stmt, (SQLCHAR*)"GRANT ALL ON odbc_test.* TO basic_connect@'%'", SQL_NTS))) {
    if (get_native_errcode(Stmt) == 1142) {
      skip("test user does not have permissions to run this test (GRANT)");
    }
    return FAIL;
  }

  FAIL_IF(SQLConnect(hdbc, my_dsn, SQL_NTS,
                     (SQLCHAR*)"basic_connect", SQL_NTS,
                     (SQLCHAR*)"s3cureP@ss", SQL_NTS) != SQL_ERROR,
          "Expected failure on attempt to establish connection for handle with established connection");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "08002");

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  FAIL_IF(SQLConnect(hdbc, my_dsn, SQL_NTS,
                     (SQLCHAR*)"basic_connect", SQL_NTS,
                     (SQLCHAR*)"s3curePass", SQL_NTS) != SQL_ERROR,
          "Expected failure on attempt to establish connection with wrong password");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "28000");
  FAIL_IF(SQLConnect(hdbc, my_dsn, SQL_NTS,
                     (SQLCHAR*)"basic_connect", SQL_NTS,
                     (SQLCHAR*)"", SQL_NTS) != SQL_ERROR,
          "Expected failure on attempt to establish connection without password for user with password");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "28000");

  CHECK_DBC_RC(hdbc, SQLConnect(hdbc, my_dsn, SQL_NTS,
                                (SQLCHAR*)"basic_connect", SQL_NTS,
                                (SQLCHAR*)"s3cureP@ss", SQL_NTS));

  SQLCHAR* wrong_dsn[10];
  memcpy((char*)wrong_dsn, "wrong_dsn", 9);
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  FAIL_IF(SQLConnect(hdbc, wrong_dsn, SQL_NTS,
                     (SQLCHAR*)"basic_connect", SQL_NTS,
                     (SQLCHAR*)"s3cureP@ss", SQL_NTS) != SQL_ERROR,
          "Expected failure on connecting to non-existing data source");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "IM002");

  FAIL_IF(SQLConnect(hdbc, my_dsn, SQL_NTS,
                     (SQLCHAR*)"wrong_connect", SQL_NTS,
                     (SQLCHAR*)"s3cureP@ss", SQL_NTS) != SQL_ERROR,
          "Expected failure on connecting with non-existing user");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "28000");

  FAIL_IF(SQLConnect(hdbc, my_dsn, -2,
                     (SQLCHAR*)"wrong_connect", -2,
                     (SQLCHAR*)"s3cureP@ss", -2) != SQL_ERROR,
          "Expected failure on passing negative number not equal to SQL_NTS as length");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY090");
  return OK;
}

ODBC_TEST(driver_connect_simple) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];
  SQLSMALLINT conn_out_len;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  sprintf((char*)conn, "DSN=%s;UID=%s;PWD=%s", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      NULL, 0, &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  is_num(conn_out_len, strlen((char*)conn));

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, sizeof(conn_out),
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  is_num(conn_out_len, sizeof(conn_out));
  IS_STR(conn, conn_out, conn_out_len);

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_COMPLETE));
  is_num(conn_out_len, strlen((char*)conn));
  IS_STR(conn, conn_out, conn_out_len);

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_COMPLETE_REQUIRED));
  is_num(conn_out_len, strlen((char*)conn));
  IS_STR(conn, conn_out, conn_out_len);

  return OK;
}

ODBC_TEST(driver_connect_savefile) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];
  SQLSMALLINT conn_out_len;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  OK_SIMPLE_STMT(Stmt, "DROP USER IF EXISTS driver_connect_savefile");
  OK_SIMPLE_STMT(Stmt, "CREATE USER driver_connect_savefile@'%' IDENTIFIED BY 's3cureP@ss'")
  if (!SQL_SUCCEEDED(SQLExecDirect(Stmt,
      (SQLCHAR*)"GRANT ALL ON odbc_test.* TO driver_connect_savefile@'%'", SQL_NTS))) {
    if (get_native_errcode(Stmt) == 1142) {
      skip("test user does not have permissions to run this test (GRANT)");
    }
    return FAIL;
  }

  sprintf((char*)conn, "SAVEFILE=driver_connect;DSN=%s;DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s",
            my_dsn, my_drivername, "driver_connect_savefile", "s3cureP@ss", my_servername, my_port, my_schema);

  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));

  sprintf((char*)conn, "FILEDSN=driver_connect;PWD=%s", my_pwd);
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));

  sprintf((char*)conn, "FILEDSN=driver_connect");
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "saved dsn file should not include password");
}

MA_ODBC_TESTS my_tests[]=
{
  {basic_connect, "basic_connect",     NORMAL, ALL_DRIVERS},
  {driver_connect_simple, "driver_connect_simple",     NORMAL, ALL_DRIVERS},
  {driver_connect_savefile, "driver_connect_savefile",     TO_FIX, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;

  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}