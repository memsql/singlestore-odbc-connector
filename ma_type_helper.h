/************************************************************************************
   Copyright (C) 2013,2018 MariaDB Corporation AB
   
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
#ifndef _ma_type_helper_h_
#define _ma_type_helper_h_
#include <ma_odbc.h>
#include <mysql.h>

#define is_char_sql_type(type) \
  ((type) == SQL_CHAR || (type) == SQL_VARCHAR || (type) == SQL_LONGVARCHAR)
#define is_wchar_sql_type(type) \
  ((type) == SQL_WCHAR || (type) == SQL_WVARCHAR || (type) == SQL_WLONGVARCHAR)
#define is_binary_sql_type(type) \
  ((type) == SQL_BINARY || (type) == SQL_VARBINARY || \
   (type) == SQL_LONGVARBINARY)
#define is_datetime_sql_type(type) \
  ((type) == SQL_TYPE_DATE || (type) == SQL_TYPE_TIME || (type) == SQL_TYPE_TIMESTAMP)

SQLINTEGER S2_GetColumnSize(const MYSQL_FIELD *field, const MADB_TypeInfo *odbc_type_info, const char *type_name_full, BOOL force_db_charset, SQLUINTEGER db_charset);
SQLLEN S2_GetCharacterOctetLength(const MYSQL_FIELD *field, const MADB_TypeInfo *odbc_type_info);
SQLSMALLINT S2_GetDecimalDigits(const MYSQL_FIELD *field);

#endif  // _ma_type_helper_h_
