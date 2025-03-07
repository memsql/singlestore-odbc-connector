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

#ifndef _tap_h_
#define _tap_h_

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "ma_odbc_version.h"


#ifdef _WIN32
# define _WINSOCKAPI_
# include <windows.h>
# include <shlwapi.h>

char* strcasestr(const char* HayStack, const char* Needle)
{
  return StrStrIA(HayStack, Needle);
}
#else

# include <string.h>
# include <errno.h>
# include <wchar.h>
# include "ma_conv_charset.h"

/* Mimicking of VS' _snprintf */
int _snprintf(char *buffer, size_t count, const char *format, ...)
{
    va_list list;
    va_start(list, format);
    int result= vsnprintf(buffer, count, format, list);

    va_end(list);

    /* _snprintf returns negative number if buffer is not big enough */
    if (result > count)
    {
      return count - result - 1;
    }
    return result;
}


int strcpy_s(char *dest, size_t buffer_size, const char *src)
{
  size_t src_len;

  if (dest == NULL)
  {
    return EINVAL;
  }

  if (src == NULL)
  {
    *dest= '\0';
    return EINVAL;
  }

  src_len= strlen(src);

  if (buffer_size < src_len + 1)
  {
    *dest= 0;
    return ERANGE;
  }

  memcpy((void*)dest, (void*)src, src_len + 1);

  return 0;
}


#define Sleep(ms) sleep(ms/1000)

# ifndef TRUE
#  define TRUE 1
# endif
# ifndef FALSE
#  define FALSE 0
# endif

#define _strdup        strdup
#define _stricmp       strcasecmp
#define _strnicmp      strncasecmp
#define _atoi64(str)   strtoll((str), NULL, 10)
#define _i64toa(a,b,c) longlong2str((a),(b),(c))

#endif

#define LENGTHOF(arr) (sizeof(arr) / sizeof(arr[0]))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#include <sql.h>
#include <sqlext.h>
#include <time.h>
#include <assert.h>
/* We need mysql for MARIADB_CHARSET_INFO type and conversion routine */
#include <mysql.h>


BOOL   UseDsnOnly= FALSE;
static SQLCHAR *my_dsn=             (SQLCHAR *)"ssodbc_test_a";
static SQLCHAR *my_uid=             (SQLCHAR *)"root";
static SQLCHAR *my_pwd=             (SQLCHAR *)"";
static SQLCHAR *my_schema=          (SQLCHAR *)"odbc_test";
static SQLCHAR *my_drivername=      (SQLCHAR *)"SingleStore ODBC ANSI Driver";
static SQLCHAR *my_servername=      (SQLCHAR *)"127.0.0.1";
static SQLCHAR *add_connstr=        (SQLCHAR*)"";
static SQLCHAR *jwt_user=           (SQLCHAR*)"test_jwt_user";
static SQLCHAR *jwt_password=       (SQLCHAR*)"";

static unsigned int  my_port=        5506;
char          ma_strport[12]= "PORT=5506";

static SQLWCHAR *wdsn;
static SQLWCHAR *wuid;
static SQLWCHAR *wpwd;
static SQLWCHAR *wschema;
static SQLWCHAR *wdrivername;
static SQLWCHAR *wservername;
static SQLWCHAR *wstrport;
static SQLWCHAR *wadd_connstr;

static unsigned long my_options= 67108866;

static SQLHANDLE     Env, Connection, Stmt, wConnection, wStmt;
static SQLINTEGER    OdbcVer=        SQL_OV_ODBC3;
static my_bool       NoSsps = 1;

static int Travis= 0, TravisOnOsx= 0;

/* To use in tests for conversion of strings to (sql)wchar strings */
SQLWCHAR  sqlwchar_buff[1048576], sqlwchar_empty[]= {0};
SQLWCHAR *buff_pos= sqlwchar_buff;

/* Copied from C/C(+ added utf32le). Otherwise we would need to link libmariadb.
   On Windows we need only codepages and utf8 and utf16 only, charset names and utf16 and/or utf32 - on others */
static MARIADB_CHARSET_INFO  utf8mb3= { 33, 1, "utf8", "utf8_general_ci", "", 65001, "UTF-8", 1, 3, NULL, NULL}
, utf16be= { 54, 1, "utf16", "utf16_general_ci", "", 0, "UTF16", 2, 4, NULL, NULL}
, utf16le= { 56, 1, "utf16le", "utf16_general_ci", "", 1200, "UTF16LE", 2, 4, NULL, NULL}
, utf32be= { 60, 1, "utf32", "utf32_general_ci", "", 0, "UTF32", 4, 4, NULL, NULL }
, utf32le= { 0, 1, "utf32le", "", "", 0, "UTF32LE", 4, 4, NULL, NULL }
, *utf8= &utf8mb3, *utf16= NULL, *utf32= NULL, *DmUnicode= NULL;

int   tests_planned= 0;
char *test_status[]= {"not ok", "ok", "skip"};

// Test status codes based on what mode is running.
#define SSPS_FAIL 1
#define SSPS_OK 2
#define SSPS_TO_FIX 4

#define CSPS_FAIL 16
#define CSPS_OK 32
#define CSPS_TO_FIX 64

// Test return codes.
#define FAIL 0
#define OK 1
#define SKIP 2

/* Test Typesv*/
/* Normal test - failure is failure, i.e. bad thing */
#define NORMAL (SSPS_OK | CSPS_OK)
/* Known failure - yes, we need to fix it, but we do not want it to distract us from
   things that are really interesting. And no, we don't know what causes failure */
#define KNOWN_FAILURE (SSPS_FAIL | CSPS_FAIL)
/* The problem is in test */
#define TEST_PROBLEM  2
/* Test for the known problem waiting for a fix. */
#define TO_FIX (SSPS_TO_FIX | CSPS_TO_FIX)

#define MAX_ROW_DATA_LEN 1000
#define MAX_NAME_LEN     255

/* We don't test corresponding EnvAttr, but assume that connections are made using heler functions, and they use OdbcVer */
#define IS_ODBC3() (OdbcVer == SQL_OV_ODBC3)
/* Atm iODBC is the only DM using SQLWCHAR of 4 bytes size */
#define iOdbc() (sizeof(SQLWCHAR)==4)

enum Platform {
    LINUX,
    WINDOWS,
    MAC
};

#ifdef _WIN32
const enum Platform cPlatform= WINDOWS;
#else
const enum Platform cPlatform= (iOdbc() ? MAC : LINUX);
#endif

#define skip(A) diag((A)); return SKIP;

typedef struct st_ma_odbc_test {
  int   (*my_test)();
  char  *title;
  int   test_type;
  int   required_driver_type;
} MA_ODBC_TESTS;

#define ALL_DRIVERS -1
#define ANSI_DRIVER 0
#define UNICODE_DRIVER 1

int unicode_driver = -1;

#define is_unicode_driver() (unicode_driver == UNICODE_DRIVER)
#define is_ansi_driver()    (unicode_driver == ANSI_DRIVER)

#define ODBC_TEST(a)\
int a()

#define plan(a)\
  tests_planned= a;\


const char* comments [3][2] = {
        {"\t#%s Test is known to fail. Either unknown reason or a known reason that WON'T FIX anytime soon or WON'T FIX at all by design",
         "\t#%s This test was expected to fail, but didn't. Consider re-running multiple times and moving to OK bucket"}, // Expected FAIL cases
         {"\t#%s TODO: not ok - the test should have passed, but didn't",
          "\t#%s Ok - test passed as expected"}, // Expected OK cases
         {"\t#%s TODO: Test is for the known problem that is waiting for a fix",
          "\t#%s The problem seems to be fixed. Mark test as normal"} // Expected TO_FIX cases
};

my_bool test_expected_to_succeed(int expected_test_result, my_bool no_ssps)
{
    return (expected_test_result & (no_ssps ? CSPS_OK : SSPS_OK)) != 0;
}

void get_test_error_message(int expected_test_result, int rc, char* comment)
{
    if (rc == SKIP)
    {
        return;
    }

    char *mode = NoSsps ? "[CSPS]" : "[SSPS]";

    if (NoSsps)
    {
        if (expected_test_result & CSPS_FAIL)
        {
            sprintf(comment, comments[0][rc], mode);
        } else if (expected_test_result & CSPS_OK)
        {
            sprintf(comment, comments[1][rc], mode);
        } else if (expected_test_result & CSPS_TO_FIX)
        {
            sprintf(comment, comments[2][rc], mode);
        }
    } else
    {
        if (expected_test_result & SSPS_FAIL)
        {
            sprintf(comment, comments[0][rc], mode);
        } else if (expected_test_result & SSPS_OK)
        {
            sprintf(comment, comments[1][rc], mode);
        } else if (expected_test_result & SSPS_TO_FIX)
        {
            sprintf(comment, comments[2][rc], mode);
        }
    }
}


void mark_all_tests_normal(MA_ODBC_TESTS *tests)
{
  while(tests && tests->title !=  NULL)
  {
    tests->test_type= NORMAL;
    ++tests;
  }
}


void Usage()
{
  fprintf(stdout, "Valid options:\n");
  fprintf(stdout, "-d DSN Name\n");
  fprintf(stdout, "-D Driver Name\n");
  fprintf(stdout, "-u Username\n");
  fprintf(stdout, "-p Password\n");
  fprintf(stdout, "-s default database (schema)\n");
  fprintf(stdout, "-S Server name/address\n");
  fprintf(stdout, "-P Port number\n");
  fprintf(stdout, "-o Use only DSN for the connection\n");
  fprintf(stdout, "-a Additional connection string parameters\n");
  fprintf(stdout, "?  Displays this text\n");
}


void get_env_defaults()
{
  char *env_val;

  if ((env_val= getenv("TEST_DSN")) != NULL)
  {
    my_dsn= (SQLCHAR*)env_val;
  }
  if ((env_val= getenv("TEST_DRIVER")) != NULL)
  {
    my_drivername= (SQLCHAR*)env_val;
  }
  if ((env_val= getenv("TEST_SERVER")) != NULL)
  {
    my_servername= (SQLCHAR*)env_val;
  }
  if ((env_val= getenv("TEST_UID")) != NULL)
  {
    my_uid= (SQLCHAR*)env_val;
  }
  if ((env_val= getenv("TEST_PASSWORD")) != NULL)
  {
    my_pwd= (SQLCHAR*)env_val;
  }
  if ((env_val= getenv("MEMSQL_JWT")) != NULL)
  {
    jwt_password= (SQLCHAR*)env_val;
  }
  if ((env_val= getenv("TEST_SCHEMA")) != NULL)
  {
    my_schema= (SQLCHAR*)env_val;
  }
  if ((env_val= getenv("TEST_PORT")) != NULL)
  {
    int port= atoi(env_val);
    if (port > 0 && port < 0x10000)
    {
      my_port= port;
      _snprintf(ma_strport, sizeof(ma_strport), "PORT=%u", my_port);
    }
  }
  if ((env_val= getenv("TEST_ADD_PARAM")) != NULL)
  {
    add_connstr= env_val;
  }
}


void get_options(int argc, char **argv)
{
  int  i;

  get_env_defaults();

  for (i= 1; i < argc; i+= 2) /* "d:u:p:P:s:S:?" */
  {
    if (argv[i][0] != '-' || argv[i][1] == 0 || argc == i + 1)
    {
      Usage();
      exit(0);
    }

    switch(argv[i][1]) {
    case 'd':
      my_dsn= (SQLCHAR*)argv[i+1];
      break;
    case 'u':
      my_uid= (SQLCHAR*)argv[i+1];
      break;
    case 'p':
      my_pwd= (SQLCHAR*)argv[i+1];
      break;
    case 's':
      my_schema= (SQLCHAR*)argv[i+1];
      break;
    case 'P':
      my_port= atoi(argv[i+1]);
      break;
    case 'S':
      my_servername= (SQLCHAR*)argv[i+1];
      break;
    case 'D':
      my_drivername= (SQLCHAR*)argv[i+1];
      break;
    case 'o':
      UseDsnOnly= TRUE;
      --i; /* -o doesn't have value, thus we need to decrement argument index so we do not miss next option */
      break;
    case 'a':
      add_connstr= (SQLCHAR*)argv[i+1];
      break;
    case '?':
      Usage();
      exit(0);
      break;
    default:
      fprintf(stdout, "Unknown option %c\n", argv[i][1]);
      Usage();
      exit(0);
    }
  }

  _snprintf(ma_strport, sizeof(ma_strport), "PORT=%u", my_port);
}


int myrowcount(SQLHSTMT Stmt)
{
  int Rows=0;
  while (SQLFetch(Stmt) != SQL_NO_DATA)
    Rows++;
  return Rows;
}

int endswith(const char* const str, const char* const substr) {
    const size_t len = strlen(str);
    const size_t sub_len = strlen(substr);
    if(len >= sub_len)
        return !strcmp(str + (len - sub_len), substr);
    return 0;
}


#define mystmt(hstmt,r)  \
  do { \
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) \
      return FAIL; \
  } while (0)


void diag(const char *fstr, ...)
{
  va_list ap;
  va_start(ap, fstr);
  fprintf(stdout, "# ");
  vfprintf(stdout, fstr, ap);
  va_end(ap);
  fprintf(stdout,"\n");
}


void odbc_print_error(SQLSMALLINT HandleType, SQLHANDLE Handle)
{
  SQLCHAR SQLState[6];
  SQLINTEGER NativeError;
  SQLCHAR SQLMessage[SQL_MAX_MESSAGE_LENGTH];
  SQLSMALLINT TextLengthPtr;

  const SQLRETURN result = SQLGetDiagRec(HandleType, Handle, 1, SQLState, &NativeError, SQLMessage, SQL_MAX_MESSAGE_LENGTH, &TextLengthPtr);
  if(result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO) {
    fprintf(stdout, "[%s] (%d) %s\n", SQLState, NativeError, SQLMessage);
  } else {
    fprintf(stdout, "Error %d getting diagnostic records with SQLGetDiagRec()\n", result);
  }
}

int odbc_expect_error(SQLSMALLINT HandleType,
                      SQLHANDLE Handle,
                      const SQLCHAR* const state,
                      const SQLINTEGER native,
                      const SQLCHAR* const msg)
{
  SQLCHAR SQLState[6];
  SQLINTEGER NativeError;
  SQLCHAR SQLMessage[SQL_MAX_MESSAGE_LENGTH];

  const SQLRETURN result = SQLGetDiagRec(HandleType, Handle, 1, SQLState, &NativeError, SQLMessage, SQL_MAX_MESSAGE_LENGTH, NULL);
  if(result == SQL_SUCCESS || result == SQL_SUCCESS_WITH_INFO) {
    if((!state || !strncmp(state, SQLState, 5))
        && (native == -1 || native == NativeError)
        && (!msg || endswith(SQLMessage, msg))) {
      return OK;
    } else {
      fprintf(stdout, "Received: [%s] (%d) %s\n", SQLState, NativeError, SQLMessage);
    }
  } else {
    fprintf(stdout, "Error %d getting diagnostic records with SQLGetDiagRec()\n", result);
  }

  fprintf(stdout, "Expected: [%s] (%d) %s\n", state, native, msg);
  return FAIL;
}


char little_endian()
{
  int   x= 1;
  char *c= (char*)&x;

  return *c;
}


/* More or less copy of the function from MariaDB C/C */
size_t madbtest_convert_string(MARIADB_CHARSET_INFO *from_cs, const char *from, size_t *from_len,
                               MARIADB_CHARSET_INFO *to_cs, char *to, size_t *to_len, int *errorcode)
{
  size_t rc= -1;
  size_t save_len= *to_len;
  int    dummy, converted_len;

  if (errorcode == NULL)
    errorcode= &dummy;

  *errorcode= 0;
#ifdef _WIN32
  converted_len= MultiByteToWideChar(from_cs->codepage, 0, from, (int)*from_len, (wchar_t*)to, (int)*to_len);
  rc= (converted_len < 1 ? -1 : converted_len*sizeof(SQLWCHAR));
  *errorcode= GetLastError();
#else
  rc= MADB_ConvertString(from, from_len, from_cs, to, to_len, to_cs, errorcode);
#endif
  if (rc == -1)
  {
    diag("An error occurred while converting from %s to  %s. Error code %d", from_cs->csname, to_cs->name, *errorcode);
  }
  return rc;
}


#define MAX_COLUMNS 1000

#define mystmt_rows(hstmt, r, row)  \
	do { \
	if (!SQL_SUCCEEDED(r)) {\
    odbc_print_error(SQL_HANDLE_STMT, (hstmt));\
	  return row; }\
	} while (0)


int my_print_non_format_result_ex(SQLHSTMT Stmt, BOOL CloseCursor)
{
    SQLRETURN   rc;
    SQLUINTEGER nRowCount=0;
    SQLULEN     pcColDef;
    SQLCHAR     szColName[MAX_NAME_LEN+1];
    SQLCHAR     szData[MAX_COLUMNS][MAX_ROW_DATA_LEN]={{0}};
    SQLSMALLINT nIndex,ncol= 0,pfSqlType, pcbScale, pfNullable;
    SQLLEN      ind_strlen;

    rc = SQLNumResultCols(Stmt, &ncol);
    
    mystmt_rows(Stmt, rc, -1);

    for (nIndex = 1; nIndex <= ncol; ++nIndex)
    {
        rc = SQLDescribeCol(Stmt,nIndex,szColName, MAX_NAME_LEN, NULL,
                            &pfSqlType,&pcColDef,&pcbScale,&pfNullable);
        /* Returning in case of an error -nIndex we will see in the log column# */
        mystmt_rows(Stmt,rc,-nIndex);


        fprintf(stdout, "%s\t", szColName);

        rc = SQLBindCol(Stmt, nIndex, SQL_C_CHAR, szData[nIndex-1],
                        MAX_ROW_DATA_LEN+1, &ind_strlen);
        mystmt_rows(Stmt, rc, -nIndex);
    }

    fprintf(stdout, "\n");

    rc = SQLFetch(Stmt);
    while (SQL_SUCCEEDED(rc))
    {
        ++nRowCount;
        for (nIndex=0; nIndex< ncol; ++nIndex)
            fprintf(stdout, "%s\t", szData[nIndex]);

        fprintf(stdout, "\n");
        rc = SQLFetch(Stmt);
    }

    SQLFreeStmt(Stmt, SQL_UNBIND);

    if (CloseCursor)
    {
      SQLFreeStmt(Stmt, SQL_CLOSE);
    }

    fprintf(stdout, "# Total rows fetched: %d\n", (int)nRowCount);

    return nRowCount;
}


int my_print_non_format_result(SQLHSTMT Stmt)
{
  return my_print_non_format_result_ex(Stmt, TRUE);
}


SQLINTEGER SqlwcsLen(SQLWCHAR *str)
{
  SQLINTEGER result= 0;

  if (str)
  {
    while (*str)
    {
      ++result;
      ++str;
    }
  }
  return result;
}


wchar_t *sqlwchar_to_wchar_t(SQLWCHAR *in)
{
  static wchar_t buff[2048];
  char *to= (char *)buff;

  if (sizeof(wchar_t) == sizeof(SQLWCHAR))
    return (wchar_t *)in;
  else
  {
    size_t len= (SqlwcsLen(in) + 1)*sizeof(SQLWCHAR);
    int    error;
    size_t buff_size=  sizeof(buff);

    madbtest_convert_string(utf16, (char*)in, &len, utf32, to, &buff_size, &error);
  }

  return buff;
}

/* Same as my_print_non_format_result_ex, but uses SQLGetData, to fetch values, and not bind buffers */
int ma_print_result_getdata_exex(SQLHSTMT Stmt, BOOL CloseCursor, BOOL FetchAsWstr)
{
  SQLRETURN   rc;
  SQLUINTEGER nRowCount=0;
  SQLULEN     pcColDef;
  SQLCHAR     szColName[MAX_NAME_LEN + 1];
  SQLCHAR     szData[MAX_ROW_DATA_LEN]= {0};
  SQLWCHAR    wData[MAX_ROW_DATA_LEN]= {0};
  SQLSMALLINT nIndex, ncol= 0, pfSqlType, pcbScale, pfNullable;
  SQLLEN      ind_strlen;

  rc = SQLNumResultCols(Stmt, &ncol);

  mystmt_rows(Stmt, rc, -1);

  for (nIndex = 1; nIndex <= ncol; ++nIndex)
  {
    rc = SQLDescribeCol(Stmt, nIndex, szColName, MAX_NAME_LEN, NULL,
      &pfSqlType, &pcColDef, &pcbScale, &pfNullable);
    /* Returning in case of an error -nIndex we will see in the log column# */
    mystmt_rows(Stmt, rc, -nIndex);

    fprintf(stdout, "%s\t", szColName);
  }

  fprintf(stdout, "\n");

  while (SQL_SUCCEEDED(SQLFetch(Stmt)))
  {
    ++nRowCount;
    for (nIndex=0; nIndex< ncol; ++nIndex)
    {
      if (FetchAsWstr != FALSE)
      {
        rc= SQLGetData(Stmt, nIndex + 1, SQL_C_WCHAR, wData, sizeof(wData), &ind_strlen);
        mystmt_rows(Stmt, rc, -nIndex);
        fprintf(stdout, "%ls\t", sqlwchar_to_wchar_t(wData));
      }
      else
      {
        rc= SQLGetData(Stmt, nIndex + 1, SQL_C_CHAR, szData, sizeof(szData), &ind_strlen);
        mystmt_rows(Stmt, rc, -nIndex);
        fprintf(stdout, "%s\t", szData);
      }
    }

    fprintf(stdout, "\n");
  }

  if (CloseCursor != FALSE)
  {
    SQLFreeStmt(Stmt, SQL_CLOSE);
  }

  fprintf(stdout, "# Total rows fetched: %d\n", (int)nRowCount);

  return nRowCount;
}


int ma_print_result_getdata_ex(SQLHSTMT Stmt, BOOL CloseCursor)
{
  return ma_print_result_getdata_exex(Stmt, CloseCursor, FALSE);
}

int ma_print_result_getdata(SQLHSTMT Stmt)
{
  return my_print_non_format_result_ex(Stmt, TRUE);
}


#define OK_SIMPLE_STMT(stmt, stmtstr)\
if (!SQL_SUCCEEDED(SQLExecDirect((stmt), (SQLCHAR*)(stmtstr), (SQLINTEGER)strlen((const char*)stmtstr))))\
{\
  fprintf(stdout, "Error in %s:%d:\n", __FILE__, __LINE__);\
  odbc_print_error(SQL_HANDLE_STMT, (stmt));\
  return FAIL;\
}

#define OK_SIMPLE_STMTW(stmt, stmtstr)\
if (SQLExecDirectW((stmt), (SQLWCHAR*)(stmtstr), SQL_NTS) != SQL_SUCCESS)\
{\
  fprintf(stdout, "Error in %s:%d:\n", __FILE__, __LINE__);\
  odbc_print_error(SQL_HANDLE_STMT, (stmt));\
  return FAIL;\
}

#define ERR_SIMPLE_STMT(stmt, stmtstr)\
if (SQLExecDirect((stmt), (SQLCHAR*)(stmtstr), SQL_NTS) != SQL_ERROR)\
{\
  fprintf(stdout, "Error expected in %s:%d:\n", __FILE__, __LINE__);\
  return FAIL;\
}

#define ERR_SIMPLE_STMTW(stmt, stmtstr)\
if (SQLExecDirectW((stmt), (SQLWCHAR*)(stmtstr), SQL_NTS) != SQL_ERROR)\
{\
  fprintf(stdout, "Error expected in %s:%d:\n", __FILE__, __LINE__);\
  return FAIL;\
}

#define CHECK_HANDLE_RC(handletype, handle ,rc)\
do {\
  SQLRETURN local_rc= (rc); \
  if (!(SQL_SUCCEEDED(local_rc)))\
  {\
    fprintf(stdout, "Error (rc=%d) in %s:%d:\n", local_rc, __FILE__, __LINE__);\
    odbc_print_error((handletype), (handle));\
    return FAIL;\
  }\
} while(0)

#define CHECK_HANDLE_ERR(handletype, handle, rc, state, native, msg)\
do {\
  SQLRETURN local_rc= (rc); \
  if (!(local_rc == SQL_ERROR && odbc_expect_error(handletype, handle, state, native, msg))) \
  {\
    fprintf(stdout, "Error (rc=%d) in %s:%d:\n", local_rc, __FILE__, __LINE__);\
    return FAIL;\
  }\
} while(0)

#define CHECK_STMT_RC(stmt,rc) CHECK_HANDLE_RC(SQL_HANDLE_STMT,stmt,rc)
#define CHECK_DBC_RC(dbc,rc) CHECK_HANDLE_RC(SQL_HANDLE_DBC,dbc,rc)
#define CHECK_ENV_RC(env,rc) CHECK_HANDLE_RC(SQL_HANDLE_ENV,env,rc)
#define CHECK_DESC_RC(desc,rc) CHECK_HANDLE_RC(SQL_HANDLE_DESC,desc,rc)

#define EXPAND(x) x
#define CHECK_STMT_ERR(stmt, rc, ...) EXPAND(CHECK_HANDLE_ERR(SQL_HANDLE_STMT, stmt, rc, __VA_ARGS__))
#define CHECK_DBC_ERR(stmt, rc, ...) EXPAND(CHECK_HANDLE_ERR(SQL_HANDLE_DBC, stmt, rc, __VA_ARGS__))

#define FAIL_IF_NE_INT(got, exp, message) \
do {\
  long long local_exp = (exp), local_got = (got);\
  if (local_exp != local_got)\
    {\
        fprintf(stdout, "%s(exp: %lld, got: %lld) (File: %s:%d)\n", message, local_exp, local_got, __FILE__, __LINE__);\
        return FAIL;\
    }\
} while(0)

#define FAIL_IF_NE_STR(exp,got,message)\
  if (_stricmp((exp), (got)) != 0)\
    {\
        fprintf(stdout, "%s(exp: %s, got: %s) (File: %s:%d)\n", message, exp, got, __FILE__, __LINE__);\
        return FAIL;\
    }

#define FAIL_IF(expr,message)\
  if (expr)\
    {\
    fprintf(stdout, "%s (File: %s:%d)\n", message, __FILE__, __LINE__);\
    return FAIL;\
    }

#define IS(A) if (!(A)) { diag("Error in %s:%d", __FILE__, __LINE__); return FAIL; }
#define IS_OK(A) if ((A) != OK) { diag("Error in %s:%d", __FILE__, __LINE__); return FAIL; }
#define IS_STR(A,B,C) do {const char *loc_a=(const char *)(A), *loc_b=(const char *)(B);\
diag("%s %s", loc_a, loc_b);\
FAIL_IF(loc_a == NULL || loc_b == NULL || strncmp(loc_a, loc_b, (C)) != 0, "Strings do not match"); } while(0)

#define is_num(A,B) \
do {\
  long long local_a= (long long)(A), local_b= (long long)(B);\
  if (local_a != local_b)\
  {\
    diag("%s:%d: %s(%lld)!=%s(%lld)", __FILE__, __LINE__, #A, local_a, #B, local_b);\
    return FAIL;\
  }\
} while(0)

#define is_double(A,B) \
do {\
  double local_a= (double)(A), local_b= (double)(B);\
  if (local_a - local_b > 1e-7 || local_a - local_b < -1e-7)\
  {\
    diag("%s:%d: %s(%lf)!=%s(%lf)", __FILE__, __LINE__, #A, local_a, #B, local_b);\
    return FAIL;\
  }\
} while(0)

#define EXPECT_DBC(_Dbc, _Function, _Expected)\
do {\
  SQLRETURN ret= (_Function);\
  if (ret != (_Expected))\
  {\
    CHECK_DBC_RC(_Dbc, ret);\
  }\
} while(0)

#define EXPECT_STMT(_Stmt, _Function, _Expected)\
do {\
  SQLRETURN ret= (_Function);\
  if (ret != (_Expected))\
  {\
    CHECK_STMT_RC(_Stmt, ret);\
    diag("%s:%d: %s returned %d, expected %s(%d)",__FILE__, __LINE__, #_Function, ret, #_Expected, _Expected);\
    return FAIL;\
  }\
} while(0)

#define EXPECT_DESC(_Desc, Function, Expected)\
do {\
  SQLRETURN ret= (Function);\
  if (ret != (Expected))\
  {\
    CHECK_DESC_RC(_Desc, ret);\
    diag("%s:%d: %s returned %d, expected %s(%d)",__FILE__, __LINE__, #Function, ret, #Expected, Expected);\
    return FAIL;\
  }\
} while(0)

#define EXPECT_FAIL_MODE(_Stmt, _Function, _Mode, _SqlState) \
do {                                                  \
    EXPECT_STMT(_Stmt, _Function, NoSsps == (_Mode) ? SQL_ERROR : SQL_SUCCESS); \
    if (NoSsps == (_Mode)) {CHECK_SQLSTATE(_Stmt, _SqlState);} \
}while(0)

// Shorthands
#define CURSOR_STATE_ERROR    "24000", -1, "Invalid cursor state"
#define NULL_PTR_ERROR    "HY009", 0, "Invalid use of null pointer"

#define PREPARE(stmt, query)                    CHECK_STMT_RC(stmt, SQLPrepare(stmt, (SQLCHAR*)query, SQL_NTS))
#define PREPARE_CURSOR_ERR(stmt, query)         CHECK_STMT_ERR(stmt, SQLPrepare(stmt, (SQLCHAR*)query, SQL_NTS), CURSOR_STATE_ERROR)
#define EXEC_DIRECT(stmt, query)                CHECK_STMT_RC(stmt, SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS))
#define EXEC_DIRECT_CURSOR_ERR(stmt, query)     CHECK_STMT_ERR(stmt, SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS), CURSOR_STATE_ERROR)
#define EXECUTE(stmt)                           CHECK_STMT_RC(stmt, SQLExecute(stmt))
#define EXECUTE_CURSOR_ERR(stmt)                CHECK_STMT_ERR(stmt, SQLExecute(stmt), CURSOR_STATE_ERROR)
#define FETCH(stmt)                             CHECK_STMT_RC(stmt, SQLFetch(stmt))
#define FETCH_NO_DATA(stmt)                     EXPECT_STMT(stmt, SQLFetch(stmt), SQL_NO_DATA)
#define FETCH_CURSOR_ERR(stmt)                  CHECK_STMT_ERR(stmt, SQLFetch(stmt), CURSOR_STATE_ERROR)
#define CLOSE(stmt)                             CHECK_STMT_RC(stmt, SQLCloseCursor(stmt))
#define UNBIND(stmt)                            CHECK_STMT_RC(Stmt, SQLFreeStmt(stmt, SQL_UNBIND)); \
                                                CHECK_STMT_RC(Stmt, SQLFreeStmt(stmt, SQL_RESET_PARAMS))
#define FREE_HANDLE(stmt)                       CHECK_STMT_RC(stmt, SQLFreeHandle(SQL_HANDLE_STMT, stmt))

#define SSPS_ENABLED 0
#define SSPS_DISABLED 1

int my_fetch_int(SQLHANDLE Stmt, unsigned int ColumnNumber)
{
  int value= 0;
  SQLGetData(Stmt, ColumnNumber, SQL_INTEGER, &value, 0, NULL);
  diag(" my_fetch_int: %d", value);
  return value;
}

SQLWCHAR *my_fetch_wstr(SQLHSTMT Stmt, SQLWCHAR *buffer, SQLUSMALLINT icol, SQLLEN Length)
{
  SQLRETURN rc;
  SQLLEN nLen;

  rc= SQLGetData(Stmt, icol, SQL_WCHAR, buffer, Length, &nLen);
  if (!SQL_SUCCEEDED(rc))
    return sqlwchar_empty;
  return buffer;
}

const char *my_fetch_str(SQLHSTMT Stmt, SQLCHAR *szData, SQLUSMALLINT icol)
{
    SQLLEN nLen;

    SQLGetData(Stmt, icol, SQL_CHAR, szData, 1000, &nLen);
    /* If Null value - putting down smth meaningful. also that allows caller to
       better/(in more easy way) test the value */
    if (nLen == SQL_NULL_DATA)
    {
      strcpy((char*)szData, "(Null)"); 
    }
    diag(" my_fetch_str: %s(%ld)", szData, nLen);
    return((const char *)szData);
}


int check_sqlstate_ex(SQLHANDLE hnd, SQLSMALLINT hndtype, char *sqlstate)
{
  SQLCHAR     sql_state[6];
  SQLINTEGER  err_code= 0;
  SQLCHAR     err_msg[SQL_MAX_MESSAGE_LENGTH]= {0};
  SQLSMALLINT err_len= 0;

  memset(err_msg, 'C', SQL_MAX_MESSAGE_LENGTH);
  SQLGetDiagRec(hndtype, hnd, 1, sql_state, &err_code, err_msg,
                SQL_MAX_MESSAGE_LENGTH - 1, &err_len);

  IS_STR(sql_state, sqlstate, 5);

  return OK;
}
#define check_sqlstate(stmt, sqlstate) \
  check_sqlstate_ex((stmt), SQL_HANDLE_STMT, (sqlstate))

#define CHECK_SQLSTATE_EX(A,B,C) FAIL_IF(check_sqlstate_ex(A,B,C) != OK, "Unexpected sqlstate!")
#define CHECK_SQLSTATE(A,C) CHECK_SQLSTATE_EX(A, SQL_HANDLE_STMT, C)


int get_native_errcode(SQLHSTMT Stmt)
{
  SQLINTEGER  err_code = 0;

  SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, 1, NULL, &err_code, NULL, 0, NULL);
  return err_code;
}


int using_dm(HDBC hdbc)
{
  SQLCHAR val[20];
  SQLSMALLINT len;

  if (SQLGetInfo(hdbc, SQL_DM_VER, val, sizeof(val), &len) == SQL_ERROR)
    return 0;

  return 1;
}

/*
  Kills the connection using connection_id, node_id obtained from Stmt1
  Uses Stmt2 to run the KILL CONNECTION command
  Function assumes that Stmt1 and Stmt2 already have a connection to the DB
*/
void killConnection(SQLHSTMT *Stmt1, SQLHSTMT *Stmt2 ) {
  
  CHECK_SQLSTATE(Stmt1, "00000");
  CHECK_SQLSTATE(Stmt2, "00000");

  SQLINTEGER connection_id, node_id;
  char       Kill[64], buf_get_node_id[128];

  OK_SIMPLE_STMT(Stmt1, "SELECT connection_id(), aggregator_id();");
  CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));
  connection_id= my_fetch_int(Stmt1, 1);
  node_id = my_fetch_int(Stmt1, 2);
  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_CLOSE));

  /* From another connection, kill the connection created above */
  sprintf(Kill, "KILL CONNECTION %d %d", connection_id, node_id);
  OK_SIMPLE_STMT(Stmt2, Kill);
}

int mydrvconnect(SQLHENV *henv, SQLHDBC *hdbc, SQLHSTMT *hstmt, SQLCHAR *connIn)
{
  SQLCHAR   connOut[MAX_NAME_LEN+1];
  SQLSMALLINT len;

  CHECK_ENV_RC(*henv, SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, henv));

  CHECK_ENV_RC(*henv, SQLSetEnvAttr(*henv, SQL_ATTR_ODBC_VERSION,
                              (SQLPOINTER)(SQLLEN)OdbcVer, 0));

  CHECK_ENV_RC(*henv, SQLAllocHandle(SQL_HANDLE_DBC, *henv,  hdbc));

  CHECK_DBC_RC(*hdbc, SQLDriverConnect(*hdbc, NULL, connIn, SQL_NTS, connOut,
                                 MAX_NAME_LEN, &len, SQL_DRIVER_NOPROMPT));

  CHECK_DBC_RC(*hdbc, SQLSetConnectAttr(*hdbc, SQL_ATTR_AUTOCOMMIT,
                                  (SQLPOINTER)SQL_AUTOCOMMIT_ON, 0));

  CHECK_DBC_RC(*hdbc, SQLAllocHandle(SQL_HANDLE_STMT, *hdbc, hstmt));

  return OK;
}


int AllocEnvConn(SQLHANDLE *Env, SQLHANDLE *Connection)
{
  if (*Env == NULL)
  {
    FAIL_IF(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, NULL, Env)), "Couldn't allocate environment handle");

    FAIL_IF(!SQL_SUCCEEDED(SQLSetEnvAttr(*Env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)(SQLLEN)OdbcVer, 0)), "Couldn't set ODBC version");
  }
  FAIL_IF(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, *Env, Connection)), "Couldn't allocate connection handle");

  return OK;
}


SQLWCHAR * str2sqlwchar_on_gbuff(const char *str, size_t len, MARIADB_CHARSET_INFO *from_cs, MARIADB_CHARSET_INFO *to_cs);


/* Returns STMT handle for newly created connection, or NULL if connection is unsuccessful */
SQLHANDLE DoConnect(SQLHANDLE Connection, BOOL DoWConnect,
                    const char *dsn, const char *uid, const char *pwd, unsigned int port, const char *schema, unsigned long *options, const char *server,
                    const char *add_parameters)
{
  SQLHANDLE   stmt= NULL;
  char        DSNString[1024];
  char        DSNOut[1024];
  SQLSMALLINT Length;
  SQLRETURN   rc;
  SQLCHAR     driver_name[128];

  /* my_options |= 4; */ /* To enable debug */
  if (UseDsnOnly != FALSE)
  {
    _snprintf(DSNString, 1024, "DSN=%s", dsn ? dsn : (const char*)my_dsn);
    diag(DSNString);
  }
  else
  {
    _snprintf(DSNString, 1024, "DSN=%s;UID=%s;PWD={%s};PORT=%u;DATABASE=%s;OPTION=%lu;SERVER=%s;NO_SSPS=%d;%s;NO_CACHE=0", dsn ? dsn : (const char*)my_dsn,
      uid ? uid : (const char*)my_uid, pwd ? pwd : (const char*)my_pwd, port ? port : my_port,
      schema ? schema : (const char*)my_schema, options ? *options : my_options, server ? server : (const char*)my_servername,
      NoSsps, add_parameters ? add_parameters : "");
    diag("DSN=%s;UID=%s;PWD={%s};PORT=%u;DATABASE=%s;OPTION=%lu;SERVER=%s;NO_SSPS=%d;%s;NO_CACHE=0", dsn ? dsn : (const char*)my_dsn,
           uid ? uid : (const char*)my_uid, "********", port ? port : my_port,
           schema ? schema : (const char*)my_schema, options ? *options : my_options, server ? server : (const char*)my_servername,
           NoSsps, add_parameters ? add_parameters : "");
  }

  if (DoWConnect == FALSE)
  {
    if (!SQL_SUCCEEDED(SQLDriverConnect(Connection, NULL, (SQLCHAR *)DSNString, SQL_NTS, (SQLCHAR *)DSNOut, sizeof(DSNOut), &Length, SQL_DRIVER_NOPROMPT)))
    {
      odbc_print_error(SQL_HANDLE_DBC, Connection);
      return NULL;
    }
  }
  else
  {
    SQLWCHAR *DsnStringW= str2sqlwchar_on_gbuff(DSNString, strlen(DSNString) + 1, utf8, DmUnicode);
    if (!SQL_SUCCEEDED(SQLDriverConnectW(Connection, NULL, DsnStringW, SQL_NTS, NULL, 0, &Length, SQL_DRIVER_NOPROMPT)))
    {
      odbc_print_error(SQL_HANDLE_DBC, Connection);
      return NULL;
    }
  }

  if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, Connection, &stmt)))
  {
    diag("Could not create Stmt handle. Connection: %x", Connection);
    return NULL;
  }

  if (unicode_driver < 0)
  {
    rc= SQLGetInfo(Connection, SQL_DRIVER_NAME, driver_name, sizeof(driver_name), NULL);
    if (SQL_SUCCEEDED(rc))
    {
      /* ANSI driver file name contains a.{dll|so} */
      unicode_driver= strstr((char*)driver_name, "a.") ? ANSI_DRIVER : UNICODE_DRIVER;
    }
  }

  return stmt;
}


int ODBC_Connect(SQLHANDLE *Env, SQLHANDLE *Connection, SQLHANDLE *Stmt)
{
  *Env=         NULL;
  *Connection=  NULL;

  IS(AllocEnvConn(Env, Connection));

  *Stmt= DoConnect(*Connection, FALSE, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);

  return (*Stmt == NULL ? FAIL : OK);
}


int ODBC_ConnectW(SQLHANDLE Env, SQLHANDLE *Connection, SQLHANDLE *Stmt)
{
  *Connection=  NULL;

  FAIL_IF(!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, Env, Connection)), "Couldn't allocate connection handle");

  *Stmt= DoConnect(*Connection, TRUE, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);

  return (*Stmt == NULL ? FAIL : OK);
}

void ODBC_Disconnect(SQLHANDLE Env, SQLHANDLE Connection, SQLHANDLE Stmt)
{
  if (Stmt != NULL)
  {
    SQLFreeHandle(SQL_HANDLE_STMT, Stmt);
  }
  if (Connection != NULL)
  {
    SQLDisconnect(Connection);
    SQLFreeHandle(SQL_HANDLE_DBC, Connection);
  }
  if ( Env!= NULL)
  {
    SQLFreeHandle(SQL_HANDLE_ENV, Env);
  }
}


SQLHANDLE ConnectWithCharset(SQLHANDLE *conn, const char *charset_name, const char *add_parameters)
{
  char charset_clause[64];

  _snprintf(charset_clause, sizeof(charset_clause), "CHARSET=%s;%s", charset_name, add_parameters ? add_parameters : "");

  return DoConnect(*conn, FALSE, NULL, NULL, NULL, 0, NULL, NULL, NULL, charset_clause);
}


struct st_ma_server_variable
{
  int     global;
  const char *  name;
  int     value;
};

/* We do not expect many variables to be changed in single test */
#define MAX_CHANGED_VARIABLES_PER_TEST 5
static struct st_ma_server_variable changed_server_variable[MAX_CHANGED_VARIABLES_PER_TEST];

const char *locality[]=     {"LOCAL ", "GLOBAL "};
const char *set_template=   "SET %s%s=%d",
           *show_template=  "SHOW %s%s LIKE '%s'";

/* Macros for 1st parameter of set_variable */
#define LOCAL  0
#define GLOBAL 1

int reset_changed_server_variables(void)
{
  int i, error= 0;
  char query[512];

  /* Traversing array in reverse direction - in this way if same variable was changed more than once,
     it will get its original value */
  for (i= MAX_CHANGED_VARIABLES_PER_TEST - 1; i > -1; --i)
  {
    if (changed_server_variable[i].name != NULL)
    {
      int size= _snprintf(query, sizeof(query), set_template, locality[changed_server_variable[i].global],
                          changed_server_variable[i].name, changed_server_variable[i].value);
      if (error == 0 && !SQL_SUCCEEDED(SQLExecDirect(Stmt, (SQLCHAR*)query, size)))
      {
        error= 1;
        diag("Following variables were not reset to their original values:");
      }

      if (error != 0)
      {
        diag(" %s%s to %d", locality[changed_server_variable[i].global],
                          changed_server_variable[i].name, changed_server_variable[i].value);
      }
    }
  }

  return error;
}


int connect_and_run_tests(MA_ODBC_TESTS *tests, BOOL ProvideWConnection, BOOL NoSSPS)
{
  int         rc, i=1, all_tests = 0, failed=0;
  SQLWCHAR   *buff_before_test;
  char        comment[256];
  char        driver_name[256];
  SQLSMALLINT driver_name_len;

  if (ODBC_Connect(&Env,&Connection,&Stmt) == FAIL)
  {
    odbc_print_error(SQL_HANDLE_DBC, Connection);
    ODBC_Disconnect(Env,Connection,Stmt);
    fprintf(stdout, "HALT! Could not connect to the server\n");
    return 1;
  }
  if (ProvideWConnection && ODBC_ConnectW(Env, &wConnection, &wStmt) == FAIL)
    {
      odbc_print_error(SQL_HANDLE_DBC, wConnection);
      ODBC_Disconnect(Env, wConnection, wStmt);
      fprintf(stdout, "HALT! Could not connect to the server with Unicode function\n");
      return 1;
    }
  if(SQLGetInfo(Connection, SQL_DRIVER_NAME, driver_name, 256, &driver_name_len))
  {
    odbc_print_error(SQL_HANDLE_DBC, Connection);
    return 1;
  }
  fprintf(stdout, "Using driver: %s, my_drivername specified: %s\n", driver_name, my_drivername);
  fprintf(stdout, "1..%d\n", tests_planned);
    while (tests->title)
    {
        buff_before_test= buff_pos;

        all_tests++;
        if (tests->required_driver_type > -1 && tests->required_driver_type != unicode_driver)
        {
            printf("ok %d # SKIP This testcase is designed for %s drivers only\n", i,
                unicode_driver == 0 ? "Unicode" : "ANSI" );
            rc = 1;
        }
        else {
            int expected_to_fail = tests->test_type & (NoSSPS ? CSPS_FAIL | CSPS_TO_FIX : SSPS_FAIL | SSPS_TO_FIX);
            rc = expected_to_fail ? FAIL : tests->my_test();
        }
        comment[0] = '\0';
        if (rc!= SKIP && !test_expected_to_succeed(tests->test_type, NoSsps))
        {
            get_test_error_message(tests->test_type, rc, comment);
            rc= OK;
        }
        else if (rc == FAIL)
        {
            failed++;
        }

        if (reset_changed_server_variables())
        {
            fprintf(stdout, "HALT! An error occurred while tried to reset server variables changed by the test!\n");
        }

        buff_pos= buff_before_test;
        *buff_pos= 0;
        if (Stmt != NULL)
        {
            SQLFreeStmt(Stmt, SQL_DROP);
        }

        fprintf(stdout, "%s %d - %s%s\n", test_status[rc], i++,tests->title, comment);
        tests++;

        SQLAllocHandle(SQL_HANDLE_STMT, Connection, &Stmt);
        /* reset Statement */
        fflush(stdout);
    }

    ODBC_Disconnect(Env,Connection,Stmt);

    if (failed)
    {
        fprintf(stdout,"Failed %d out of %d tests!\n", failed, all_tests);
        return 1;
    }
    return 0;
}

void cleanup()
{
    time_t start_time = time(NULL);
    const char *errorMessage = NULL;
    char buff[1024];

    printf("Running cleanup...\n");

    MYSQL* mysql = mysql_init(NULL);
    MYSQL_FIELD *field;
    MYSQL_RES *tbl_result = NULL, *proc_result = NULL;
    MYSQL_ROW row;

    if (!mysql)
    {
        errorMessage = "failed to init the connection";
        goto end_no_free;
    }
    if (!mysql_real_connect(mysql, (const char *)my_servername, (const char *)my_uid, (const char *)my_pwd, (const char *)my_schema, my_port, NULL, 0))
    {
        errorMessage = mysql_error(mysql);
        goto end_no_free;
    }
    sprintf(buff, "SELECT CONCAT('DROP ', routine_type, ' IF EXISTS `', routine_name, '`;')\
                   FROM information_schema.routines WHERE routine_schema = '%s'", my_schema);
    if (mysql_query(mysql, buff) || !(proc_result = mysql_store_result(mysql)))
    {
        errorMessage = mysql_error(mysql);
        goto end;
    }
    while ( (row = mysql_fetch_row(proc_result)) )
    {
        unsigned long *lengths = mysql_fetch_lengths(proc_result);
        if (mysql_real_query(mysql, row[0], lengths[0]))
        {
            errorMessage = mysql_error(mysql);
            goto end;
        }
    }

    sprintf(buff, "SELECT CONCAT('DROP ', IF(table_type = 'BASE TABLE', 'TABLE', 'VIEW'), ' IF EXISTS `', table_name, '`;')\
                   FROM information_schema.tables WHERE table_schema = '%s'", my_schema);
    if (mysql_query(mysql, buff) || !(tbl_result = mysql_store_result(mysql)))
    {
        errorMessage = mysql_error(mysql);
        goto end;
    }
    while ( (row = mysql_fetch_row(tbl_result)) )
    {
        unsigned long *lengths = mysql_fetch_lengths(tbl_result);
        if (mysql_real_query(mysql, row[0], lengths[0]))
        {
            errorMessage = mysql_error(mysql);
            goto end;
        }
    }
end:
    mysql_free_result(tbl_result);
    mysql_free_result(proc_result);
end_no_free:
    if (errorMessage)   // mysql_close(mysql) frees the memory pointed to by errorMessage
    {
        printf("Cleanup failed: %s\n\n", errorMessage);
    }

    mysql_close(mysql);

    if (!errorMessage)
    {
        printf("Cleanup finished successfully in %ld seconds\n\n", time(NULL) - start_time);
    }
}

int run_tests_ex(MA_ODBC_TESTS *tests, BOOL ProvideWConnection)
{
  utf16= (little_endian() ? &utf16le : &utf16be);
  utf32= (little_endian() ? &utf32le : &utf32be);

  if (sizeof(SQLWCHAR) == 4)
  {
    DmUnicode= utf32;
  }
  else
  {
    DmUnicode= utf16;
  }

  if (utf8 == NULL || utf16 == NULL || utf32 == NULL)
  {
    fprintf(stdout, "HALT! Could not load charset info %p %p %p\n", utf8, utf16, utf32);
    return 1;
  }

  wdsn=        str2sqlwchar_on_gbuff((const char*)my_dsn,        strlen((const char*)my_dsn) + 1,        utf8, DmUnicode);
  wuid=        str2sqlwchar_on_gbuff((const char*)my_uid,        strlen((const char*)my_uid) + 1,        utf8, DmUnicode);
  wpwd=        str2sqlwchar_on_gbuff((const char*)my_pwd,        strlen((const char*)my_pwd) + 1,        utf8, DmUnicode);
  wschema=     str2sqlwchar_on_gbuff((const char*)my_schema,     strlen((const char*)my_schema) + 1,     utf8, DmUnicode);
  wservername= str2sqlwchar_on_gbuff((const char*)my_servername, strlen((const char*)my_servername) + 1, utf8, DmUnicode);
  wdrivername= str2sqlwchar_on_gbuff((const char*)my_drivername, strlen((const char*)my_drivername) + 1, utf8, DmUnicode);
  wstrport=    str2sqlwchar_on_gbuff((const char*)ma_strport,    strlen((const char*)ma_strport) + 1,    utf8, DmUnicode);
  wadd_connstr=str2sqlwchar_on_gbuff((const char*)add_connstr,   strlen((const char*)add_connstr) + 1,   utf8, DmUnicode);
  if (getenv("TRAVIS") != NULL)
  {
    Travis= 1;
    if (_stricmp(getenv("TRAVIS_OS_NAME"), "osx") == 0)
    {
      TravisOnOsx= 1;
    }
  }

  cleanup();

  fprintf(stdout, "Running tests in the client-side prepared statements mode...\n");
  NoSsps = 1;
  int csps_fail = connect_and_run_tests(tests, ProvideWConnection, NoSsps);
  fprintf(stdout, "Tests %s in the client-side prepared statements mode!\n\n", csps_fail ? "failed" : "passed");

  cleanup();

  fprintf(stdout, "Running tests in the server-side prepared statements mode...\n");
  NoSsps = 0;
  int ssps_fail = connect_and_run_tests(tests, ProvideWConnection, NoSsps);
  fprintf(stdout, "Tests %s in the server-side prepared statements mode!\n\n", ssps_fail ? "failed" : "passed");

  return csps_fail || ssps_fail;
}

int run_tests(MA_ODBC_TESTS *tests)
{
  return run_tests_ex(tests, FALSE);
}

int get_show_value(int global, const char * show_type, const char * var_name)
{
  int size, result;
  char query[512];

  size= _snprintf(query, sizeof(query), show_template, global ? locality[GLOBAL] : locality[LOCAL],
            show_type, var_name);

  /* Using automatically allocated (by the framework) STMT handle*/
  if (!SQL_SUCCEEDED(SQLExecDirect(Stmt, (SQLCHAR*)query, size))
   || !SQL_SUCCEEDED(SQLFetch(Stmt)))
  {
    /* atm I can't thing about any -1 value that can be interesting for tests.
       otherwise it will have to be changed */
    return -1;
  }

  result= my_fetch_int(Stmt, 2);

  SQLFreeStmt(Stmt, SQL_CLOSE);

  return result;
}


int get_server_status(int global, const char * var_name)
{
  return get_show_value(global, "STATUS", var_name); 
}


int get_server_variable(int global, const char * var_name)
{
  return get_show_value(global, "VARIABLES", var_name);
}

#define GET_SERVER_STATUS(int_result, global, name) int_result= get_server_status(global, name);\
  FAIL_IF(int_result < 0, "Could not get server status");

#define GET_SERVER_VAR(int_result, global, name) int_result= get_server_variable(global, name);\
  FAIL_IF(int_result < 0, "Could not get server variable");


/* Helper function to change server variable's value, but preserve its current
   value in order to set it back after test*/
int set_variable(int global, const char * var_name, int value)
{
  int i, size, cur_value;
  char query[512];

  for (i= 0; changed_server_variable[i].name != NULL && i < MAX_CHANGED_VARIABLES_PER_TEST; ++i);

  if (i == MAX_CHANGED_VARIABLES_PER_TEST)
  {
    FAIL_IF(TRUE, "For developer: the test has reached limit of variable changes. Please make MAX_CHANGED_VARIABLES_PER_TEST bigger")
  }

  GET_SERVER_VAR(cur_value, global, var_name);

  size= _snprintf(query, sizeof(query), set_template, global ? locality[GLOBAL] : locality[LOCAL],
                var_name, value);

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, (SQLCHAR*)query, size));

  changed_server_variable[i].global=  global;
  changed_server_variable[i].name=    var_name;
  changed_server_variable[i].value=   cur_value;

  return OK;
}

void printHex(char *str, int len)
{
  int i;
  for (i= 0; i<len; ++i)
  {
    printf("%02hhx", str[i]);
  }
}

#define IS_WSTR(a, b, c) \
do { \
  SQLWCHAR *val_a= (SQLWCHAR*)(a), *val_b= (SQLWCHAR*)(b); \
  int val_len= (int)(c); \
  if (memcmp(val_a, val_b, val_len * sizeof(SQLWCHAR)) != 0) { \
    printf("#%s('", #a); \
    printHex((char*)val_a, val_len*sizeof(SQLWCHAR)); \
    printf("') != %s('", #b); \
    printHex((char*)val_b, val_len*sizeof(SQLWCHAR)); \
    printf("') in %s:%d", __FILE__, __LINE__); \
    printf("\n"); \
    return FAIL;\
  } \
} while (0);



SQLWCHAR *dup_char_as_sqlwchar(SQLCHAR *from)
{
  SQLWCHAR *to= malloc((strlen((char *)from) + 1) * sizeof(SQLWCHAR));
  SQLWCHAR *out= to;
  while (from && *from)
    *(to++)= (SQLWCHAR)*(from++);
  *to= 0;
  return out;
}


/* It can be utf8 or something, but the function does not recode to utf16 - simply copies each byte to SQLWCHAR */
SQLWCHAR* latin_as_sqlwchar(const char *str, SQLWCHAR *buffer)
{
  SQLWCHAR *res= buffer;

  if (str == NULL)
  {
    return NULL;
  }

  while(*str)
  {
    *buffer++= *str++;
  }

  *buffer= 0;

  return res;
}

/**
  @len[in] - length of the source string in bytes, including teminating NULL
 */
SQLWCHAR * str2sqlwchar_on_gbuff(const char *str, size_t len, MARIADB_CHARSET_INFO *from_cs, MARIADB_CHARSET_INFO *to_cs)
{
  SQLWCHAR   *res= buff_pos;
  size_t      rc, buff_size= sizeof(sqlwchar_buff) - (buff_pos - sqlwchar_buff)*sizeof(SQLWCHAR);
  int         error;
  const char *src= str;

  if (len > buff_size/sizeof(SQLWCHAR))
  {
    /* That should not normally happen, but happened. Size of buffer has to be adjusted. */
    diag("NOTE: Global buffer for strings conversion is exhausted. The string will be truncated and the test will probably fail because of that");
    diag("Adjust buffer size and re-compile test. Otherwise using of shorter uid/pwd/dsn etc may help");

    return sqlwchar_empty;
  }

  rc= madbtest_convert_string(from_cs, src, &len, to_cs, (char*)buff_pos, &buff_size, &error);

  if (rc != (size_t)(-1))
  {
    buff_pos+= rc/sizeof(SQLWCHAR);
  }
  else
  {
    return sqlwchar_empty;
  }
  return res;
}


#define LW(latin_str) latin_as_sqlwchar((char*)latin_str, sqlwchar_buff)
/* wchar_t to SQLLWCHAR, used by both W and WW macros */
#define WL(A,B) (sizeof(wchar_t) == sizeof(SQLWCHAR) ? (SQLWCHAR*)A : str2sqlwchar_on_gbuff((char*)(A), (B+1)*sizeof(wchar_t), utf32, utf16))
/* Converting char const to sqlWchar */
#define WW(A) WL(L##A,wcslen(L##A))
/* Converts wchar_t string into SQLWCHAR, const or variable */
#define W(A) WL(A,wcslen(A))
/**
 Helper for converting a (char *) to a (SQLWCHAR *)
*/
/* Char(utf8) to slqWchar */
#define CW(str) str2sqlwchar_on_gbuff((const char*)(str), strlen((const char*)(str))+1, utf8, DmUnicode)

/* @n[in] - number of characters to compare. Negative means treating of strings as null-terminated */
int sqlwcharcmp(const SQLWCHAR *s1, const SQLWCHAR *s2, int n)
{
  if(s1 == NULL || s2 == NULL)
  {
    return s1!=s2;
  }

  while (n != 0 && *s1 && *s2 && *s1==*s2)
  {
    ++s1; ++s2; --n;
  }

  return n != 0 && *s1!=*s2;
}

int sqlwcharequal(const SQLWCHAR *s1, const SQLWCHAR *s2)
{
  while(*s1)
  {
    if(*s1++ != *s2++)
    {
      return 0;
    }
  }

  return !*s2;
}

size_t sqlwcharlen(const SQLWCHAR *s)
{
    size_t i;
    for(i = 0; s[i]; ++i) {}
    return i;
}


BOOL ServerNotOlderThan(SQLHDBC Conn, unsigned int major, unsigned int minor, unsigned int patch)
{
  unsigned int ServerMajor= 0, ServerMinor= 0, ServerPatch= 0;
  SQLCHAR ServerVersion[32];

  SQLGetInfo(Conn, SQL_DBMS_VER, ServerVersion, sizeof(ServerVersion), NULL);

  sscanf((const char*)ServerVersion, "%u.%u.%u", &ServerMajor, &ServerMinor, &ServerPatch);

  if (ServerMajor < major || (ServerMajor == major && (ServerMinor < minor || (ServerMinor == minor && ServerPatch < patch))))
  {
    return FALSE;
  }

  return TRUE;
}

#ifdef _WIN32
char * GenGUID(char *buffer)
{
  char *ptr= buffer;

  return ptr;
}
#endif

BOOL UnixOdbc()
{
#ifdef SQL_ATTR_UNIXODBC_VERSION
  return TRUE;
#endif
  return FALSE;
 }


int GetDefaultCharType(int WType, BOOL isAnsiConnection)
{
    if (isAnsiConnection)
    {
        switch (WType) {
            case SQL_WCHAR:
                return SQL_CHAR;
            case SQL_WVARCHAR:
                return SQL_VARCHAR;
            case SQL_WLONGVARCHAR:
                return SQL_LONGVARCHAR;
            default:
                return WType;
        }
    }
    return WType;
}

int getDbCharSize()
{
  MYSQL* mysql = mysql_init(NULL);
  MYSQL_RES *res;
  MYSQL_ROW row;
  char *collation;
  int char_size = 1;
  const char *query = "SELECT @@collation_database";

  if (!mysql_real_connect(mysql, (const char *)my_servername, (const char *)my_uid, (const char *)my_pwd, NULL, my_port, NULL, 0))
    return 0;

  if (mysql_query(mysql, query))
    return 0;
  res = mysql_store_result(mysql);
  if ((row = mysql_fetch_row(res)))
    collation = row[0];
  else
  {
    mysql_free_result(res);
    return 0;
  }
  if (!strncmp(collation, "utf8mb4", 7))
    char_size = 4;
  else if (!strncmp(collation, "utf8", 4))
    char_size = 3;
  mysql_free_result(res);
  mysql_close(mysql);
  return char_size;
}

/* Looks like same version of iOdbc behaves differently on os x and linux, thus for some tests we need to be able to tell there is iOdbc run */
BOOL iOdbcOnOsX()
{
#ifdef __APPLE__
  return iOdbc();
#endif
  return FALSE;
}


/* Needed for crazy iODBC on OS X */
int iOdbcSetParamBufferSize(SQLHSTMT hStmt, SQLUSMALLINT ParamIdx, SQLLEN BufferSize)
{
  if (iOdbcOnOsX())
  {
    SQLHDESC Apd;

    CHECK_STMT_RC(hStmt, SQLGetStmtAttr(hStmt, SQL_ATTR_APP_PARAM_DESC, &Apd, SQL_IS_POINTER, NULL));
    CHECK_DESC_RC(Apd, SQLSetDescField(Apd, ParamIdx, SQL_DESC_OCTET_LENGTH, (SQLPOINTER)BufferSize, SQL_IS_INTEGER));
  }

  return OK;
}


const char * OdbcTypeAsString(SQLSMALLINT TypeId, char *Buffer)
{
  static char AsNumber[8];

  switch (TypeId)
  {
  case SQL_C_TINYINT: return "SQL_TINYINT";
  case SQL_C_STINYINT: return "SQL_C_STINYINT";
  case SQL_C_UTINYINT: return "SQL_C_UTINYINT";
  case SQL_C_SHORT: return "SQL_SMALLINT";
  case SQL_C_SSHORT: return "SQL_C_SSHORT";
  case SQL_C_USHORT: return "SQL_C_USHORT";
  case SQL_C_LONG: return "SQL_INTEGER";
  case SQL_C_SLONG: return "SQL_C_SLONG";
  case SQL_C_ULONG: return "SQL_C_ULONG";
  case SQL_C_UBIGINT: return "SQL_C_UBIGINT";
  case SQL_C_SBIGINT: return "SQL_C_SBIGINT";
  case SQL_BIGINT: return "SQL_BIGINT";
  case SQL_C_DOUBLE: return "SQL_DOUBLE";
  case SQL_C_FLOAT: return "SQL_REAL";
  case SQL_DECIMAL: return "SQL_DECIMAL";
  case SQL_VARCHAR: return "SQL_VARCHAR";
  default:
    if (Buffer == NULL)
    {
      Buffer= AsNumber;
    }
    sprintf(Buffer, "%hu", TypeId);
  }
  return Buffer;
}

BOOL WindowsDM(HDBC hdbc)
{
  return using_dm(hdbc) && UnixOdbc() == FALSE && iOdbc() == FALSE;
}

#ifdef WIN32
#define my_alloca(SZ) _alloca((size_t) (SZ))
#else
#define my_alloca(SZ) alloca((size_t) (SZ))
#endif

#define BROWSER_AUTH_FLAG_TEST_FIRST_CALL (1 << 1)
#define BROWSER_AUTH_FLAG_TEST_SECOND_CALL (1 << 2)
#define BROWSER_AUTH_FLAG_TEST_ENDPOINT (1 << 3)
#define BROWSER_AUTH_FLAG_TEST_SHORT_TIMEOUT (1 << 4)

#endif      /* #ifndef _tap_h_ */
