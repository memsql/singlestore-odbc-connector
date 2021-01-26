/*
 This file contains expected results for SQLGetTypeInfo for both ODBC v2 and v3.
 It pretty much has copies of TypeInfoV2 and TypeInfoV3, defined in ma_info.c, modified to represent how user receives them in fetch
 */

#ifndef MARIADB_CONNECTOR_ODBC_TYPES_H
#define MARIADB_CONNECTOR_ODBC_TYPES_H

#include "tap.h"

#define TYPES_COUNT 56
#define TYPE_INFO_FIELDS_COUNT 19

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
    SQLSMALLINT SqlDataType1;
    SQLSMALLINT SqlDateTimeSub;
    SQLINTEGER NumPrecRadix;
    SQLSMALLINT IntervalPrecision;
    SQLSMALLINT SqlDataType;
} MADB_TypeInfo;

char* fieldNames[TYPE_INFO_FIELDS_COUNT] = {"TYPE_NAME", "DATA_TYPE", "COLUMN_SIZE", "LITERAL_PREFIX", "LITERAL_SUFFIX",
                                            "CREATE_PARAMS", "NULLABLE", "CASE_SENSITIVE", "SEARCHABLE", "UNSIGNED_ATTRIBUTE", "FIXED_PREC_SCALE",
                                            "AUTO_UNIQUE_VALUE", "LOCAL_TYPE_NAME", "MINIMUM_SCALE", "MAXIMUM_SCALE", "SQL_DATA_TYPE", "SQL_DATETIME_SUB",
                                            "NUM_PREC_RADIX", "INTERVAL_PRECISION"};

short fieldTypes[TYPE_INFO_FIELDS_COUNT] = {SQL_VARCHAR, SQL_SMALLINT, SQL_INTEGER, SQL_VARCHAR, SQL_VARCHAR,
                                            SQL_VARCHAR, SQL_SMALLINT, SQL_SMALLINT, SQL_SMALLINT, SQL_SMALLINT,
                                            SQL_SMALLINT, SQL_SMALLINT, SQL_VARCHAR, SQL_SMALLINT, SQL_SMALLINT,
                                            SQL_SMALLINT, SQL_SMALLINT, SQL_INTEGER, SQL_SMALLINT};

short fieldSizes[TYPE_INFO_FIELDS_COUNT] = {19, 5, 10, 2, 2, 16, 5, 5, 5, 5, 5, 5, 19, 5, 5, 5, 5, 10, 5};

MADB_TypeInfo TypeInfoV3[]=
{
    // Integer numbers
    {"bool",SQL_TINYINT,1,"","","NULL",1,1,3,0,0,0,"bool",0,0,0,0,10, SQL_BIT},
    {"boolean",SQL_TINYINT,1,"","","NULL",1,1,3,0,0,0,"boolean",0,0,0,0,10, SQL_BIT},
    {"bit",SQL_BIT,8,"","","NULL",1,1,3,0,0,0,"bit",0,0,0,0,10, SQL_BIT},
    {"tinyint",SQL_TINYINT,3,"","","NULL",1,0,3,SQL_FALSE,0,1,"tinyint",0,0,0,0,10, SQL_TINYINT},
    {"tinyint unsigned",SQL_TINYINT,3,"","","NULL",1,0,3,SQL_TRUE,0,1,"tinyint unsigned",0,0,0,0,10, SQL_TINYINT},
    {"smallint",SQL_SMALLINT,5,"","","NULL",1,0,3,SQL_FALSE,0,1,"smallint",0,0,0,0,10, SQL_SMALLINT},
    {"smallint unsigned",SQL_SMALLINT,5,"","","NULL",1,0,3,1,0,1,"smallint unsigned",0,0,0,0,10, SQL_SMALLINT},
    {"mediumint",SQL_INTEGER,7,"","","NULL",1,0,3,SQL_FALSE,0,1,"mediumint",0,0,0,0,10},
    {"mediumint unsigned",SQL_INTEGER,8,"","","NULL",1,0,3,1,0,1,"mediumint unsigned",0,0,0,0,10, SQL_INTEGER},
    {"integer",SQL_INTEGER,10,"","","NULL",1,0,3,SQL_FALSE,0,1,"integer",0,0,0,0,10,SQL_INTEGER},
    {"integer unsigned",SQL_INTEGER,10,"","","NULL",1,0,3,1,0,1,"integer unsigned",0,0,0,0,10, SQL_INTEGER},
    {"int",SQL_INTEGER,10,"","","NULL",1,0,3,SQL_FALSE,0,1,"int",0,0,0,0,10, SQL_INTEGER},
    {"int unsigned",SQL_INTEGER,10,"","","NULL",1,0,3,1,0,1,"int unsigned",0,0,0,0,10, SQL_INTEGER},
    {"bigint",SQL_BIGINT,19,"","","NULL",1,0,3,SQL_FALSE,0,1,"bigint",0,0,0,0,10, SQL_BIGINT},
    {"bigint unsigned",SQL_BIGINT,20,"","","NULL",1,0,3,1,0,1,"bigint unsigned",0,0,0,0,10, SQL_BIGINT},
    // Real numbers
    {"float",SQL_FLOAT,10,"","","precision,scale",1,0,3,0,0,1,"float",-38,38,0,0,10, SQL_FLOAT},
    {"double",SQL_DOUBLE,17,"","","precision,scale",1,0,3,0,0,1,"double",-308,308,0,0,10, SQL_DOUBLE},
    {"double precision",SQL_DOUBLE,17,"","","precision,scale",1,0,3,0,0,1,"double precision",-308,308,0,0,10, SQL_DOUBLE},
    {"real",SQL_DOUBLE,17,"","","precision,scale",1,0,3,0,0,1,"real",-308,308,0,0,10, SQL_DOUBLE},
    {"decimal",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"decimal",-308,308,0,0,10, SQL_DECIMAL},
    {"dec",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"dec",-308,308,0,0,10, SQL_DECIMAL},
    {"fixed",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"fixed",-308,308,0,0,10, SQL_DECIMAL},
    {"numeric",SQL_NUMERIC,65,"","","precision,scale",1,0,3,0,0,1,"numeric", -308,308,0,0,10, SQL_NUMERIC}, /* Todo: ?? */
    // Time and Date
    {"date",SQL_TYPE_DATE,10,"\'","\'","NULL",1,0,3,0,0,0,"date",0,0,0,0,10, SQL_DATETIME},
    {"time",SQL_TYPE_TIME,8,"\'","\'","NULL",1,0,3,0,0,0,"time",0,0,0,0,10, SQL_DATETIME},
    {"datetime",SQL_TYPE_TIMESTAMP,16,"\'","\'","NULL",1,0,3,0,0,0,"datetime",0,0,0,0,10, SQL_DATETIME},
    {"timestamp",SQL_TYPE_TIMESTAMP,16,"\'","\'","scale",1,0,3,0,0,0,"timestamp",0,0,0,0,10, SQL_DATETIME},
    {"year",SQL_SMALLINT,4,"","","NULL",1,0,3,SQL_FALSE,0,1,"year",0,0,0,0,10, SQL_SMALLINT},
    // String Types
    {"binary",SQL_BINARY,255,"\'","\'","length",1,1,3,0,0,0,"binary",0,0,0,0,10, SQL_BINARY},
    {"char",SQL_CHAR,255,"\'","\'","length",1,0,3,0,0,0,"char",0,0,0,0,10, SQL_CHAR},
    {"char",SQL_WCHAR,255,"\'","\'","length",1,0,3,0,0,0,"char",0,0,0,0,10, SQL_WCHAR},
    {"varbinary",SQL_VARBINARY,255,"\'","\'","length",1,1,3,0,0,0,"varbinary",0,0,0,0,10, SQL_VARBINARY},
    {"long varbinary",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,3,0,0,0,"long varbinary",0,0,0,0,10, SQL_LONGVARBINARY},
    {"long varchar",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"long varchar",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"varchar",SQL_VARCHAR,255,"\'","\'","length",1,0,3,0,0,0,"varchar",0,0,0,0,10, SQL_VARCHAR},
    {"varchar",SQL_WVARCHAR,255,"\'","\'","length",1,0,3,0,0,0,"varchar",0,0,0,0,10, SQL_WVARCHAR},
    {"longblob",SQL_LONGVARBINARY,2147483647,"\'","\'","NULL",1,1,3,0,0,0,"longblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"mediumblob",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,3,0,0,0,"mediumblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"blob",SQL_LONGVARBINARY,65535,"\'","\'","NULL",1,1,3,0,0,0,"blob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"tinyblob",SQL_LONGVARBINARY,255,"\'","\'","NULL",1,1,3,0,0,0,"tinyblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"longtext",SQL_LONGVARCHAR,2147483647,"\'","\'","NULL",1,0,3,0,0,0,"longtext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"mediumtext",SQL_LONGVARCHAR,16777215,"\'","\'","NULL",1,0,3,0,0,0,"mediumtext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"text",SQL_LONGVARCHAR,65535,"\'","\'","NULL",1,0,3,0,0,0,"text",0,0,0,0,10, SQL_LONGVARCHAR},
    {"tinytext",SQL_LONGVARCHAR,255,"\'","\'","NULL",1,0,3,0,0,0,"tinytext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"longtext",SQL_WLONGVARCHAR,2147483647,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"longtext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"mediumtext",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"mediumtext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"text",SQL_WLONGVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"text",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"tinytext",SQL_WLONGVARCHAR,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"tinytext",0,0,0,0,10, SQL_WLONGVARCHAR},
    // Other types
    {"json",SQL_LONGVARCHAR,16777216,"\'","\'","NULL",1,0,3,0,0,0,"json",0,0,0,0,10, SQL_LONGVARCHAR},
    {"json",SQL_WLONGVARCHAR,16777216,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"json",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"geographypoint",SQL_VARBINARY,20,"\'","\'","NULL",1,0,3,0,0,0,"geographypoint",0,0,0,0,10, SQL_VARBINARY},
    {"geography",SQL_VARBINARY,255,"\'","\'","NULL",1,0,3,0,0,0,"geography",0,0,0,0,10, SQL_VARBINARY},
    {"enum",SQL_VARCHAR,65535,"\'","\'","NULL",1,0,3,0,0,0,"enum",0,0,0,0,10, SQL_VARCHAR},
    {"enum",SQL_WVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"enum",0,0,0,0,10, SQL_WVARCHAR},
    {"set",SQL_VARCHAR,64,"\'","\'","NULL",1,0,3,0,0,0,"set",0,0,0,0,10, SQL_VARCHAR},
    {"set",SQL_WVARCHAR,64,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"set",0,0,0,0,10, SQL_WVARCHAR},
    {NULL,0,0,NULL,NULL,NULL,0,0,0,0,0,0,NULL,0,0,0,0,0}
};

MADB_TypeInfo TypeInfoV2[]=
{
    // Integer numbers
    {"bool",SQL_TINYINT,1,"","","NULL",1,1,3,0,0,0,"bool",0,0,0,0,10, SQL_BIT},
    {"boolean",SQL_TINYINT,1,"","","NULL",1,1,3,0,0,0,"boolean",0,0,0,0,10, SQL_BIT},
    {"bit",SQL_BIT,8,"","","NULL",1,1,3,0,0,0,"bit",0,0,0,0,10, SQL_BIT},
    {"tinyint",SQL_TINYINT,3,"","","NULL",1,0,3,SQL_FALSE,0,1,"tinyint",0,0,0,0,10, SQL_TINYINT},
    {"tinyint unsigned",SQL_TINYINT,3,"","","NULL",1,0,3,1,0,1,"tinyint unsigned",0,0,0,0,10, SQL_TINYINT},
    {"smallint",SQL_SMALLINT,5,"","","NULL",1,0,3,SQL_FALSE,0,1,"smallint",0,0,0,0,10, SQL_SMALLINT},
    {"smallint unsigned",SQL_SMALLINT,5,"","","NULL",1,0,3,SQL_TRUE,0,1,"smallint unsigned",0,0,0,0,10, SQL_SMALLINT},
    {"mediumint",SQL_INTEGER,7,"","","NULL",1,0,3,SQL_FALSE,0,1,"mediumint",0,0,0,0,10},
    {"mediumint unsigned",SQL_INTEGER,8,"","","NULL",1,0,3,1,0,1,"mediumint unsigned",0,0,0,0,10, SQL_INTEGER},
    {"integer",SQL_INTEGER,10,"","","NULL",1,0,3,SQL_FALSE,0,1,"integer",0,0,0,0,10,SQL_INTEGER},
    {"integer unsigned",SQL_INTEGER,10,"","","NULL",1,0,3,1,0,1,"integer unsigned",0,0,0,0,10, SQL_INTEGER},
    {"int",SQL_INTEGER,10,"","","NULL",1,0,3,SQL_FALSE,0,1,"int",0,0,0,0,10, SQL_INTEGER},
    {"int unsigned",SQL_INTEGER,10,"","","NULL",1,0,3,1,0,1,"int unsigned",0,0,0,0,10, SQL_INTEGER},
    {"bigint",SQL_BIGINT,19,"","","NULL",1,0,3,SQL_FALSE,0,1,"bigint",0,0,0,0,10, SQL_BIGINT},
    {"bigint unsigned",SQL_BIGINT,20,"","","NULL",1,0,3,1,0,1,"bigint unsigned",0,0,0,0,10, SQL_BIGINT},
    // Real numbers
    {"float",SQL_FLOAT,10,"","","precision,scale",1,0,3,0,0,1,"float",-38,38,0,0,10, SQL_FLOAT},
    {"double",SQL_DOUBLE,17,"","","precision,scale",1,0,3,0,0,1,"double",-308,308,0,0,10, SQL_DOUBLE},
    {"double precision",SQL_DOUBLE,17,"","","precision,scale",1,0,3,0,0,1,"double precision",-308,308,0,0,10, SQL_DOUBLE},
    {"real",SQL_DOUBLE,17,"","","precision,scale",1,0,3,0,0,1,"real",-308,308,0,0,10, SQL_DOUBLE},
    {"decimal",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"decimal",-308,308,0,0,10, SQL_DECIMAL},
    {"dec",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"dec",-308,308,0,0,10, SQL_DECIMAL},
    {"fixed",SQL_DECIMAL,65,"","","precision,scale",1,0,3,0,0,1,"fixed",-308,308,0,0,10, SQL_DECIMAL},
    {"numeric",SQL_NUMERIC,65,"","","precision,scale",1,0,3,0,0,1,"numeric", -308,308,0,0,10, SQL_NUMERIC}, /* Todo: ?? */
    // Time and Date
    {"date",SQL_DATE,10,"\'","\'","NULL",1,0,3,0,0,0,"date",0,0,0,0,10, SQL_DATETIME},
    {"time",SQL_TIME,18,"\'","\'","NULL",1,0,3,0,0,0,"time",0,0,0,0,10, SQL_DATETIME},
    {"datetime",SQL_TIMESTAMP,27,"\'","\'","NULL",1,0,3,0,0,0,"datetime",0,0,0,0,10, SQL_DATETIME},
    {"timestamp",SQL_TIMESTAMP,27,"\'","\'","scale",1,0,3,0,0,0,"timestamp",0,0,0,0,10, SQL_DATETIME},
    {"year",SQL_SMALLINT,4,"","","NULL",1,0,3,SQL_FALSE,0,1,"year",0,0,0,0,10, SQL_SMALLINT},
    // String Types
    {"binary",SQL_BINARY,255,"\'","\'","length",1,1,3,0,0,0,"binary",0,0,0,0,10, SQL_BINARY},
    {"char",SQL_CHAR,255,"\'","\'","length",1,0,3,0,0,0,"char",0,0,0,0,10, SQL_CHAR},
    {"char",SQL_WCHAR,255,"\'","\'","length",1,0,3,0,0,0,"char",0,0,0,0,10, SQL_WCHAR},
    {"varbinary",SQL_VARBINARY,255,"\'","\'","length",1,1,3,0,0,0,"varbinary",0,0,0,0,10, SQL_VARBINARY},
    {"long varbinary",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,3,0,0,0,"long varbinary",0,0,0,0,10, SQL_LONGVARBINARY},
    {"long varchar",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"long varchar",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"varchar",SQL_VARCHAR,255,"\'","\'","length",1,0,3,0,0,0,"varchar",0,0,0,0,10, SQL_VARCHAR},
    {"varchar",SQL_WVARCHAR,255,"\'","\'","length",1,0,3,0,0,0,"varchar",0,0,0,0,10, SQL_WVARCHAR},
    {"longblob",SQL_LONGVARBINARY,2147483647,"\'","\'","NULL",1,1,3,0,0,0,"longblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"mediumblob",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,3,0,0,0,"mediumblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"blob",SQL_LONGVARBINARY,65535,"\'","\'","NULL",1,1,3,0,0,0,"blob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"tinyblob",SQL_LONGVARBINARY,255,"\'","\'","NULL",1,1,3,0,0,0,"tinyblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"longtext",SQL_LONGVARCHAR,2147483647,"\'","\'","NULL",1,0,3,0,0,0,"longtext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"mediumtext",SQL_LONGVARCHAR,16777215,"\'","\'","NULL",1,0,3,0,0,0,"mediumtext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"text",SQL_LONGVARCHAR,65535,"\'","\'","NULL",1,0,3,0,0,0,"text",0,0,0,0,10, SQL_LONGVARCHAR},
    {"tinytext",SQL_LONGVARCHAR,255,"\'","\'","NULL",1,0,3,0,0,0,"tinytext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"longtext",SQL_WLONGVARCHAR,2147483647,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"longtext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"mediumtext",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"mediumtext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"text",SQL_WLONGVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"text",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"tinytext",SQL_WLONGVARCHAR,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"tinytext",0,0,0,0,10, SQL_WLONGVARCHAR},
    // Other types
    {"json",SQL_LONGVARCHAR,16777216,"\'","\'","NULL",1,0,3,0,0,0,"json",0,0,0,0,10, SQL_LONGVARCHAR},
    {"json",SQL_WLONGVARCHAR,16777216,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"json",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"geographypoint",SQL_VARBINARY,20,"\'","\'","NULL",1,0,3,0,0,0,"geographypoint",0,0,0,0,10, SQL_VARBINARY},
    {"geography",SQL_VARBINARY,255,"\'","\'","NULL",1,0,3,0,0,0,"geography",0,0,0,0,10, SQL_VARBINARY},
    {"enum",SQL_VARCHAR,65535,"\'","\'","NULL",1,0,3,0,0,0,"enum",0,0,0,0,10, SQL_VARCHAR},
    {"enum",SQL_WVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"enum",0,0,0,0,10, SQL_WVARCHAR},
    {"set",SQL_VARCHAR,64,"\'","\'","NULL",1,0,3,0,0,0,"set",0,0,0,0,10, SQL_VARCHAR},
    {"set",SQL_WVARCHAR,64,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"set",0,0,0,0,10, SQL_WVARCHAR},

    {NULL,0,0,NULL,NULL,NULL,0,0,0,0,0,0,NULL,0,0,0,0,0}
};

#endif //MARIADB_CONNECTOR_ODBC_TYPES_H