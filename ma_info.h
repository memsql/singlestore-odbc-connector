/************************************************************************************
   Copyright (C) 2013, 2016 MariaDB Corporation AB

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
#ifndef _ma_info_h_
#define _ma_info_h_

typedef struct
{
  char *TypeName;
  SQLSMALLINT DataType;
  SQLINTEGER ColumnSize;
  /* size in bytes for binary/string data; otherwise
     the number of characters in the character representation of data */
  char *LiteralPrefix;
  char *LiteralSuffix;
  char *CreateParams;
  SQLSMALLINT Nullable;
  SQLSMALLINT CaseSensitive;
  SQLSMALLINT Searchable;
  SQLSMALLINT Unsigned;
  SQLSMALLINT FixedPrecScale;
  SQLSMALLINT AutoUniqueValue;
  char *LocalTypeName;
  SQLSMALLINT MinimumScale;
  SQLSMALLINT MaximumScale;
  SQLSMALLINT SqlDateTimeSub;
  SQLSMALLINT IntervalPrecision;
  SQLINTEGER NumPrecRadix;
  SQLSMALLINT SqlDataType;
} MADB_TypeInfo;

const MADB_TypeInfo* GetTypeInfo(SQLSMALLINT SqlType, MYSQL_FIELD *Field);
SQLRETURN MADB_GetTypeInfo(SQLHSTMT StatementHandle,
                           SQLSMALLINT DataType);

#endif /* _ma_info_h_ */
