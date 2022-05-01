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
#include "stdio.h"
#include <wchar.h>
#if !defined(_WIN32) && !defined(__APPLE__)
#include <glib.h>
#include <libsecret/secret.h>
#endif

ODBC_TEST(basic_connect) {
  HSTMT hdbc;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));

  CHECK_DBC_RC(hdbc, SQLConnect(hdbc, my_dsn, SQL_NTS, my_uid, SQL_NTS, my_pwd, SQL_NTS));

  OK_SIMPLE_STMT(Stmt, "DROP USER IF EXISTS basic_connect");
  OK_SIMPLE_STMT(Stmt, "CREATE USER basic_connect@'%' IDENTIFIED BY 's3cureP@ss'");
  OK_SIMPLE_STMT(Stmt, "GRANT ALL ON odbc_test.* TO basic_connect@'%'");

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

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  FAIL_IF(SQLConnect(hdbc, (SQLCHAR*)"wrong_dsn", SQL_NTS,
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

  // S1090 is returned on MacOS instead of HY090, but error message is correct
  //
  FAIL_IF(check_sqlstate_ex(hdbc, SQL_HANDLE_DBC, "HY090") != OK && check_sqlstate_ex(hdbc, SQL_HANDLE_DBC, "S1090") != OK,
      "Unexpected sqlstate!");
  return OK;
}

ODBC_TEST(basic_connect_w) {
  HSTMT hdbc;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));

  CHECK_DBC_RC(hdbc, SQLConnectW(hdbc, wdsn, SQL_NTS, wuid, SQL_NTS, wpwd, SQL_NTS));

  OK_SIMPLE_STMT(Stmt, "DROP USER IF EXISTS basic_connect");
  OK_SIMPLE_STMT(Stmt, "CREATE USER basic_connect@'%' IDENTIFIED BY 's3cureP@ss'");
  OK_SIMPLE_STMT(Stmt, "GRANT ALL ON odbc_test.* TO basic_connect@'%'");

  FAIL_IF(SQLConnectW(hdbc, wdsn, SQL_NTS,
                     CW("basic_connect"), SQL_NTS,
                     CW("s3cureP@ss"), SQL_NTS) != SQL_ERROR,
          "Expected failure on attempt to establish connection for handle with established connection");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "08002");

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  FAIL_IF(SQLConnectW(hdbc, wdsn, SQL_NTS,
                     CW("basic_connect"), SQL_NTS,
                     CW("s3curePass"), SQL_NTS) != SQL_ERROR,
          "Expected failure on attempt to establish connection with wrong password");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "28000");

  FAIL_IF(SQLConnectW(hdbc, wdsn, SQL_NTS,
                     CW("basic_connect"), SQL_NTS,
                     CW(""), SQL_NTS) != SQL_ERROR,
          "Expected failure on attempt to establish connection without password for user with password");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "28000");

  CHECK_DBC_RC(hdbc, SQLConnectW(hdbc, wdsn, SQL_NTS,
                                CW("basic_connect"), SQL_NTS,
                                CW("s3cureP@ss"), SQL_NTS));

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  FAIL_IF(SQLConnectW(hdbc, CW("wrong_dsn"), SQL_NTS,
                     CW("basic_connect"), SQL_NTS,
                     CW("s3cureP@ss"), SQL_NTS) != SQL_ERROR,
          "Expected failure on connecting to non-existing data source");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "IM002");

  FAIL_IF(SQLConnectW(hdbc, wdsn, SQL_NTS,
                     CW("wrong_connect"), SQL_NTS,
                     CW("s3cureP@ss"), SQL_NTS) != SQL_ERROR,
          "Expected failure on connecting with non-existing user");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "28000");

  FAIL_IF(SQLConnectW(hdbc, wdsn, -2,
                     CW("wrong_connect"), -2,
                     CW("s3cureP@ss"), -2) != SQL_ERROR,
          "Expected failure on passing negative number not equal to SQL_NTS as length");

  // S1090 is returned on MacOS instead of HY090, but error message is correct
  //
  FAIL_IF(check_sqlstate_ex(hdbc, SQL_HANDLE_DBC, "HY090") != OK && check_sqlstate_ex(hdbc, SQL_HANDLE_DBC, "S1090") != OK,
          "Unexpected sqlstate!");
  return OK;
}

int check_connection_string(size_t len, size_t expected_len, const SQLCHAR conn[1024], const SQLCHAR expected_conn[1024]) {
#ifndef _WIN32
  is_num(len, expected_len);
  IS_STR(conn, expected_conn, expected_len);
  return OK;
#endif
  // SQLDriverConnect function on Windows adds curly braces around driver name in connection string,
  // we should ignore this in order to check conn and conn_out equality.
  //

  SQLCHAR sanitized_conn[2048];
  size_t i, sanitized_len = 0;
  for (i = 0; i < len; i++) {
    if (strncmp((char*)(conn + i), "DRIVER=", 7) == 0) {
      if (conn[i + 7] == '{') {
        strncpy((char*)sanitized_conn, (char*)conn, i + 7);
        sanitized_len = i + 7;
        size_t j;
        for (j = i + 8; j < len; j++) {
          if (conn[j] == '}') {
            break;
          }
        }
        strncpy((char*)(sanitized_conn + sanitized_len), (char*)(conn + i + 8), (j - i - 8));
        sanitized_len += j - i - 8;
        if (j < len - 1) {
          strncpy((char*)(sanitized_conn + sanitized_len), (char*)(conn + j + 1), (len - j - 1));
          sanitized_len += len - j - 1;
        }
      } else {
        strncpy((char*)sanitized_conn, (char*)conn, len);
        sanitized_len = len;
      }
      break;
    }
  }
  if (sanitized_len == 0) {
    strncpy((char*)sanitized_conn, (char*)conn, len);
    sanitized_len = len;
  }
  is_num(sanitized_len, expected_len);
  IS_STR(sanitized_conn, expected_conn, sanitized_len);
  return OK;
}

int check_connection_string_w(size_t len, size_t expected_len, const SQLWCHAR conn[1024], const SQLWCHAR expected_conn[1024]) {
#ifndef _WIN32
  is_num(len, expected_len);
  IS_WSTR(conn, expected_conn, expected_len);
  return OK;
#endif
  // SQLDriverConnect function on Windows adds curly braces around driver name in connection string,
  // we should ignore this in order to check conn and conn_out equality.
  //

  SQLWCHAR sanitized_conn[1024];
  size_t i, sanitized_len = 0;
  for (i = 0; i < len; i++) {
    if (memcmp((conn + i), CW("DRIVER="), 7 * sizeof(SQLWCHAR)) == 0) {
      if (conn[i + 7] == L'{') {
        memcpy(sanitized_conn, conn, (i + 7) * sizeof(SQLWCHAR));
        sanitized_len = i + 7;
        size_t j;
        for (j = i + 8; j < len; j++) {
          if (conn[j] == L'}') {
            break;
          }
        }
        memcpy((sanitized_conn + sanitized_len), (conn + i + 8), (j - i - 8) * sizeof(SQLWCHAR));
        sanitized_len += j - i - 8;
        if (j < len - 1) {
          memcpy((sanitized_conn + sanitized_len), (conn + j + 1), (len - j - 1) * sizeof(SQLWCHAR));
          sanitized_len += len - j - 1;
        }
      } else {
        memcpy(sanitized_conn, conn, len * sizeof(SQLWCHAR));
        sanitized_len = len;
      }
      break;
    }
  }
  if (sanitized_len == 0) {
    memcpy(sanitized_conn, conn, len * sizeof(SQLWCHAR));
    sanitized_len = len;
  }
  is_num(sanitized_len, expected_len);
  IS_WSTR(sanitized_conn, expected_conn, sanitized_len);
  return OK;
}

ODBC_TEST(driver_connect_simple) {
  HSTMT hdbc, hstmt;
  SQLCHAR conn[1024], conn_out[1024], buff[1024];
  SQLSMALLINT conn_out_len;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  sprintf((char*)conn, "DSN=%s;UID=%s;PWD=%s;", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      NULL, 0, &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
#ifndef _WIN32
  is_num(conn_out_len, strlen((char*)conn));
#endif

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, strlen((char*)conn),
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  IS_STR(conn, conn_out, conn_out_len);

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_COMPLETE));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_COMPLETE_REQUIRED));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  sprintf((char*)conn, "DSN=%s;UID=%s;PWD=%s;DESCRIPTION=%s;", my_dsn, my_uid, my_pwd, "some description");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;CHARSET=%s;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, "utf8");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "SELECT * FROM INFORMATION_SCHEMA.GLOBAL_VARIABLES WHERE VARIABLE_NAME = 'CHARACTER_SET_CONNECTION'");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  IS_STR(my_fetch_str(hstmt, buff, 2), "utf8", sizeof("utf8"));
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));

  // For this test machine on which it is running has to have my.cnf file with section
  // [odbc]
  // database=odbc_test_mycnf
  //
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE IF NOT EXISTS odbc_test_mycnf");
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;USE_MYCNF=1;",
          my_drivername, my_uid, my_pwd, my_servername, my_port);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "SELECT DATABASE()");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  IS_STR(my_fetch_str(hstmt, buff, 1), "odbc_test_mycnf", sizeof("odbc_test_mycnf"));
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PORT=%u;DB=%s;NO_PROMPT=1;",
          my_drivername, my_uid, my_port, my_schema);
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_COMPLETE) != SQL_ERROR,
          "Should not be able to connect with incomplete parameters with NO_PROMPT");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");

#if !defined(_WIN32) && !defined(__APPLE__)
  sprintf((char*)conn, "DSN=%s;", my_dsn);
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                       conn_out, sizeof(conn_out), &conn_out_len,
                                       SQL_DRIVER_PROMPT) != SQL_ERROR, "Can't use SQL_DRIVER_PROMPT on Unix");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY092");

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PORT=%u;DB=%s;",
          my_drivername, my_uid, my_port, my_schema);
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_COMPLETE) != SQL_ERROR,
          "Should not be able to connect with incomplete parameters on Unix");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000"); // general error state
#endif

  return OK;
}

ODBC_TEST(driver_connect_simple_w) {
  HSTMT hdbc, hstmt;
  SQLCHAR conn[1024], buff[1024];
  SQLWCHAR conn_out[1024];
  SQLSMALLINT conn_out_len;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  sprintf((char*)conn, "DSN=%s;UID=%s;PWD=%s;", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      NULL, 0, &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
#ifndef _WIN32
  is_num(conn_out_len, strlen((char*)conn));
#endif

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), strlen((char*)conn),
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  IS_WSTR(CW(conn), conn_out, conn_out_len);

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_COMPLETE));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_COMPLETE_REQUIRED));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  sprintf((char*)conn, "DSN=%s;UID=%s;PWD=%s;DESCRIPTION=%s;", my_dsn, my_uid, my_pwd, "some description");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;CHARSET=%s;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, "utf8");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "SELECT * FROM INFORMATION_SCHEMA.GLOBAL_VARIABLES WHERE VARIABLE_NAME = 'CHARACTER_SET_CONNECTION'");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  IS_STR(my_fetch_str(hstmt, buff, 2), "utf8", sizeof("utf8"));
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));

  // For this test machine on which it is running has to have my.cnf file with section
  // [odbc]
  // database=odbc_test_mycnf
  //
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE IF NOT EXISTS odbc_test_mycnf");
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;USE_MYCNF=1;",
          my_drivername, my_uid, my_pwd, my_servername, my_port);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "SELECT DATABASE()");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  IS_STR(my_fetch_str(hstmt, buff, 1), "odbc_test_mycnf", sizeof("odbc_test_mycnf"));
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PORT=%u;DB=%s;NO_PROMPT=1;",
          my_drivername, my_uid, my_port, my_schema);
  FAIL_IF(SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_COMPLETE) != SQL_ERROR,
          "Should not be able to connect with incomplete parameters with NO_PROMPT");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");

#if !defined(_WIN32) && !defined(__APPLE__)
  sprintf((char*)conn, "DSN=%s;", my_dsn);
  FAIL_IF(SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_PROMPT) != SQL_ERROR, "Can't use SQL_DRIVER_PROMPT on Unix");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY092");

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PORT=%u;DB=%s;",
          my_drivername, my_uid, my_port, my_schema);
  FAIL_IF(SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_COMPLETE) != SQL_ERROR,
          "Should not be able to connect with incomplete parameters on Unix");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000"); // general error state
#endif

  return OK;
}

ODBC_TEST(driver_connect_trace) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;TRACE=%s;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, "test/test_trace.log");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }
  FILE* file = fopen("test/test_trace.log", "r");
  FAIL_IF(!file, "Trace file should have been created");
  fclose(file);

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

ODBC_TEST(driver_connect_unsupported) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];
  SQLSMALLINT conn_out_len;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;GARBAGE=%s;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, "garbage");
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_SUCCESS_WITH_INFO,
          "Should return SQL_SUCCESS_WITH_INFO when using parameters unsupported by SingleStore");
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "01S00");

  return OK;
}

ODBC_TEST(driver_connect_savefile) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];
  SQLSMALLINT conn_out_len;
  SQLRETURN rc;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  OK_SIMPLE_STMT(Stmt, "DROP USER IF EXISTS driver_connect_savefile");
  OK_SIMPLE_STMT(Stmt, "CREATE USER driver_connect_savefile@'%' IDENTIFIED BY 's3cureP@ss'")
  OK_SIMPLE_STMT(Stmt, "GRANT ALL ON odbc_test.* TO driver_connect_savefile@'%'");

  sprintf((char*)conn, "SAVEFILE=./driver_connect.dsn;DSN=%s;DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s",
            my_dsn, my_drivername, "driver_connect_savefile", "s3cureP@ss", my_servername, my_port, my_schema);

  rc = SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                            conn_out, sizeof(conn_out), &conn_out_len,
                            SQL_DRIVER_NOPROMPT);
  if (rc == SQL_SUCCESS) {
    sprintf((char *) conn, "FILEDSN=./driver_connect.dsn;PWD=%s", my_pwd);
    CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
    CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                        conn_out, sizeof(conn_out), &conn_out_len,
                                        SQL_DRIVER_NOPROMPT));

    sprintf((char *) conn, "FILEDSN=./driver_connect.dsn");
    CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
    FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                             conn_out, sizeof(conn_out), &conn_out_len,
                             SQL_DRIVER_NOPROMPT) != SQL_ERROR, "saved dsn file should not include password");
  } else if (rc != SQL_SUCCESS_WITH_INFO) {
    return FAIL;
  }
  return OK;
}

ODBC_TEST(driver_connect_ssl) {
  HSTMT hdbc, hstmt;
  SQLCHAR conn[1024], conn_out[1024], buff[1024];
  char version[128];
  SQLSMALLINT conn_out_len;
  SQLRETURN rc;

  OK_SIMPLE_STMT(Stmt, "SELECT @@memsql_version");
  CHECK_STMT_RC(hstmt, SQLFetch(Stmt));
  my_fetch_str(Stmt, (SQLCHAR *) version, 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &hdbc));
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;",
      my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema,
      "ssl/test-memsql-cert.pem", "ssl/test-memsql-key.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));

  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  OK_SIMPLE_STMT(Stmt, "DROP USER IF EXISTS driver_connect_ssl");
  OK_SIMPLE_STMT(Stmt, "CREATE USER driver_connect_ssl@'%' REQUIRE SSL")
  OK_SIMPLE_STMT(Stmt, "GRANT ALL ON odbc_test.* TO driver_connect_ssl@'%'");

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-ca-cert.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;SSLCA=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem",
          "ssl/test-ca-cert.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCAPATH=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/capath");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;SSLCIPHER=%s;NO_SSPS=1;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem",
          "AES128-GCM-SHA256");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                         conn_out, sizeof(conn_out), &conn_out_len,
                                         SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  // `SHOW STATUS` queries work only in client-side mode
  //
  if (NoSsps) {
    CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
    OK_SIMPLE_STMT(hstmt, "SHOW STATUS LIKE 'Ssl_cipher'");
    CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
    IS_STR(my_fetch_str(hstmt, buff, 2), "AES128-GCM-SHA256", strlen("AES128-GCM-SHA256"));
    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;TLSVERSION=%s;NO_SSPS=1;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem",
          "TLSv1.2");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                         conn_out, sizeof(conn_out), &conn_out_len,
                                         SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  // `SHOW STATUS` queries work only in client-side mode
  //
  if (NoSsps) {
    CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
    OK_SIMPLE_STMT(hstmt, "SHOW STATUS LIKE 'Ssl_version'");
    CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
    IS_STR(my_fetch_str(hstmt, buff, 2), "TLSv1.2", strlen("TLSv1.2"));
    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;FORCETLS=1;NO_SSPS=1;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                         conn_out, sizeof(conn_out), &conn_out_len,
                                         SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  // `SHOW STATUS` queries work only in client-side mode
  //
  if (NoSsps) {
    CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
    OK_SIMPLE_STMT(hstmt, "SHOW STATUS LIKE 'Ssl_version'");
    CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
    IS_STR(my_fetch_str(hstmt, buff, 2), "TLS", strlen("TLS")); // Checking if ssl version in use starts with TLS
    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;SSLCA=%s;SSLVERIFY=1;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem",
          "ssl/test-ca-cert.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                         conn_out, sizeof(conn_out), &conn_out_len,
                                         SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

#ifndef WIN32 // TODO PLAT-5661
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;TLSKEYPWD=%s",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-memsql-pass-cert.pem",
          "ssl/test-memsql-pass-key.pem",
          "secure_password");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                         conn_out, sizeof(conn_out), &conn_out_len,
                                         SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
#endif
  // In case of ever changing test certificates please generate new valid fingerprint using following command:
  // openssl x509 -noout -fingerprint -sha1 -inform pem -in ssl/test-memsql-cert.pem
  //
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;SSLVERIFY=1;TLSPEERFP=%s;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-ca-cert.pem",
          "7B:DD:F2:86:1B:7B:C5:71:66:2A:CD:A1:E9:9B:D4:6F:6B:F3:4D:A5");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                         conn_out, sizeof(conn_out), &conn_out_len,
                                         SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  // In case of ever changing test certificates please add generated fingerprint to ssl/fplist.txt
  //
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;SSLVERIFY=1;TLSPEERFPLIST=%s;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-ca-cert.pem",
          "ssl/fplist.txt");
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                         conn_out, sizeof(conn_out), &conn_out_len,
                                         SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
#ifndef WIN32 // TODO PLAT-5661
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;TLSKEYPWD=%s",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-memsql-pass-cert.pem",
          "ssl/test-memsql-pass-key.pem",
          "wrong_password");
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "Should not be able to connect with wrong key password");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");
#endif
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;SSLVERIFY=1;TLSPEERFP=%s;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-ca-cert.pem",
          "7B:DD:F2:86:1B:7B:C5:71:66:2A:CD:A1:E9:9B:D4:6F:6B:F3:4D:A4");
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "Should not be able to connect with wrong fingerprint");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;SSLVERIFY=1;TLSPEERFPLIST=%s;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-ca-cert.pem",
          "ssl/wrongfplist.txt");
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "Should not be able to connect with wrong fingerprint");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema);
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "Should not be able to connect without SSL for "
                                                              "SSL required user");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, strcmp(version, "7.0.15") ? "HY000" : "28000");


  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;FORCE_TLS=1;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema);
  SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                       conn_out, sizeof(conn_out), &conn_out_len,
                       SQL_DRIVER_NOPROMPT);
  return OK;
}

ODBC_TEST(driver_connect_ssl_w) {
  HSTMT hdbc, hstmt;
  SQLCHAR conn[1024], buff[1024];
  char version[128];
  SQLWCHAR conn_out[1024];
  SQLSMALLINT conn_out_len;
  SQLRETURN rc;

  OK_SIMPLE_STMT(Stmt, "SELECT @@memsql_version");
  CHECK_STMT_RC(hstmt, SQLFetch(Stmt));
  my_fetch_str(Stmt, (SQLCHAR *) version, 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &hdbc));
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem", "ssl/test-memsql-key.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                        conn_out, sizeof(conn_out), &conn_out_len,
                        SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  OK_SIMPLE_STMT(Stmt, "DROP USER IF EXISTS driver_connect_ssl");
  OK_SIMPLE_STMT(Stmt, "CREATE USER driver_connect_ssl@'%' REQUIRE SSL")
  OK_SIMPLE_STMT(Stmt, "GRANT ALL ON odbc_test.* TO driver_connect_ssl@'%'");

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-ca-cert.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;SSLCA=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem",
          "ssl/test-ca-cert.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCAPATH=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/capath");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;SSLCIPHER=%s;NO_SSPS=1;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem",
          "AES128-GCM-SHA256");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  // `SHOW STATUS` queries work only in client-side mode
  //
  if (NoSsps) {
    CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
    OK_SIMPLE_STMT(hstmt, "SHOW STATUS LIKE 'Ssl_cipher'");
    CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
    IS_STR(my_fetch_str(hstmt, buff, 2), "AES128-GCM-SHA256", strlen("AES128-GCM-SHA256"));
    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;TLSVERSION=%s;NO_SSPS=1;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem",
          "TLSv1.2");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  // `SHOW STATUS` queries work only in client-side mode
  //
  if (NoSsps) {
    CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
    OK_SIMPLE_STMT(hstmt, "SHOW STATUS LIKE 'Ssl_version'");
    CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
    IS_STR(my_fetch_str(hstmt, buff, 2), "TLSv1.2", strlen("TLSv1.2"));
    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;FORCETLS=1;NO_SSPS=1;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  // `SHOW STATUS` queries work only in client-side mode
  //
  if (NoSsps) {
    CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
    OK_SIMPLE_STMT(hstmt, "SHOW STATUS LIKE 'Ssl_version'");
    CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
    IS_STR(my_fetch_str(hstmt, buff, 2), "TLS", strlen("TLS")); // Checking if ssl version in use starts with TLS
    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;SSLCA=%s;SSLVERIFY=1;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-memsql-cert.pem",
          "ssl/test-memsql-key.pem",
          "ssl/test-ca-cert.pem");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

#ifndef WIN32 // TODO PLAT-5661
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;TLSKEYPWD=%s",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-memsql-pass-cert.pem",
          "ssl/test-memsql-pass-key.pem",
          "secure_password");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
#endif
  // In case of ever changing test certificates please generate new valid fingerprint using following command:
  // openssl x509 -noout -fingerprint -sha1 -inform pem -in ssl/test-memsql-cert.pem
  //
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;SSLVERIFY=1;TLSPEERFP=%s;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-ca-cert.pem",
          "7B:DD:F2:86:1B:7B:C5:71:66:2A:CD:A1:E9:9B:D4:6F:6B:F3:4D:A5");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  // In case of ever changing test certificates please add generated fingerprint to ssl/fplist.txt
  //
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;SSLVERIFY=1;TLSPEERFPLIST=%s;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-ca-cert.pem",
          "ssl/fplist.txt");
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
#ifndef WIN32 // TODO PLAT-5661
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCERT=%s;SSLKEY=%s;TLSKEYPWD=%s",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-memsql-pass-cert.pem",
          "ssl/test-memsql-pass-key.pem",
          "wrong_password");
  FAIL_IF(SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "Should not be able to connect with wrong key password");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");
#endif
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;SSLVERIFY=1;TLSPEERFP=%s;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-ca-cert.pem",
          "7B:DD:F2:86:1B:7B:C5:71:66:2A:CD:A1:E9:9B:D4:6F:6B:F3:4D:A4");
  FAIL_IF(SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "Should not be able to connect with wrong fingerprint");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;SSLCA=%s;SSLVERIFY=1;TLSPEERFPLIST=%s;",
          my_drivername, "driver_connect_ssl", "", "test-memsql-server", my_port, my_schema,
          "ssl/test-ca-cert.pem",
          "ssl/wrongfplist.txt");
  FAIL_IF(SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "Should not be able to connect with wrong fingerprint");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");

  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;",
          my_drivername, "driver_connect_ssl", "", my_servername, my_port, my_schema);
  FAIL_IF(SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                           conn_out, sizeof(conn_out), &conn_out_len,
                           SQL_DRIVER_NOPROMPT) != SQL_ERROR, "Should not be able to connect without SSL for "
                                                              "SSL required user");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, strcmp(version, "7.0.15") ? "HY000" : "28000");


  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;FORCE_TLS=1;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema);
  SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                   conn_out, sizeof(conn_out), &conn_out_len,
                   SQL_DRIVER_NOPROMPT);
  return OK;
}

ODBC_TEST(driver_connect_initstmt) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];
  SQLSMALLINT conn_out_len;

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS driver_connect_initstmt")
  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  sprintf((char *) conn, "DSN=%s;UID=%s;PWD=%s;INITSTMT=CREATE DATABASE driver_connect_initstmt;",
      my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }
  OK_SIMPLE_STMT(Stmt, "USE driver_connect_initstmt");
  return OK;
}

ODBC_TEST(driver_connect_initstmt_w) {
  HSTMT hdbc;
  SQLCHAR conn[1024];
  SQLWCHAR conn_out[1024];
  SQLSMALLINT conn_out_len;

  OK_SIMPLE_STMT(Stmt, "DROP DATABASE IF EXISTS driver_connect_initstmt")
  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  sprintf((char *) conn, "DSN=%s;UID=%s;PWD=%s;INITSTMT=CREATE DATABASE driver_connect_initstmt;",
      my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }
  OK_SIMPLE_STMT(Stmt, "USE driver_connect_initstmt");
  return OK;
}

ODBC_TEST(driver_connect_timeout) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];
  SQLSMALLINT conn_out_len;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;CONN_TIMEOUT=1",
          my_drivername, "root-ssl", my_pwd, my_servername, my_port, my_schema);
  time_t before = clock();
  if (SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, sizeof(conn_out), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT) == SQL_ERROR) {
    time_t elapsed = clock() - before;
    long elapsed_milliseconds = (elapsed * 1000) / CLOCKS_PER_SEC;
    char err_msg[128];
    sprintf(err_msg, "Connection should have been timed out in 1 s, instead it failed in %ld ms", elapsed_milliseconds);
    FAIL_IF(elapsed_milliseconds > 2000, err_msg);
  } else {
    if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }
  }

  return OK;
}

ODBC_TEST(driver_connect_timeout_w) {
  HSTMT hdbc;
  SQLCHAR conn[1024];
  SQLWCHAR conn_out[1024];
  SQLSMALLINT conn_out_len;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;CONN_TIMEOUT=1",
          my_drivername, "root-ssl", my_pwd, my_servername, my_port, my_schema);
  time_t before = clock();
  if (SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                        conn_out, sizeof(conn_out), &conn_out_len,
                        SQL_DRIVER_NOPROMPT) == SQL_ERROR) {
    time_t elapsed = clock() - before;
    long elapsed_milliseconds = (elapsed * 1000) / CLOCKS_PER_SEC;
    char err_msg[128];
    sprintf(err_msg, "Connection should have been timed out in 1 s, instead it failed in %ld ms", elapsed_milliseconds);
    FAIL_IF(elapsed_milliseconds > 2000, err_msg);
  } else {
    if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }
  }

  return OK;
}

int test_option_matched_rows(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {

  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_matched_rows");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_matched_rows (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_matched_rows VALUES (1), (2)");
  OK_SIMPLE_STMT(hstmt, "UPDATE test_option_matched_rows SET a = 1 WHERE a < 3");

  SQLLEN row_count;
  CHECK_STMT_RC(hstmt, SQLRowCount(hstmt, &row_count));
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  if (options & target_bit) {
    FAIL_IF_NE_INT(2, (int)row_count, "Should have 2 matched rows");
  } else {
    FAIL_IF_NE_INT(1, (int)row_count, "Should have 1 changed row");
  }
  return OK;
}

int test_option_matched_rows_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {

  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLWCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_matched_rows");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_matched_rows (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_matched_rows VALUES (1), (2)");
  OK_SIMPLE_STMT(hstmt, "UPDATE test_option_matched_rows SET a = 1 WHERE a < 3");

  SQLLEN row_count;
  CHECK_STMT_RC(hstmt, SQLRowCount(hstmt, &row_count));
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  if (options & target_bit) {
    FAIL_IF_NE_INT(2, (int)row_count, "Should have 2 matched rows");
  } else {
    FAIL_IF_NE_INT(1, (int)row_count, "Should have 1 changed row");
  }
  return OK;
}

int test_option_no_prompt(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  if (!(options & target_bit)) {
    return OK;
  }
  SQLSMALLINT conn_out_len;
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PORT=%u;DB=%s;OPTIONS=%d;",
          my_drivername, my_uid, my_port, my_schema, options);
  FAIL_IF(SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                           conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                           SQL_DRIVER_COMPLETE) != SQL_ERROR,
          "Should not be able to connect with incomplete parameters with NO_PROMPT");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");
  return OK;
}

int test_option_no_prompt_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  if (!(options & target_bit)) {
    return OK;
  }
  SQLSMALLINT conn_out_len;
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PORT=%u;DB=%s;OPTIONS=%d;",
          my_drivername, my_uid, my_port, my_schema, options);
  FAIL_IF(SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                           conn_out, 1024 * sizeof(SQLWCHAR), &conn_out_len,
                           SQL_DRIVER_COMPLETE) != SQL_ERROR,
          "Should not be able to connect with incomplete parameters with NO_PROMPT");
  CHECK_SQLSTATE_EX(hdbc, SQL_HANDLE_DBC, "HY000");
  return OK;
}

int test_option_dynamic_cursor(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_dynamic_cursor");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_dynamic_cursor (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_dynamic_cursor VALUES (0), (1), (2), (3), (4)");

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_dynamic_cursor ORDER BY a");
  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
  CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0));

  SQLULEN cursor_type;
  CHECK_STMT_RC(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&cursor_type, 0, NULL));
  if (options & target_bit && !(options & (1 << 21))) { // 1 << 21 is FORWARDONLY
    is_num(cursor_type, SQL_CURSOR_DYNAMIC);
  } else {
    FAIL_IF(cursor_type == (SQLULEN)SQL_CURSOR_DYNAMIC,
        "Should not be able to make cursor dynamic without option provided or with FORWARDONLY enforced")
  }

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_dynamic_cursor");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_dynamic_cursor_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLWCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_dynamic_cursor");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_dynamic_cursor (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_dynamic_cursor VALUES (0), (1), (2), (3), (4)");

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_dynamic_cursor ORDER BY a");
  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
  CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0));

  SQLULEN cursor_type;
  CHECK_STMT_RC(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&cursor_type, 0, NULL));
  if (options & target_bit && !(options & (1 << 21))) {
    is_num(cursor_type, SQL_CURSOR_DYNAMIC);
  } else {
    FAIL_IF(cursor_type == (SQLULEN)SQL_CURSOR_DYNAMIC,
        "Should not be able to make cursor dynamic without option provided or with FORWARDONLY enforced")
  }

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_dynamic_cursor");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_no_schema(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  return OK; // TODO: NO_SCHEMA option has no effect on execution. Remove this after the bug is fixed

  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "CREATE DATABASE IF NOT EXISTS test_option_no_schema");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_no_schema.test_table (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_no_schema.test_table VALUES (1)");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_table (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_table VALUES (2)");
  OK_SIMPLE_STMT(hstmt, "SELECT test_option_no_schema.test_table.a FROM "
                        "test_option_no_schema.test_table JOIN odbc_test.test_table");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  if (options & target_bit) {
    is_num(my_fetch_int(hstmt, 1), 2);
  } else {
    is_num(my_fetch_int(hstmt, 1), 1);
  }

  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_no_schema.test_table");
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_table");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_no_schema_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  return OK; // TODO: NO_SCHEMA option has no effect on execution. Remove this after the bug is fixed

  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLWCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "CREATE DATABASE IF NOT EXISTS test_option_no_schema");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_no_schema.test_table (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_no_schema.test_table VALUES (1)");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_table (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_table VALUES (2)");
  OK_SIMPLE_STMT(hstmt, "SELECT test_option_no_schema.test_table.a FROM "
                        "test_option_no_schema.test_table JOIN odbc_test.test_table");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  if (options & target_bit) {
    is_num(my_fetch_int(hstmt, 1), 2);
  } else {
    is_num(my_fetch_int(hstmt, 1), 1);
  }

  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_no_schema.test_table");
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_table");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_compress(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  // This test verifies that compression flag does not corrupt execution but it does not verify that compression is
  // actually happening. TODO: verify with engine team whether this option makes any sense for SingleStore
  //
  if (!(options & target_bit)) {
    return OK;
  }
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_compress");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE IF NOT EXISTS test_option_compress (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_compress VALUES (1)");
  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_compress");

  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 1);
  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_compress");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_compress_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  // This test verifies that compression flag does not corrupt execution but it does not verify that compression is
  // actually happening. TODO: verify with engine team whether this option makes any sense for SingleStore
  //
  if (!(options & target_bit)) {
    return OK;
  }
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLWCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_compress");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE IF NOT EXISTS test_option_compress (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_compress VALUES (1)");
  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_compress");

  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 1);
  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_compress");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_use_mycnf(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  // For this test machine on which it is running has to have my.cnf file with section
  // [odbc]
  // database=odbc_test_mycnf
  //
  SQLSMALLINT conn_out_len;
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE IF NOT EXISTS odbc_test_mycnf");
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;OPTION=%d;NO_SSPS=0",
          my_drivername, my_uid, my_pwd, my_servername, my_port, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "SELECT DATABASE()");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  SQLCHAR buff[1024];
  if (options & target_bit) {
    IS_STR(my_fetch_str(hstmt, buff, 1), "odbc_test_mycnf", sizeof("odbc_test_mycnf"));
  } else {
    IS_STR(my_fetch_str(hstmt, buff, 1), "", sizeof(""));
  }
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS odbc_test_mycnf");
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}


int test_option_use_mycnf_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  // For this test machine on which it is running has to have my.cnf file with section
  // [odbc]
  // database=odbc_test_mycnf
  //
  SQLSMALLINT conn_out_len;
  OK_SIMPLE_STMT(Stmt, "CREATE DATABASE IF NOT EXISTS odbc_test_mycnf");
  sprintf((char*)conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;OPTION=%d;NO_SSPS=0",
          my_drivername, my_uid, my_pwd, my_servername, my_port, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "SELECT DATABASE()");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  SQLCHAR buff[1024];
  if (options & target_bit) {
    IS_STR(my_fetch_str(hstmt, buff, 1), "odbc_test_mycnf", sizeof("odbc_test_mycnf"));
  } else {
    IS_STR(my_fetch_str(hstmt, buff, 1), "", sizeof(""));
  }
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS odbc_test_mycnf");
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_forwardonly_cursor(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_forwardonly_cursor");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_forwardonly_cursor (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_forwardonly_cursor VALUES (0), (1), (2), (3), (4)");

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_forwardonly_cursor ORDER BY a");

  SQLULEN cursor_type;

  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
  CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0));
  CHECK_STMT_RC(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&cursor_type, 0, NULL));
  if (options & target_bit) {
    is_num(cursor_type, SQL_CURSOR_FORWARD_ONLY);
  } else {
    is_num(cursor_type, SQL_CURSOR_STATIC);
  }

  FAIL_IF(SQL_SUCCEEDED(SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 3)),
          "only SQL_FETCH_NEXT is allowed for a FORWARD_ONLY cursor");

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_forwardonly_cursor");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_forwardonly_cursor_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_forwardonly_cursor");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_forwardonly_cursor (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_forwardonly_cursor VALUES (0), (1), (2), (3), (4)");

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_forwardonly_cursor ORDER BY a");

  SQLULEN cursor_type;

  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
  CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0));
  CHECK_STMT_RC(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&cursor_type, 0, NULL));
  if (options & target_bit) {
    is_num(cursor_type, SQL_CURSOR_FORWARD_ONLY);
  } else {
    is_num(cursor_type, SQL_CURSOR_STATIC);
  }

  FAIL_IF(SQL_SUCCEEDED(SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 3)),
          "only SQL_FETCH_NEXT is allowed for a FORWARD_ONLY cursor");

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_forwardonly_cursor");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_auto_reconnect(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  // This test verifies that auto_reconnect flag does not corrupt execution but it does not verify that it actually
  // has any effect, because we don't have a mechanism to crash a connection from inside the C code.
  //
  if (!(options & target_bit)) {
    return OK;
  }
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_auto_reconnect");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE IF NOT EXISTS test_option_auto_reconnect (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_auto_reconnect VALUES (1)");
  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_auto_reconnect");

  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 1);
  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_auto_reconnect");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_auto_reconnect_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  // This test verifies that auto_reconnect flag does not corrupt execution but it does not verify that it actually
  // has any effect, because we don't have a mechanism to crash a connection from inside the C code.
  //
  if (!(options & target_bit)) {
    return OK;
  }
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_auto_reconnect");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE IF NOT EXISTS test_option_auto_reconnect (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_auto_reconnect VALUES (1)");
  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_auto_reconnect");

  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 1);
  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_auto_reconnect");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_multistatements(SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_multistatements");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_multistatements (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_multistatements VALUES (0), (1), (2), (3), (4)");

  if (!(options & target_bit)) {
    FAIL_IF(SQLExecDirect(hstmt,
            (SQLCHAR*)"SELECT a FROM test_option_multistatements WHERE a = 1; SELECT a FROM test_option_multistatements WHERE a > 2 ORDER BY a",
            SQL_NTS) != SQL_ERROR,
        "Should not be able to execute multistatements on connection without this option");
    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
    CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
    return OK;
  }

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_multistatements WHERE a = 1; SELECT a FROM test_option_multistatements WHERE a > 2 ORDER BY a");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 1);
  FAIL_IF(SQLFetch(hstmt) != SQL_NO_DATA, "SQL_NO_DATA expected");

  CHECK_STMT_RC(hstmt, SQLMoreResults(hstmt));
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 3);
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 4);
  FAIL_IF(SQLFetch(hstmt) != SQL_NO_DATA, "SQL_NO_DATA expected");
  FAIL_IF(SQLMoreResults(hstmt) != SQL_NO_DATA, "SQL_NO_DATA expected");

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_multistatements");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int test_option_multistatements_w(SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int options, int target_bit) {
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;OPTION=%d;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema, options);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_multistatements");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE test_option_multistatements (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO test_option_multistatements VALUES (0), (1), (2), (3), (4)");

  if (!(options & target_bit)) {
    FAIL_IF(SQLExecDirect(hstmt,
            (SQLCHAR*)"SELECT a FROM test_option_multistatements WHERE a = 1; SELECT a FROM test_option_multistatements WHERE a > 2 ORDER BY a",
            SQL_NTS) != SQL_ERROR,
        "Should not be able to execute multistatements on connection without this option");
    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
    CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
    return OK;
  }

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM test_option_multistatements WHERE a = 1; SELECT a FROM test_option_multistatements WHERE a > 2 ORDER BY a");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 1);
  FAIL_IF(SQLFetch(hstmt) != SQL_NO_DATA, "SQL_NO_DATA expected");

  CHECK_STMT_RC(hstmt, SQLMoreResults(hstmt));
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 3);
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));
  is_num(my_fetch_int(hstmt, 1), 4);
  FAIL_IF(SQLFetch(hstmt) != SQL_NO_DATA, "SQL_NO_DATA expected");
  FAIL_IF(SQLMoreResults(hstmt) != SQL_NO_DATA, "SQL_NO_DATA expected");

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS test_option_multistatements");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

int (*test_option_functions[]) (SQLCHAR[1024], SQLCHAR[1024], HSTMT, int, int) = {
    test_option_matched_rows,
    test_option_no_prompt,
    test_option_dynamic_cursor,
    test_option_no_schema,
    test_option_compress,
    test_option_use_mycnf,
    test_option_forwardonly_cursor,
    test_option_auto_reconnect,
    test_option_multistatements
};

int (*test_option_functions_w[]) (SQLCHAR[1024], SQLWCHAR[1024], HSTMT, int, int) = {
    test_option_matched_rows_w,
    test_option_no_prompt_w,
    test_option_dynamic_cursor_w,
    test_option_no_schema_w,
    test_option_compress_w,
    test_option_use_mycnf_w,
    test_option_forwardonly_cursor_w,
    test_option_auto_reconnect_w,
    test_option_multistatements_w
};

int count_bits(int mask) {
#ifndef _WIN32
  return __builtin_popcount(mask);
#endif
  int result = 0;
  while (mask) {
    result += (mask % 2);
    mask /= 2;
  }
  return result;
}

int recurrently_check_options(
    SQLCHAR conn[1024], SQLCHAR conn_out[1024], HSTMT hdbc, int* option_bits, int option_count, int current_options_mask, int pos) {
  if (pos == option_count) {
    int i;
    // We have no reason to run tests on all options combinations at every run. Randomly run option combination
    // with 0.02 probability of run. Also always run combinations with only one option set and with all/no options set
    //
    if (count_bits(current_options_mask) == 0 ||
        count_bits(current_options_mask) == 1 ||
        count_bits(current_options_mask) == option_count ||
        rand() < RAND_MAX * 0.02) {
      for (i = 0; i < option_count; i++) {
        if ((*test_option_functions[i])(conn, conn_out, hdbc, current_options_mask, option_bits[i]) == FAIL) {
          printf("Failed test with option %d\n", current_options_mask);
          return FAIL;
        }
      }
    }
    return OK;
  }
  if (recurrently_check_options(conn, conn_out, hdbc, option_bits, option_count, current_options_mask, pos + 1) == FAIL) {
    return FAIL;
  }
  if (recurrently_check_options(conn, conn_out, hdbc, option_bits, option_count, current_options_mask | option_bits[pos], pos + 1) == FAIL) {
    return FAIL;
  }
  return OK;
}

int recurrently_check_options_w(
    SQLCHAR conn[1024], SQLWCHAR conn_out[1024], HSTMT hdbc, int* option_bits, int option_count, int current_options_mask, int pos) {
  if (pos == option_count) {
    int i;
    // We have no reason to run tests on all options combinations at every run. Randomly run option combination
    // with 0.02 probability of run. Also always run combinations with only one option set and with all/no options set
    //
    if (count_bits(current_options_mask) == 0 ||
        count_bits(current_options_mask) == 1 ||
        count_bits(current_options_mask) == option_count ||
        rand() < RAND_MAX * 0.02) {
      for (i = 0; i < option_count; i++) {
        if ((*test_option_functions_w[i])(conn, conn_out, hdbc, current_options_mask, option_bits[i]) == FAIL) {
          printf("Failed test with option %d\n", current_options_mask);
          return FAIL;
        }
      }
    }
    return OK;
  }
  if (recurrently_check_options_w(conn, conn_out, hdbc, option_bits, option_count, current_options_mask, pos + 1) == FAIL) {
    return FAIL;
  }
  if (recurrently_check_options_w(conn, conn_out, hdbc, option_bits, option_count, current_options_mask | option_bits[pos], pos + 1) == FAIL) {
    return FAIL;
  }
  return OK;
}

int option_bits[] = {1 << 1,   // Tells connector to return the number of matched rows instead of number of changed rows
                     1 << 4,   // Sets NO_PROMPT=1
                     1 << 5,   // Forces use of dynamic cursor
                     1 << 6,   // Forbids the use of database.tablename.column syntax
                     1 << 11,  // Tells connector to use compression protocol
                     // TODO: Add tests for NamedPipe option 1 << 13, relevant only for Windows
                     1 << 16,  // Sets USE_MYCNF=1
                     1 << 21,  // Sets FORWARDONLY=1
                     1 << 22,  // Sets AUTO_RECONNECT=1
                     1 << 26   // Allows to send multiple statements in one query
};

ODBC_TEST(driver_connect_options) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];

  srand(time(NULL));

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));

  if (recurrently_check_options(conn, conn_out, hdbc, option_bits, 9, 0, 0) == FAIL) {
    return FAIL;
  }

  return OK;
}

ODBC_TEST(driver_connect_options_w) {
  HSTMT hdbc;
  SQLCHAR conn[1024];
  SQLWCHAR conn_out[1024];

  srand(time(NULL));

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  if (recurrently_check_options_w(conn, conn_out, hdbc, option_bits, 9, 0, 0) == FAIL) {
    return FAIL;
  }

  return OK;
}

ODBC_TEST(driver_connect_forwardonly) {
    HSTMT hdbc;
    SQLCHAR conn[1024], conn_out[1024];

    CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
    SQLSMALLINT conn_out_len;
    sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;FORWARDONLY=1;",
    my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema);
    CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                        conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                        SQL_DRIVER_NOPROMPT));
    if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

    HSTMT hstmt;
    CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
    OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS driver_connect_forwardonly");
    OK_SIMPLE_STMT(hstmt, "CREATE TABLE driver_connect_forwardonly (a INT)");
    OK_SIMPLE_STMT(hstmt, "INSERT INTO driver_connect_forwardonly VALUES (0), (1), (2), (3), (4)");

    OK_SIMPLE_STMT(hstmt, "SELECT a FROM driver_connect_forwardonly ORDER BY a");

    SQLULEN cursor_type;

    CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
    CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0));
    CHECK_STMT_RC(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&cursor_type, 0, NULL));

    is_num(cursor_type, SQL_CURSOR_FORWARD_ONLY);

    FAIL_IF(SQL_SUCCEEDED(SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 3)),
            "only SQL_FETCH_NEXT is allowed for a FORWARD_ONLY cursor");

    OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS driver_connect_forwardonly");

    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
    CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
    return OK;
}

ODBC_TEST(driver_connect_forwardonly_w) {
    HSTMT hdbc;
    SQLCHAR conn[1024];
    SQLWCHAR conn_out[1024];

    CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
    SQLSMALLINT conn_out_len;
    sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;FORWARDONLY=1;",
    my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema);
    CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                        conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                        SQL_DRIVER_NOPROMPT));
    if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

    HSTMT hstmt;
    CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
    OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS driver_connect_forwardonly");
    OK_SIMPLE_STMT(hstmt, "CREATE TABLE driver_connect_forwardonly (a INT)");
    OK_SIMPLE_STMT(hstmt, "INSERT INTO driver_connect_forwardonly VALUES (0), (1), (2), (3), (4)");

    OK_SIMPLE_STMT(hstmt, "SELECT a FROM driver_connect_forwardonly ORDER BY a");

    SQLULEN cursor_type;

    CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
    CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_STATIC, 0));
    CHECK_STMT_RC(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&cursor_type, 0, NULL));

    is_num(cursor_type, SQL_CURSOR_FORWARD_ONLY);

    FAIL_IF(SQL_SUCCEEDED(SQLFetchScroll(hstmt, SQL_FETCH_ABSOLUTE, 3)),
            "only SQL_FETCH_NEXT is allowed for a FORWARD_ONLY cursor");

    OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS driver_connect_forwardonly");

    CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
    CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
    return OK;
}

ODBC_TEST(driver_connect_no_cache) {
  HSTMT hdbc;
  SQLCHAR conn[1024], conn_out[1024];

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;NO_CACHE=1;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS driver_connect_no_cache");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE driver_connect_no_cache (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO driver_connect_no_cache VALUES (0), (1), (2), (3), (4)");

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM driver_connect_no_cache ORDER BY a");

  SQLULEN cursor_type;

  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
  CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));
  CHECK_STMT_RC(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&cursor_type, 0, NULL));
  is_num(cursor_type, SQL_CURSOR_FORWARD_ONLY);

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM driver_connect_no_cache ORDER BY a");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));

  FAIL_IF(SQL_SUCCEEDED(SQLSetPos(hstmt, 3, SQL_POSITION, SQL_LOCK_NO_CHANGE)),
          "Can't use SQLSetPos with FORWARD_ONLY cursor when NO_CACHE option is set");
  CHECK_SQLSTATE_EX(hstmt, SQL_HANDLE_STMT, "HY109");
  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS driver_connect_no_cache");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

ODBC_TEST(driver_connect_no_cache_w) {
  HSTMT hdbc;
  SQLCHAR conn[1024];
  SQLWCHAR conn_out[1024];

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  SQLSMALLINT conn_out_len;
  sprintf((char *) conn, "DRIVER=%s;UID=%s;PWD=%s;SERVER=%s;PORT=%u;DB=%s;NO_CACHE=1;",
          my_drivername, my_uid, my_pwd, my_servername, my_port, my_schema);
  CHECK_DBC_RC(hdbc, SQLDriverConnectW(hdbc, NULL, CW(conn), SQL_NTS,
                                      conn_out, 1024 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string_w(conn_out_len, strlen((char*)conn), conn_out, CW(conn)) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS driver_connect_no_cache");
  OK_SIMPLE_STMT(hstmt, "CREATE TABLE driver_connect_no_cache (a INT)");
  OK_SIMPLE_STMT(hstmt, "INSERT INTO driver_connect_no_cache VALUES (0), (1), (2), (3), (4)");

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM driver_connect_no_cache ORDER BY a");

  SQLULEN cursor_type;

  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));
  CHECK_STMT_RC(hstmt, SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY, 0));
  CHECK_STMT_RC(hstmt, SQLGetStmtAttr(hstmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)&cursor_type, 0, NULL));
  is_num(cursor_type, SQL_CURSOR_FORWARD_ONLY);

  OK_SIMPLE_STMT(hstmt, "SELECT a FROM driver_connect_no_cache ORDER BY a");
  CHECK_STMT_RC(hstmt, SQLFetch(hstmt));

  FAIL_IF(SQL_SUCCEEDED(SQLSetPos(hstmt, 3, SQL_POSITION, SQL_LOCK_NO_CHANGE)),
          "Can't use SQLSetPos with FORWARD_ONLY cursor when NO_CACHE option is set");
  CHECK_SQLSTATE_EX(hstmt, SQL_HANDLE_STMT, "HY109");
  CHECK_STMT_RC(hstmt, SQLCloseCursor(hstmt));

  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS driver_connect_no_cache");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

ODBC_TEST(driver_connect_jwt) {
  HSTMT hdbc;
  SQLCHAR conn[2048];
  SQLCHAR conn_out[2048];

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  SQLSMALLINT conn_out_len;
  sprintf((char*)conn, "DRIVER=%s;UID=%s;JWT=%s;SERVER=%s;PORT=%u;DB=%s;",
          my_drivername, jwt_user, jwt_password, my_servername, my_port, my_schema);
  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 2048 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  if (check_connection_string(conn_out_len, strlen((char*)conn), conn_out, conn) == FAIL) { return FAIL; }

  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "DROP TABLE IF EXISTS some_table");
  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));
  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));
  return OK;
}

ODBC_TEST(driver_connect_browser_sso) {
  HSTMT hdbc;
  SQLCHAR conn[2048];
  SQLCHAR conn_out[2048];
  int test_mode = BROWSER_AUTH_FLAG_TEST_FIRST_CALL;

  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc));
  SQLSMALLINT conn_out_len;
  sprintf((char*) conn, "DRIVER=%s;SERVER=%s;PORT=%u;DB=%s;OPTIONS=4;BROWSER_SSO=1;TEST_MODE=%d",
          my_drivername, my_servername, my_port, my_schema, test_mode);

#if !defined(__APPLE__) && !defined(_WIN32)
  GError *err = NULL;
  SecretService *service = secret_service_get_sync(0, NULL, &err);
  SecretCollection *defaultCollection = secret_collection_for_alias_sync(
    service, SECRET_COLLECTION_DEFAULT, 0 /*SecretCollectionFlags*/, NULL, &err);
  if (err != NULL)
  {
    printf("Not running driver_connect_browser_sso test \
    as gnome-keyring secret service cannot be started or no collection is available\n");
    return OK;
  }
  gboolean is_locked = secret_collection_get_locked(defaultCollection);
  if (is_locked)
  {
    printf("Not running driver_connect_browser_sso test as gnome-keyring secret service is locked\n");
    return OK;
  }
#endif

  CHECK_DBC_RC(hdbc, SQLDriverConnect(hdbc, NULL, conn, SQL_NTS,
                                      conn_out, 2048 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  HSTMT hstmt;
  CHECK_DBC_RC(hdbc, SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt));
  OK_SIMPLE_STMT(hstmt, "SELECT 1");

  CHECK_STMT_RC(hstmt, SQLFreeHandle(SQL_HANDLE_STMT, hstmt));

  CHECK_DBC_RC(hdbc, SQLDisconnect(hdbc));

  HDBC hdbc2;
  CHECK_ENV_RC(Env, SQLAllocConnect(Env, &hdbc2));
  test_mode = BROWSER_AUTH_FLAG_TEST_SECOND_CALL;

  sprintf((char*) conn, "DRIVER=%s;SERVER=%s;PORT=%u;DB=%s;OPTIONS=4;BROWSER_SSO=1;TEST_MODE=%d",
          my_drivername, my_servername, my_port, my_schema, test_mode);
  CHECK_DBC_RC(hdbc2, SQLDriverConnect(hdbc2, NULL, conn, SQL_NTS,
                                      conn_out, 2048 * sizeof(SQLCHAR), &conn_out_len,
                                      SQL_DRIVER_NOPROMPT));
  HSTMT hstmt2;
  CHECK_DBC_RC(hdbc2, SQLAllocHandle(SQL_HANDLE_STMT, hdbc2, &hstmt2));
  OK_SIMPLE_STMT(hstmt2, "SELECT 1");
  CHECK_STMT_RC(hstmt2, SQLFreeHandle(SQL_HANDLE_STMT, hstmt2));
  CHECK_DBC_RC(hdbc2, SQLDisconnect(hdbc2));

  return OK;
}

// TODO: test NamedPipe parameter and NamedPipe bit in options (has effect only on Windows)
// TODO: test GUI prompts for Windows and Mac

MA_ODBC_TESTS my_tests[]=
{
  {basic_connect, "basic_connect",     NORMAL, ALL_DRIVERS},
  {basic_connect_w, "basic_connect_w",     NORMAL, UNICODE_DRIVER},
  {driver_connect_simple, "driver_connect_simple",     NORMAL, ALL_DRIVERS},
  {driver_connect_simple_w, "driver_connect_simple_w",     NORMAL, UNICODE_DRIVER},
  {driver_connect_trace, "driver_connect_trace", TO_FIX, ALL_DRIVERS},
  {driver_connect_unsupported, "driver_connect_unsupported",     TO_FIX, ALL_DRIVERS},
  {driver_connect_savefile, "driver_connect_savefile",     TO_FIX, ALL_DRIVERS},
  {driver_connect_ssl, "driver_connect_ssl",     NORMAL, ALL_DRIVERS},
  {driver_connect_ssl_w, "driver_connect_ssl_w",     NORMAL, UNICODE_DRIVER},
  {driver_connect_initstmt, "driver_connect_initstmt", NORMAL, ALL_DRIVERS},
  {driver_connect_initstmt_w, "driver_connect_initstmt_w", NORMAL, UNICODE_DRIVER},
  {driver_connect_timeout, "driver_connect_timeout", NORMAL, ALL_DRIVERS},
  {driver_connect_timeout_w, "driver_connect_timeout_w", NORMAL, UNICODE_DRIVER},
  {driver_connect_options, "driver_connect_options", NORMAL, ALL_DRIVERS},
  {driver_connect_options_w, "driver_connect_options_w", NORMAL, UNICODE_DRIVER},
  {driver_connect_forwardonly, "driver_connect_forwardonly", NORMAL, ALL_DRIVERS},
  {driver_connect_forwardonly_w, "driver_connect_forwardonly_w", NORMAL, UNICODE_DRIVER},
  {driver_connect_no_cache, "driver_connect_no_cache", NORMAL, ALL_DRIVERS},
  {driver_connect_no_cache_w, "driver_connect_no_cache_w", NORMAL, UNICODE_DRIVER},
  {driver_connect_jwt, "driver_connect_jwt", NORMAL, ALL_DRIVERS},
  {driver_connect_browser_sso, "driver_connect_browser_sso", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;

  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
