/************************************************************************************
   Copyright (C) 2013, 2019 MariaDB Corporation AB
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
#ifndef _ma_statement_h_
#define _ma_statement_h_

struct st_ma_stmt_methods
{
  SQLRETURN (*Prepare)(MADB_Stmt *Stmt, char *StatementText, SQLINTEGER TextLength, BOOL ExecDirect);
  SQLRETURN (*Execute)(MADB_Stmt *Stmt, BOOL ExecDirect);
  SQLRETURN (*Fetch)(MADB_Stmt *Stmt);
  SQLRETURN (*BindColumn)(MADB_Stmt *Stmt, SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
                     SQLPOINTER TargetValuePtr, SQLLEN BufferLength, SQLLEN *StrLen_or_Ind);
  SQLRETURN (*BindParam)(MADB_Stmt *Stmt,  SQLUSMALLINT ParameterNumber, SQLSMALLINT InputOutputType, SQLSMALLINT ValueType,
                        SQLSMALLINT ParameterType, SQLULEN ColumnSize, SQLSMALLINT DecimalDigits, SQLPOINTER ParameterValuePtr,
                        SQLLEN BufferLength, SQLLEN *StrLen_or_IndPtr);
  SQLRETURN (*ExecDirect)(MADB_Stmt *Stmt, char *StatementText, SQLINTEGER TextLength);
  SQLRETURN (*GetData)(SQLHSTMT StatementHandle, SQLUSMALLINT Col_or_Param_Num, SQLSMALLINT TargetType,
                       SQLPOINTER TargetValuePtr, SQLLEN BufferLength, SQLLEN * StrLen_or_IndPtr, BOOL InternalUse);
  SQLRETURN (*RowCount)(MADB_Stmt *Stmt, SQLLEN *RowCountPtr);
  SQLRETURN (*ParamCount)(MADB_Stmt *Stmt, SQLSMALLINT *ParamCountPtr);
  SQLRETURN (*ColumnCount)(MADB_Stmt *Stmt, SQLSMALLINT *ColumnCountPtr);
  SQLRETURN (*GetAttr)(MADB_Stmt *Stmt, SQLINTEGER Attribute, SQLPOINTER Value, SQLINTEGER BufferLength,
                       SQLINTEGER *StringLength);
  SQLRETURN (*SetAttr)(MADB_Stmt *Stmt, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER StringLength);
  SQLRETURN (*StmtFree)(MADB_Stmt *Stmt, SQLUSMALLINT Option);
  SQLRETURN (*ColAttribute)(MADB_Stmt *Stmt, SQLUSMALLINT ColumnNumber, SQLUSMALLINT FieldIdentifier, SQLPOINTER CharacterAttributePtr,
             SQLSMALLINT BufferLength, SQLSMALLINT *StringLengthPtr, SQLLEN *NumericAttributePtr,
             my_bool isWchar);
  SQLRETURN (*ColumnPrivileges)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                                char *SchemaName, SQLSMALLINT NameLength2, char *TableName,
                                SQLSMALLINT NameLength3, char *ColumnName, SQLSMALLINT NameLength4);
  SQLRETURN (*TablePrivileges)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                               char *SchemaName, SQLSMALLINT NameLength2,
                               char *TableName, SQLSMALLINT NameLength3);
  SQLRETURN (*Tables)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                      char *SchemaName, SQLSMALLINT NameLength2, char *TableName,
                      SQLSMALLINT NameLength3, char *TableType, SQLSMALLINT NameLength4);
  SQLRETURN (*Statistics)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                          char *SchemaName, SQLSMALLINT NameLength2,
                          char *TableName, SQLSMALLINT NameLength3,
                          SQLUSMALLINT Unique, SQLUSMALLINT Reserved);
  SQLRETURN (*Columns)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                       char *SchemaName, SQLSMALLINT NameLength2,
                       char *TableName, SQLSMALLINT NameLength3,
                       char *ColumnName, SQLSMALLINT NameLength4);
  SQLRETURN (*ProcedureColumns)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                                char *SchemaName, SQLSMALLINT NameLength2, char *ProcName,
                                SQLSMALLINT NameLength3, char *ColumnName, SQLSMALLINT NameLength4);
  SQLRETURN (*PrimaryKeys)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                               char *SchemaName, SQLSMALLINT NameLength2, char *TableName,
                               SQLSMALLINT NameLength3);
  SQLRETURN (*SpecialColumns)(MADB_Stmt *Stmt, SQLUSMALLINT IdentifierType,
                              char *CatalogName, SQLSMALLINT NameLength1, 
                              char *SchemaName, SQLSMALLINT NameLength2,
                              char *TableName, SQLSMALLINT NameLength3,
                              SQLUSMALLINT Scope, SQLUSMALLINT Nullable);
  SQLRETURN (*Procedures)(MADB_Stmt *Stmt, char *CatalogName, SQLSMALLINT NameLength1,
                          char *SchemaName, SQLSMALLINT NameLength2, char *ProcName,
                          SQLSMALLINT NameLength3);
  SQLRETURN (*ForeignKeys)(MADB_Stmt *Stmt, char *PKCatalogName, SQLSMALLINT NameLength1,
                           char *PKSchemaName, SQLSMALLINT NameLength2, char *PKTableName,
                           SQLSMALLINT NameLength3, char *FKCatalogName, SQLSMALLINT NameLength4,
                           char *FKSchemaName, SQLSMALLINT NameLength5,  char *FKTableName,
                           SQLSMALLINT NameLength6);
  SQLRETURN (*DescribeCol)(MADB_Stmt *Stmt, SQLUSMALLINT ColumnNumber, void *ColumnName,
                         SQLSMALLINT BufferLength, SQLSMALLINT *NameLengthPtr,
                         SQLSMALLINT *DataTypePtr, SQLULEN *ColumnSizePtr, SQLSMALLINT *DecimalDigitsPtr,
                         SQLSMALLINT *NullablePtr, my_bool isWChar);
  SQLRETURN (*SetCursorName)(MADB_Stmt *Stmt, char *Buffer, SQLINTEGER BufferLength);
  SQLRETURN (*GetCursorName)(MADB_Stmt *Stmt, void *CursorName, SQLSMALLINT BufferLength, 
                             SQLSMALLINT *NameLengthPtr, my_bool isWChar);
  SQLRETURN (*SetPos)(MADB_Stmt *Stmt, SQLSETPOSIROW Row, SQLUSMALLINT Operation,
                      SQLUSMALLINT LockType, int ArrayOffset);
  SQLRETURN (*FetchScroll)(MADB_Stmt *Stmt, SQLSMALLINT FetchOrientation,
                           SQLLEN FetchOffset);
  SQLRETURN (*ParamData)(MADB_Stmt *Stmt, SQLPOINTER *ValuePtrPtr);
  SQLRETURN (*PutData)(MADB_Stmt *Stmt, SQLPOINTER DataPtr, SQLLEN StrLen_or_Ind);
  SQLRETURN (*BulkOperations)(MADB_Stmt *Stmt, SQLSMALLINT Operation);
  SQLRETURN (*RefreshDynamicCursor)(MADB_Stmt *Stmt);
  SQLRETURN (*RefreshRowPtrs)(MADB_Stmt *Stmt);
  SQLRETURN (*GetOutParams)(MADB_Stmt *Stmt, int CurrentOffset);
};

SQLRETURN    MADB_StmtInit          (MADB_Dbc *Connection, SQLHANDLE *pHStmt);
void         MADB_StmtReset         (MADB_Stmt *Stmt);
SQLUSMALLINT MapColAttributeDescType(SQLUSMALLINT FieldIdentifier);
MYSQL_RES*   FetchMetadata          (MADB_Stmt *Stmt);
SQLRETURN    MADB_DoExecute(MADB_Stmt *Stmt);

SQLRETURN    MADB_StmtFetchColumn(MADB_Stmt* Stmt, MYSQL_BIND *bind, unsigned int column, unsigned long offset);
SQLRETURN    MADB_FetchCsps(MADB_Stmt* Stmt);
SQLRETURN    MADB_FetchColumnCsps(MADB_Stmt* Stmt, MYSQL_BIND *bind, unsigned int column, unsigned long offset);

void         MADB_CspsFreeResult(MADB_Stmt *Stmt, MYSQL_RES** CspsRes, MYSQL_STMT* stmt);
void         MADB_CspsCopyResult(MADB_Stmt *Stmt, MYSQL_RES* CspsRes, MYSQL_STMT* stmt);
void         MADB_CspsFreeDAE(MADB_Stmt *Stmt);

#define MADB_MAX_CURSOR_NAME 64 * 3 + 1
#define MADB_CHECK_STMT_HANDLE(a,b)\
  if (!(a) || !(a)->b)\
    return SQL_INVALID_HANDLE

#define MADB_STMT_COLUMN_COUNT(aStmt) (aStmt)->Ird->Header.Count
#define MADB_RESET_COLUMT_COUNT(aStmt) (aStmt)->Ird->Header.Count= 0
#define MADB_STMT_PARAM_COUNT(aStmt)  (aStmt)->ParamCount
#define MADB_POSITIONED_COMMAND(aStmt) ((aStmt)->PositionedCommand && (aStmt)->PositionedCursor)
/* So far we always use all fields for index. Once that is changed, this should be changed as well */
#define MADB_POS_COMM_IDX_FIELD_COUNT(aStmt) MADB_STMT_COLUMN_COUNT((aStmt)->PositionedCursor)
#define MADB_STMT_FORGET_NEXT_POS(aStmt) (aStmt)->Cursor.Next= NULL
#define MADB_STMT_RESET_CURSOR(aStmt) (aStmt)->Cursor.Position= -1; MADB_STMT_FORGET_NEXT_POS(aStmt)
#define MADB_STMT_CLOSE_STMT(aStmt)   mysql_stmt_close((aStmt)->stmt);(aStmt)->stmt= NULL
#define MADB_SSPS_ENABLED(aStmt) (aStmt)->Connection->Dsn->NoSsps == FALSE
#define MADB_SSPS_DISABLED(aStmt) !(MADB_SSPS_ENABLED(aStmt))
#define NO_CACHE(aStmt) ((aStmt)->Options.CursorType == SQL_CURSOR_FORWARD_ONLY && (aStmt)->Connection->Dsn->NoCache)
/************** SQLColumns       *************/
#define MADB_DATA_TYPE_ODBC2 \
    " WHEN 'date' THEN " XSTR(SQL_DATE) \
    " WHEN 'time' THEN " XSTR(SQL_TIME) \
    " WHEN 'datetime' THEN " XSTR(SQL_TIMESTAMP) \
    " WHEN 'timestamp' THEN " XSTR(SQL_TIMESTAMP)

#define MADB_DATA_TYPE_ODBC3 \
    " WHEN 'date' THEN " XSTR(SQL_TYPE_DATE) \
    " WHEN 'time' THEN " XSTR(SQL_TYPE_TIME) \
    " WHEN 'datetime' THEN " XSTR(SQL_TYPE_TIMESTAMP) \
    " WHEN 'timestamp' THEN " XSTR(SQL_TYPE_TIMESTAMP)

#define MADB_SQL_DATA_TYPE \
    " WHEN 'date' THEN " XSTR(SQL_DATETIME) \
    " WHEN 'time' THEN " XSTR(SQL_DATETIME) \
    " WHEN 'datetime' THEN " XSTR(SQL_DATETIME) \
    " WHEN 'timestamp' THEN " XSTR(SQL_DATETIME)

#define MADB_DATA_TYPE_UNICODE \
    " WHEN 'char' THEN " XSTR(SQL_WCHAR) \
    " WHEN 'varchar' THEN " XSTR(SQL_WVARCHAR) \
    " WHEN 'longtext' THEN " XSTR(SQL_WLONGVARCHAR) \
    " WHEN 'mediumtext' THEN " XSTR(SQL_WLONGVARCHAR) \
    " WHEN 'text' THEN " XSTR(SQL_WLONGVARCHAR) \
    " WHEN 'tinytext' THEN " XSTR(SQL_WLONGVARCHAR) \
    " WHEN 'json' THEN " XSTR(SQL_WLONGVARCHAR)\
    " WHEN 'enum' THEN " XSTR(SQL_WVARCHAR) \
    " WHEN 'set' THEN " XSTR(SQL_WVARCHAR)

#define MADB_DATA_TYPE_ANSI \
    " WHEN 'char' THEN " XSTR(SQL_CHAR) \
    " WHEN 'varchar' THEN " XSTR(SQL_VARCHAR) \
    " WHEN 'longtext' THEN " XSTR(SQL_LONGVARCHAR) \
    " WHEN 'mediumtext' THEN " XSTR(SQL_LONGVARCHAR) \
    " WHEN 'text' THEN " XSTR(SQL_LONGVARCHAR) \
    " WHEN 'tinytext' THEN " XSTR(SQL_LONGVARCHAR) \
    " WHEN 'json' THEN " XSTR(SQL_LONGVARCHAR)\
    " WHEN 'enum' THEN " XSTR(SQL_VARCHAR) \
    " WHEN 'set' THEN " XSTR(SQL_VARCHAR)

#define MADB_DATA_TYPE_NUMERIC \
    "CASE DATA_TYPE WHEN 'bit' THEN " XSTR(SQL_BINARY) \
    " WHEN 'tinyint' THEN " XSTR(SQL_TINYINT) \
    " WHEN 'smallint' THEN " XSTR(SQL_SMALLINT) \
    " WHEN 'mediumint' THEN " XSTR(SQL_INTEGER) \
    " WHEN 'int' THEN " XSTR(SQL_INTEGER) \
    " WHEN 'bigint' THEN " XSTR(SQL_BIGINT) \
    " WHEN 'double' THEN " XSTR(SQL_DOUBLE) \
    " WHEN 'float' THEN " XSTR(SQL_REAL) \
    " WHEN 'decimal' THEN " XSTR(SQL_DECIMAL) \
    " WHEN 'newdecimal' THEN " XSTR(SQL_DECIMAL) \
    " WHEN 'year' THEN " XSTR(SQL_SMALLINT)

#define MADB_DATA_TYPE_BINARY \
    " WHEN 'binary' THEN " XSTR(SQL_BINARY) \
    " WHEN 'varbinary' THEN " XSTR(SQL_VARBINARY) \
    " WHEN 'longblob' THEN " XSTR(SQL_LONGVARBINARY) \
    " WHEN 'mediumblob' THEN " XSTR(SQL_LONGVARBINARY) \
    " WHEN 'blob' THEN " XSTR(SQL_LONGVARBINARY) \
    " WHEN 'tinyblob' THEN " XSTR(SQL_LONGVARBINARY) \
    " WHEN 'geography' THEN " XSTR(SQL_VARBINARY) \
    " WHEN 'geographypoint' THEN " XSTR(SQL_VARBINARY) " END :> SMALLINT"

#define MADB_DATA_TYPE_ODBC3A MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC3 MADB_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_DATA_TYPE_ODBC3U MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC3 MADB_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY
#define MADB_DATA_TYPE_ODBC2A MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC2 MADB_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_DATA_TYPE_ODBC2U MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC2 MADB_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY

#define MADB_COLUMNS_TYPE_NAME "IF (COLUMN_TYPE LIKE '%UNSIGNED%', CONCAT(DATA_TYPE, ' unsigned'), DATA_TYPE)"

#define MADB_COLUMNS_COLUMN_SIZE "CASE WHEN DATA_TYPE = 'bit' THEN 1"\
    " WHEN DATA_TYPE IN ('tinyint', 'smallint', 'mediumint', 'int', 'bigint', 'decimal', 'newdecimal', 'double', 'float') THEN NUMERIC_PRECISION" \
    " WHEN DATA_TYPE = 'date' THEN 10" \
    " WHEN DATA_TYPE = 'time' THEN IF(COLUMN_TYPE = 'time(6)', 15, 8)" \
    " WHEN DATA_TYPE = 'datetime' THEN IF(COLUMN_TYPE = 'datetime(6)', 26, 19)" \
    " WHEN DATA_TYPE = 'timestamp' THEN IF(COLUMN_TYPE = 'timestamp(6)', 26, 19)" \
    " WHEN DATA_TYPE = 'year' THEN 4" \
    " ELSE CHARACTER_MAXIMUM_LENGTH END :> INT"

#define MADB_COLUMNS_BUFFER_LEN "CASE DATA_TYPE WHEN 'bit' THEN 1"\
    " WHEN 'tinyint' THEN 1" \
    " WHEN 'smallint' THEN 2" \
    " WHEN 'mediumint' THEN 3" \
    " WHEN 'int' THEN 4" \
    " WHEN 'bigint' THEN 8" \
    " WHEN 'double' THEN 8"\
    " WHEN 'float' THEN 4"\
    " WHEN 'decimal' THEN NUMERIC_PRECISION+2" \
    " WHEN 'newdecimal' THEN NUMERIC_PRECISION+2" \
    " WHEN 'date' THEN 6" \
    " WHEN 'time' THEN 6" \
    " WHEN 'datetime' THEN 16" \
    " WHEN 'timestamp' THEN 16" \
    " WHEN 'year' THEN 2" \
    " ELSE CHARACTER_OCTET_LENGTH END :> INT"

#define MADB_COLUMNS_DECIMAL_DIGITS "CASE DATA_TYPE WHEN 'decimal' THEN NUMERIC_SCALE" \
    " WHEN 'newdecimal' THEN NUMERIC_SCALE " \
    " WHEN 'tinyint' THEN 0" \
    " WHEN 'smallint' THEN 0" \
    " WHEN 'mediumint' THEN 0" \
    " WHEN 'int' THEN 0" \
    " WHEN 'bigint' THEN 0" \
    " WHEN 'double' THEN IF(NUMERIC_SCALE IS NULL, 6, NUMERIC_SCALE)" \
    " WHEN 'float' THEN IF(NUMERIC_SCALE IS NULL, 0, NUMERIC_SCALE)" \
    " WHEN 'date' THEN 0" \
    " WHEN 'time' THEN IF(COLUMN_TYPE = 'time(6)', 6, 0)" \
    " WHEN 'datetime' THEN IF(COLUMN_TYPE = 'datetime(6)', 6, 0)" \
    " WHEN 'timestamp' THEN IF(COLUMN_TYPE = 'timestamp(6)', 6, 0) END :> SMALLINT"

#define MADB_COLUMNS_NUM_PREC_RADIX \
    "CASE WHEN DATA_TYPE IN ('tinyint', 'smallint', 'mediumint', 'int', 'bigint', 'double', 'float', 'decimal', 'newdecimal', 'year') THEN 10 END :> SMALLINT"

#define MADB_COLUMNS_NULLABLE "CASE WHEN IS_NULLABLE = 'YES' THEN " XSTR(SQL_NULLABLE) \
    " ELSE " XSTR(SQL_NO_NULLS) " END :> SMALLINT"

#define MADB_SQL_DATA_TYPE_ODBCA MADB_DATA_TYPE_NUMERIC MADB_SQL_DATA_TYPE MADB_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_SQL_DATA_TYPE_ODBCU MADB_DATA_TYPE_NUMERIC MADB_SQL_DATA_TYPE MADB_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY

#define MADB_COLUMNS_DATETIME_SUB "CASE WHEN DATA_TYPE = 'date' THEN " XSTR(SQL_CODE_DATE) \
    " WHEN DATA_TYPE = 'time' THEN " XSTR(SQL_CODE_TIME) \
    " WHEN DATA_TYPE = 'datetime' THEN " XSTR(SQL_CODE_TIMESTAMP) \
    " WHEN DATA_TYPE = 'timestamp' THEN " XSTR(SQL_CODE_TIMESTAMP) " END :> SMALLINT"

#define MADB_COLUMNSp1 \
    "SELECT TABLE_SCHEMA AS TABLE_CAT, NULL AS TABLE_SCHEM, TABLE_NAME, COLUMN_NAME, "
#define MADB_COLUMNSp2 \
    " AS DATA_TYPE, " MADB_COLUMNS_TYPE_NAME " AS TYPE_NAME, " MADB_COLUMNS_COLUMN_SIZE " AS COLUMN_SIZE, " \
    MADB_COLUMNS_BUFFER_LEN " AS BUFFER_LENGTH, " MADB_COLUMNS_DECIMAL_DIGITS " AS DECIMAL_DIGITS, " \
    MADB_COLUMNS_NUM_PREC_RADIX " AS NUM_PREC_RADIX, " MADB_COLUMNS_NULLABLE " AS NULLABLE, NULL AS REMARKS, " \
    "COLUMN_DEFAULT AS COLUMN_DEF, "
#define MADB_COLUMNSp3 \
    " AS SQL_DATA_TYPE, " MADB_COLUMNS_DATETIME_SUB " AS SQL_DATETIME_SUB, " \
    "CHARACTER_OCTET_LENGTH :> INT AS CHAR_OCTET_LENGTH, ORDINAL_POSITION, IS_NULLABLE FROM INFORMATION_SCHEMA.COLUMNS"

#define MADB_COLUMNS_ODBC3U MADB_COLUMNSp1 MADB_DATA_TYPE_ODBC3U MADB_COLUMNSp2 MADB_SQL_DATA_TYPE_ODBCU MADB_COLUMNSp3
#define MADB_COLUMNS_ODBC2U MADB_COLUMNSp1 MADB_DATA_TYPE_ODBC2U MADB_COLUMNSp2 MADB_SQL_DATA_TYPE_ODBCU MADB_COLUMNSp3
#define MADB_COLUMNS_ODBC3A MADB_COLUMNSp1 MADB_DATA_TYPE_ODBC3A MADB_COLUMNSp2 MADB_SQL_DATA_TYPE_ODBCA MADB_COLUMNSp3
#define MADB_COLUMNS_ODBC2A MADB_COLUMNSp1 MADB_DATA_TYPE_ODBC2A MADB_COLUMNSp2 MADB_SQL_DATA_TYPE_ODBCA MADB_COLUMNSp3

#define MADB_COLUMNS(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ?\
        (StmtHndl->Connection->IsAnsi ? MADB_COLUMNS_ODBC3A : MADB_COLUMNS_ODBC3U) : \
        (StmtHndl->Connection->IsAnsi ? MADB_COLUMNS_ODBC2A : MADB_COLUMNS_ODBC2U))

/************** End of SQLColumns ************/

/************** SQLProcedureColumns **********/

#define MADB_PROCEDURE_COLUMNS_COLUMN_TYPE "CASE PARAMETER_MODE "\
  "  WHEN 'IN' THEN " XSTR(SQL_PARAM_INPUT)\
  "  WHEN 'OUT' THEN " XSTR(SQL_PARAM_OUTPUT)\
  "  WHEN 'INOUT' THEN " XSTR(SQL_PARAM_INPUT_OUTPUT)\
  "  ELSE IF(PARAMETER_MODE IS NULL, " XSTR(SQL_RETURN_VALUE) ", " XSTR(SQL_PARAM_TYPE_UNKNOWN) ") END :> SMALLINT"

#define MADB_PROCEDURE_COLUMNS_NULLABLE "CASE WHEN DTD_IDENTIFIER LIKE '%%NOT NULL%%' THEN " XSTR(SQL_NO_NULLS) \
    " ELSE " XSTR(SQL_NULLABLE) " END :> SMALLINT"

#define MADB_PROCEDURE_COLUMNS_DEFAULT_VALUE "IF(dtd_identifier like '%%DEFAULT%%', "\
    "RIGHT(dtd_identifier, LENGTH(dtd_identifier) - (REGEXP_INSTR(dtd_identifier, 'DEFAULT [^\\s]+') + 7)), NULL) "

#define MADB_PROCEDURE_COLUMNS_COLUMN_SIZE "CASE WHEN DATA_TYPE = 'bit' THEN 1"\
    " WHEN DATA_TYPE IN ('tinyint', 'smallint', 'mediumint', 'int', 'bigint', 'decimal', 'newdecimal', 'double', 'float') THEN NUMERIC_PRECISION" \
    " WHEN DATA_TYPE = 'date' THEN 10" \
    " WHEN DATA_TYPE = 'time' THEN IF(DTD_IDENTIFIER like '%%time(6)%%', 15, 8)" \
    " WHEN DATA_TYPE = 'datetime' THEN IF(DTD_IDENTIFIER like '%%datetime(6)%%', 26, 19)" \
    " WHEN DATA_TYPE = 'timestamp' THEN IF(DTD_IDENTIFIER like '%%timestamp(6)%%', 26, 19)" \
    " WHEN DATA_TYPE = 'year' THEN 4" \
    " ELSE CHARACTER_MAXIMUM_LENGTH END :> INT"

#define MADB_PROCEDURE_COLUMNS_BUFFER_LEN \
    "CASE DATA_TYPE WHEN 'bit' THEN 1" \
    " WHEN 'tinyint' THEN 1" \
    " WHEN 'smallint' THEN 2" \
    " WHEN 'mediumint' THEN 3" \
    " WHEN 'int' THEN 4" \
    " WHEN 'bigint' THEN 8" \
    " WHEN 'double' THEN 8"\
    " WHEN 'float' THEN 4"\
    " WHEN 'decimal' THEN NUMERIC_PRECISION+2" \
    " WHEN 'newdecimal' THEN NUMERIC_PRECISION+2" \
    " WHEN 'date' THEN 6" \
    " WHEN 'time' THEN 6" \
    " WHEN 'datetime' THEN 16" \
    " WHEN 'timestamp' THEN 16" \
    " WHEN 'year' THEN 2" \
    " ELSE CHARACTER_OCTET_LENGTH END :> INT"

#define MADB_PROCEDURE_COLUMNS_DECIMAL_DIGITS "CASE DATA_TYPE WHEN 'decimal' THEN NUMERIC_SCALE" \
    " WHEN 'newdecimal' THEN NUMERIC_SCALE" \
    " WHEN 'tinyint' THEN 0" \
    " WHEN 'smallint' THEN 0" \
    " WHEN 'mediumint' THEN 0" \
    " WHEN 'int' THEN 0" \
    " WHEN 'bigint' THEN 0" \
    " WHEN 'double' THEN IF(NUMERIC_SCALE IS NULL, 6, NUMERIC_SCALE)" \
    " WHEN 'float' THEN IF(NUMERIC_SCALE IS NULL, 0, NUMERIC_SCALE)" \
    " WHEN 'date' THEN 0" \
    " WHEN 'time' THEN IF(DTD_IDENTIFIER like '%%time(6)%%', 6, 0)" \
    " WHEN 'datetime' THEN IF(DTD_IDENTIFIER like '%%datetime(6)%%', 6, 0)" \
    " WHEN 'timestamp' THEN IF(DTD_IDENTIFIER like '%%timestamp(6)%%', 6, 0) END :> SMALLINT"

#define MADB_PROCEDURE_COLUMNS_TYPE_NAME "CASE DATA_TYPE" \
    " WHEN 'char' THEN IF(DTD_IDENTIFIER LIKE '%%binary%%', 'binary', LOWER(DATA_TYPE))" \
    " WHEN 'varchar' THEN IF(DTD_IDENTIFIER LIKE '%%varbinary%%', 'varbinary', LOWER(DATA_TYPE))" \
    " WHEN 'longtext' THEN IF(DTD_IDENTIFIER LIKE '%%longblob%%', 'longblob', LOWER(DATA_TYPE))"  \
    " WHEN 'mediumtext' THEN IF(DTD_IDENTIFIER LIKE '%%mediumblob%%', 'mediumblob', LOWER(DATA_TYPE))" \
    " WHEN 'text' THEN IF(DTD_IDENTIFIER LIKE '%%blob%%', 'blob', LOWER(DATA_TYPE))"  \
    " WHEN 'tinytext' THEN IF(DTD_IDENTIFIER LIKE '%%tinyblob%%', 'tinyblob', LOWER(DATA_TYPE))" \
    " ELSE IF (DTD_IDENTIFIER LIKE '%%UNSIGNED%%', CONCAT(LOWER(DATA_TYPE), ' unsigned'), LOWER(DATA_TYPE)) END"

#define MADB_PROCEDURE_COLUMNSp1 \
  "SELECT SPECIFIC_SCHEMA AS PROCEDURE_CAT, "\
  "NULL AS PROCEDURE_SCHEM, "\
  "SPECIFIC_NAME AS PROCEDURE_NAME, "\
  "IF(PARAMETER_NAME IS NULL, '', PARAMETER_NAME) AS COLUMN_NAME, "\
  MADB_PROCEDURE_COLUMNS_COLUMN_TYPE " AS COLUMN_TYPE,"

#define MADB_PROCEDURE_COLUMNSp2 \
  " AS DATA_TYPE, "\
  MADB_PROCEDURE_COLUMNS_TYPE_NAME " AS TYPE_NAME, "\
  MADB_PROCEDURE_COLUMNS_COLUMN_SIZE " AS COLUMN_SIZE, "\
  MADB_PROCEDURE_COLUMNS_BUFFER_LEN " AS BUFFER_LENGTH, "\
  MADB_PROCEDURE_COLUMNS_DECIMAL_DIGITS " AS DECIMAL_DIGITS, "\
  MADB_COLUMNS_NUM_PREC_RADIX " AS NUM_PREC_RADIX,"\
  MADB_PROCEDURE_COLUMNS_NULLABLE " AS NULLABLE,"\
  "NULL AS REMARKS, "\
  MADB_PROCEDURE_COLUMNS_DEFAULT_VALUE " AS COLUMN_DEF,"

#define MADB_PROCEDURE_COLUMNSp3 \
  " AS SQL_DATA_TYPE, "\
  MADB_COLUMNS_DATETIME_SUB " AS SQL_DATETIME_SUB, "\
  "CHARACTER_OCTET_LENGTH :> INT AS CHAR_OCTET_LENGTH, "\
  "ORDINAL_POSITION, "\
  "IF(DTD_IDENTIFIER LIKE '%%NOT NULL%%', 'NO', 'YES') AS IS_NULLABLE FROM INFORMATION_SCHEMA.PARAMETERS "


#define MADB_PROCEDURE_COLUMNS_DATA_TYPE_UNICODE \
    " WHEN 'char' THEN IF(DTD_IDENTIFIER LIKE '%%binary%%', " XSTR(SQL_BINARY) ", " XSTR(SQL_WCHAR) ")"\
    " WHEN 'varchar' THEN IF(DTD_IDENTIFIER LIKE '%%varbinary%%', " XSTR(SQL_VARBINARY) ", " XSTR(SQL_WVARCHAR) ")" \
    " WHEN 'longtext' THEN IF(DTD_IDENTIFIER LIKE '%%longblob%%', " XSTR(SQL_LONGVARBINARY) ", " XSTR(SQL_WLONGVARCHAR) ")" \
    " WHEN 'mediumtext' THEN IF(DTD_IDENTIFIER LIKE '%%mediumblob%%', " XSTR(SQL_LONGVARBINARY) ", " XSTR(SQL_WLONGVARCHAR) ")" \
    " WHEN 'text' THEN IF(DTD_IDENTIFIER LIKE '%%blob%%', " XSTR(SQL_LONGVARBINARY) ", " XSTR(SQL_WLONGVARCHAR) ")" \
    " WHEN 'tinytext' THEN IF(DTD_IDENTIFIER LIKE '%%tinyblob%%', " XSTR(SQL_LONGVARBINARY) ", " XSTR(SQL_WLONGVARCHAR) ")" \
    " WHEN 'json' THEN " XSTR(SQL_WLONGVARCHAR) \
    " WHEN 'enum' THEN " XSTR(SQL_WVARCHAR) \
    " WHEN 'set' THEN " XSTR(SQL_WVARCHAR)

#define MADB_PROCEDURE_COLUMNS_DATA_TYPE_ANSI \
    " WHEN 'char' THEN IF(DTD_IDENTIFIER LIKE '%%binary%%', " XSTR(SQL_BINARY) ", " XSTR(SQL_CHAR) ")" \
    " WHEN 'varchar' THEN IF(DTD_IDENTIFIER LIKE '%%varbinary%%', " XSTR(SQL_VARBINARY) ", " XSTR(SQL_VARCHAR) ")" \
    " WHEN 'longtext' THEN IF(DTD_IDENTIFIER LIKE '%%longblob%%', " XSTR(SQL_LONGVARBINARY) ", " XSTR(SQL_LONGVARCHAR) ")" \
    " WHEN 'mediumtext' THEN IF(DTD_IDENTIFIER LIKE '%%mediumblob%%', " XSTR(SQL_LONGVARBINARY) ", " XSTR(SQL_LONGVARCHAR) ")" \
    " WHEN 'text' THEN IF(DTD_IDENTIFIER LIKE '%%blob%%', " XSTR(SQL_LONGVARBINARY) ", " XSTR(SQL_LONGVARCHAR) ")" \
    " WHEN 'tinytext' THEN IF(DTD_IDENTIFIER LIKE '%%tinyblob%%', " XSTR(SQL_LONGVARBINARY) ", " XSTR(SQL_LONGVARCHAR) ")" \
    " WHEN 'json' THEN " XSTR(SQL_LONGVARCHAR) \
    " WHEN 'enum' THEN " XSTR(SQL_VARCHAR) \
    " WHEN 'set' THEN " XSTR(SQL_VARCHAR)

#define MADB_PROCEDURE_COLUMNS_DATA_TYPE_ODBC3A MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC3 MADB_PROCEDURE_COLUMNS_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_PROCEDURE_COLUMNS_DATA_TYPE_ODBC3U MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC3 MADB_PROCEDURE_COLUMNS_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY
#define MADB_PROCEDURE_COLUMNS_DATA_TYPE_ODBC2A MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC2 MADB_PROCEDURE_COLUMNS_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_PROCEDURE_COLUMNS_DATA_TYPE_ODBC2U MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC2 MADB_PROCEDURE_COLUMNS_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY

#define MADB_PROCEDURE_COLUMNS_SQL_DATA_TYPE_ODBCA MADB_DATA_TYPE_NUMERIC MADB_SQL_DATA_TYPE MADB_PROCEDURE_COLUMNS_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_PROCEDURE_COLUMNS_SQL_DATA_TYPE_ODBCU MADB_DATA_TYPE_NUMERIC MADB_SQL_DATA_TYPE MADB_PROCEDURE_COLUMNS_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY

#define MADB_PROCEDURE_COLUMNS_ODBC3U MADB_PROCEDURE_COLUMNSp1 MADB_PROCEDURE_COLUMNS_DATA_TYPE_ODBC3U MADB_PROCEDURE_COLUMNSp2 MADB_PROCEDURE_COLUMNS_SQL_DATA_TYPE_ODBCU MADB_PROCEDURE_COLUMNSp3
#define MADB_PROCEDURE_COLUMNS_ODBC2U MADB_PROCEDURE_COLUMNSp1 MADB_PROCEDURE_COLUMNS_DATA_TYPE_ODBC2U MADB_PROCEDURE_COLUMNSp2 MADB_PROCEDURE_COLUMNS_SQL_DATA_TYPE_ODBCU MADB_PROCEDURE_COLUMNSp3
#define MADB_PROCEDURE_COLUMNS_ODBC3A MADB_PROCEDURE_COLUMNSp1 MADB_PROCEDURE_COLUMNS_DATA_TYPE_ODBC3A MADB_PROCEDURE_COLUMNSp2 MADB_PROCEDURE_COLUMNS_SQL_DATA_TYPE_ODBCA MADB_PROCEDURE_COLUMNSp3
#define MADB_PROCEDURE_COLUMNS_ODBC2A MADB_PROCEDURE_COLUMNSp1 MADB_PROCEDURE_COLUMNS_DATA_TYPE_ODBC2A MADB_PROCEDURE_COLUMNSp2 MADB_PROCEDURE_COLUMNS_SQL_DATA_TYPE_ODBCA MADB_PROCEDURE_COLUMNSp3

#define MADB_PROCEDURE_COLUMNS(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ?\
        (StmtHndl->Connection->IsAnsi ? MADB_PROCEDURE_COLUMNS_ODBC3A : MADB_PROCEDURE_COLUMNS_ODBC3U) : \
        (StmtHndl->Connection->IsAnsi ? MADB_PROCEDURE_COLUMNS_ODBC2A : MADB_PROCEDURE_COLUMNS_ODBC2U))

/********* End of SQLProcedureColumns *********/

/************** SQLSpecialColumns *************/

#define MADB_SPECIAL_COLUMNSp1 "SELECT NULL AS SCOPE, COLUMN_NAME, "
#define MADB_SPECIAL_COLUMNSp2 \
    " AS DATA_TYPE, " MADB_COLUMNS_TYPE_NAME " AS TYPE_NAME, " MADB_COLUMNS_COLUMN_SIZE " AS COLUMN_SIZE, " \
    MADB_COLUMNS_BUFFER_LEN " AS BUFFER_LENGTH, " MADB_COLUMNS_DECIMAL_DIGITS " AS DECIMAL_DIGITS, " \
    XSTR(SQL_PC_UNKNOWN) " AS PSEUDO_COLUMN FROM INFORMATION_SCHEMA.COLUMNS"

#define MADB_SPECIAL_COLUMNS_ODBC3U MADB_SPECIAL_COLUMNSp1 MADB_DATA_TYPE_ODBC3U MADB_SPECIAL_COLUMNSp2
#define MADB_SPECIAL_COLUMNS_ODBC3A MADB_SPECIAL_COLUMNSp1 MADB_DATA_TYPE_ODBC3A MADB_SPECIAL_COLUMNSp2
#define MADB_SPECIAL_COLUMNS_ODBC2U MADB_SPECIAL_COLUMNSp1 MADB_DATA_TYPE_ODBC2U MADB_SPECIAL_COLUMNSp2
#define MADB_SPECIAL_COLUMNS_ODBC2A MADB_SPECIAL_COLUMNSp1 MADB_DATA_TYPE_ODBC2A MADB_SPECIAL_COLUMNSp2

#define MADB_SPECIAL_COLUMNS(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ?\
        (StmtHndl->Connection->IsAnsi ? MADB_SPECIAL_COLUMNS_ODBC3A : MADB_SPECIAL_COLUMNS_ODBC3U) : \
        (StmtHndl->Connection->IsAnsi ? MADB_SPECIAL_COLUMNS_ODBC2A : MADB_SPECIAL_COLUMNS_ODBC2U))

/********** End of SQLSpecialColumns *********/

#endif  // _ma_statement_h_
