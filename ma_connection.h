/************************************************************************************
   Copyright (C) 2013,2018 MariaDB Corporation AB
                 2021 SingleStore, Inc.

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
#ifndef _ma_connection_h_
#define _ma_connection_h_

#define MADB_CONN_OPT_NOT_SUPPORTED 0
#define MADB_CONN_OPT_BEFORE        1
#define MADB_CONN_OPT_AFTER         2
#define MADB_CONN_OPT_BOTH          3

/* sql_mode's identifiers */
enum enum_madb_sql_mode {MADB_NO_BACKSLASH_ESCAPES, MADB_ANSI_QUOTES };

struct st_ma_connection_methods;

struct st_madb_isolation {
  long SqlIsolation;
  char *StrIsolation;
};

struct st_ma_connection_methods
{
  SQLRETURN (*SetConnectAttr)(MADB_Dbc *Dbc, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER StringLength, my_bool isWChar);
  SQLRETURN (*GetConnectAttr)(MADB_Dbc *Dbc, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER BufferLength, SQLINTEGER *StringLengthPtr, my_bool isWChar);
  SQLRETURN (*Connect)(MADB_Dbc *Connection, MADB_Dsn *Dsn);
  SQLRETURN (*EndTran)(MADB_Dbc *Dbc, SQLSMALLINT CompletionType);
  SQLRETURN (*GetFunctions)(MADB_Dbc *Dbc, SQLUSMALLINT FunctionId, SQLUSMALLINT *SupportedPtr);
  SQLRETURN(*GetInfo)(MADB_Dbc *Dnc, SQLUSMALLINT InfoType, SQLPOINTER InfoValuePtr,
                      SQLSMALLINT BufferLength, SQLSMALLINT *StringLengthPtr, my_bool isWChar);
  SQLRETURN (*DriverConnect)(MADB_Dbc *Dbc, SQLHWND WindowHandle, SQLCHAR *InConnectionString,
                             SQLULEN StringLength1, SQLCHAR *OutConnectionString,
                             SQLULEN BufferLength, SQLSMALLINT *StringLength2Ptr,
                             SQLUSMALLINT DriverCompletion);
};

my_bool CheckConnection(MADB_Dbc *Dbc);

SQLRETURN MADB_DbcFree(MADB_Dbc *Connection);
MADB_Dbc * MADB_DbcInit(MADB_Env *Env);
SQLRETURN MADB_Dbc_GetCurrentDB(MADB_Dbc *Connection, SQLPOINTER CurrentDB, SQLINTEGER CurrentDBLength, 
                                SQLSMALLINT *StringLengthPtr, my_bool isWChar);
/* Has platform versions */
char* MADB_GetDefaultPluginsDir(char* Buffer, size_t Size);

#define MADB_SUPPORTED_CONVERSIONS  SQL_CVT_BIGINT | SQL_CVT_BINARY | SQL_CVT_BIT | SQL_CVT_CHAR | SQL_CVT_DATE |\
                                    SQL_CVT_DECIMAL | SQL_CVT_DOUBLE | SQL_CVT_FLOAT |\
                                    SQL_CVT_INTEGER | SQL_CVT_LONGVARBINARY | SQL_CVT_LONGVARCHAR | SQL_CVT_NUMERIC |\
                                    SQL_CVT_REAL | SQL_CVT_SMALLINT | SQL_CVT_TIME | SQL_CVT_TIMESTAMP |\
                                    SQL_CVT_TINYINT | SQL_CVT_VARBINARY | SQL_CVT_VARCHAR | SQL_CVT_WCHAR | \
                                    SQL_CVT_WLONGVARCHAR | SQL_CVT_WVARCHAR
/**************** Helper macros ****************/
/* check if the connection is established */
#define MADB_Dbc_ACTIVE(a) \
  ((a)->mariadb && mysql_get_socket((a)->mariadb) != MARIADB_INVALID_SOCKET)

#define MADB_Dbc_DSN(a) \
(a) && (a)->Dsn  

#define MADB_CONNECTED(DbConnHandler) (DbConnHandler->mariadb && mysql_get_socket(DbConnHandler->mariadb) != MARIADB_INVALID_SOCKET)
#endif /* _ma_connection_h */
