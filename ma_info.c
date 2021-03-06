/************************************************************************************
   Copyright (C) 2013, 2017 MariaDB Corporation AB
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
#include <ma_odbc.h>

#define TYPE_INFO_FIELDS_COUNT 19
#define TYPES_COUNT 56

char* fieldNames[TYPE_INFO_FIELDS_COUNT] = {"TYPE_NAME", "DATA_TYPE", "COLUMN_SIZE", "LITERAL_PREFIX", "LITERAL_SUFFIX",
                                            "CREATE_PARAMS", "NULLABLE", "CASE_SENSITIVE", "SEARCHABLE", "UNSIGNED_ATTRIBUTE", "FIXED_PREC_SCALE",
                                            "AUTO_UNIQUE_VALUE", "LOCAL_TYPE_NAME", "MINIMUM_SCALE", "MAXIMUM_SCALE", "SQL_DATA_TYPE", "SQL_DATETIME_SUB",
                                            "NUM_PREC_RADIX", "INTERVAL_PRECISION"};

enum enum_field_types fieldTypes[TYPE_INFO_FIELDS_COUNT] = {MYSQL_TYPE_VAR_STRING, MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG,
                                                            MYSQL_TYPE_VAR_STRING, MYSQL_TYPE_VAR_STRING, MYSQL_TYPE_VAR_STRING, MYSQL_TYPE_SHORT, MYSQL_TYPE_SHORT,
                                                            MYSQL_TYPE_SHORT, MYSQL_TYPE_SHORT, MYSQL_TYPE_SHORT, MYSQL_TYPE_SHORT, MYSQL_TYPE_VAR_STRING, MYSQL_TYPE_SHORT,
                                                            MYSQL_TYPE_SHORT, MYSQL_TYPE_SHORT, MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG, MYSQL_TYPE_SHORT};

MADB_TypeInfo TypeInfoV3[]=
{
    {"json",SQL_WLONGVARCHAR,16777216,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"json",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"long varchar",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"long varchar",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"longtext",SQL_WLONGVARCHAR,2147483647,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"longtext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"mediumtext",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"mediumtext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"text",SQL_WLONGVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"text",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"tinytext",SQL_WLONGVARCHAR,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"tinytext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"enum",SQL_WVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"enum",0,0,0,0,10, SQL_WVARCHAR},
    {"set",SQL_WVARCHAR,64,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"set",0,0,0,0,10, SQL_WVARCHAR},
    {"varchar",SQL_WVARCHAR,255,"\'","\'","length",1,0,SQL_SEARCHABLE,0,0,0,"varchar",0,0,0,0,10, SQL_WVARCHAR},
    {"char",SQL_WCHAR,255,"\'","\'","length",1,0,SQL_SEARCHABLE,0,0,0,"char",0,0,0,0,10, SQL_WCHAR},
    {"bit",SQL_BIT,8,"","","NULL",1,1,SQL_SEARCHABLE,0,0,0,"bit",0,0,0,0,10, SQL_BIT},
    {"bool",SQL_TINYINT,1,"","","NULL",1,1,SQL_SEARCHABLE,0,0,0,"bool",0,0,0,0,10, SQL_BIT},
    {"boolean",SQL_TINYINT,1,"","","NULL",1,1,SQL_SEARCHABLE,0,0,0,"boolean",0,0,0,0,10, SQL_BIT},
    {"tinyint",SQL_TINYINT,3,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"tinyint",0,0,0,0,10, SQL_TINYINT},
    {"tinyint unsigned",SQL_TINYINT,3,"","","NULL",1,0,SQL_SEARCHABLE,SQL_TRUE,0,1,"tinyint unsigned",0,0,0,0,10, SQL_TINYINT},
    {"bigint",SQL_BIGINT,19,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"bigint",0,0,0,0,10, SQL_BIGINT},
    {"bigint unsigned",SQL_BIGINT,20,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"bigint unsigned",0,0,0,0,10, SQL_BIGINT},
    {"blob",SQL_LONGVARBINARY,65535,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"blob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"long varbinary",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"long varbinary",0,0,0,0,10, SQL_LONGVARBINARY},
    {"longblob",SQL_LONGVARBINARY,2147483647,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"longblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"mediumblob",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"mediumblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"tinyblob",SQL_LONGVARBINARY,255,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"tinyblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"geography",SQL_VARBINARY,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"geography",0,0,0,0,10, SQL_VARBINARY},
    {"geographypoint",SQL_VARBINARY,20,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"geographypoint",0,0,0,0,10, SQL_VARBINARY},
    {"varbinary",SQL_VARBINARY,255,"\'","\'","length",1,1,SQL_SEARCHABLE,0,0,0,"varbinary",0,0,0,0,10, SQL_VARBINARY},
    {"binary",SQL_BINARY,255,"\'","\'","length",1,1,SQL_SEARCHABLE,0,0,0,"binary",0,0,0,0,10, SQL_BINARY},
    {"json",SQL_LONGVARCHAR,16777216,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"json",0,0,0,0,10, SQL_LONGVARCHAR},
    {"longtext",SQL_LONGVARCHAR,2147483647,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"longtext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"mediumtext",SQL_LONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"mediumtext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"text",SQL_LONGVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"text",0,0,0,0,10, SQL_LONGVARCHAR},
    {"tinytext",SQL_LONGVARCHAR,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"tinytext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"char",SQL_CHAR,255,"\'","\'","length",1,0,SQL_SEARCHABLE,0,0,0,"char",0,0,0,0,10, SQL_CHAR},
    {"numeric",SQL_NUMERIC,65,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"numeric", -308,308,0,0,10, SQL_NUMERIC},
    {"dec",SQL_DECIMAL,65,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"dec",-308,308,0,0,10, SQL_DECIMAL},
    {"decimal",SQL_DECIMAL,65,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"decimal",-308,308,0,0,10, SQL_DECIMAL},
    {"fixed",SQL_DECIMAL,65,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"fixed",-308,308,0,0,10, SQL_DECIMAL},
    {"int",SQL_INTEGER,10,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"int",0,0,0,0,10, SQL_INTEGER},
    {"int unsigned",SQL_INTEGER,10,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"int unsigned",0,0,0,0,10, SQL_INTEGER},
    {"integer",SQL_INTEGER,10,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"integer",0,0,0,0,10, SQL_INTEGER},
    {"integer unsigned",SQL_INTEGER,10,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"integer unsigned",0,0,0,0,10, SQL_INTEGER},
    {"mediumint",SQL_INTEGER,7,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"mediumint",0,0,0,0,10, SQL_INTEGER},
    {"mediumint unsigned",SQL_INTEGER,8,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"mediumint unsigned",0,0,0,0,10, SQL_INTEGER},
    {"smallint",SQL_SMALLINT,5,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"smallint",0,0,0,0,10, SQL_SMALLINT},
    {"smallint unsigned",SQL_SMALLINT,5,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"smallint unsigned",0,0,0,0,10, SQL_SMALLINT},
    {"year",SQL_SMALLINT,4,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"year",0,0,0,0,10, SQL_SMALLINT},
    {"float",SQL_FLOAT,10,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"float",-38,38,0,0,10, SQL_FLOAT},
    {"double",SQL_DOUBLE,17,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"double",-308,308,0,0,10, SQL_DOUBLE},
    {"double precision",SQL_DOUBLE,17,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"double precision",-308,308,0,0,10, SQL_DOUBLE},
    {"real",SQL_DOUBLE,17,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"real",-308,308,0,0,10, SQL_DOUBLE},
    {"enum",SQL_VARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"enum",0,0,0,0,10, SQL_VARCHAR},
    {"set",SQL_VARCHAR,64,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"set",0,0,0,0,10, SQL_VARCHAR},
    {"varchar",SQL_VARCHAR,255,"\'","\'","length",1,0,SQL_SEARCHABLE,0,0,0,"varchar",0,0,0,0,10, SQL_VARCHAR},
    {"date",SQL_TYPE_DATE,10,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"date",0,0,0,0,10, SQL_DATETIME},
    {"time",SQL_TYPE_TIME,8,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"time",0,0,0,0,10, SQL_DATETIME},
    {"datetime",SQL_TYPE_TIMESTAMP,16,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"datetime",0,0,0,0,10, SQL_DATETIME},
    {"timestamp",SQL_TYPE_TIMESTAMP,16,"\'","\'","scale",1,0,SQL_SEARCHABLE,0,0,0,"timestamp",0,0,0,0,10, SQL_DATETIME},
};

MADB_TypeInfo TypeInfoV2[]=
{
    {"json",SQL_WLONGVARCHAR,16777216,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"json",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"long varchar",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"long varchar",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"longtext",SQL_WLONGVARCHAR,2147483647,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"longtext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"mediumtext",SQL_WLONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"mediumtext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"text",SQL_WLONGVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"text",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"tinytext",SQL_WLONGVARCHAR,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"tinytext",0,0,0,0,10, SQL_WLONGVARCHAR},
    {"enum",SQL_WVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"enum",0,0,0,0,10, SQL_WVARCHAR},
    {"set",SQL_WVARCHAR,64,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"set",0,0,0,0,10, SQL_WVARCHAR},
    {"varchar",SQL_WVARCHAR,255,"\'","\'","length",1,0,SQL_SEARCHABLE,0,0,0,"varchar",0,0,0,0,10, SQL_WVARCHAR},
    {"char",SQL_WCHAR,255,"\'","\'","length",1,0,SQL_SEARCHABLE,0,0,0,"char",0,0,0,0,10, SQL_WCHAR},
    {"bit",SQL_BIT,8,"","","NULL",1,1,SQL_SEARCHABLE,0,0,0,"bit",0,0,0,0,10, SQL_BIT},
    {"bool",SQL_TINYINT,1,"","","NULL",1,1,SQL_SEARCHABLE,0,0,0,"bool",0,0,0,0,10, SQL_BIT},
    {"boolean",SQL_TINYINT,1,"","","NULL",1,1,SQL_SEARCHABLE,0,0,0,"boolean",0,0,0,0,10, SQL_BIT},
    {"tinyint",SQL_TINYINT,3,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"tinyint",0,0,0,0,10, SQL_TINYINT},
    {"tinyint unsigned",SQL_TINYINT,3,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"tinyint unsigned",0,0,0,0,10, SQL_TINYINT},
    {"bigint",SQL_BIGINT,19,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"bigint",0,0,0,0,10, SQL_BIGINT},
    {"bigint unsigned",SQL_BIGINT,20,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"bigint unsigned",0,0,0,0,10, SQL_BIGINT},
    {"blob",SQL_LONGVARBINARY,65535,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"blob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"long varbinary",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"long varbinary",0,0,0,0,10, SQL_LONGVARBINARY},
    {"longblob",SQL_LONGVARBINARY,2147483647,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"longblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"mediumblob",SQL_LONGVARBINARY,16777215,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"mediumblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"tinyblob",SQL_LONGVARBINARY,255,"\'","\'","NULL",1,1,SQL_SEARCHABLE,0,0,0,"tinyblob",0,0,0,0,10, SQL_LONGVARBINARY},
    {"geography",SQL_VARBINARY,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"geography",0,0,0,0,10, SQL_VARBINARY},
    {"geographypoint",SQL_VARBINARY,20,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"geographypoint",0,0,0,0,10, SQL_VARBINARY},
    {"varbinary",SQL_VARBINARY,255,"\'","\'","length",1,1,SQL_SEARCHABLE,0,0,0,"varbinary",0,0,0,0,10, SQL_VARBINARY},
    {"binary",SQL_BINARY,255,"\'","\'","length",1,1,SQL_SEARCHABLE,0,0,0,"binary",0,0,0,0,10, SQL_BINARY},
    {"json",SQL_LONGVARCHAR,16777216,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"json",0,0,0,0,10, SQL_LONGVARCHAR},
    {"longtext",SQL_LONGVARCHAR,2147483647,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"longtext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"mediumtext",SQL_LONGVARCHAR,16777215,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"mediumtext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"text",SQL_LONGVARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"text",0,0,0,0,10, SQL_LONGVARCHAR},
    {"tinytext",SQL_LONGVARCHAR,255,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"tinytext",0,0,0,0,10, SQL_LONGVARCHAR},
    {"char",SQL_CHAR,255,"\'","\'","length",1,0,SQL_SEARCHABLE,0,0,0,"char",0,0,0,0,10, SQL_CHAR},
    {"numeric",SQL_NUMERIC,65,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"numeric", -308,308,0,0,10, SQL_NUMERIC}, /* Todo: ?? */
    {"dec",SQL_DECIMAL,65,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"dec",-308,308,0,0,10, SQL_DECIMAL},
    {"decimal",SQL_DECIMAL,65,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"decimal",-308,308,0,0,10, SQL_DECIMAL},
    {"fixed",SQL_DECIMAL,65,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"fixed",-308,308,0,0,10, SQL_DECIMAL},
    {"int",SQL_INTEGER,10,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"int",0,0,0,0,10, SQL_INTEGER},
    {"int unsigned",SQL_INTEGER,10,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"int unsigned",0,0,0,0,10, SQL_INTEGER},
    {"integer",SQL_INTEGER,10,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"integer",0,0,0,0,10,SQL_INTEGER},
    {"integer unsigned",SQL_INTEGER,10,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"integer unsigned",0,0,0,0,10, SQL_INTEGER},
    {"mediumint",SQL_INTEGER,7,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"mediumint",0,0,0,0,10, SQL_INTEGER},
    {"mediumint unsigned",SQL_INTEGER,8,"","","NULL",1,0,SQL_SEARCHABLE,1,0,1,"mediumint unsigned",0,0,0,0,10, SQL_INTEGER},
    {"smallint",SQL_SMALLINT,5,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"smallint",0,0,0,0,10, SQL_SMALLINT},
    {"smallint unsigned",SQL_SMALLINT,5,"","","NULL",1,0,SQL_SEARCHABLE,SQL_TRUE,0,1,"smallint unsigned",0,0,0,0,10, SQL_SMALLINT},
    {"year",SQL_SMALLINT,4,"","","NULL",1,0,SQL_SEARCHABLE,SQL_FALSE,0,1,"year",0,0,0,0,10, SQL_SMALLINT},
    {"float",SQL_FLOAT,10,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"float",-38,38,0,0,10, SQL_FLOAT},
    {"double",SQL_DOUBLE,17,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"double",-308,308,0,0,10, SQL_DOUBLE},
    {"double precision",SQL_DOUBLE,17,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"double precision",-308,308,0,0,10, SQL_DOUBLE},
    {"real",SQL_DOUBLE,17,"","","precision,scale",1,0,SQL_SEARCHABLE,0,0,1,"real",-308,308,0,0,10, SQL_DOUBLE},
    {"date",SQL_DATE,10,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"date",0,0,0,0,10, SQL_DATETIME},
    {"time",SQL_TIME,18,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"time",0,0,0,0,10, SQL_DATETIME},
    {"datetime",SQL_TIMESTAMP,27,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"datetime",0,0,0,0,10, SQL_DATETIME},
    {"timestamp",SQL_TIMESTAMP,27,"\'","\'","scale",1,0,SQL_SEARCHABLE,0,0,0,"timestamp",0,0,0,0,10, SQL_DATETIME},
    {"enum",SQL_VARCHAR,65535,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"enum",0,0,0,0,10, SQL_VARCHAR},
    {"set",SQL_VARCHAR,64,"\'","\'","NULL",1,0,SQL_SEARCHABLE,0,0,0,"set",0,0,0,0,10, SQL_VARCHAR},
    {"varchar",SQL_VARCHAR,255,"\'","\'","length",1,0,SQL_SEARCHABLE,0,0,0,"varchar",0,0,0,0,10, SQL_VARCHAR},
};

static MADB_ShortTypeInfo gtiDefType[19]= {{0, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0}, {SQL_INTEGER, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
                                 /*7*/     {SQL_SMALLINT, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0},
                                 /*11*/    {SQL_SMALLINT, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0}, {0, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0},
                                 /*16*/    {SQL_SMALLINT, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0}, {SQL_INTEGER, 0, 0, 0}, {SQL_SMALLINT, 0, 0, 0} };

short allocAndFormatInt(char** buf, int value)
{
    // maximum 11 chars for a signed integers.
    if(!(*buf = MADB_CALLOC(12))) { return 1; }
    sprintf(*buf, "%d", value);
    return 0;
}

void freeData(char ***data, int dataLen)
{
    const int rowFreeIDs[14] = {1, 2, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 18};
    int i, j;
    for(i = 0; i < dataLen; i++) {
        for(j = 0; j < 14; j++){
            if(data[i][rowFreeIDs[j]]) {
                free(data[i][rowFreeIDs[j]]);
            }
        }
        free(data[i]);
    }
}

/* {{{ MADB_GetTypeInfo */
SQLRETURN MADB_GetTypeInfo(SQLHSTMT StatementHandle,
                           SQLSMALLINT DataType)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  int       i;
  MADB_TypeInfo *TypeInfo= TypeInfoV3;

  if (Stmt->Connection->Environment->OdbcVersion == SQL_OV_ODBC2)
  {
    TypeInfo= TypeInfoV2;
    /* We need to map time types */
    switch(DataType) {
      case SQL_TYPE_TIMESTAMP:
        DataType=SQL_TIMESTAMP;
        break;
      case SQL_TYPE_DATE:
        DataType= SQL_DATE;
        break;
      case SQL_TYPE_TIME:
        DataType= SQL_TIME;
        break;
      default:
      break;
    }
  }

  int dataLen = 0;
  char** data[TYPES_COUNT];
  for (i=0;i < TYPES_COUNT; i++)
  {
      if (DataType == SQL_ALL_TYPES ||
          TypeInfo[i].DataType == DataType) {
          int j = 0, fail = 0;
          data[dataLen] = malloc(sizeof(char*) * TYPE_INFO_FIELDS_COUNT);
          data[dataLen][j++] = TypeInfo[i].TypeName,
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].DataType);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].ColumnSize);
          data[dataLen][j++] = TypeInfo[i].LiteralPrefix;
          data[dataLen][j++] = TypeInfo[i].LiteralSuffix;
          if (strcmp(TypeInfo[i].CreateParams, "NULL") == 0) {
              data[dataLen][j++] = NULL;
          } else {
              data[dataLen][j++] = TypeInfo[i].CreateParams;
          }
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].Nullable);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].CaseSensitive);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].Searchable);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].Unsigned);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].FixedPrecScale);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].AutoUniqueValue);
          data[dataLen][j++] = TypeInfo[i].LocalTypeName;
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].MinimumScale);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].MaximumScale);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].SqlDataType);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].SqlDateTimeSub);
          fail += allocAndFormatInt(&data[dataLen][j++], TypeInfo[i].NumPrecRadix);
          fail += allocAndFormatInt(&data[dataLen][j], TypeInfo[i].IntervalPrecision);
          if (fail) {
              freeData(data, dataLen);
              return MADB_SetError(&Stmt->Error, MADB_ERR_HY001, "Failed to allocate memory for type info data", 0);
          }
          dataLen++;
      }
  }

  if (!SQL_SUCCEEDED(MADB_FakeRequest(Stmt, fieldNames, fieldTypes, TYPE_INFO_FIELDS_COUNT, (char ***) data, dataLen))) {
      freeData((char ***) data, dataLen);
      return Stmt->Error.ReturnValue;
  }
  MADB_FixColumnDataTypes(Stmt, gtiDefType);
  freeData((char ***) data, dataLen);
  return SQL_SUCCESS;
}
/* }}} */
