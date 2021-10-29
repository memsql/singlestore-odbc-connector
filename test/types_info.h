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

/*
 This file contains expected results for SQLGetTypeInfo for both ODBC v2 and v3.
 It pretty much has copies of TypeInfoV2 and TypeInfoV3, defined in ma_info.c, modified to represent how user receives them in fetch
 */

#ifndef MARIADB_CONNECTOR_ODBC_TYPES_H
#define MARIADB_CONNECTOR_ODBC_TYPES_H

#include "tap.h"

#define TYPES_COUNT 56
#define TYPE_INFO_FIELDS_COUNT 19
#define SQL_DATA_TYPES_COUNT 38

typedef struct
{
    char *TypeName;
    SQLSMALLINT DataType;
    SQLINTEGER ColumnSize;
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
    SQLSMALLINT DataTypeAlias;
} MADB_TypeInfo;

char* fieldNames[TYPE_INFO_FIELDS_COUNT] = {"TYPE_NAME", "DATA_TYPE", "COLUMN_SIZE", "LITERAL_PREFIX", "LITERAL_SUFFIX",
                                            "CREATE_PARAMS", "NULLABLE", "CASE_SENSITIVE", "SEARCHABLE", "UNSIGNED_ATTRIBUTE", "FIXED_PREC_SCALE",
                                            "AUTO_UNIQUE_VALUE", "LOCAL_TYPE_NAME", "MINIMUM_SCALE", "MAXIMUM_SCALE", "SQL_DATA_TYPE", "SQL_DATETIME_SUB",
                                            "NUM_PREC_RADIX", "INTERVAL_PRECISION"};

short fieldTypes[TYPE_INFO_FIELDS_COUNT] = {SQL_VARCHAR, SQL_SMALLINT, SQL_INTEGER, SQL_VARCHAR, SQL_VARCHAR,
                                            SQL_VARCHAR, SQL_SMALLINT, SQL_SMALLINT, SQL_SMALLINT, SQL_SMALLINT,
                                            SQL_SMALLINT, SQL_SMALLINT, SQL_VARCHAR, SQL_SMALLINT, SQL_SMALLINT,
                                            SQL_SMALLINT, SQL_SMALLINT, SQL_INTEGER, SQL_SMALLINT};

int fieldSizes[TYPE_INFO_FIELDS_COUNT] = {715827882, 5, 10, 715827882, 715827882, 21, 5, 5, 5, 5, 5, 5, 715827882, 5, 5, 5, 5, 10, 5};

short sqlDataTypes[SQL_DATA_TYPES_COUNT] = {
  SQL_CHAR, SQL_VARCHAR, SQL_LONGVARCHAR, SQL_WCHAR, SQL_WVARCHAR, SQL_WLONGVARCHAR, SQL_DECIMAL, SQL_NUMERIC, SQL_SMALLINT,
  SQL_INTEGER, SQL_REAL, SQL_REAL, SQL_DOUBLE, SQL_BINARY, SQL_TINYINT, SQL_BIGINT, SQL_BINARY, SQL_VARBINARY, SQL_LONGVARBINARY,
  SQL_TYPE_DATE, SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, SQL_INTERVAL_MONTH, SQL_INTERVAL_YEAR, SQL_INTERVAL_YEAR_TO_MONTH,
  SQL_INTERVAL_DAY, SQL_INTERVAL_HOUR, SQL_INTERVAL_MINUTE, SQL_INTERVAL_SECOND, SQL_INTERVAL_DAY_TO_HOUR, SQL_INTERVAL_DAY_TO_MINUTE,
  SQL_INTERVAL_DAY_TO_SECOND, SQL_INTERVAL_HOUR_TO_MINUTE, SQL_INTERVAL_HOUR_TO_SECOND, SQL_INTERVAL_MINUTE_TO_SECOND, SQL_GUID, SQL_ALL_TYPES,
  -1234 /* garbage */
};

MADB_TypeInfo TypeInfoV3[TYPES_COUNT]=
{
    // Integer numbers
    {"bool",SQL_TINYINT,1,"","","NULL",1,1,3,0,0,0,"bool",0,0,0,0,10, SQL_TINYINT},
    {"boolean",SQL_TINYINT,1,"","","NULL",1,1,3,0,0,0,"boolean",0,0,0,0,10, SQL_TINYINT},
    {"tinyint",SQL_TINYINT,3,"","","NULL",1,0,3,SQL_FALSE,0,1,"tinyint",0,0,0,0,10, SQL_TINYINT},
    {"tinyint unsigned",SQL_TINYINT,3,"","","NULL",1,0,3,SQL_TRUE,0,1,"tinyint unsigned",0,0,0,0,10, SQL_TINYINT},
    {"smallint",SQL_SMALLINT,5,"","","NULL",1,0,3,SQL_FALSE,0,1,"smallint",0,0,0,0,10, SQL_SMALLINT},
    {"smallint unsigned",SQL_SMALLINT,5,"","","NULL",1,0,3,1,0,1,"smallint unsigned",0,0,0,0,10, SQL_SMALLINT},
    {"mediumint",SQL_INTEGER,7,"","","NULL",1,0,3,SQL_FALSE,0,1,"mediumint",0,0,0,0,10, SQL_INTEGER},
    {"mediumint unsigned",SQL_INTEGER,8,"","","NULL",1,0,3,1,0,1,"mediumint unsigned",0,0,0,0,10, SQL_INTEGER},
    {"int",SQL_INTEGER,10,"","","NULL",1,0,3,SQL_FALSE,0,1,"int",0,0,0,0,10,SQL_INTEGER},
    {"int unsigned",SQL_INTEGER,10,"","","NULL",1,0,3,1,0,1,"int unsigned",0,0,0,0,10, SQL_INTEGER},
    {"integer",SQL_INTEGER,10,"","","NULL",1,0,3,SQL_FALSE,0,1,"integer",0,0,0,0,10, SQL_INTEGER},
    {"integer unsigned",SQL_INTEGER,10,"","","NULL",1,0,3,1,0,1,"integer unsigned",0,0,0,0,10, SQL_INTEGER},
    {"bigint",SQL_BIGINT,19,"","","NULL",1,0,3,SQL_FALSE,0,1,"bigint",0,0,0,0,10, SQL_BIGINT},
    {"bigint unsigned",SQL_BIGINT,20,"","","NULL",1,0,3,1,0,1,"bigint unsigned",0,0,0,0,10, SQL_BIGINT},
    // Real numbers
    {"float",SQL_REAL,7,"","","precision,scale",1,0,3,0,0,1,"float",-38,38,0,0,10, SQL_REAL},
    {"double",SQL_DOUBLE,15,"","","precision,scale",1,0,3,0,0,1,"double",-308,308,0,0,10, SQL_DOUBLE},
    {"double precision",SQL_DOUBLE,15,"","","precision,scale",1,0,3,0,0,1,"double precision",-308,308,0,0,10, SQL_DOUBLE},
    {"real",SQL_DOUBLE,15,"","","precision,scale",1,0,3,0,0,1,"real",-308,308,0,0,10, SQL_DOUBLE},
    {"decimal",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"decimal",-308,308,0,0,10, SQL_DECIMAL},
    {"dec",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"dec",-308,308,0,0,10, SQL_DECIMAL},
    {"fixed",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"fixed",-308,308,0,0,10, SQL_DECIMAL},
    {"numeric",SQL_NUMERIC,65,"","","precision,scale",1,0,3,0,0,1,"numeric", -308,308,0,0,10, SQL_NUMERIC}, /* Todo: ?? */
    // Time and Date
    {"date",SQL_TYPE_DATE,10,"\'","\'","NULL",1,0,3,0,0,0,"date",0,0,SQL_CODE_DATE,0,0, SQL_DATETIME},
    {"time",SQL_TYPE_TIME,8,"\'","\'","scale",1,0,3,0,0,0,"time",0,0,SQL_CODE_TIME,0,0, SQL_DATETIME},
    {"datetime",SQL_TYPE_TIMESTAMP,26,"\'","\'","scale",1,0,3,0,0,0,"datetime",0,0,SQL_CODE_TIMESTAMP,0,0, SQL_DATETIME},
    {"timestamp",SQL_TYPE_TIMESTAMP,26,"\'","\'","scale",1,0,3,0,0,0,"timestamp",0,0,SQL_CODE_TIMESTAMP,0,0, SQL_DATETIME},
    {"year",SQL_SMALLINT,4,"","","NULL",1,0,3,SQL_FALSE,0,1,"year",0,0,0,0,10, SQL_SMALLINT},
    // String Types
    {"binary",SQL_BINARY,255,"\'","\'","length",1,1,3,0,0,0,"binary",0,0,0,0,0, SQL_BINARY},
    {"bit",SQL_BINARY,8,"","","NULL",1,1,3,0,0,0,"bit",0,0,0,0,0, SQL_BINARY},
    {"char",SQL_CHAR,255,"\'","\'","length",1,0,3,0,0,0,"char",0,0,0,0,0, SQL_CHAR},
    {"char",SQL_WCHAR,255,"\'","\'","length",1,0,3,0,0,0,"char",0,0,0,0,0, SQL_WCHAR},
    {"varbinary",SQL_VARBINARY,255,"\'","\'","length",1,1,3,0,0,0,"varbinary",0,0,0,0,0, SQL_VARBINARY},
    {"long varbinary",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,3,0,0,0,"long varbinary",0,0,0,0,0, SQL_LONGVARBINARY},
    {"long varchar",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"long varchar",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"varchar",SQL_VARCHAR,255,"\'","\'","length",1,0,3,0,0,0,"varchar",0,0,0,0,0, SQL_VARCHAR},
    {"varchar",SQL_WVARCHAR,255,"\'","\'","length",1,0,3,0,0,0,"varchar",0,0,0,0,0, SQL_WVARCHAR},
    {"longblob",SQL_LONGVARBINARY,2147483647,"\'","\'","NULL",1,1,3,0,0,0,"longblob",0,0,0,0,0, SQL_LONGVARBINARY},
    {"mediumblob",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,3,0,0,0,"mediumblob",0,0,0,0,0, SQL_LONGVARBINARY},
    {"blob",SQL_LONGVARBINARY,65535,"\'","\'","NULL",1,1,3,0,0,0,"blob",0,0,0,0,0, SQL_LONGVARBINARY},
    {"tinyblob",SQL_LONGVARBINARY,255,"\'","\'","NULL",1,1,3,0,0,0,"tinyblob",0,0,0,0,0, SQL_LONGVARBINARY},
    {"longtext",SQL_LONGVARCHAR,2147483647,"\'","\'","NULL",1,0,3,0,0,0,"longtext",0,0,0,0,0, SQL_LONGVARCHAR},
    {"mediumtext",SQL_LONGVARCHAR,16777215,"\'","\'","NULL",1,0,3,0,0,0,"mediumtext",0,0,0,0,0, SQL_LONGVARCHAR},
    {"text",SQL_LONGVARCHAR,65535,"\'","\'","NULL",1,0,3,0,0,0,"text",0,0,0,0,0, SQL_LONGVARCHAR},
    {"tinytext",SQL_LONGVARCHAR,255,"\'","\'","NULL",1,0,3,0,0,0,"tinytext",0,0,0,0,0, SQL_LONGVARCHAR},
    {"longtext",SQL_WLONGVARCHAR,2147483647,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"longtext",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"mediumtext",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"mediumtext",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"text",SQL_WLONGVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"text",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"tinytext",SQL_WLONGVARCHAR,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"tinytext",0,0,0,0,0, SQL_WLONGVARCHAR},
    // Other types
    {"json",SQL_LONGVARCHAR,16777216,"\'","\'","NULL",1,0,3,0,0,0,"json",0,0,0,0,0, SQL_LONGVARCHAR},
    {"json",SQL_WLONGVARCHAR,16777216,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"json",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"geographypoint",SQL_VARBINARY,20,"\'","\'","NULL",1,0,3,0,0,0,"geographypoint",0,0,0,0,0, SQL_VARBINARY},
    {"geography",SQL_VARBINARY,255,"\'","\'","NULL",1,0,3,0,0,0,"geography",0,0,0,0,0, SQL_VARBINARY},
    {"enum",SQL_VARCHAR,65535,"\'","\'","NULL",1,0,3,0,0,0,"enum",0,0,0,0,0, SQL_VARCHAR},
    {"enum",SQL_WVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"enum",0,0,0,0,0, SQL_WVARCHAR},
    {"set",SQL_VARCHAR,64,"\'","\'","NULL",1,0,3,0,0,0,"set",0,0,0,0,0, SQL_VARCHAR},
    {"set",SQL_WVARCHAR,64,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"set",0,0,0,0,0, SQL_WVARCHAR}
};

MADB_TypeInfo TypeInfoV2[TYPES_COUNT]=
{
    // Integer numbers
    {"bool",SQL_TINYINT,1,"","","NULL",1,1,3,0,0,0,"bool",0,0,0,0,10, SQL_TINYINT},
    {"boolean",SQL_TINYINT,1,"","","NULL",1,1,3,0,0,0,"boolean",0,0,0,0,10, SQL_TINYINT},
    {"tinyint",SQL_TINYINT,3,"","","NULL",1,0,3,SQL_FALSE,0,1,"tinyint",0,0,0,0,10, SQL_TINYINT},
    {"tinyint unsigned",SQL_TINYINT,3,"","","NULL",1,0,3,1,0,1,"tinyint unsigned",0,0,0,0,10, SQL_TINYINT},
    {"smallint",SQL_SMALLINT,5,"","","NULL",1,0,3,SQL_FALSE,0,1,"smallint",0,0,0,0,10, SQL_SMALLINT},
    {"smallint unsigned",SQL_SMALLINT,5,"","","NULL",1,0,3,SQL_TRUE,0,1,"smallint unsigned",0,0,0,0,10, SQL_SMALLINT},
    {"mediumint",SQL_INTEGER,7,"","","NULL",1,0,3,SQL_FALSE,0,1,"mediumint",0,0,0,0,10, SQL_INTEGER},
    {"mediumint unsigned",SQL_INTEGER,8,"","","NULL",1,0,3,1,0,1,"mediumint unsigned",0,0,0,0,10, SQL_INTEGER},
    {"int",SQL_INTEGER,10,"","","NULL",1,0,3,SQL_FALSE,0,1,"int",0,0,0,0,10,SQL_INTEGER},
    {"int unsigned",SQL_INTEGER,10,"","","NULL",1,0,3,1,0,1,"int unsigned",0,0,0,0,10, SQL_INTEGER},
    {"integer",SQL_INTEGER,10,"","","NULL",1,0,3,SQL_FALSE,0,1,"integer",0,0,0,0,10, SQL_INTEGER},
    {"integer unsigned",SQL_INTEGER,10,"","","NULL",1,0,3,1,0,1,"integer unsigned",0,0,0,0,10, SQL_INTEGER},
    {"bigint",SQL_BIGINT,19,"","","NULL",1,0,3,SQL_FALSE,0,1,"bigint",0,0,0,0,10, SQL_BIGINT},
    {"bigint unsigned",SQL_BIGINT,20,"","","NULL",1,0,3,1,0,1,"bigint unsigned",0,0,0,0,10, SQL_BIGINT},
    // Real numbers
    {"float",SQL_REAL,7,"","","precision,scale",1,0,3,0,0,1,"float",-38,38,0,0,10, SQL_REAL},
    {"double",SQL_DOUBLE,15,"","","precision,scale",1,0,3,0,0,1,"double",-308,308,0,0,10, SQL_DOUBLE},
    {"double precision",SQL_DOUBLE,15,"","","precision,scale",1,0,3,0,0,1,"double precision",-308,308,0,0,10, SQL_DOUBLE},
    {"real",SQL_DOUBLE,15,"","","precision,scale",1,0,3,0,0,1,"real",-308,308,0,0,10, SQL_DOUBLE},
    {"decimal",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"decimal",-308,308,0,0,10, SQL_DECIMAL},
    {"dec",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"dec",-308,308,0,0,10, SQL_DECIMAL},
    {"fixed",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"fixed",-308,308,0,0,10, SQL_DECIMAL},
    {"numeric",SQL_NUMERIC,65,"","","precision,scale",1,0,3,0,0,1,"numeric", -308,308,0,0,10, SQL_NUMERIC}, /* Todo: ?? */
    // Time and Date
    {"date",SQL_DATE,10,"\'","\'","NULL",1,0,3,0,0,0,"date",0,0,0,0,0, SQL_DATETIME, SQL_TYPE_DATE},
    {"time",SQL_TIME,18,"\'","\'","scale",1,0,3,0,0,0,"time",0,0,0,0,0, SQL_DATETIME, SQL_TYPE_TIME},
    {"datetime",SQL_TIMESTAMP,26,"\'","\'","scale",1,0,3,0,0,0,"datetime",0,0,0,0,0, SQL_DATETIME, SQL_TYPE_TIMESTAMP},
    {"timestamp",SQL_TIMESTAMP,26,"\'","\'","scale",1,0,3,0,0,0,"timestamp",0,0,0,0,0, SQL_DATETIME, SQL_TYPE_TIMESTAMP},
    {"year",SQL_SMALLINT,4,"","","NULL",1,0,3,SQL_FALSE,0,1,"year",0,0,0,0,10, SQL_SMALLINT},
    // String Types
    {"binary",SQL_BINARY,255,"\'","\'","length",1,1,3,0,0,0,"binary",0,0,0,0,0, SQL_BINARY},
    {"bit",SQL_BINARY,8,"","","NULL",1,1,3,0,0,0,"bit",0,0,0,0,0, SQL_BINARY},
    {"char",SQL_CHAR,255,"\'","\'","length",1,0,3,0,0,0,"char",0,0,0,0,0, SQL_CHAR},
    {"char",SQL_WCHAR,255,"\'","\'","length",1,0,3,0,0,0,"char",0,0,0,0,0, SQL_WCHAR},
    {"varbinary",SQL_VARBINARY,255,"\'","\'","length",1,1,3,0,0,0,"varbinary",0,0,0,0,0, SQL_VARBINARY},
    {"long varbinary",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,3,0,0,0,"long varbinary",0,0,0,0,0, SQL_LONGVARBINARY},
    {"long varchar",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"long varchar",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"varchar",SQL_VARCHAR,255,"\'","\'","length",1,0,3,0,0,0,"varchar",0,0,0,0,0, SQL_VARCHAR},
    {"varchar",SQL_WVARCHAR,255,"\'","\'","length",1,0,3,0,0,0,"varchar",0,0,0,0,0, SQL_WVARCHAR},
    {"longblob",SQL_LONGVARBINARY,2147483647,"\'","\'","NULL",1,1,3,0,0,0,"longblob",0,0,0,0,0, SQL_LONGVARBINARY},
    {"mediumblob",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,3,0,0,0,"mediumblob",0,0,0,0,0, SQL_LONGVARBINARY},
    {"blob",SQL_LONGVARBINARY,65535,"\'","\'","NULL",1,1,3,0,0,0,"blob",0,0,0,0,0, SQL_LONGVARBINARY},
    {"tinyblob",SQL_LONGVARBINARY,255,"\'","\'","NULL",1,1,3,0,0,0,"tinyblob",0,0,0,0,0, SQL_LONGVARBINARY},
    {"longtext",SQL_LONGVARCHAR,2147483647,"\'","\'","NULL",1,0,3,0,0,0,"longtext",0,0,0,0,0, SQL_LONGVARCHAR},
    {"mediumtext",SQL_LONGVARCHAR,16777215,"\'","\'","NULL",1,0,3,0,0,0,"mediumtext",0,0,0,0,0, SQL_LONGVARCHAR},
    {"text",SQL_LONGVARCHAR,65535,"\'","\'","NULL",1,0,3,0,0,0,"text",0,0,0,0,0, SQL_LONGVARCHAR},
    {"tinytext",SQL_LONGVARCHAR,255,"\'","\'","NULL",1,0,3,0,0,0,"tinytext",0,0,0,0,0, SQL_LONGVARCHAR},
    {"longtext",SQL_WLONGVARCHAR,2147483647,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"longtext",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"mediumtext",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"mediumtext",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"text",SQL_WLONGVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"text",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"tinytext",SQL_WLONGVARCHAR,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"tinytext",0,0,0,0,0, SQL_WLONGVARCHAR},
    // Other types
    {"json",SQL_LONGVARCHAR,16777216,"\'","\'","NULL",1,0,3,0,0,0,"json",0,0,0,0,0, SQL_LONGVARCHAR},
    {"json",SQL_WLONGVARCHAR,16777216,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"json",0,0,0,0,0, SQL_WLONGVARCHAR},
    {"geographypoint",SQL_VARBINARY,20,"\'","\'","NULL",1,0,3,0,0,0,"geographypoint",0,0,0,0,0, SQL_VARBINARY},
    {"geography",SQL_VARBINARY,255,"\'","\'","NULL",1,0,3,0,0,0,"geography",0,0,0,0,0, SQL_VARBINARY},
    {"enum",SQL_VARCHAR,65535,"\'","\'","NULL",1,0,3,0,0,0,"enum",0,0,0,0,0, SQL_VARCHAR},
    {"enum",SQL_WVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"enum",0,0,0,0,0, SQL_WVARCHAR},
    {"set",SQL_VARCHAR,64,"\'","\'","NULL",1,0,3,0,0,0,"set",0,0,0,0,0, SQL_VARCHAR},
    {"set",SQL_WVARCHAR,64,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"set",0,0,0,0,0, SQL_WVARCHAR}
};

#endif //MARIADB_CONNECTOR_ODBC_TYPES_H
