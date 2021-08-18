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
#include <keywords/keywords.hpp>

int CheckUSmallInt(SQLHANDLE Hdbc, SQLUSMALLINT InfoType, SQLUSMALLINT CorrectValue) {
  SQLUSMALLINT small_int_value = 0;
  SQLSMALLINT length = 0;

  // ANSI functions
#ifndef WIN32
  // Windows DM crashes when InfoValuePtr is NULL
  CHECK_DBC_RC(Connection,SQLGetInfo(Hdbc, InfoType, NULL, 0, NULL));

  CHECK_DBC_RC(Connection,SQLGetInfo(Hdbc, InfoType, NULL, 0, &length));
  is_num(length, sizeof(SQLUSMALLINT));
  length = 0;
#endif

  CHECK_DBC_RC(Connection,SQLGetInfo(Hdbc, InfoType, &small_int_value, 0, NULL));
  is_num(small_int_value, CorrectValue);
  small_int_value = 0;

  CHECK_DBC_RC(Connection,SQLGetInfo(Hdbc, InfoType, &small_int_value, 0, &length));
  is_num(small_int_value, CorrectValue);
  is_num(length, sizeof(SQLUSMALLINT));
  small_int_value = 0;
  length = 0;

  // UNICODE functions
#ifndef WIN32
  // Windows DM crashes when InfoValuePtr is NULL
  CHECK_DBC_RC(Connection,SQLGetInfoW(Hdbc, InfoType, NULL, 0, NULL));

  CHECK_DBC_RC(Connection,SQLGetInfoW(Hdbc, InfoType, NULL, 0, &length));
  is_num(length, sizeof(SQLUSMALLINT));
  length = 0;
#endif

  CHECK_DBC_RC(Connection,SQLGetInfoW(Hdbc, InfoType, &small_int_value, 0, NULL));
  is_num(small_int_value, CorrectValue);
  small_int_value = 0;

  CHECK_DBC_RC(Connection,SQLGetInfoW(Hdbc, InfoType, &small_int_value, 0, &length));
  is_num(small_int_value, CorrectValue);
  is_num(length, sizeof(SQLUSMALLINT));

  return OK;
}

int CheckUInteger(SQLHANDLE Hdbc, SQLUSMALLINT InfoType, SQLUINTEGER CorrectValue) {
  SQLUINTEGER int_value = 0;
  SQLSMALLINT length = 0;

  // ANSI functions
#ifndef WIN32
  // Windows DM crashes when InfoValuePtr is NULL
  CHECK_DBC_RC(Connection,SQLGetInfo(Hdbc, InfoType, NULL, 0, NULL));

  CHECK_DBC_RC(Connection,SQLGetInfo(Hdbc, InfoType, NULL, 0, &length));
  is_num(length, sizeof(SQLUINTEGER));
  length = 0;
#endif
  CHECK_DBC_RC(Connection,SQLGetInfo(Hdbc, InfoType, &int_value, 0, NULL));
  is_num(int_value, CorrectValue);
  int_value = 0;

  CHECK_DBC_RC(Connection,SQLGetInfo(Hdbc, InfoType, &int_value, 0, &length));
  is_num(int_value, CorrectValue);
  is_num(length, sizeof(SQLUINTEGER));
  int_value = 0;
  length = 0;

  // UNICODE functions
#ifndef WIN32
  // Windows DM crashes when InfoValuePtr is NULL
  CHECK_DBC_RC(Connection,SQLGetInfoW(Hdbc, InfoType, NULL, 0, NULL));

  CHECK_DBC_RC(Connection,SQLGetInfoW(Hdbc, InfoType, NULL, 0, &length));
  is_num(length, sizeof(SQLUINTEGER));
  length = 0;
#endif
  CHECK_DBC_RC(Connection,SQLGetInfoW(Hdbc, InfoType, &int_value, 0, NULL));
  is_num(int_value, CorrectValue);
  int_value = 0;

  CHECK_DBC_RC(Connection,SQLGetInfoW(Hdbc, InfoType, &int_value, 0, &length));
  is_num(int_value, CorrectValue);
  is_num(length, sizeof(SQLUINTEGER));

  return OK;
}

#define BUF_LEN 16382
#define DEADBEEF 0xDEADBEEF
int CheckChar(SQLHANDLE Hdbc, SQLUSMALLINT InfoType, char *CorrectValue) {
  SQLCHAR string_value[BUF_LEN];
  uint32_t padding = DEADBEEF;
  SQLWCHAR stringw_value[BUF_LEN / sizeof(SQLWCHAR)];
  uint32_t paddingw = DEADBEEF;
  SQLSMALLINT length = 0;
  SQLSMALLINT cmpLength;

  // ANSI tests
  CHECK_DBC_ERR(Hdbc, SQLGetInfo(Hdbc, InfoType, string_value, -1, &length), NULL, -1, "Invalid string or buffer length");
  /* The next test fails for all platforms */
#if 0
  /* This causes size == -1 sanitizer error with Linux DM */
  /* Windows DM returns incorrect length */
  /* Mac DM returns garbage as length */
  CHECK_DBC_RC(Hdbc, SQLGetInfo(Hdbc, InfoType, string_value, 0, &length));
  is_num(length, strlen(CorrectValue));
#endif
  CHECK_DBC_ERR(Hdbc, SQLGetInfo(Hdbc, InfoType, string_value, SQL_NTS, &length), NULL, -1, "Invalid string or buffer length");

  memset(string_value, 0xFF, sizeof(string_value));
  CHECK_DBC_RC(Hdbc, SQLGetInfo(Hdbc, InfoType, string_value, BUF_LEN, &length));
  is_num(length, strlen(CorrectValue));
  IS_STR(string_value, CorrectValue, length + 1);
  IS(padding == DEADBEEF);

  memset(string_value, 0xFF, sizeof(string_value));
  CHECK_DBC_RC(Hdbc, SQLGetInfo(Hdbc, InfoType, string_value, BUF_LEN, NULL));
  IS_STR(string_value, CorrectValue, length + 1);
  IS(padding == DEADBEEF);

  // Too short buffer
  /* The next test fails for all platforms */
#if 0
  /* Linux DM messes up output length and trailing 0 if the buffer is too short. */
  /* Windows DM returns incorrect length */
  /* Mac DM returns the length of the truncated string instead of the length of the available string */
  memset(string_value, 0xFF, sizeof(string_value));
  CHECK_DBC_RC(Hdbc, SQLGetInfo(Hdbc, InfoType, string_value, strlen(CorrectValue) / 2 + 1, &length));
  is_num(length, strlen(CorrectValue));
  IS_STR(string_value, CorrectValue, strlen(CorrectValue) / 2);
  IS(!string_value[strlen(CorrectValue) / 2]);
  IS(padding == DEADBEEF);
#endif

  // UNICODE tests
  CHECK_DBC_ERR(Hdbc, SQLGetInfoW(Hdbc, InfoType, stringw_value, -1, &length), NULL, -1, "Invalid string or buffer length");
  if (cPlatform == WINDOWS) {
    /* This causes size == -2 sanitizer error with Linux DM */
    /* A negative length is returned on MacOS */
    CHECK_DBC_RC(Hdbc, SQLGetInfoW(Hdbc, InfoType, stringw_value, 0, &length));
    is_num(length, strlen(CorrectValue)*sizeof(SQLWCHAR));
  }
  CHECK_DBC_ERR(Hdbc, SQLGetInfoW(Hdbc, InfoType, stringw_value, SQL_NTS, &length), NULL, -1, "Invalid string or buffer length");

  cmpLength = strlen(CorrectValue) + 1;
  if (BUF_LEN/sizeof(SQLWCHAR) < cmpLength) {
    if (cPlatform != WINDOWS) {
      /* Linux DM corrupts memory as it thinks OUT buffer length is in WCARS, not in bytes. */
      /* MacOS DM returns incorrect length */
      return OK;
    }

    // SQL_KEYWORDS length is bigger then 17k
    // We can't bigger BUF_LEN, because when Unicode driver is used,
    // Driver Manager limits BufferLength to 16382
    // If we pass bigger value, it overflows and becomes negative
    // So we will check only prefix if the buffer is too small
    cmpLength = BUF_LEN/sizeof(SQLWCHAR)-1;
  }

  memset(stringw_value, 0xFF, sizeof(stringw_value));
  CHECK_DBC_RC(Hdbc, SQLGetInfoW(Hdbc, InfoType, stringw_value,  sizeof(stringw_value), &length));
  is_num(length, strlen(CorrectValue)*sizeof(SQLWCHAR));
  IS_WSTR(stringw_value, CW(CorrectValue), cmpLength);
  IS(paddingw == DEADBEEF);

  memset(stringw_value, 0xFF, sizeof(stringw_value));
  CHECK_DBC_RC(Hdbc, SQLGetInfoW(Hdbc, InfoType, stringw_value,  sizeof(stringw_value), NULL));
  IS_WSTR(stringw_value, CW(CorrectValue), cmpLength);
  IS(paddingw == DEADBEEF);

  memset(stringw_value, 0xFF, sizeof(stringw_value));
  CHECK_DBC_RC(Hdbc, SQLGetInfoW(Hdbc, InfoType, stringw_value,  sizeof(stringw_value) - 1, &length)); /* odd length */
  is_num(length, strlen(CorrectValue)*sizeof(SQLWCHAR));
  IS_WSTR(stringw_value, CW(CorrectValue), cmpLength - 1);
  IS(!stringw_value[cmpLength - 1]);
  IS(paddingw == DEADBEEF);

  // Too short buffer
  if (cPlatform == WINDOWS) {
    /* Linux DM does not add the trailing 0 if the buffer is too short. */
    /* MacOS DM returns wrong length */
    memset(stringw_value, 0xFF, sizeof(stringw_value));
    CHECK_DBC_RC(Hdbc, SQLGetInfoW(Hdbc, InfoType, stringw_value, (strlen(CorrectValue) / 2 + 1) * sizeof(SQLWCHAR), &length));
    is_num(length, strlen(CorrectValue)*sizeof(SQLWCHAR));
    IS_WSTR(stringw_value, CW(CorrectValue), strlen(CorrectValue) / 2);
    IS(!stringw_value[strlen(CorrectValue) / 2]);
    IS(paddingw == DEADBEEF);
  }

  return OK;
}

int GetVersion(const char *query, char *buff) {
  char internal_buff[1024];
  int major, minor, patch;

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, (SQLCHAR *)query, SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  my_fetch_str(Stmt, (SQLCHAR *)internal_buff, 1);
  CHECK_STMT_RC(Stmt, SQLCloseCursor(Stmt));

  if (sscanf(internal_buff, "%d.%d.%d", &major, &minor, &patch) != 3) {
    printf("Wrong version returned: %s\n", internal_buff);
    return FAIL;
  }

  if (major < 5 || (major == 5 && minor < 6)) {
    major = 5;
    minor = 6;
    patch = 0;
  }

  sprintf(buff, "%02d.%02d.%04d", major, minor, patch);
  return OK;
}

#define CHECK_U_SMALL_INT(Hdbc, InfoType, CorrectValue)\
do {\
  if (CheckUSmallInt(Hdbc, InfoType, CorrectValue) != OK)\
  {\
    return FAIL;\
  }\
} while(0)

#define CHECK_U_INTEGER(Hdbc, InfoType, CorrectValue)\
do {\
  if (CheckUInteger(Hdbc, InfoType, CorrectValue) != OK)\
  {\
    return FAIL;\
  }\
} while(0)

#define CHECK_CHAR(Hdbc, InfoType, CorrectValue)\
do {\
  if (CheckChar(Hdbc, InfoType, CorrectValue) != OK)\
  {\
    return FAIL;\
  }\
} while(0)

#define GET_VERSION(query, buff)\
do {\
  if (GetVersion(query, buff) != OK)\
  {\
    return FAIL;\
  }\
} while(0)

ODBC_TEST(driver_information)
{
  SQLHDBC Hdbc;
  SQLCHAR conn[1024];

  CHECK_ENV_RC(Env, SQLAllocHandle(SQL_HANDLE_DBC, Env, &Hdbc));

  CHECK_U_SMALL_INT(Connection, SQL_ACTIVE_ENVIRONMENTS, 0);
  // CHECK_U_INTEGER(Connection, SQL_ASYNC_DBC_FUNCTIONS, SQL_ASYNC_DBC_NOT_CAPABLE); TODO PLAT-5514
  CHECK_U_INTEGER(Connection, SQL_ASYNC_MODE, SQL_AM_NONE);
  // CHECK_U_INTEGER(Connection, SQL_ASYNC_NOTIFICATION, SQL_ASYNC_NOTIFICATION_NOT_CAPABLE); TODO PLAT-5514
  CHECK_U_INTEGER(Connection, SQL_BATCH_ROW_COUNT, SQL_BRC_EXPLICIT);
  CHECK_U_INTEGER(Connection, SQL_BATCH_SUPPORT, SQL_BS_SELECT_EXPLICIT |
                                                   SQL_BS_ROW_COUNT_EXPLICIT |
                                                   SQL_BS_SELECT_PROC |
                                                   SQL_BS_ROW_COUNT_PROC);

  // check SQL_DATA_SOURCE_NAME when connecting with SQLConnect
  CHECK_DBC_RC(Hdbc, SQLConnect(Hdbc, my_dsn, SQL_NTS, my_uid, SQL_NTS, my_pwd, SQL_NTS));
  CHECK_CHAR(Hdbc, SQL_DATA_SOURCE_NAME, (char *)my_dsn);
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  // check SQL_DATA_SOURCE_NAME when connecting with SQLDriverConnect and with DSN keyword in the connection string
  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_CHAR(Hdbc, SQL_DATA_SOURCE_NAME, (char *)my_dsn);
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  // check SQL_DATA_SOURCE_NAME when connecting with SQLDriverConnect and with DRIVER keyword in the connection string
  sprintf((char *)conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;%s;%s",
          my_drivername, my_servername, my_uid, my_pwd, ma_strport, add_connstr);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_CHAR(Hdbc, SQL_DATA_SOURCE_NAME, "");
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  CHECK_CHAR(Connection, SQL_DRIVER_NAME, WindowsDM(Connection) ? (is_unicode_driver() ? "ssodbcw.dll" : "ssodbca.dll") : (is_unicode_driver() ? "libssodbcw.so" : "libssodbca.so"));
  CHECK_CHAR(Connection, SQL_DRIVER_ODBC_VER, "03.51");
  CHECK_CHAR(Connection, SQL_DRIVER_VER, MARIADB_ODBC_VERSION);
  // CHECK_U_INTEGER(Connection, SQL_DRIVER_AWARE_POOLING_SUPPORTED, SQL_DRIVER_AWARE_POOLING_NOT_CAPABLE); TODO PLAT-5514
  CHECK_U_INTEGER(Connection, SQL_DYNAMIC_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT |
                                                                SQL_CA1_ABSOLUTE |
                                                                SQL_CA1_RELATIVE |
                                                                // SQL_CA1_BOOKMARK | TODO PLAT-5471
                                                                SQL_CA1_LOCK_NO_CHANGE |
                                                                SQL_CA1_POS_POSITION |
                                                                // SQL_CA1_POS_UPDATE | TODO PLAT-5080
                                                                SQL_CA1_POS_DELETE |
                                                                // SQL_CA1_POS_REFRESH | TODO PLAT-5466
                                                                // SQL_CA1_POSITIONED_UPDATE | TODO PLAT-5080
                                                                SQL_CA1_POSITIONED_DELETE |
                                                                SQL_CA1_BULK_ADD);
  CHECK_U_INTEGER(Connection, SQL_DYNAMIC_CURSOR_ATTRIBUTES2, SQL_CA2_READ_ONLY_CONCURRENCY |
                                                                SQL_CA2_SENSITIVITY_ADDITIONS |
                                                                SQL_CA2_SENSITIVITY_DELETIONS |
                                                                SQL_CA2_SENSITIVITY_UPDATES |
                                                                SQL_CA2_MAX_ROWS_SELECT |
                                                                SQL_CA2_MAX_ROWS_INSERT |
                                                                SQL_CA2_MAX_ROWS_DELETE |
                                                                SQL_CA2_MAX_ROWS_UPDATE |
                                                                SQL_CA2_CRC_EXACT |
                                                                SQL_CA2_SIMULATE_TRY_UNIQUE);
  CHECK_U_INTEGER(Connection, SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT |
                                                                   SQL_CA1_LOCK_NO_CHANGE |
                                                                   SQL_CA1_POS_POSITION |
                                                                   // SQL_CA1_POS_UPDATE | TODO PLAT-5080
                                                                   SQL_CA1_POS_DELETE |
                                                                   // SQL_CA1_POS_REFRESH | TODO PLAT-5466
                                                                   // SQL_CA1_POSITIONED_UPDATE | TODO PLAT-5466
                                                                   SQL_CA1_POSITIONED_DELETE |
                                                                   SQL_CA1_BULK_ADD);
  CHECK_U_INTEGER(Connection, SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2, SQL_CA2_CRC_EXACT |
                                                                     SQL_CA2_READ_ONLY_CONCURRENCY |
                                                                     SQL_CA2_MAX_ROWS_SELECT |
                                                                     SQL_CA2_MAX_ROWS_INSERT |
                                                                     SQL_CA2_MAX_ROWS_DELETE |
                                                                     SQL_CA2_MAX_ROWS_UPDATE);
  // check SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES with NO_CACHE option
  sprintf((char *)conn, "DRIVER=%s;SERVER=%s;UID=%s;PASSWORD=%s;NO_CACHE=1;%s;%s",
          my_drivername, my_servername, my_uid, my_pwd, ma_strport, add_connstr);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_U_INTEGER(Hdbc, SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT);
  CHECK_U_INTEGER(Hdbc, SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2, SQL_CA2_READ_ONLY_CONCURRENCY |
                                                                   SQL_CA2_MAX_ROWS_SELECT |
                                                                   SQL_CA2_MAX_ROWS_INSERT |
                                                                   SQL_CA2_MAX_ROWS_DELETE |
                                                                   SQL_CA2_MAX_ROWS_UPDATE);
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  CHECK_U_SMALL_INT(Connection, SQL_FILE_USAGE, SQL_FILE_NOT_SUPPORTED);
  CHECK_U_INTEGER(Connection, SQL_GETDATA_EXTENSIONS, SQL_GD_ANY_COLUMN |
                                                        SQL_GD_ANY_ORDER |
                                                        SQL_GD_BLOCK |
                                                        SQL_GD_BOUND);
  CHECK_U_INTEGER(Connection, SQL_INFO_SCHEMA_VIEWS, SQL_ISV_CHARACTER_SETS |
                                                       SQL_ISV_COLLATIONS |
                                                       SQL_ISV_COLUMNS |
                                                       SQL_ISV_COLUMN_PRIVILEGES |
                                                       SQL_ISV_KEY_COLUMN_USAGE |
                                                       SQL_ISV_REFERENTIAL_CONSTRAINTS |
                                                       SQL_ISV_SCHEMATA |
                                                       SQL_ISV_TABLES |
                                                       SQL_ISV_TABLE_PRIVILEGES |
                                                       SQL_ISV_TABLE_CONSTRAINTS |
                                                       SQL_ISV_VIEWS);
  CHECK_U_INTEGER(Connection, SQL_KEYSET_CURSOR_ATTRIBUTES1, 0);
  CHECK_U_INTEGER(Connection, SQL_KEYSET_CURSOR_ATTRIBUTES2, 0);
  CHECK_U_INTEGER(Connection, SQL_MAX_ASYNC_CONCURRENT_STATEMENTS, 0); // TODO PLAT-5472
  CHECK_U_SMALL_INT(Connection, SQL_MAX_CONCURRENT_ACTIVITIES, 0); // TODO PLAT-5472
  CHECK_U_SMALL_INT(Connection, SQL_MAX_DRIVER_CONNECTIONS, 0);
  CHECK_U_SMALL_INT(Connection, SQL_ODBC_INTERFACE_CONFORMANCE, SQL_OIC_CORE);
  CHECK_CHAR(Connection, SQL_ROW_UPDATES, "N");
  CHECK_CHAR(Connection, SQL_SEARCH_PATTERN_ESCAPE, "\\");
  CHECK_CHAR(Connection, SQL_SERVER_NAME, (char *)my_servername);
  CHECK_U_INTEGER(Connection, SQL_PARAM_ARRAY_ROW_COUNTS, SQL_PARC_NO_BATCH);
  CHECK_U_INTEGER(Connection, SQL_PARAM_ARRAY_SELECTS, SQL_PAS_NO_BATCH);
  CHECK_U_INTEGER(Connection, SQL_STATIC_CURSOR_ATTRIBUTES1, SQL_CA1_NEXT |
                                                             SQL_CA1_LOCK_NO_CHANGE |
                                                             SQL_CA1_POS_POSITION |
                                                             // SQL_CA1_POS_UPDATE | TODO PLAT-5080
                                                             SQL_CA1_POS_DELETE |
                                                             // SQL_CA1_POS_REFRESH | TODO PLAT-5466
                                                             // SQL_CA1_POSITIONED_UPDATE | TODO PLAT-5466
                                                             SQL_CA1_POSITIONED_DELETE |
                                                             SQL_CA1_BULK_ADD);
  CHECK_U_INTEGER(Connection, SQL_STATIC_CURSOR_ATTRIBUTES2, SQL_CA2_CRC_EXACT |
                                                             SQL_CA2_READ_ONLY_CONCURRENCY |
                                                             SQL_CA2_MAX_ROWS_SELECT |
                                                             SQL_CA2_MAX_ROWS_INSERT |
                                                             SQL_CA2_MAX_ROWS_DELETE |
                                                             SQL_CA2_MAX_ROWS_UPDATE);

  CHECK_DBC_RC(Hdbc, SQLFreeHandle(SQL_HANDLE_DBC, Hdbc));

  return OK;
}

ODBC_TEST(dbms_product_information)
{
  SQLHDBC Hdbc;
  SQLCHAR conn[1024];
  char version[1024];
  SQLAllocHandle(SQL_HANDLE_DBC, Env, &Hdbc);

  CHECK_CHAR(Connection, SQL_DATABASE_NAME, (char *)my_schema);

  // check SQL_DATABASE_NAME when no database specified
  sprintf((char *)conn, "DRIVER=%s;SERVER=%s;UID=%s;PWD=%s;PORT=%d;%s", my_drivername,
          my_servername, my_uid, my_pwd, my_port, add_connstr);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_CHAR(Hdbc, SQL_DATABASE_NAME, "null");
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  // check SQL_DBMS_NAME and SQL_DBMS_VER without COMPAT_MODE
  CHECK_CHAR(Connection, SQL_DBMS_NAME, "SingleStore");
  GET_VERSION("SELECT @@memsql_version", version);
  CHECK_CHAR(Connection, SQL_DBMS_VER, version);

  // check SQL_DBMS_NAME and SQL_DBMS_VER with COMPAT_MODE
  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s;COMPAT_MODE=1", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_CHAR(Hdbc, SQL_DBMS_NAME, "MySQL");
  GET_VERSION("SELECT @@version", version);
  CHECK_CHAR(Hdbc, SQL_DBMS_VER, version);
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  SQLFreeHandle(SQL_HANDLE_DBC, Hdbc);
  return OK;
}

ODBC_TEST(data_source_information)
{
  SQLHDBC Hdbc;
  SQLCHAR conn[1024];
  SQLAllocHandle(SQL_HANDLE_DBC, Env, &Hdbc);


  CHECK_CHAR(Connection, SQL_ACCESSIBLE_PROCEDURES, "N");
  CHECK_CHAR(Connection, SQL_ACCESSIBLE_TABLES, "N");
  CHECK_U_INTEGER(Connection, SQL_BOOKMARK_PERSISTENCE, 0); // TODO PLAT-5471
  CHECK_CHAR(Connection, SQL_CATALOG_TERM, "database");
  CHECK_CHAR(Connection, SQL_COLLATION_SEQ, "utf8_general_ci");
  CHECK_U_SMALL_INT(Connection, SQL_CONCAT_NULL_BEHAVIOR, SQL_CB_NULL);
  CHECK_U_SMALL_INT(Connection, SQL_CURSOR_COMMIT_BEHAVIOR, SQL_CB_PRESERVE);
  CHECK_U_SMALL_INT(Connection, SQL_CURSOR_ROLLBACK_BEHAVIOR, SQL_CB_PRESERVE);
  CHECK_CHAR(Connection, SQL_DATA_SOURCE_READ_ONLY, "N");
  CHECK_U_INTEGER(Connection, SQL_DEFAULT_TXN_ISOLATION, SQL_TXN_READ_COMMITTED);
  CHECK_CHAR(Connection, SQL_DESCRIBE_PARAMETER, "N");
  CHECK_CHAR(Connection, SQL_MULT_RESULT_SETS, "Y");
  CHECK_CHAR(Connection, SQL_MULTIPLE_ACTIVE_TXN, "Y");
  CHECK_CHAR(Connection, SQL_NEED_LONG_DATA_LEN, "N");
  CHECK_U_SMALL_INT(Connection, SQL_NULL_COLLATION, SQL_NC_LOW);
  CHECK_CHAR(Connection, SQL_PROCEDURE_TERM, "procedure");
  CHECK_CHAR(Connection, SQL_SCHEMA_TERM, "");

  // check SQL_SCROLL_OPTIONS when MADB_OPT_FLAG_DYNAMIC_CURSOR option is set and MADB_OPT_FLAG_FORWARD_CURSOR is not set
  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s;OPTION=32", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_U_INTEGER(Hdbc, SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY|SQL_SO_STATIC|SQL_SO_DYNAMIC);
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  // check SQL_SCROLL_OPTIONS when MADB_OPT_FLAG_DYNAMIC_CURSOR option is not set and and MADB_OPT_FLAG_FORWARD_CURSOR is not set
  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s;", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_U_INTEGER(Hdbc, SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY|SQL_SO_STATIC);
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  // check SQL_SCROLL_OPTIONS when MADB_OPT_FLAG_DYNAMIC_CURSOR option is set and MADB_OPT_FLAG_FORWARD_CURSOR is set
  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s;OPTION=2097184", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_U_INTEGER(Hdbc, SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY|SQL_SO_DYNAMIC);
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  // check SQL_SCROLL_OPTIONS when MADB_OPT_FLAG_DYNAMIC_CURSOR option is not set and and MADB_OPT_FLAG_FORWARD_CURSOR is set
  sprintf((char *)conn, "DSN=%s;UID=%s;PWD=%s;OPTION=2097152", my_dsn, my_uid, my_pwd);
  CHECK_DBC_RC(Hdbc, SQLDriverConnect(Hdbc, NULL, conn, strlen((char *)conn), NULL, 0, NULL, SQL_DRIVER_NOPROMPT));
  CHECK_U_INTEGER(Hdbc, SQL_SCROLL_OPTIONS, SQL_SO_FORWARD_ONLY);
  CHECK_DBC_RC(Hdbc, SQLDisconnect(Hdbc));

  CHECK_CHAR(Connection, SQL_TABLE_TERM, "table");
  CHECK_U_SMALL_INT(Connection, SQL_TXN_CAPABLE, SQL_TC_DDL_COMMIT);
  CHECK_U_INTEGER(Connection, SQL_TXN_ISOLATION_OPTION, SQL_TXN_READ_COMMITTED);
  CHECK_CHAR(Connection, SQL_USER_NAME, "root");

  SQLFreeHandle(SQL_HANDLE_DBC, Hdbc);
  return OK;
}

ODBC_TEST(supported_sql)
{
  CHECK_U_INTEGER(Connection, SQL_AGGREGATE_FUNCTIONS, SQL_AF_AVG | SQL_AF_COUNT |
                                                       SQL_AF_MAX | SQL_AF_MIN | SQL_AF_SUM |
                                                       SQL_AF_ALL | SQL_AF_DISTINCT);
  CHECK_U_INTEGER(Connection, SQL_ALTER_DOMAIN, 0);
  CHECK_U_INTEGER(Connection, SQL_ALTER_TABLE, SQL_AT_ADD_COLUMN |
                                                 SQL_AT_DROP_COLUMN |
                                                 SQL_AT_ADD_CONSTRAINT |
                                                 SQL_AT_ADD_COLUMN_SINGLE |
                                                 SQL_AT_ADD_COLUMN_DEFAULT);
  CHECK_U_SMALL_INT(Connection, SQL_CATALOG_LOCATION, SQL_CL_START);
  CHECK_CHAR(Connection, SQL_CATALOG_NAME, "Y");
  CHECK_CHAR(Connection, SQL_CATALOG_NAME_SEPARATOR, ".");
  CHECK_U_INTEGER(Connection, SQL_CATALOG_USAGE, SQL_CU_DML_STATEMENTS |
                                            SQL_CU_INDEX_DEFINITION |
                                            SQL_CU_PROCEDURE_INVOCATION |
                                            SQL_CU_PRIVILEGE_DEFINITION |
                                            SQL_CU_TABLE_DEFINITION);
  CHECK_CHAR(Connection, SQL_COLUMN_ALIAS, "Y");
  CHECK_U_SMALL_INT(Connection, SQL_CORRELATION_NAME, SQL_CN_ANY);
  CHECK_U_INTEGER(Connection, SQL_CREATE_ASSERTION, 0);
  CHECK_U_INTEGER(Connection, SQL_CREATE_CHARACTER_SET, 0);
  CHECK_U_INTEGER(Connection, SQL_CREATE_COLLATION, 0);
  CHECK_U_INTEGER(Connection, SQL_CREATE_DOMAIN, 0);
  CHECK_U_INTEGER(Connection, SQL_CREATE_SCHEMA, 0);
  CHECK_U_INTEGER(Connection, SQL_CREATE_TABLE, SQL_CT_COLUMN_COLLATION |
                                                  SQL_CT_COLUMN_DEFAULT |
                                                  SQL_CT_COLUMN_CONSTRAINT |
                                                  SQL_CT_COMMIT_DELETE |
                                                  SQL_CT_CREATE_TABLE |
                                                  SQL_CT_TABLE_CONSTRAINT |
                                                  SQL_CT_GLOBAL_TEMPORARY |
                                                  SQL_CT_LOCAL_TEMPORARY |
                                                  SQL_CT_CONSTRAINT_NAME_DEFINITION);
  CHECK_U_INTEGER(Connection, SQL_CREATE_VIEW, SQL_CV_CREATE_VIEW);
  CHECK_U_INTEGER(Connection, SQL_CREATE_TRANSLATION, 0);
  CHECK_U_INTEGER(Connection, SQL_DDL_INDEX, SQL_DI_CREATE_INDEX |
                                                        SQL_DI_DROP_INDEX);
  CHECK_U_INTEGER(Connection, SQL_DROP_ASSERTION, 0);
  CHECK_U_INTEGER(Connection, SQL_DROP_CHARACTER_SET, 0);
  CHECK_U_INTEGER(Connection, SQL_DROP_COLLATION, 0);
  CHECK_U_INTEGER(Connection, SQL_DROP_DOMAIN, 0);
  CHECK_U_INTEGER(Connection, SQL_DROP_SCHEMA, 0);
  CHECK_U_INTEGER(Connection, SQL_DROP_TABLE, SQL_DT_DROP_TABLE);
  CHECK_U_INTEGER(Connection, SQL_DROP_TRANSLATION, 0);
  CHECK_U_INTEGER(Connection, SQL_DROP_VIEW, SQL_DV_DROP_VIEW);
  CHECK_CHAR(Connection, SQL_EXPRESSIONS_IN_ORDERBY, "Y");
  CHECK_U_SMALL_INT(Connection, SQL_GROUP_BY, SQL_GB_NO_RELATION);
  CHECK_U_SMALL_INT(Connection, SQL_IDENTIFIER_CASE, SQL_IC_SENSITIVE);
  CHECK_CHAR(Connection, SQL_IDENTIFIER_QUOTE_CHAR, "`");
  CHECK_U_INTEGER(Connection, SQL_INDEX_KEYWORDS, SQL_IK_ALL);
  CHECK_U_INTEGER(Connection, SQL_INSERT_STATEMENT, SQL_IS_INSERT_LITERALS |
                                                        SQL_IS_INSERT_SEARCHED |
                                                        SQL_IS_SELECT_INTO);
  CHECK_CHAR(Connection, SQL_INTEGRITY, "N");
  CHECK_CHAR(Connection, SQL_KEYWORDS, SINGLESTORE_KEYWORDS);
  CHECK_CHAR(Connection, SQL_LIKE_ESCAPE_CLAUSE, "Y");
  CHECK_U_SMALL_INT(Connection, SQL_NON_NULLABLE_COLUMNS, SQL_NNC_NON_NULL);
  CHECK_U_INTEGER(Connection, SQL_OJ_CAPABILITIES, SQL_OJ_LEFT |
                                                     SQL_OJ_RIGHT |
                                                     SQL_OJ_FULL |
                                                     SQL_OJ_NESTED |
                                                     SQL_OJ_NOT_ORDERED |
                                                     SQL_OJ_INNER |
                                                     SQL_OJ_ALL_COMPARISON_OPS);
  CHECK_CHAR(Connection, SQL_ORDER_BY_COLUMNS_IN_SELECT, "N");
  CHECK_CHAR(Connection, SQL_OUTER_JOINS, "Y");
  CHECK_CHAR(Connection, SQL_PROCEDURES, "Y");
  CHECK_U_SMALL_INT(Connection, SQL_QUOTED_IDENTIFIER_CASE, SQL_IC_SENSITIVE);
  CHECK_U_INTEGER(Connection, SQL_SCHEMA_USAGE, 0);
  CHECK_CHAR(Connection, SQL_SPECIAL_CHARACTERS, "\"\\/!@#$%^&*()-=+<>|:;,.%?");
  CHECK_U_INTEGER(Connection, SQL_SQL_CONFORMANCE, SQL_SC_SQL92_INTERMEDIATE);
  CHECK_U_INTEGER(Connection, SQL_SUBQUERIES, SQL_SQ_COMPARISON |
                                                SQL_SQ_CORRELATED_SUBQUERIES |
                                                SQL_SQ_EXISTS |
                                                SQL_SQ_IN);
  CHECK_U_INTEGER(Connection, SQL_UNION, SQL_U_UNION | SQL_U_UNION_ALL);

  return OK;
}

ODBC_TEST(sql_limits)
{
  CHECK_U_INTEGER(Connection, SQL_MAX_BINARY_LITERAL_LEN, 0);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_CATALOG_NAME_LEN, 60*4);
  CHECK_U_INTEGER(Connection, SQL_MAX_CHAR_LITERAL_LEN, 0);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_COLUMN_NAME_LEN, 64*4);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_COLUMNS_IN_GROUP_BY, 0);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_COLUMNS_IN_GROUP_BY, 0);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_COLUMNS_IN_ORDER_BY, 0);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_COLUMNS_IN_SELECT, 0);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_COLUMNS_IN_TABLE, 4096);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_CURSOR_NAME_LEN, 64 * 3 + 1);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_IDENTIFIER_LEN, 64*4);
  CHECK_U_INTEGER(Connection, SQL_MAX_INDEX_SIZE, 0);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_PROCEDURE_NAME_LEN, 64*4);
  CHECK_U_INTEGER(Connection, SQL_MAX_ROW_SIZE, 1<<26);
  CHECK_CHAR(Connection, SQL_MAX_ROW_SIZE_INCLUDES_LONG,"N");
  CHECK_U_SMALL_INT(Connection, SQL_MAX_SCHEMA_NAME_LEN, 0);
  CHECK_U_INTEGER(Connection, SQL_MAX_STATEMENT_LEN, 1073741824);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_TABLE_NAME_LEN, 64*4);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_TABLES_IN_SELECT, 253);
  CHECK_U_SMALL_INT(Connection, SQL_MAX_USER_NAME_LEN, 32*4);

  return OK;
}

ODBC_TEST(scalar_functions_information)
{
  CHECK_U_INTEGER(Connection, SQL_CONVERT_FUNCTIONS, SQL_FN_CVT_CONVERT);
  CHECK_U_INTEGER(Connection, SQL_NUMERIC_FUNCTIONS, SQL_FN_NUM_ABS | SQL_FN_NUM_ACOS | SQL_FN_NUM_ASIN |
                                                     SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2 | SQL_FN_NUM_CEILING |
                                                     SQL_FN_NUM_COS | SQL_FN_NUM_COT | SQL_FN_NUM_EXP |
                                                     SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_MOD |
                                                     SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT |
                                                     SQL_FN_NUM_TAN | SQL_FN_NUM_PI | SQL_FN_NUM_RAND |
                                                     SQL_FN_NUM_DEGREES | SQL_FN_NUM_LOG10 | SQL_FN_NUM_POWER |
                                                     SQL_FN_NUM_RADIANS | SQL_FN_NUM_ROUND | SQL_FN_NUM_TRUNCATE);
  CHECK_U_INTEGER(Connection, SQL_STRING_FUNCTIONS, SQL_FN_STR_ASCII | SQL_FN_STR_BIT_LENGTH |
                                                    SQL_FN_STR_CHAR | SQL_FN_STR_CHAR_LENGTH | SQL_FN_STR_CHARACTER_LENGTH |
                                                    SQL_FN_STR_CONCAT | SQL_FN_STR_INSERT |
                                                    SQL_FN_STR_LCASE | SQL_FN_STR_LEFT |
                                                    SQL_FN_STR_LENGTH | SQL_FN_STR_LOCATE |
                                                    SQL_FN_STR_LOCATE_2 | SQL_FN_STR_LTRIM |
                                                    SQL_FN_STR_OCTET_LENGTH | SQL_FN_STR_POSITION |
                                                    SQL_FN_STR_REPEAT | SQL_FN_STR_REPLACE |
                                                    SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM | SQL_FN_STR_SPACE |
                                                    SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE);
  CHECK_U_INTEGER(Connection, SQL_SYSTEM_FUNCTIONS, SQL_FN_SYS_DBNAME | SQL_FN_SYS_IFNULL |
                                                    SQL_FN_SYS_USERNAME);
  CHECK_U_INTEGER(Connection, SQL_TIMEDATE_ADD_INTERVALS, SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE |
                                                          SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER |
                                                          SQL_FN_TSI_YEAR);
  CHECK_U_INTEGER(Connection, SQL_TIMEDATE_DIFF_INTERVALS, SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE |
                                                           SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER |
                                                           SQL_FN_TSI_YEAR);
  CHECK_U_INTEGER(Connection, SQL_TIMEDATE_FUNCTIONS, SQL_FN_TD_CURDATE | SQL_FN_TD_CURRENT_DATE |
                                                      SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_CURRENT_TIMESTAMP |
                                                      SQL_FN_TD_CURTIME | SQL_FN_TD_DAYNAME |
                                                      SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK |
                                                      SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_EXTRACT |
                                                      SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE |
                                                      SQL_FN_TD_MONTH | SQL_FN_TD_MONTHNAME |
                                                      SQL_FN_TD_NOW | SQL_FN_TD_QUARTER |
                                                      SQL_FN_TD_SECOND | SQL_FN_TD_TIMESTAMPADD |
                                                      SQL_FN_TD_TIMESTAMPDIFF | SQL_FN_TD_WEEK | SQL_FN_TD_YEAR);
  return OK;
}

ODBC_TEST(conversion_information)
{
  SQLUINTEGER conversions = SQL_CVT_BIGINT | SQL_CVT_BINARY | SQL_CVT_BIT | SQL_CVT_CHAR | SQL_CVT_DATE |
                                    SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT |
                                    SQL_CVT_INTEGER | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC |
                                    SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TIME | SQL_CVT_TIMESTAMP |
                                    SQL_CVT_TINYINT | SQL_CVT_VARBINARY | SQL_CVT_VARCHAR | SQL_CVT_WCHAR |
                                    SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR;
  CHECK_U_INTEGER(Connection, SQL_CONVERT_BIGINT, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_BINARY, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_BIT, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_CHAR, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_GUID, 0);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_DATE, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_DECIMAL, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_DOUBLE, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_FLOAT, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_INTEGER, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_INTERVAL_YEAR_MONTH, 0);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_INTERVAL_DAY_TIME, 0);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_LONGVARBINARY, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_LONGVARCHAR, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_NUMERIC, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_REAL, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_SMALLINT, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_TIME, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_TIMESTAMP, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_TINYINT, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_VARBINARY, conversions);
  CHECK_U_INTEGER(Connection, SQL_CONVERT_VARCHAR, conversions);
  return OK;
}

ODBC_TEST(sql92)
{
  CHECK_U_INTEGER(Connection, SQL_SQL92_DATETIME_FUNCTIONS, SQL_SDF_CURRENT_DATE | SQL_SDF_CURRENT_TIME |
                                                            SQL_SDF_CURRENT_TIMESTAMP);
  CHECK_U_INTEGER(Connection, SQL_SQL92_FOREIGN_KEY_DELETE_RULE, 0);
  CHECK_U_INTEGER(Connection, SQL_SQL92_FOREIGN_KEY_UPDATE_RULE, 0);
  CHECK_U_INTEGER(Connection, SQL_SQL92_GRANT, SQL_SG_DELETE_TABLE | SQL_SG_INSERT_COLUMN |
                                               SQL_SG_INSERT_TABLE | SQL_SG_SELECT_TABLE |
                                               SQL_SG_UPDATE_COLUMN | SQL_SG_UPDATE_TABLE |
                                               SQL_SG_WITH_GRANT_OPTION);
  CHECK_U_INTEGER(Connection, SQL_SQL92_NUMERIC_VALUE_FUNCTIONS, SQL_SNVF_BIT_LENGTH | SQL_SNVF_CHARACTER_LENGTH |
                                                                 SQL_SNVF_CHAR_LENGTH | SQL_SNVF_EXTRACT |
                                                                 SQL_SNVF_OCTET_LENGTH | SQL_SNVF_POSITION);
  CHECK_U_INTEGER(Connection, SQL_SQL92_PREDICATES, SQL_SP_BETWEEN | SQL_SP_COMPARISON |
                                                    SQL_SP_EXISTS | SQL_SP_IN | SQL_SP_ISNOTNULL |
                                                    SQL_SP_ISNULL | SQL_SP_LIKE | SQL_SP_QUANTIFIED_COMPARISON);
  CHECK_U_INTEGER(Connection, SQL_SQL92_RELATIONAL_JOIN_OPERATORS, SQL_SRJO_CROSS_JOIN | SQL_SRJO_FULL_OUTER_JOIN | SQL_SRJO_INNER_JOIN |
                                                                   SQL_SRJO_LEFT_OUTER_JOIN | SQL_SRJO_RIGHT_OUTER_JOIN |
                                                                   SQL_SRJO_NATURAL_JOIN);
  CHECK_U_INTEGER(Connection, SQL_SQL92_REVOKE, SQL_SR_DELETE_TABLE | SQL_SR_INSERT_COLUMN |
                                                SQL_SR_INSERT_TABLE | SQL_SR_SELECT_TABLE |
                                                SQL_SR_UPDATE_COLUMN | SQL_SR_UPDATE_TABLE);
  CHECK_U_INTEGER(Connection, SQL_SQL92_ROW_VALUE_CONSTRUCTOR, SQL_SRVC_DEFAULT | SQL_SRVC_NULL |
                                                               SQL_SRVC_ROW_SUBQUERY | SQL_SRVC_VALUE_EXPRESSION);
  CHECK_U_INTEGER(Connection, SQL_SQL92_STRING_FUNCTIONS, SQL_SSF_CONVERT | SQL_SSF_LOWER |
                                                          SQL_SSF_SUBSTRING | SQL_SSF_TRIM_BOTH |
                                                          SQL_SSF_TRIM_LEADING | SQL_SSF_TRIM_TRAILING |
                                                          SQL_SSF_UPPER);
  CHECK_U_INTEGER(Connection, SQL_SQL92_VALUE_EXPRESSIONS, SQL_SVE_CASE | SQL_SVE_CAST | SQL_SVE_COALESCE |
                                                           SQL_SVE_NULLIF);
  CHECK_U_INTEGER(Connection, SQL_DATETIME_LITERALS, SQL_DL_SQL92_DATE | SQL_DL_SQL92_TIME |
                                                     SQL_DL_SQL92_TIMESTAMP);
  return OK;
}

MA_ODBC_TESTS my_tests[]=
{
  {driver_information, "driver_information", NORMAL, ALL_DRIVERS},
  {dbms_product_information, "dbms_product_information", NORMAL, ALL_DRIVERS},
  {data_source_information, "data_source_information", NORMAL, ALL_DRIVERS},
  {supported_sql, "supported_sql", NORMAL, ALL_DRIVERS},
  {sql_limits, "sql_limits", NORMAL, ALL_DRIVERS},
  {scalar_functions_information, "scalar_functions_information", NORMAL, ALL_DRIVERS},
  {conversion_information, "conversion_information", NORMAL, ALL_DRIVERS},
  {sql92, "sql92", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
