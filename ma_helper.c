/************************************************************************************
   Copyright (C) 2013, 2018 MariaDB Corporation AB
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
#include <mysql.h>
#include <stdint.h>

#define MADB_FIELD_IS_BINARY(_field) ((_field)->charsetnr == BINARY_CHARSETNR)

#define SHOW_COLUMNS_NAME_IDX 0
#define SHOW_COLUMNS_TYPE_IDX 1
#define SHOW_COLUMNS_DEFAULT_VALUE_IDX 4

extern const MARIADB_CHARSET_INFO *mysql_find_charset_by_collation(const char *name);

void CloseMultiStatements(MADB_Stmt *Stmt)
{
  unsigned int i;

  for (i=0; i < STMT_COUNT(Stmt->Query); ++i)
  {
    MADB_CspsFreeResult(Stmt, &Stmt->CspsMultiStmtResult[i], Stmt->MultiStmts[i]);
    MDBUG_C_PRINT(Stmt->Connection, "-->closing %0x", Stmt->MultiStmts[i]);
    if (Stmt->MultiStmts[i] != NULL)
    {
      mysql_stmt_close(Stmt->MultiStmts[i]);
    }
  }
  MADB_FREE(Stmt->CspsMultiStmtResult);
  MADB_FREE(Stmt->MultiStmts);
  Stmt->CspsResult = NULL;
  Stmt->stmt= NULL;
}


MYSQL_STMT* MADB_NewStmtHandle(MADB_Stmt *Stmt)
{
  static const my_bool UpdateMaxLength= 1;
  MYSQL_STMT* stmt= mysql_stmt_init(Stmt->Connection->mariadb);

  if (stmt != NULL)
  {
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &UpdateMaxLength);
  }
  else
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_HY001, NULL, 0);
  }

  return stmt;
}

/* Required, but not sufficient condition */
BOOL QueryIsPossiblyMultistmt(MADB_QUERY *Query)
{
  return Query->QueryType != MADB_QUERY_CREATE_PROC && Query->QueryType != MADB_QUERY_CREATE_FUNC &&
         Query->QueryType != MADB_QUERY_CREATE_DEFINER && Query->QueryType != MADB_NOT_ATOMIC_BLOCK &&
         Query->QueryType != MADB_ANONYMOUS_CODE_BLOCKS;
}


/* Trims spaces and/or ';' at the end of query */
int SqlRtrim(char *StmtStr, int Length)
{
  if (Length > 0)
  {
    char *end= StmtStr + Length - 1;
    while (end > StmtStr && (isspace(0x000000ff & *end) || *end == ';'))
    {
      *end= '\0';
      --end;
      --Length;
    }
  }

  return Length;
}

/* Function assumes that the query is multistatement. And, e.g. STMT_COUNT(Stmt->Query) > 1 */
unsigned int GetMultiStatements(MADB_Stmt *Stmt, BOOL ExecDirect)
{
  int          i= 0;
  unsigned int MaxParams= 0;
  char        *p= Stmt->Query.RefinedText;

  Stmt->MultiStmtNr= 0;
  Stmt->MultiStmts= (MYSQL_STMT **)MADB_CALLOC(sizeof(MYSQL_STMT) * STMT_COUNT(Stmt->Query));

  if (MADB_SSPS_DISABLED(Stmt))
  {
      Stmt->CspsMultiStmtResult = (MYSQL_RES**)MADB_CALLOC(sizeof(MYSQL_RES) * STMT_COUNT(Stmt->Query));
  }

  while (p < Stmt->Query.RefinedText + Stmt->Query.RefinedLength)
  {
    Stmt->MultiStmts[i]= i == 0 ? Stmt->stmt : MADB_NewStmtHandle(Stmt);
    MDBUG_C_PRINT(Stmt->Connection, "-->inited&preparing %0x(%d,%s)", Stmt->MultiStmts[i], i, p);

    // For the client-side prepared statements we don't want to do anything besides allocating the MultiStmt objects.
    // We prepare only the first statement
    // All other will be prepared during SQLExecute
    if (MADB_SSPS_ENABLED(Stmt) && i == 0)
    {
        if (mysql_stmt_prepare(Stmt->MultiStmts[i], p, (unsigned long) strlen(p)))
        {
            MADB_SetNativeError(&Stmt->Error, SQL_HANDLE_STMT, Stmt->MultiStmts[i]);
            CloseMultiStatements(Stmt);

            /* Last paranoid attempt make sure that we did not have a parsing error.
               More to preserve "backward-compatibility" - we did this before, but before trying to
               prepare "multi-statement". */
            if (i == 0 && Stmt->Error.NativeError != 1295 /*ER_UNSUPPORTED_PS*/)
            {
                Stmt->stmt = MADB_NewStmtHandle(Stmt);
                if (mysql_stmt_prepare(Stmt->stmt, STMT_STRING(Stmt), (unsigned long) strlen(STMT_STRING(Stmt))))
                {
                    MADB_STMT_CLOSE_STMT(Stmt);
                } else
                {
                    MADB_DeleteSubqueries(&Stmt->Query);
                    return 0;
                }
            }
            return 1;
        }
    }

    p+= strlen(p) + 1;
    ++i;
  }

  if(MADB_SSPS_ENABLED(Stmt))
  {
    /* If we have result returning query - fill descriptor records with metadata */
    if (mysql_stmt_field_count(Stmt->stmt) > 0)
    {
      MADB_DescSetIrdMetadata(Stmt, mysql_fetch_fields(FetchMetadata(Stmt)), mysql_stmt_field_count(Stmt->stmt));
    }
  }

  return 0;
}


my_bool MADB_CheckPtrLength(SQLINTEGER MaxLength, char *Ptr, SQLINTEGER NameLen)
{
  if(!Ptr)
    return TRUE;
  if ((NameLen == SQL_NTS && strlen(Ptr) >(size_t) MaxLength) || NameLen > MaxLength)
    return FALSE;
  return TRUE;
}

int  MADB_GetWCharType(int Type)
{
  switch (Type) {
  case SQL_CHAR:
    return SQL_WCHAR;
  case SQL_VARCHAR:
    return SQL_WVARCHAR;
  case SQL_LONGVARCHAR:
    return SQL_WLONGVARCHAR;
  default:
    return Type;
  }
}

int MADB_KeyTypeCount(MADB_Dbc *Connection, char *TableName, int KeyFlag)
{
  int          Count= 0;
  unsigned int i;
  char         StmtStr[1024];
  char         *p= StmtStr;
  char         Database[65]= {'\0'};
  MADB_Stmt    *Stmt= NULL;
  MYSQL_FIELD  *Field;
  
  Connection->Methods->GetAttr(Connection, SQL_ATTR_CURRENT_CATALOG, Database, 65, NULL, FALSE);
  p+= _snprintf(p, 1024, "SELECT * FROM ");
  if (Database[0] != '\0')
  {
    p+= _snprintf(p, sizeof(StmtStr) - strlen(p), "`%s`.", Database);
  }
  p+= _snprintf(p, sizeof(StmtStr) - strlen(p), "`%s` LIMIT 0", TableName);
  if (MA_SQLAllocHandle(SQL_HANDLE_STMT, (SQLHANDLE)Connection, (SQLHANDLE*)&Stmt) == SQL_ERROR ||
    Stmt->Methods->ExecDirect(Stmt, (char *)StmtStr, SQL_NTS) == SQL_ERROR ||
    Stmt->Methods->Fetch(Stmt) == SQL_ERROR)
  {
    goto end;
  }

  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    Field= mysql_fetch_field_direct(Stmt->metadata, i);
    if (Field->flags & KeyFlag)
    {
      ++Count;
    }
  }
end:
  if (Stmt)
  {
    Stmt->Methods->StmtFree(Stmt, SQL_DROP);
  }
  return Count;
}

int SetDBCharsetnr(MADB_Dbc *Connection)
{
  MYSQL_RES *res;
  MYSQL_ROW row;
  char *collation;
  const char *query = "SELECT @@collation_database";

  LOCK_MARIADB(Connection);
  if (mysql_query(Connection->mariadb, query))
  {
    UNLOCK_MARIADB(Connection);
    Connection->DBCharsetnr = 0;
    return MADB_SetNativeError(&Connection->Error, SQL_HANDLE_DBC, Connection->mariadb);
  }
  res = mysql_store_result(Connection->mariadb);
  UNLOCK_MARIADB(Connection);
  if ((row = mysql_fetch_row(res)))
  {
    collation = (char*)row[0];
  }
  else
  {
    mysql_free_result(res);
    Connection->DBCharsetnr = 0;
    return 1;
  }
  const MARIADB_CHARSET_INFO *cs_info = mysql_find_charset_by_collation(collation);
  mysql_free_result(res);
  if (!cs_info)
  {
    Connection->DBCharsetnr = 0;
    return 1;
  }
  Connection->DBCharsetnr = cs_info->nr;
  return 0;
}


/* {{{ MADB_CheckODBCType */
BOOL MADB_CheckODBCType(SQLSMALLINT Type)
{
  switch(Type)
  {
  case SQL_C_CHAR:
  case SQL_C_WCHAR:
  case SQL_C_SSHORT:
  case SQL_C_SHORT:
  case SQL_C_USHORT:
  case SQL_C_SLONG:
  case SQL_C_LONG:
  case SQL_C_ULONG:
  case SQL_C_FLOAT:
  case SQL_C_DOUBLE:
  case SQL_C_BIT:
  case SQL_C_STINYINT:
  case SQL_C_TINYINT:
  case SQL_C_UTINYINT:
  case SQL_C_SBIGINT:
  case SQL_C_UBIGINT:
  case SQL_C_BINARY:
  case SQL_C_TYPE_DATE:
  case SQL_C_TYPE_TIME:
  case SQL_C_TYPE_TIMESTAMP:
  case SQL_C_NUMERIC:
#if (ODBCVER>=0x0350)
  case SQL_C_GUID:
#endif
  case SQL_C_DEFAULT:
    return TRUE;
  default:
    return FALSE;
  }
}

/* {{{ MADB_GetTypeFromConciseType */
SQLSMALLINT MADB_GetTypeFromConciseType(SQLSMALLINT ConciseType)
{
  switch (ConciseType)
  {
  case SQL_C_DATE:
  case SQL_C_TIME:
  case SQL_C_TIMESTAMP:
  case SQL_TYPE_DATE:
  case SQL_TYPE_TIME:
  case SQL_TYPE_TIMESTAMP:
    return SQL_DATETIME;
  case SQL_C_INTERVAL_YEAR:
  case SQL_C_INTERVAL_YEAR_TO_MONTH:
  case SQL_C_INTERVAL_MONTH:
  case SQL_C_INTERVAL_DAY:
  case SQL_C_INTERVAL_DAY_TO_HOUR:
  case SQL_C_INTERVAL_DAY_TO_MINUTE:
  case SQL_C_INTERVAL_DAY_TO_SECOND:
  case SQL_C_INTERVAL_HOUR:
  case SQL_C_INTERVAL_HOUR_TO_MINUTE:
  case SQL_C_INTERVAL_HOUR_TO_SECOND:
  case SQL_C_INTERVAL_MINUTE:
  case SQL_C_INTERVAL_MINUTE_TO_SECOND:
  case SQL_C_INTERVAL_SECOND:
      return SQL_INTERVAL;
  default:
    return ConciseType;
  }
}
/* }}} */

/* {{{ MADB_GetTypeName */
char *MADB_GetTypeName(MYSQL_FIELD *Field)
{
  switch(Field->type) {
  case MYSQL_TYPE_DECIMAL:
  case MYSQL_TYPE_NEWDECIMAL:
    return "decimal";
  case MYSQL_TYPE_NULL:
    return "null";
  case MYSQL_TYPE_TINY:
    return (Field->flags & NUM_FLAG) ? ((Field->flags & UNSIGNED_FLAG) ? "tinyint unsigned" : "tinyint") : "char";
  case MYSQL_TYPE_SHORT:
    return (Field->flags & UNSIGNED_FLAG) ? "smallint unsigned" : "smallint";
  case MYSQL_TYPE_LONG:
    return (Field->flags & UNSIGNED_FLAG) ? "int unsigned" : "int";
  case MYSQL_TYPE_FLOAT:
    return "float";
  case MYSQL_TYPE_DOUBLE:
    return "double";
  case MYSQL_TYPE_TIMESTAMP:
    return "timestamp";
  case MYSQL_TYPE_LONGLONG:
    return (Field->flags & UNSIGNED_FLAG) ? "bigint unsigned" : "bigint";
  case MYSQL_TYPE_INT24:
    return (Field->flags & UNSIGNED_FLAG) ? "mediumint unsigned" : "mediumint";
  case MYSQL_TYPE_DATE:
    return "date";
  case MYSQL_TYPE_TIME:
    return "time";
  case MYSQL_TYPE_DATETIME:
    return "datetime";
  case MYSQL_TYPE_YEAR:
    return "year";
  case MYSQL_TYPE_NEWDATE:
    return "date";
  case MYSQL_TYPE_VARCHAR:
  case MYSQL_TYPE_VAR_STRING:
    return MADB_FIELD_IS_BINARY(Field) ? "varbinary" : "varchar";
  case MYSQL_TYPE_BIT:
    return "bit";
  case MYSQL_TYPE_ENUM:
    return "enum";
  case MYSQL_TYPE_SET:
    return "set";
  case MYSQL_TYPE_TINY_BLOB:
    return MADB_FIELD_IS_BINARY(Field) ? "tinyblob" : "tinytext";
  case MYSQL_TYPE_MEDIUM_BLOB:
    return MADB_FIELD_IS_BINARY(Field) ? "mediumblob" : "mediumtext";
  case MYSQL_TYPE_LONG_BLOB:
    return MADB_FIELD_IS_BINARY(Field) ? "longblob" : "longtext";
  case MYSQL_TYPE_BLOB:
    return MADB_FIELD_IS_BINARY(Field) ? "blob" : "text";
  case MYSQL_TYPE_STRING:
    return MADB_FIELD_IS_BINARY(Field) ? "binary" : "char";
  case MYSQL_TYPE_GEOMETRY:
    return "geometry";
  case MYSQL_TYPE_JSON:
    return "json";
  default:
    return "";
  }
}
/* }}} */

MYSQL_RES *MADB_GetDefaultColumnValues(MADB_Stmt *Stmt, MYSQL_FIELD *fields)
{
  MADB_DynString DynStr;
  unsigned int i;
  MYSQL_RES *result= NULL;
  
  MADB_InitDynamicString(&DynStr, "SELECT COLUMN_NAME, COLUMN_DEFAULT FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA='", 512, 512);
  if (MADB_DynstrAppend(&DynStr, fields[0].db) ||
      MADB_DynstrAppend(&DynStr, "' AND TABLE_NAME='") ||
      MADB_DynstrAppend(&DynStr, fields[0].org_table) ||
      MADB_DynstrAppend(&DynStr, "' AND COLUMN_NAME IN ("))
    goto error;

  for (i=0; i < mysql_stmt_field_count(Stmt->stmt); i++)
  {
    MADB_DescRecord *Rec= MADB_DescGetInternalRecord(Stmt->Ard, i, MADB_DESC_READ);

    if (!Rec->inUse || MADB_ColumnIgnoredInAllRows(Stmt->Ard, Rec) == TRUE)
    {
      continue;
    }
    if (MADB_DynstrAppend(&DynStr, i > 0 ? ",'" : "'") ||
      MADB_DynstrAppend(&DynStr, fields[i].org_name) ||
      MADB_DynstrAppend(&DynStr, "'"))
    {
      goto error;
    }
  }
  if (MADB_DynstrAppend(&DynStr, ") AND COLUMN_DEFAULT IS NOT NULL"))
    goto error;

  LOCK_MARIADB(Stmt->Connection);
  if (mysql_query(Stmt->Connection->mariadb, DynStr.str))
    goto error;
  result= mysql_store_result(Stmt->Connection->mariadb);
  
error:
    UNLOCK_MARIADB(Stmt->Connection);
    MADB_DynstrFree(&DynStr);
    return result;
}

char *MADB_GetDefaultColumnValue(MYSQL_RES *res, const char *Column)
{
  MYSQL_ROW row;

  if (res == NULL || !res->row_count)
    return NULL;
  mysql_data_seek(res, 0);
  while ((row= mysql_fetch_row(res)))
  {
    if (_stricmp(row[0], Column) == 0)
     return _strdup(row[1]);
  }
  return NULL;
}

SQLLEN MADB_GetDataSize(SQLSMALLINT SqlType, SQLLEN OctetLength, BOOL Unsigned,
                        SQLSMALLINT Precision, SQLSMALLINT Scale, unsigned int CharMaxLen)
{
  switch(SqlType)
  {
  case SQL_BIT:
    return 1;
  case SQL_TINYINT:
    return 3;
  case SQL_SMALLINT:
    return 5;
  case SQL_INTEGER:
    return 10;
  case SQL_BIGINT:
    return 20 - test(Unsigned != FALSE);
  case SQL_REAL:
    return 7;
  case SQL_DOUBLE:
  case SQL_FLOAT:
    return 15;
  case SQL_DECIMAL:
  case SQL_NUMERIC:
    return Precision;
  case SQL_TYPE_DATE:
    return SQL_DATE_LEN;
  case SQL_TYPE_TIME:
    return SQL_TIME_LEN + MADB_FRACTIONAL_PART(Scale);
  case SQL_TYPE_TIMESTAMP:
    return SQL_TIMESTAMP_LEN + MADB_FRACTIONAL_PART(Scale);
  case SQL_BINARY:
  case SQL_VARBINARY:
  case SQL_LONGVARBINARY:
    return OctetLength;
  case SQL_GUID:
    return 36;;
  default:
    {
      if (CharMaxLen < 2/*i.e.0||1*/)
      {
        return OctetLength;
      }
      else
      {
        return OctetLength/CharMaxLen;
      }
    }
  }
}

/* {{{ MADB_GetDisplaySize */
size_t MADB_GetDisplaySize(MYSQL_FIELD *Field, MARIADB_CHARSET_INFO *charset)
{
  /* Todo: check these values with output from mysql --with-columntype-info */
  switch (Field->type) {
  case MYSQL_TYPE_NULL:
    return 1;
  case MYSQL_TYPE_BIT:
    return 16;
  case MYSQL_TYPE_TINY:
    return 4 - test(Field->flags & UNSIGNED_FLAG);
  case MYSQL_TYPE_SHORT:
  case MYSQL_TYPE_YEAR:
    return 6 - test(Field->flags & UNSIGNED_FLAG);
  case MYSQL_TYPE_INT24:
    return 9 - test(Field->flags & UNSIGNED_FLAG);
  case MYSQL_TYPE_LONG:
    return 11 - test(Field->flags & UNSIGNED_FLAG);
  case MYSQL_TYPE_LONGLONG:
    return 20;
  case MYSQL_TYPE_DOUBLE:
    return 15;
  case MYSQL_TYPE_FLOAT:
    return 7;
  case MYSQL_TYPE_DECIMAL:
  case MYSQL_TYPE_NEWDECIMAL:
  {
    /* The edge case like decimal(1,1)*/
    size_t Precision= Field->length - test((Field->flags & UNSIGNED_FLAG) == 0) - test(Field->decimals != 0);
    return Field->length + test(Precision == Field->decimals);
  }
  case MYSQL_TYPE_DATE:
    return SQL_DATE_LEN; /* YYYY-MM-DD */
  case MYSQL_TYPE_TIME:
    return SQL_TIME_LEN + MADB_FRACTIONAL_PART(Field->decimals); /* HH:MM:SS.ffffff */
  case MYSQL_TYPE_NEWDATE:
  case MYSQL_TYPE_TIMESTAMP:
  case MYSQL_TYPE_DATETIME:
    return SQL_TIMESTAMP_LEN + MADB_FRACTIONAL_PART(Field->decimals);
  case MYSQL_TYPE_BLOB:
  case MYSQL_TYPE_ENUM:
  case MYSQL_TYPE_GEOMETRY:
  case MYSQL_TYPE_LONG_BLOB:
  case MYSQL_TYPE_MEDIUM_BLOB:
  case MYSQL_TYPE_SET:
  case MYSQL_TYPE_STRING:
  case MYSQL_TYPE_TINY_BLOB:
  case MYSQL_TYPE_VARCHAR:
  case MYSQL_TYPE_VAR_STRING:
  {
    if (MADB_FIELD_IS_BINARY(Field))
    {
      return Field->length*2; /* ODBC specs says we should give 2 characters per byte to display binaray data in hex form */
    }
    else if (charset == NULL || charset->char_maxlen < 2/*i.e.0||1*/)
    {
      return Field->length;
    }
    else
    {
      return Field->length/charset->char_maxlen;
    }
  }
  default:
    return SQL_NO_TOTAL;
  }
}
/* }}} */

/* {{{ MADB_GetOctetLength */
size_t MADB_GetOctetLength(MYSQL_FIELD *Field, unsigned short MaxCharLen)
{
  size_t Length= MIN(MADB_INT_MAX32, Field->length);

  switch (Field->type) {
  case MYSQL_TYPE_NULL:
    return 1;
  case MYSQL_TYPE_BIT:
    return 8;
  case MYSQL_TYPE_TINY:
    return 1;
  case MYSQL_TYPE_YEAR:
  case MYSQL_TYPE_SHORT:
    return 2;
  case MYSQL_TYPE_INT24:
  case MYSQL_TYPE_LONG:
    return 4;
  case MYSQL_TYPE_LONGLONG:
    return 8;
  case MYSQL_TYPE_DOUBLE:
    return 8;
  case MYSQL_TYPE_FLOAT:
    return 4;
  case MYSQL_TYPE_DECIMAL:
  case MYSQL_TYPE_NEWDECIMAL:
  {
    /* The edge case like decimal(1,1)*/
    size_t Precision= Field->length - test((Field->flags & UNSIGNED_FLAG) == 0) - test(Field->decimals != 0);
    return Field->length + test(Precision == Field->decimals);
  }
  case MYSQL_TYPE_DATE:
    return sizeof(SQL_DATE_STRUCT);
  case MYSQL_TYPE_TIME:
    return sizeof(SQL_TIME_STRUCT);
   case MYSQL_TYPE_NEWDATE:
  case MYSQL_TYPE_TIMESTAMP:
  case MYSQL_TYPE_DATETIME:
    return sizeof(SQL_TIMESTAMP_STRUCT);
  case MYSQL_TYPE_BLOB:
  case MYSQL_TYPE_ENUM:
  case MYSQL_TYPE_GEOMETRY:
  case MYSQL_TYPE_LONG_BLOB:
  case MYSQL_TYPE_MEDIUM_BLOB:
  case MYSQL_TYPE_TINY_BLOB:
    return Length;
  case MYSQL_TYPE_SET:
  case MYSQL_TYPE_STRING:
  case MYSQL_TYPE_VARCHAR:
  case MYSQL_TYPE_VAR_STRING:
    return Length; /* Field->length is calculated using current charset */
  default:
    return SQL_NO_TOTAL;
  }
}
/* }}} */

/* {{{ MADB_GetDefaultType */
int MADB_GetDefaultType(int SQLDataType)
{
  switch(SQLDataType)
  {
  case SQL_BIGINT:
    return SQL_C_SBIGINT;
  case SQL_BINARY:
    return SQL_C_BINARY;
  case SQL_BIT:
    return SQL_C_BIT;
  case SQL_CHAR:
    return SQL_C_CHAR;
  case SQL_DATE:
  case SQL_TYPE_DATE:
    return SQL_C_DATE;
  case SQL_DECIMAL:
    return SQL_C_CHAR;
  case SQL_DOUBLE:
    return SQL_C_DOUBLE; 
  case SQL_FLOAT:
    return SQL_C_FLOAT;
  case SQL_INTEGER:
    return SQL_C_LONG;
  case SQL_LONGVARBINARY:
    return SQL_C_BINARY;
  case SQL_LONGVARCHAR:
    return SQL_C_CHAR;
  case SQL_NUMERIC:
    return SQL_C_CHAR;
  case SQL_REAL:
    return SQL_C_FLOAT;
  case SQL_SMALLINT:
    return SQL_C_SHORT;
  case SQL_TIME:
  case SQL_TYPE_TIME:
    return SQL_C_TIME;
  case SQL_TIMESTAMP:
  case SQL_TYPE_TIMESTAMP:
    return SQL_C_TIMESTAMP;
  case SQL_TINYINT:
    return SQL_C_TINYINT;
  case SQL_VARBINARY:
    return SQL_C_BINARY;
  case SQL_VARCHAR:
    return SQL_C_CHAR;
  case SQL_WCHAR:
  case SQL_WVARCHAR:
  case SQL_WLONGVARCHAR:
    return SQL_WCHAR;
  default:
    return SQL_C_CHAR;
  }
}
/* }}} */

/* {{{ MapMariadDbToOdbcType */
       /* It's not quite right to mix here C and SQL types, even though constants are sort of equal */
SQLSMALLINT MapMariadDbToOdbcType(MYSQL_FIELD *field)
{
  switch (field->type) {
    case MYSQL_TYPE_BIT:
      return SQL_BINARY;
    case MYSQL_TYPE_NULL:
      return SQL_VARCHAR;
    case MYSQL_TYPE_TINY:
      return field->flags & NUM_FLAG ? SQL_TINYINT : SQL_CHAR;
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_SHORT:
      return SQL_SMALLINT;
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
      return SQL_INTEGER;
    case MYSQL_TYPE_FLOAT:
      return SQL_REAL;
    case MYSQL_TYPE_DOUBLE:
      return SQL_DOUBLE;
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATETIME:
      return SQL_TYPE_TIMESTAMP;
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_DATE:
      return SQL_TYPE_DATE;
    case MYSQL_TYPE_TIME:
       return SQL_TYPE_TIME;
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
      return MADB_FIELD_IS_BINARY(field) ? SQL_LONGVARBINARY : SQL_LONGVARCHAR;
    case MYSQL_TYPE_LONGLONG:
      return SQL_BIGINT;
    case MYSQL_TYPE_STRING:
      return MADB_FIELD_IS_BINARY(field) ? SQL_BINARY : SQL_CHAR;
    case MYSQL_TYPE_VAR_STRING:
      return MADB_FIELD_IS_BINARY(field) ? SQL_VARBINARY : SQL_VARCHAR;
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_ENUM:
      return SQL_CHAR;
    case MYSQL_TYPE_GEOMETRY:
      return SQL_LONGVARBINARY;
    case MYSQL_TYPE_JSON:
      return SQL_LONGVARCHAR;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
      return SQL_DECIMAL;
    default:
      return SQL_UNKNOWN_TYPE;
  }
}

SQLSMALLINT MapToV2Type(SQLSMALLINT type)
{
  /* We need to map time types */
  switch(type)
  {
  case SQL_TYPE_TIMESTAMP:
    return SQL_TIMESTAMP;
  case SQL_TYPE_DATE:
    return SQL_DATE;
  case SQL_TYPE_TIME:
    return SQL_TIME;
  default:
    return type;
  }
}

/* }}} */
/* {{{ MADB_GetTypeLength */
size_t MADB_GetTypeLength(SQLINTEGER SqlDataType, size_t Length)
{
  switch(SqlDataType)
  {
  case SQL_C_BIT:
  case SQL_C_TINYINT:
  case SQL_C_STINYINT:
  case SQL_C_UTINYINT:
    return 1;
  case SQL_C_SHORT:
  case SQL_C_SSHORT:
  case SQL_C_USHORT:
    return 2;
  case SQL_C_LONG:
  case SQL_C_SLONG:
  case SQL_C_ULONG:
    return sizeof(SQLINTEGER);
  case SQL_C_UBIGINT:
  case SQL_C_SBIGINT:
    return sizeof(long long);
  case SQL_C_DOUBLE:
    return sizeof(SQLDOUBLE);
  case SQL_C_FLOAT:
    return sizeof(SQLFLOAT);
  case SQL_C_NUMERIC:
    return sizeof(SQL_NUMERIC_STRUCT);
  case SQL_C_TYPE_TIME:
  case SQL_C_TIME:
    return sizeof(SQL_TIME_STRUCT);
  case SQL_C_TYPE_DATE:
  case SQL_C_DATE:
    return sizeof(SQL_DATE_STRUCT);
  case SQL_C_TYPE_TIMESTAMP:
  case SQL_C_TIMESTAMP:
    return sizeof(SQL_TIMESTAMP_STRUCT);
  default:
    return Length;
  }
}
/* }}} */

/* {{{ MADB_GetMaDBTypeAndLength */
int MADB_GetMaDBTypeAndLength(SQLINTEGER SqlDataType, my_bool *Unsigned, unsigned long *Length)
{
  *Unsigned= 0;
  switch(SqlDataType)
  {
  case SQL_C_BIT:
  case SQL_C_TINYINT:
  case SQL_C_STINYINT:
  case SQL_C_UTINYINT:
    *Length= 1;
    *Unsigned= (SqlDataType == SQL_C_UTINYINT);

    return MYSQL_TYPE_TINY;

  case SQL_C_SHORT:
  case SQL_C_SSHORT:
  case SQL_C_USHORT:
    *Length= 2;
    *Unsigned= (SqlDataType == SQL_C_USHORT);

    return MYSQL_TYPE_SHORT;
 
  case SQL_C_LONG:
  case SQL_C_SLONG:
  case SQL_C_ULONG:
    *Length= sizeof(SQLINTEGER);
    *Unsigned= (SqlDataType == SQL_C_ULONG);
    return MYSQL_TYPE_LONG;
  case SQL_C_UBIGINT:
  case SQL_C_SBIGINT:
    *Length= sizeof(long long);
    *Unsigned= (SqlDataType == SQL_C_UBIGINT);
    return MYSQL_TYPE_LONGLONG;
  case SQL_C_DOUBLE:
    *Length= sizeof(SQLDOUBLE);
    return MYSQL_TYPE_DOUBLE;
  case SQL_C_FLOAT:
    *Length =sizeof(SQLFLOAT);
    return MYSQL_TYPE_FLOAT;
  case SQL_C_NUMERIC:
    /**Length= sizeof(SQL_NUMERIC_STRUCT);*/
    return MYSQL_TYPE_DECIMAL;
  case SQL_C_TYPE_TIME:
  case SQL_C_TIME:
    *Length= sizeof(SQL_TIME_STRUCT);
    return MYSQL_TYPE_TIME;
  case SQL_C_TYPE_DATE:
  case SQL_C_DATE:
    *Length= sizeof(SQL_DATE_STRUCT);
    return MYSQL_TYPE_DATE;
  case SQL_C_TYPE_TIMESTAMP:
  case SQL_C_TIMESTAMP:
    *Length= sizeof(SQL_TIMESTAMP_STRUCT);
    return MYSQL_TYPE_TIMESTAMP;
  case SQL_C_INTERVAL_HOUR_TO_MINUTE:
  case SQL_C_INTERVAL_HOUR_TO_SECOND:
    *Length= sizeof(SQL_INTERVAL_STRUCT);
    return MYSQL_TYPE_TIME;
  case SQL_C_CHAR:
    return MYSQL_TYPE_STRING;
  default:
    return MYSQL_TYPE_BLOB;
  }
}
/* }}} */

void MADB_CopyOdbcTsToMadbTime(SQL_TIMESTAMP_STRUCT *Src, MYSQL_TIME *Dst)
{
  Dst->year=        Src->year;
  Dst->month=       Src->month;
  Dst->day=         Src->day;
  Dst->hour=        Src->hour;
  Dst->minute=      Src->minute;
  Dst->second=      Src->second;
  Dst->second_part= Src->fraction / 1000;
}

void MADB_CopyMadbTimeToOdbcTs(MYSQL_TIME *Src, SQL_TIMESTAMP_STRUCT *Dst)
{
  Dst->year=        Src->year;
  Dst->month=       Src->month;
  Dst->day=         Src->day;
  Dst->hour=        Src->hour;
  Dst->minute=      Src->minute;
  Dst->second=      Src->second;
  Dst->fraction=    Src->second_part*1000;
}

SQLRETURN MADB_CopyMadbTimestamp(MADB_Stmt *Stmt, MYSQL_TIME *tm, SQLPOINTER DataPtr, SQLLEN *Length, SQLLEN *Ind,
                                 SQLSMALLINT CType, SQLSMALLINT SqlType)
{
  SQLLEN Dummy;

  Length= Length == NULL ? &Dummy : Length;

  switch(CType)
  {
    case SQL_C_TIMESTAMP:
    case SQL_C_TYPE_TIMESTAMP:
    {
      SQL_TIMESTAMP_STRUCT *ts= (SQL_TIMESTAMP_STRUCT *)DataPtr;

      if (ts != NULL)
      {
        /* If time converted to timestamp - fraction is set to 0, date is set to current date */
        if (SqlType == SQL_TIME || SqlType == SQL_TYPE_TIME)
        {
          time_t sec_time;
          struct tm * cur_tm;

          sec_time= time(NULL);
          cur_tm= localtime(&sec_time);

          ts->year= 1900 + cur_tm->tm_year;
          ts->month= cur_tm->tm_mon + 1;
          ts->day= cur_tm->tm_mday;
          ts->fraction= 0;
        }
        else
        {
          ts->year= tm->year;
          ts->month= tm->month;
          ts->day= tm->day;
          ts->fraction= tm->second_part * 1000;
        }
        ts->hour= tm->hour;
        ts->minute= tm->minute;
        ts->second= tm->second;

        if (ts->year + ts->month + ts->day + ts->hour + ts->minute + ts->fraction + ts->second == 0)
        {
          if (Ind != NULL)
          {
            *Ind= SQL_NULL_DATA;
          }
          else
          {
            return MADB_SetError(&Stmt->Error, MADB_ERR_22002, NULL, 0);
          }
          break;
        }
      }
      *Length= sizeof(SQL_TIMESTAMP_STRUCT);
    }
    break;
    case SQL_C_TIME:
    case SQL_C_TYPE_TIME:
    {
      SQL_TIME_STRUCT *ts= (SQL_TIME_STRUCT *)DataPtr;

      if (ts != NULL)
      {
        /* tm(buffer from MYSQL_BIND) can be NULL. And that happens if ts(app's buffer) is null */
        if (!VALID_TIME(tm))
        {
          return MADB_SetError(&Stmt->Error, MADB_ERR_22007, NULL, 0);
        }

        ts->hour= tm->hour;
        ts->minute= tm->minute;
        ts->second= tm->second;

        *Length= sizeof(SQL_TIME_STRUCT);

        if (tm->second_part)
        {
          return MADB_SetError(&Stmt->Error, MADB_ERR_01S07, NULL, 0);
        }
      }
    }
    break;
    case SQL_C_DATE:
    case SQL_TYPE_DATE:
    {
      SQL_DATE_STRUCT *ts= (SQL_DATE_STRUCT *)DataPtr;

      if (ts != NULL)
      {
        ts->year= tm->year;
        ts->month= tm->month;
        ts->day= tm->day;
        if (ts->year + ts->month + ts->day == 0)
        {
          if (Ind != NULL)
          {
            *Ind= SQL_NULL_DATA;
          }
          else
          {
            return MADB_SetError(&Stmt->Error, MADB_ERR_22002, NULL, 0);
          }
          break;
        }
      }
      *Length= sizeof(SQL_DATE_STRUCT);
    }
    break;
  }
    
  return SQL_SUCCESS;
}


void *GetBindOffset(MADB_Desc *Desc, MADB_DescRecord *Record, void *Ptr, SQLULEN RowNumber, size_t PtrSize)
{
  size_t BindOffset= 0;

  /* This is not quite clear - I'd imagine, that if BindOffset is set, then Ptr can be NULL.
     Makes perfect sense in case of row-based binding - setting pointers to offset in structure, and BindOffset to the begin of array.
     One of members would have 0 offset then. But specs are rather against that, and other drivers also don't support such interpretation */
  if (Ptr == NULL)
  {
    return NULL;
  }
  if (Desc->Header.BindOffsetPtr != NULL)
  {
    BindOffset= (size_t)*Desc->Header.BindOffsetPtr;
  }

  /* row wise binding */
  if (Desc->Header.BindType == SQL_BIND_BY_COLUMN ||
    Desc->Header.BindType == SQL_PARAM_BIND_BY_COLUMN)
  {
    BindOffset+= PtrSize * RowNumber;
  }
  else
  {
    BindOffset+= Desc->Header.BindType * RowNumber;
  }

  return (char *)Ptr + BindOffset;
}

/* Checking if column ignored in all bound rows. Should hel*/
BOOL MADB_ColumnIgnoredInAllRows(MADB_Desc *Desc, MADB_DescRecord *Rec)
{
  SQLULEN row;
  SQLLEN *IndicatorPtr;

  for (row= 0; row < Desc->Header.ArraySize; ++row)
  {
    IndicatorPtr= (SQLLEN *)GetBindOffset(Desc, Rec, Rec->IndicatorPtr, row, sizeof(SQLLEN));

    if (IndicatorPtr == NULL || *IndicatorPtr != SQL_COLUMN_IGNORE)
    {
      return FALSE;
    }
  }

  return TRUE;
}


void MADB_NumericInit(SQL_NUMERIC_STRUCT *number, MADB_DescRecord *Ard)
{
  if (!number)
    return;
  number->precision= (SQLCHAR)Ard->Precision;
  number->scale= (SQLCHAR)Ard->Scale;
  memset(number->val, 0, sizeof(number->val));
}

/* {{{ MADB_CharToSQLNumeric */
int MADB_CharToSQLNumeric(char *buffer, MADB_Desc *Ard, MADB_DescRecord *ArdRecord, SQL_NUMERIC_STRUCT *dst_buffer, unsigned long RowNumber)
{
  char *p;
  SQL_NUMERIC_STRUCT *number= dst_buffer != NULL ? dst_buffer :
    (SQL_NUMERIC_STRUCT *)GetBindOffset(Ard, ArdRecord, ArdRecord->DataPtr, RowNumber, ArdRecord->OctetLength);
  int ret= 0;

  if (!buffer || !number)
    return ret;

  p= trim(buffer);
  MADB_NumericInit(number, ArdRecord);

  if (!(number->sign= (*p=='-') ? 0 : 1))
    p++;
  if (!*p)
    return FALSE;

  if (number->precision == 0)
  {
    number->precision= MADB_DEFAULT_PRECISION;
  }

  while (*p=='0')
  {
    p++;
  }
  if (*p)
  {
    int i;
    int bit, hval, tv, dig, sta, olen;
    int tmp_digit= 0;
    int leading_zeros= 0;
    char *dot= strchr(p, '.');
    char digits[100];
    short digits_count= 0;

    /* Overflow check */
    if (number->precision > 0 && (dot - p) > number->precision)
      return MADB_ERR_22003;
    
    if (dot && number->scale > 0)
    {
      short digits_total= 0, 
            digits_significant= 0;
      digits_count= (short)(dot - p);
      memcpy(digits, p, digits_count);
      p= dot + 1;
      while (*p)
      {
        /* ignore non numbers */
        if (!isdigit(0x000000ff & *p))
          break;
        digits_total++;
        /* ignore trailing zeros */
        if (*p != '0')
        {
          digits_significant= digits_total;
        }
        p++;
      }

      if (digits_count + number->scale > number->precision)
      {
        int i;
        /* if digits are zero there is no overflow */
        for (i=1; i <= digits_significant; i++)
        {
          p= dot + i;
          if (*p != '0')
            return MADB_ERR_22003;
        }
      }
      
      memcpy(digits + digits_count, dot + 1, digits_significant);
      if (number->scale > digits_significant)
      {
        for (i= digits_count + digits_significant; i < number->precision && i < digits_count +number->scale; ++i)
        {
          digits[i]= '0';
        }
        digits_significant= number->scale;
      }
      digits_count+= digits_significant;
    }
    else 
    {
      char *start= p;
      while (*p && isdigit(0x000000ff & *p))
        p++;
      /* check overflow */
      if (p - start > number->precision)
      {
        return MADB_ERR_22003;
      }
      digits_count= (short)(p - start);
      memcpy(digits, start, digits_count);
      number->scale= ArdRecord->Scale ? ArdRecord->Scale : 0;
    }

    /* Rounding */
    if (number->scale < 0)
    {
      int64_t OldVal, Val;
      int64_t RoundNumber= (int64_t)pow(10.0, -number->scale);

      digits[number->precision]= 0;
      Val= _atoi64(digits);

      OldVal= Val;
      Val= (Val + RoundNumber / 2) / RoundNumber * RoundNumber;
      if (OldVal != Val)
        return MADB_ERR_22003;
      _snprintf(digits, sizeof(digits), "%lld", Val);
      digits_count= (short)strlen(digits);
      if (digits_count > number->precision)
        return MADB_ERR_22003;
    }

    digits_count= MIN(digits_count, MADB_DEFAULT_PRECISION);
    for (hval = 0, bit = 1L, sta = 0, olen = 0; sta < digits_count;)
    {
      for (dig = 0, i = sta; i < digits_count; i++)
      {
        tv = dig * 10 + digits[i] - '0';
        dig = tv % 2;
        digits[i] = tv / 2 + '0';
        if (i == sta && tv < 2)
          sta++;
      }
      if (dig > 0)
        hval |= bit;
      bit <<= 1;
      if (bit >= (1L << 8))
      {
        number->val[olen++] = hval;
        hval = 0;
        bit = 1L;
        if (olen >= SQL_MAX_NUMERIC_LEN - 1)
        {
          //number->scale = sta - number->precision;
          //ret= MADB_ERR_22003;
          break;
        }
      } 
    }
    if (hval && olen < SQL_MAX_NUMERIC_LEN - 1)
    {
      number->val[olen++] = hval;
    }
  } 
  return ret;
}

/* {{{ MADB_GetHexString */
size_t MADB_GetHexString(char *BinaryBuffer, size_t BinaryLength,
                          char *HexBuffer, size_t HexLength)
{
  const char HexDigits[]= "0123456789ABCDEF";
  char *Start= HexBuffer;
  size_t CurrentLength= HexLength;

  if (!HexBuffer || !BinaryBuffer)
    return 0;
   
  while (BinaryLength-- && CurrentLength > 2)
  {
    *HexBuffer++=HexDigits[*BinaryBuffer >> 4];
    *HexBuffer++=HexDigits[*BinaryBuffer & 0x0F];
    BinaryBuffer++;
    CurrentLength-= 2;
  }
  *HexBuffer= 0;
  return (HexBuffer - Start);
}


SQLRETURN MADB_DaeStmt(MADB_Stmt *Stmt, SQLUSMALLINT Operation)
{
  char          *TableName=   MADB_GetTableName(Stmt);
  char          *CatalogName= MADB_GetCatalogName(Stmt);
  MADB_DynString DynStmt;

  MADB_CLEAR_ERROR(&Stmt->Error);
  memset(&DynStmt, 0, sizeof(MADB_DynString));

  if (Stmt->DaeStmt)
    Stmt->Methods->StmtFree(Stmt->DaeStmt, SQL_DROP);
  Stmt->DaeStmt= NULL;

  if (!SQL_SUCCEEDED(MA_SQLAllocHandle(SQL_HANDLE_STMT, (SQLHANDLE)Stmt->Connection, (SQLHANDLE *)&Stmt->DaeStmt)))
  {
    MADB_CopyError(&Stmt->Error, &Stmt->Connection->Error);
    goto end;
  }

  switch(Operation)
  {
  case SQL_ADD:
    if (MADB_InitDynamicString(&DynStmt, "INSERT INTO ", 1024, 1024) ||
        MADB_DynStrAppendQuoted(&DynStmt, CatalogName) ||
        MADB_DynstrAppend(&DynStmt, ".") ||
        MADB_DynStrAppendQuoted(&DynStmt, TableName)||
        MADB_DynStrUpdateSet(Stmt, &DynStmt))
    {
      MADB_DynstrFree(&DynStmt);
      return Stmt->Error.ReturnValue;
    }
    Stmt->DataExecutionType= MADB_DAE_ADD;
    break;
  case SQL_DELETE:
    if (MADB_InitDynamicString(&DynStmt, "DELETE FROM ", 1024, 1024) ||
        MADB_DynStrAppendQuoted(&DynStmt, CatalogName) ||
        MADB_DynstrAppend(&DynStmt, ".") ||
        MADB_DynStrAppendQuoted(&DynStmt, TableName) ||
        MADB_DynStrGetWhere(Stmt, &DynStmt, TableName, FALSE))
    {
      MADB_DynstrFree(&DynStmt);
      return Stmt->Error.ReturnValue;
    }
    Stmt->DataExecutionType= MADB_DAE_DELETE;
    break;
  case SQL_UPDATE:
    if (MADB_InitDynamicString(&DynStmt, "UPDATE ", 1024, 1024) ||
        MADB_DynStrAppendQuoted(&DynStmt, CatalogName) ||
        MADB_DynstrAppend(&DynStmt, ".") ||
        MADB_DynStrAppendQuoted(&DynStmt, TableName)||
        MADB_DynStrUpdateSet(Stmt, &DynStmt)||
        MADB_DynStrGetWhere(Stmt, &DynStmt, TableName, FALSE))
    {
      MADB_DynstrFree(&DynStmt);
      return Stmt->Error.ReturnValue;
    }
    Stmt->DataExecutionType= MADB_DAE_UPDATE;
    break;
  }
  
  if (!SQL_SUCCEEDED(Stmt->DaeStmt->Methods->Prepare(Stmt->DaeStmt, DynStmt.str, SQL_NTS, FALSE)))
  {
    MADB_CopyError(&Stmt->Error, &Stmt->DaeStmt->Error);
    Stmt->Methods->StmtFree(Stmt->DaeStmt, SQL_DROP);
  }
   
end:
  MADB_DynstrFree(&DynStmt);
  return Stmt->Error.ReturnValue;

}


int MADB_FindNextDaeParam(MADB_Desc *Desc, int InitialParam, SQLSMALLINT RowNumber)
{
  int             i;
  MADB_DescRecord *Record;

  for (i= InitialParam > -1 ? InitialParam + 1 : 0; i < Desc->Header.Count; i++)
  {
    if ((Record= MADB_DescGetInternalRecord(Desc, i, MADB_DESC_READ)))
    {
      if (Record->OctetLengthPtr)
      {
        /* Stmt->DaeRowNumber is 1 based */
        SQLLEN *OctetLength = (SQLLEN *)GetBindOffset(Desc, Record, Record->OctetLengthPtr, RowNumber > 1 ? RowNumber - 1 : 0, sizeof(SQLLEN));
        if (PARAM_IS_DAE(OctetLength))
        {
          return i;
        }
      }
    }
  }

  return MADB_NOPARAM;
}


BOOL MADB_IsNumericType(SQLSMALLINT ConciseType)
{
  switch (ConciseType)
  {
    case SQL_C_DOUBLE:
    case SQL_C_FLOAT:
    case SQL_DECIMAL:
      return TRUE;
  }

  return MADB_IsIntType(ConciseType);
}


BOOL MADB_IsIntType(SQLSMALLINT ConciseType)
{
  switch (ConciseType)
  {
  case SQL_C_TINYINT:
  case SQL_C_STINYINT:
  case SQL_C_UTINYINT:
  case SQL_C_SHORT:
  case SQL_C_SSHORT:
  case SQL_C_USHORT:
  case SQL_C_LONG:
  case SQL_C_SLONG:
  case SQL_C_ULONG:
  case SQL_C_UBIGINT:
  case SQL_C_SBIGINT:
  case SQL_BIGINT:
    return TRUE;
  }
  return FALSE;
}

/* Now it's more like installing result */
void MADB_InstallStmt(MADB_Stmt *Stmt, MYSQL_STMT *stmt)
{
  Stmt->stmt= stmt;

  if (mysql_stmt_field_count(Stmt->stmt) == 0)
  {
    MADB_DescFree(Stmt->Ird, TRUE);
    Stmt->AffectedRows= mysql_stmt_affected_rows(Stmt->stmt);
  }
  else
  {
    Stmt->AffectedRows= 0;
    MADB_StmtResetResultStructures(Stmt);
    MADB_DescSetIrdMetadata(Stmt, mysql_fetch_fields(FetchMetadata(Stmt)), mysql_stmt_field_count(Stmt->stmt));
  }
}


/**
Get the table status for a table or tables using SHOW TABLE STATUS.
Lengths may not be SQL_NTS.

@param[in] stmt           Handle to statement
@param[in] catalog        Catalog (database) of table, @c NULL for current
@param[in] catalog_length Length of catalog name
@param[in] table          Name of table
@param[in] table_length   Length of table name
@param[in] wildcard       Whether the table name is a wildcard

@return Result of SHOW TABLES, or NULL if there is an error
or empty result (check mysql_errno(stmt->Connection->mariadb) != 0)
*/
MYSQL_RES *S2_ShowTables(MADB_Stmt   *stmt,
                          SQLCHAR     *catalog,
                          SQLSMALLINT  catalog_length,
                          SQLCHAR     *table,
                          SQLSMALLINT  table_length,
                          BOOL         wildcard)
{
	char tmpbuff[1024];
  char query[1024] = "SHOW TABLES ";
  size_t cnt = 0;

	if (catalog && *catalog)
	{
    strcat(query, "FROM `");
    strncat(query, catalog, catalog_length);
    strcat(query, "` ");
	}

	/*
	As a pattern-value argument, an empty string needs to be treated
	literally. (It's not the same as NULL, which is the same as '%'.)
	But it will never match anything, so bail out now.
	*/
	if (table && wildcard && !*table)
		return NULL;

	if (table && *table)
	{
		strcat(query, "LIKE '");
		cnt = mysql_real_escape_string(stmt->Connection->mariadb, tmpbuff, (char *)table, table_length);
    strncat(query, tmpbuff, cnt);
		strcat(query, "'");
	}

  LOCK_MARIADB(stmt->Connection);

  if (mysql_real_query(stmt->Connection->mariadb, query, strlen(query)))
  {
    UNLOCK_MARIADB(stmt->Connection);
    MADB_SetError(&stmt->Error, MADB_ERR_HY001, mysql_error(stmt->Connection->mariadb), mysql_errno(stmt->Connection->mariadb));
    return NULL;
  }
  UNLOCK_MARIADB(stmt->Connection);

  return mysql_store_result(stmt->Connection->mariadb);
}

/**
Get the result of SHOW COLUMNS FROM table LIKE column_like

@param[in] stmt           Handle to statement
@param[in] catalog        Catalog (database) of table, @c NULL for current
@param[in] catalog_length Length of catalog name
@param[in] table          Name of table
@param[in] table_length   Length of table name
@param[in] column_like    Column name pattern to match

@return Result of SHOW COLUMNS FROM, or NULL if there is an error,
or empty result (check mysql_errno(stmt->Connection->mariadb) != 0)
*/
MYSQL_RES *S2_ShowColumnsInTable(MADB_Stmt  *stmt,
                                 SQLCHAR     *catalog,
                                 SQLSMALLINT  catalog_length,
                                 SQLCHAR     *table,
                                 SQLSMALLINT  table_length,
                                 SQLCHAR     *column_like,
                                 SQLSMALLINT  column_length)
{
	char tmpbuff[1024];
  char query[1024] = "SHOW COLUMNS FROM ";
  size_t cnt = 0;

	if (catalog && *catalog)
	{
    strcat(query, "`");
    strncat(query, catalog, catalog_length);
    strcat(query, "`");
	  strcat(query, ".");
	}

  strcat(query, "`");
  strncat(query, table, table_length);
  strcat(query, "`");

  if (column_like && *column_like && column_length)
  {
    strcat(query, " LIKE '");
		cnt = mysql_real_escape_string(stmt->Connection->mariadb, tmpbuff, (char *)column_like, column_length);
    strncat(query, tmpbuff, cnt);
    strcat(query, "'");
  }

  LOCK_MARIADB(stmt->Connection);
  if (mysql_real_query(stmt->Connection->mariadb, query, strlen(query)))
  {
    UNLOCK_MARIADB(stmt->Connection);
    MADB_SetError(&stmt->Error, MADB_ERR_HY001, mysql_error(stmt->Connection->mariadb), mysql_errno(stmt->Connection->mariadb));
    return NULL;
  }
  UNLOCK_MARIADB(stmt->Connection);

  return mysql_store_result(stmt->Connection->mariadb);
}

FieldDescrList *ProcessShowColumns(MYSQL_RES *res)
{
  int nRows = mysql_num_rows(res);

  MYSQL_ROW columnsRow;
  FieldDescrList *fieldsResult = (FieldDescrList*)malloc(sizeof(FieldDescrList));
  fieldsResult->list = (FieldDescr*)malloc(sizeof(FieldDescr) * nRows);
  fieldsResult->n_fields = nRows;

  int rowNum = 0;

  while ((columnsRow = mysql_fetch_row(res)))
  {
    (fieldsResult->list)[rowNum].FieldName = strdup(columnsRow[SHOW_COLUMNS_NAME_IDX]);
    (fieldsResult->list)[rowNum].FieldTypeS2 = strdup(columnsRow[SHOW_COLUMNS_TYPE_IDX]);
    if (columnsRow[SHOW_COLUMNS_DEFAULT_VALUE_IDX] && *columnsRow[SHOW_COLUMNS_DEFAULT_VALUE_IDX])
    {
      (fieldsResult->list)[rowNum].DefaultValue = strdup(columnsRow[SHOW_COLUMNS_DEFAULT_VALUE_IDX]);
    }
    else
    {
       (fieldsResult->list)[rowNum].DefaultValue = NULL;
    }
    rowNum++;
  }
  return fieldsResult;
}

FieldDescr *GetFieldDescr(const char *name, FieldDescrList *allFields)
{
  int i;
  for (i = 0; i < allFields->n_fields; ++i)
  {
    if (!strcmp(name, (allFields->list)[i].FieldName))
    {
      return allFields->list + i;
    }
  }
  return NULL;
}

int GetFieldTypeShortLen(const char *name)
{
  int i;
  for (i = 0; i < strlen(name); ++i)
  {
    if (name[i] == '(')
    {
      return i;
    }
  }
  return 0;
}

void FreeFieldDescrList(FieldDescrList *allFields)
{
  int i;
  for (i = 0; i < allFields->n_fields; ++i)
  {
    free(allFields->list[i].FieldName);
    free(allFields->list[i].FieldTypeS2);
    free(allFields->list[i].DefaultValue);
  }
  free(allFields->list);
  free(allFields);
}

/**
Get the result of SHOW FIELDS FROM <catalog>.<table>

@param[in] stmt           Handle to statement
@param[in] catalog        Catalog (database) of table, @c NULL for current
@param[in] catalog_length Length of the catalog name
@param[in] table          Name of the table, not NULL
@param[in] table_length   Length of table name

@return Result of SHOW FIELDS FROM, or NULL if there is an error,
or empty result (check mysql_errno(stmt->Connection->mariadb) != 0)
*/
MYSQL_RES *S2_ShowKeysInTable(MADB_Stmt  *stmt,
                              SQLCHAR     *catalog,
                              SQLSMALLINT  catalog_length,
                              SQLCHAR     *table,
                              SQLSMALLINT  table_length)
{
  MADB_DynString query;
  MADB_InitDynamicString(&query, "SHOW KEYS FROM ", 1024, 512);

	if (catalog && *catalog)
	{
    MADB_DynstrAppend(&query, "`");
    MADB_DynstrAppendMem(&query, catalog, catalog_length);
    MADB_DynstrAppend(&query, "`");
    MADB_DynstrAppend(&query, ".");
	}

  MADB_DynstrAppend(&query, "`");
  MADB_DynstrAppendMem(&query, table, table_length);
  MADB_DynstrAppend(&query, "`");

  LOCK_MARIADB(stmt->Connection);
  if (mysql_real_query(stmt->Connection->mariadb, query.str, query.length))
  {
    MADB_DynstrFree(&query);
    UNLOCK_MARIADB(stmt->Connection);
    MADB_SetError(&stmt->Error, MADB_ERR_HY001, mysql_error(stmt->Connection->mariadb), mysql_errno(stmt->Connection->mariadb));
    return NULL;
  }
  MADB_DynstrFree(&query);
  UNLOCK_MARIADB(stmt->Connection);

  return mysql_store_result(stmt->Connection->mariadb);
}


MYSQL_RES *
S2_ListFields(MADB_Stmt   *stmt,
                SQLCHAR     *catalog,
                SQLSMALLINT  catalog_length,
                SQLCHAR     *table,
                SQLSMALLINT  table_length,
                SQLCHAR     *column_like,
                SQLSMALLINT  column_length)
{
  MYSQL_RES *result;
  char buff[NAME_LEN * 2 + 64], column_buff[NAME_LEN * 2 + 64];
  char *current_db;
  if (stmt->Connection->mariadb->db)
  {
    current_db = strdup(stmt->Connection->mariadb->db);
  }
  else
  {
    current_db = NULL;
  }
  if (table_length <= 0 || !table)
  {
    return NULL;
  }

  /* If a catalog was specified, we have to change working catalog
     to be able to use mysql_list_fields. */
  int need_db_change = !current_db || (catalog_length > 0 && catalog && strcmp(current_db, catalog));
  if (need_db_change)
  {
    strncpy(buff, (const char*)catalog, catalog_length);
    buff[catalog_length]= '\0';

    if (mysql_select_db(stmt->Connection->mariadb, buff))
    {
      return NULL;
    }
  }
  strcpy(buff, "`");
  strncat(buff, (const char*)table, table_length);
  strcat(buff, "`");
  if (column_length > 0)
  {
    strncpy(column_buff, (const char *) column_like, column_length);
    column_buff[column_length] = '\0';
    fflush(stdout);
    result = mysql_list_fields(stmt->Connection->mariadb, buff, column_buff);
  }
  else
  {
    result = mysql_list_fields(stmt->Connection->mariadb, buff, NULL);
  }
  if (!result)
  {
    MADB_SetError(&stmt->Error, MADB_ERR_HY001, mysql_error(stmt->Connection->mariadb), mysql_errno(stmt->Connection->mariadb));
    free(current_db);
    return NULL;
  }

  if (current_db && need_db_change)
  {
    if (mysql_select_db(stmt->Connection->mariadb, current_db))
    {
      MADB_SetError(&stmt->Error, MADB_ERR_HY001, mysql_error(stmt->Connection->mariadb), mysql_errno(stmt->Connection->mariadb));
      mysql_free_result(result);
      free(current_db);
      return NULL;
    }
  }
  free(current_db);
  return result;
}
