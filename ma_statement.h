/************************************************************************************
   Copyright (C) 2013, 2019 MariaDB Corporation AB
   
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

#define MADB_FETCH_TYPE_FETCH       1
#define MADB_FETCH_TYPE_BULK        2
#define MADB_FETCH_TYPE_SETPOS      3


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
SQLUSMALLINT MapColAttributeDescType(SQLUSMALLINT FieldIdentifier);
MYSQL_RES*   FetchMetadata          (MADB_Stmt *Stmt);
SQLRETURN    MADB_DoExecute(MADB_Stmt *Stmt, BOOL ExecDirect);

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

#define MADB_OCTETS_PER_CHAR 2

#define MADB_TRANSFER_OCTET_LENGTH(TYPE_DEF_COL_NAME)\
  "@tol:=CAST(CASE @dt"\
  "  WHEN " XSTR(SQL_BIT) " THEN 1 "\
  "  WHEN " XSTR(SQL_TINYINT) " THEN 1 "\
  "  WHEN " XSTR(SQL_SMALLINT) " THEN 2 "\
  "  WHEN " XSTR(SQL_INTEGER) " THEN IF(DATA_TYPE='mediumint',3,4) "\
  "  WHEN " XSTR(SQL_BIGINT) " THEN 20 "\
  "  WHEN " XSTR(SQL_REAL) " THEN 4 "\
  "  WHEN " XSTR(SQL_FLOAT) " THEN 8 "\
  "  WHEN " XSTR(SQL_DOUBLE) " THEN 8 "\
  "  WHEN " XSTR(SQL_DATE) " THEN 6 "\
  "  WHEN " XSTR(SQL_TYPE_DATE) " THEN 6 "\
  "  WHEN " XSTR(SQL_TIME) " THEN 6 "\
  "  WHEN " XSTR(SQL_TYPE_TIME) " THEN 6 "\
  "  WHEN " XSTR(SQL_TIMESTAMP) " THEN 16 "\
  "  WHEN " XSTR(SQL_TYPE_TIMESTAMP) " THEN 16 "\
  "  WHEN " XSTR(SQL_GUID) " THEN 16 "\
  "  WHEN " XSTR(SQL_DECIMAL) " THEN @ColSize + IF(" #TYPE_DEF_COL_NAME " LIKE '%%unsigned',1,2) "\
  "  WHEN " XSTR(SQL_BINARY) " THEN IF(DATA_TYPE='bit', CAST(((NUMERIC_PRECISION + 7) / 8) AS SIGNED), CHARACTER_OCTET_LENGTH) "\
  "  WHEN " XSTR(SQL_VARBINARY) " THEN CHARACTER_OCTET_LENGTH "\
  "  WHEN " XSTR(SQL_LONGVARBINARY) " THEN CHARACTER_OCTET_LENGTH "\
  "  ELSE CHARACTER_MAXIMUM_LENGTH*%u "\
  "END AS SIGNED)"

/* CASE for DATA_TYPE glued in 2 parts for ODBC v2 or v3 */
#define MADB_SQL_DATATYPEp1\
  "CASE DATA_TYPE"\
  "  WHEN 'bit' THEN @dt:= IF(NUMERIC_PRECISION=1," XSTR(SQL_BIT) ", " XSTR(SQL_BINARY) ")"\
  "  WHEN 'tinyint' THEN @dt:=" XSTR(SQL_TINYINT)\
  "  WHEN 'smallint' THEN @dt:=" XSTR(SQL_SMALLINT)\
  "  WHEN 'year' THEN @dt:= " XSTR(SQL_SMALLINT)\
  "  WHEN 'mediumint' THEN @dt:=" XSTR(SQL_INTEGER)\
  "  WHEN 'int' THEN @dt:=" XSTR(SQL_INTEGER)\
  "  WHEN 'bigint' THEN @dt:=" XSTR(SQL_BIGINT)\
  "  WHEN 'blob' THEN @dt:=" XSTR(SQL_LONGVARBINARY)\
  "  WHEN 'tinyblob' THEN @dt:=" XSTR(SQL_LONGVARBINARY)\
  "  WHEN 'mediumblob' THEN @dt:=" XSTR(SQL_LONGVARBINARY)\
  "  WHEN 'longblob' THEN @dt:=" XSTR(SQL_LONGVARBINARY)\
  "  WHEN 'decimal' THEN @dt:=" XSTR(SQL_DECIMAL)\
  "  WHEN 'float' THEN @dt:=IF(NUMERIC_SCALE IS NULL," XSTR(SQL_REAL) ", "  XSTR(SQL_DECIMAL) ")"\
  "  WHEN 'double' THEN @dt:=IF(NUMERIC_SCALE IS NULL," XSTR(SQL_DOUBLE) ", "  XSTR(SQL_DECIMAL) ")"\
  "  WHEN 'binary' THEN @dt:=" XSTR(SQL_BINARY)\
  "  WHEN 'varbinary' THEN @dt:=" XSTR(SQL_VARBINARY)

#define MADB_SQL_DATATYPEp1U\
  "  WHEN 'text' THEN @dt:=" XSTR(SQL_WLONGVARCHAR)\
  "  WHEN 'tinytext' THEN @dt:=" XSTR(SQL_WLONGVARCHAR)\
  "  WHEN 'mediumtext' THEN @dt:=" XSTR(SQL_WLONGVARCHAR)\
  "  WHEN 'longtext' THEN @dt:=" XSTR(SQL_WLONGVARCHAR)\
  "  WHEN 'char' THEN @dt:=" XSTR(SQL_WCHAR)\
  "  WHEN 'enum' THEN @dt:=" XSTR(SQL_WCHAR)\
  "  WHEN 'set' THEN @dt:=" XSTR(SQL_WCHAR)\
  "  WHEN 'varchar' THEN @dt:=" XSTR(SQL_WVARCHAR)

#define MADB_SQL_DATATYPEp1A\
  "  WHEN 'text' THEN @dt:=" XSTR(SQL_LONGVARCHAR)\
  "  WHEN 'tinytext' THEN @dt:=" XSTR(SQL_LONGVARCHAR)\
  "  WHEN 'mediumtext' THEN @dt:=" XSTR(SQL_LONGVARCHAR)\
  "  WHEN 'longtext' THEN @dt:=" XSTR(SQL_LONGVARCHAR)\
  "  WHEN 'char' THEN @dt:=" XSTR(SQL_CHAR)\
  "  WHEN 'enum' THEN @dt:=" XSTR(SQL_CHAR)\
  "  WHEN 'set' THEN @dt:=" XSTR(SQL_CHAR)\
  "  WHEN 'varchar' THEN @dt:=" XSTR(SQL_VARCHAR)

#define MADB_SQL_DATATYPEp2_ODBC3\
  "  WHEN 'date' THEN @dt:=" XSTR(SQL_TYPE_DATE)\
  "  WHEN 'time' THEN @dt:=" XSTR(SQL_TYPE_TIME)\
  "  WHEN 'datetime' THEN @dt:=" XSTR(SQL_TYPE_TIMESTAMP)\
  "  WHEN 'timestamp' THEN @dt:=" XSTR(SQL_TYPE_TIMESTAMP)\
  "  ELSE @dt:=" XSTR(SQL_LONGVARBINARY)\
  "END AS DATA_TYPE"

#define MADB_SQL_DATATYPEp2_ODBC2\
  "  WHEN 'date' THEN @dt:=" XSTR(SQL_DATE)\
  "  WHEN 'time' THEN @dt:=" XSTR(SQL_TIME)\
  "  WHEN 'datetime' THEN @dt:=" XSTR(SQL_TIMESTAMP)\
  "  WHEN 'timestamp' THEN @dt:=" XSTR(SQL_TIMESTAMP)\
  "  ELSE @dt:=" XSTR(SQL_LONGVARBINARY)\
  "END AS DATA_TYPE"

#define MADB_SQL_DATATYPE_ODBC3U MADB_SQL_DATATYPEp1 MADB_SQL_DATATYPEp1U MADB_SQL_DATATYPEp2_ODBC3
#define MADB_SQL_DATATYPE_ODBC3A MADB_SQL_DATATYPEp1 MADB_SQL_DATATYPEp1A MADB_SQL_DATATYPEp2_ODBC3
#define MADB_SQL_DATATYPE_ODBC2U MADB_SQL_DATATYPEp1 MADB_SQL_DATATYPEp1U MADB_SQL_DATATYPEp2_ODBC2
#define MADB_SQL_DATATYPE_ODBC2A MADB_SQL_DATATYPEp1 MADB_SQL_DATATYPEp1A MADB_SQL_DATATYPEp2_ODBC2

#define MADB_SQL_DATATYPE(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ?\
 (StmtHndl->Connection->IsAnsi ? MADB_SQL_DATATYPE_ODBC3A : MADB_SQL_DATATYPE_ODBC3U) :\
 (StmtHndl->Connection->IsAnsi ? MADB_SQL_DATATYPE_ODBC2A : MADB_SQL_DATATYPE_ODBC2U))

/************** End of DATA_TYPE *************/

/************** SQLColumns       *************/
#define MADB_COLUMN_SIZE\
    "CAST(CASE" \
    "  WHEN DATA_TYPE = 'bit' THEN @ColSize:=((NUMERIC_PRECISION + 7) / 8) "\
    "  WHEN DATA_TYPE in ('tinyint', 'smallint', 'mediumint', 'int',"\
                         "'bigint', 'decimal') THEN @ColSize:=NUMERIC_PRECISION "\
    "  WHEN DATA_TYPE = 'float' THEN if(NUMERIC_SCALE IS NULL, @ColSize:=7, @ColSize:=NUMERIC_PRECISION)"\
    "  WHEN DATA_TYPE = 'double' THEN if(NUMERIC_SCALE IS NULL, @ColSize:=15, @ColSize:=NUMERIC_PRECISION)"\
    "  WHEN DATA_TYPE = 'date' THEN @ColSize:=10"\
    "  WHEN DATA_TYPE = 'time' THEN @ColSize:=8"\
    "  WHEN DATA_TYPE = 'year' THEN @ColSize:=4"\
    "  WHEN DATA_TYPE in ('timestamp', 'datetime') THEN @ColSize:=19 "\
    "  ELSE @ColSize:=CHARACTER_MAXIMUM_LENGTH "\
  "END AS UNSIGNED)"

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

#define MADB_SQL_DATA_TYPE_ODBC3 \
    " WHEN 'date' THEN " XSTR(SQL_DATETIME) \
    " WHEN 'time' THEN " XSTR(SQL_DATETIME) \
    " WHEN 'datetime' THEN " XSTR(SQL_DATETIME) \
    " WHEN 'timestamp' THEN " XSTR(SQL_DATETIME)

#define MADB_DATA_TYPE_UNICODE \
    " WHEN 'char' THEN " XSTR(SQL_WCHAR) \
    " WHEN 'varchar' THEN " XSTR(SQL_WVARCHAR) \
    " WHEN 'longtext' THEN " XSTR(SQL_WLONGVARCHAR) \
    " WHEN 'mediumtext' THEN " XSTR(SQL_WLONGVARCHAR) \
    " WHEN 'text' THEN " XSTR(SQL_WVARCHAR) \
    " WHEN 'tinytext' THEN " XSTR(SQL_WVARCHAR)

#define MADB_DATA_TYPE_ANSI \
    " WHEN 'char' THEN " XSTR(SQL_CHAR) \
    " WHEN 'varchar' THEN " XSTR(SQL_VARCHAR) \
    " WHEN 'longtext' THEN " XSTR(SQL_LONGVARCHAR) \
    " WHEN 'mediumtext' THEN " XSTR(SQL_LONGVARCHAR) \
    " WHEN 'text' THEN " XSTR(SQL_VARCHAR) \
    " WHEN 'tinytext' THEN " XSTR(SQL_VARCHAR) \

#define MADB_DATA_TYPE_NUMERIC \
    "CASE DATA_TYPE WHEN 'bit' THEN " XSTR(SQL_BIT) \
    " WHEN 'tinyint' THEN " XSTR(SQL_TINYINT) \
    " WHEN 'smallint' THEN " XSTR(SQL_SMALLINT) \
    " WHEN 'mediumint' THEN " XSTR(SQL_INTEGER) \
    " WHEN 'int' THEN " XSTR(SQL_INTEGER) \
    " WHEN 'bigint' THEN " XSTR(SQL_BIGINT) \
    " WHEN 'double' THEN " XSTR(SQL_DOUBLE) \
    " WHEN 'float' THEN " XSTR(SQL_FLOAT) \
    " WHEN 'decimal' THEN " XSTR(SQL_DECIMAL) \
    " WHEN 'year' THEN " XSTR(SQL_SMALLINT)

#define MADB_DATA_TYPE_BINARY \
    " WHEN 'binary' THEN " XSTR(SQL_BINARY) \
    " WHEN 'varbinary' THEN " XSTR(SQL_VARBINARY) \
    " WHEN 'longblob' THEN " XSTR(SQL_LONGVARBINARY) \
    " WHEN 'mediumblob' THEN " XSTR(SQL_LONGVARBINARY) \
    " WHEN 'blob' THEN " XSTR(SQL_VARBINARY) \
    " WHEN 'tinyblob' THEN " XSTR(SQL_VARBINARY) \
    " WHEN 'json' THEN " XSTR(SQL_LONGVARBINARY) \
    " WHEN 'geography' THEN " XSTR(SQL_VARBINARY) \
    " WHEN 'geographypoint' THEN " XSTR(SQL_VARBINARY) \
    " WHEN 'enum' THEN " XSTR(SQL_BINARY) \
    " WHEN 'set' THEN " XSTR(SQL_BINARY) " END :> SMALLINT"

#define MADB_DATA_TYPE_ODBC3A MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC3 MADB_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_DATA_TYPE_ODBC3U MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC3 MADB_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY
#define MADB_DATA_TYPE_ODBC2A MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC2 MADB_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_DATA_TYPE_ODBC2U MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC2 MADB_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY

#define MADB_COLUMNS_COLUMN_SIZE "CASE WHEN DATA_TYPE = 'bit' THEN ((NUMERIC_PRECISION + 7) / 8)"\
    " WHEN DATA_TYPE IN ('tinyint', 'smallint', 'mediumint', 'int', 'bigint', 'decimal') THEN NUMERIC_PRECISION" \
    " WHEN DATA_TYPE = 'double' THEN IF(NUMERIC_SCALE IS NULL, 15, NUMERIC_PRECISION)" \
    " WHEN DATA_TYPE = 'float' THEN IF(NUMERIC_SCALE IS NULL, 7, NUMERIC_PRECISION)" \
    " WHEN DATA_TYPE = 'date' THEN 10" \
    " WHEN DATA_TYPE = 'time' THEN 8" \
    " WHEN DATA_TYPE = 'datetime' THEN IF(COLUMN_TYPE = 'datetime(6)', 26, 19)" \
    " WHEN DATA_TYPE = 'timestamp' THEN IF(COLUMN_TYPE = 'timestamp(6)', 26, 19)" \
    " WHEN DATA_TYPE = 'year' THEN 4" \
    " ELSE CHARACTER_MAXIMUM_LENGTH END :> INT"

#define MADB_COLUMNS_BUFFER_LEN "CASE DATA_TYPE WHEN 'bit' THEN ((NUMERIC_PRECISION + 7) / 8)"\
    " WHEN 'tinyint' THEN 1" \
    " WHEN 'smallint' THEN 2" \
    " WHEN 'mediumint' THEN 3" \
    " WHEN 'int' THEN 4" \
    " WHEN 'bigint' THEN 8" \
    " WHEN 'double' THEN 8"\
    " WHEN 'float' THEN 4"\
    " WHEN 'decimal' THEN NUMERIC_PRECISION" \
    " WHEN 'date' THEN 10" \
    " WHEN 'time' THEN 8" \
    " WHEN 'datetime' THEN IF(COLUMN_TYPE = 'datetime(6)', 26, 19)" \
    " WHEN 'timestamp' THEN IF(COLUMN_TYPE = 'timestamp(6)', 26, 19)" \
    " WHEN 'year' THEN 2" \
    " ELSE CHARACTER_OCTET_LENGTH END :> INT"

#define MADB_COLUMNS_DECIMAL_DIGITS "CASE DATA_TYPE WHEN 'decimal' THEN NUMERIC_SCALE" \
    " WHEN 'double' THEN IF(NUMERIC_SCALE IS NULL, 6, NUMERIC_SCALE)" \
    " WHEN 'float' THEN IF(NUMERIC_SCALE IS NULL, 0, NUMERIC_SCALE)" \
    " WHEN 'date' THEN 0" \
    " WHEN 'time' THEN 0" \
    " WHEN 'datetime' THEN IF(COLUMN_TYPE = 'datetime(6)', 6, 0)" \
    " WHEN 'timestamp' THEN IF(COLUMN_TYPE = 'timestamp(6)', 6, 0) END :> SMALLINT"

#define MADB_COLUMNS_NUM_PREC_RADIX \
    "CASE WHEN DATA_TYPE IN ('tinyint', 'smallint', 'mediumint', 'int', 'bigint', 'double', 'float', 'decimal', 'year') THEN 10 END :> SMALLINT"

#define MADB_COLUMNS_NULLABLE "CASE WHEN IS_NULLABLE = 'YES' THEN " XSTR(SQL_NULLABLE) \
    " ELSE " XSTR(SQL_NO_NULLS) " END :> SMALLINT"

#define MADB_SQL_DATA_TYPE_ODBC3A MADB_DATA_TYPE_NUMERIC MADB_SQL_DATA_TYPE_ODBC3 MADB_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_SQL_DATA_TYPE_ODBC3U MADB_DATA_TYPE_NUMERIC MADB_SQL_DATA_TYPE_ODBC3 MADB_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY
#define MADB_SQL_DATA_TYPE_ODBC2A MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC2 MADB_DATA_TYPE_ANSI MADB_DATA_TYPE_BINARY
#define MADB_SQL_DATA_TYPE_ODBC2U MADB_DATA_TYPE_NUMERIC MADB_DATA_TYPE_ODBC2 MADB_DATA_TYPE_UNICODE MADB_DATA_TYPE_BINARY

#define MADB_COLUMNS_DATETIME_SUB "CASE WHEN DATA_TYPE = 'date' THEN " XSTR(SQL_CODE_DATE) \
    " WHEN DATA_TYPE = 'time' THEN " XSTR(SQL_CODE_TIME) \
    " WHEN DATA_TYPE = 'datetime' THEN " XSTR(SQL_CODE_TIMESTAMP) \
    " WHEN DATA_TYPE = 'timestamp' THEN " XSTR(SQL_CODE_TIMESTAMP) " END :> SMALLINT"

#define MADB_COLUMNSp1 \
    "SELECT TABLE_SCHEMA AS TABLE_CAT, NULL AS TABLE_SCHEM, TABLE_NAME, COLUMN_NAME, "
#define MADB_COLUMNSp2 \
    " AS DATA_TYPE, DATA_TYPE AS TYPE_NAME, " MADB_COLUMNS_COLUMN_SIZE " AS COLUMN_SIZE, " \
    MADB_COLUMNS_BUFFER_LEN " AS BUFFER_LENGTH, " MADB_COLUMNS_DECIMAL_DIGITS " AS DECIMAL_DIGITS, " \
    MADB_COLUMNS_NUM_PREC_RADIX " AS NUM_PREC_RADIX, " MADB_COLUMNS_NULLABLE " AS NULLABLE, NULL AS REMARKS, " \
    "COLUMN_DEFAULT AS COLUMN_DEF, "
#define MADB_COLUMNSp3 \
    " AS SQL_DATA_TYPE, " MADB_COLUMNS_DATETIME_SUB " AS SQL_DATETIME_SUB, " \
    "CHARACTER_OCTET_LENGTH :> INT AS CHAR_OCTET_LENGTH, ORDINAL_POSITION, IS_NULLABLE FROM INFORMATION_SCHEMA.COLUMNS"

#define MADB_COLUMNS_ODBC3U MADB_COLUMNSp1 MADB_DATA_TYPE_ODBC3U MADB_COLUMNSp2 MADB_SQL_DATA_TYPE_ODBC3U MADB_COLUMNSp3
#define MADB_COLUMNS_ODBC2U MADB_COLUMNSp1 MADB_DATA_TYPE_ODBC2U MADB_COLUMNSp2 MADB_SQL_DATA_TYPE_ODBC2U MADB_COLUMNSp3
#define MADB_COLUMNS_ODBC3A MADB_COLUMNSp1 MADB_DATA_TYPE_ODBC3A MADB_COLUMNSp2 MADB_SQL_DATA_TYPE_ODBC3A MADB_COLUMNSp3
#define MADB_COLUMNS_ODBC2A MADB_COLUMNSp1 MADB_DATA_TYPE_ODBC2A MADB_COLUMNSp2 MADB_SQL_DATA_TYPE_ODBC2A MADB_COLUMNSp3

#define MADB_COLUMNS(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ?\
        (StmtHndl->Connection->IsAnsi ? MADB_COLUMNS_ODBC3A : MADB_COLUMNS_ODBC3U) : \
        (StmtHndl->Connection->IsAnsi ? MADB_COLUMNS_ODBC2A : MADB_COLUMNS_ODBC2U))

#define MADB_DEFAULT_COLUMN_OLD  "IF(COLLATION_NAME IS NOT NULL AND COLUMN_DEFAULT IS NOT NULL, CONCAT(CHAR(39), COLUMN_DEFAULT, CHAR(39)), COLUMN_DEFAULT)"
#define MADB_DEFAULT_COLUMN_NEW  "COLUMN_DEFAULT"
#define MADB_DEFAULT_COLUMN(DbcHndl) (MADB_ServerSupports(DbcHndl,MADB_ENCLOSES_COLUMN_DEF_WITH_QUOTES) ? MADB_DEFAULT_COLUMN_NEW : MADB_DEFAULT_COLUMN_OLD)
#define MADB_CATALOG_TYPE_SUB "CAST(CASE"\
  "  WHEN DATA_TYPE = 'date' THEN " XSTR(SQL_DATETIME)\
  "  WHEN DATA_TYPE = 'time' THEN " XSTR(SQL_DATETIME)\
  "  WHEN DATA_TYPE = 'datetime' THEN " XSTR(SQL_DATETIME)\
  "  WHEN DATA_TYPE = 'timestamp' THEN " XSTR(SQL_DATETIME)\
  " ELSE @dt "\
  "END AS SIGNED) SQL_DATA_TYPE,"\
  "CAST(CASE"\
  "  WHEN DATA_TYPE = 'date' THEN " XSTR(SQL_CODE_DATE)\
  "  WHEN DATA_TYPE = 'time' THEN " XSTR(SQL_CODE_TIME)\
  "  WHEN DATA_TYPE = 'datetime' THEN " XSTR(SQL_CODE_TIMESTAMP)\
  "  WHEN DATA_TYPE = 'timestamp' THEN " XSTR(SQL_CODE_TIMESTAMP)\
  " ELSE NULL "\
  "END AS SIGNED) SQL_DATETIME_SUB,"

/************** End of SQLColumns ************/

/************** SQLProcedureColumns **********/
#define MADB_PROCEDURE_COLUMNSp1 \
  "SELECT SPECIFIC_SCHEMA AS PROCEDURE_CAT, NULL AS PROCEDURE_SCHEM, "\
  "SPECIFIC_NAME PROCEDURE_NAME, IF(PARAMETER_NAME IS NULL, '', PARAMETER_NAME) COLUMN_NAME, "\
  "CASE PARAMETER_MODE "\
  "  WHEN 'IN' THEN " XSTR(SQL_PARAM_INPUT)\
  "  WHEN 'OUT' THEN " XSTR(SQL_PARAM_OUTPUT)\
  "  WHEN 'INOUT' THEN " XSTR(SQL_PARAM_INPUT_OUTPUT)\
  "  ELSE IF(PARAMETER_MODE IS NULL, " XSTR(SQL_RETURN_VALUE) ", " XSTR(SQL_PARAM_TYPE_UNKNOWN) ")"\
  "END COLUMN_TYPE, "
#define MADB_PROCEDURE_COLUMNSp3\
  ", DATA_TYPE TYPE_NAME, "\
  MADB_COLUMN_SIZE " AS COLUMN_SIZE, "\
  MADB_TRANSFER_OCTET_LENGTH(DTD_IDENTIFIER) " AS BUFFER_LENGTH, "\
  "NUMERIC_SCALE DECIMAL_DIGITS, IF(NUMERIC_PRECISION IS NULL, NULL, 10) AS NUM_PREC_RADIX,"\
  XSTR(SQL_NULLABLE_UNKNOWN) " NULLABLE,"\
  "NULL REMARKS, NULL COLUMN_DEF," MADB_CATALOG_TYPE_SUB \
  "IF(CHARACTER_MAXIMUM_LENGTH IS NULL, NULL, @tol) CHAR_OCTET_LENGTH, "\
  "ORDINAL_POSITION, 'YES' IS_NULLABLE FROM INFORMATION_SCHEMA.PARAMETERS "

#define MADB_PROCEDURE_COLUMNS_ODBC3U MADB_PROCEDURE_COLUMNSp1 MADB_SQL_DATATYPE_ODBC3U MADB_PROCEDURE_COLUMNSp3
#define MADB_PROCEDURE_COLUMNS_ODBC2U MADB_PROCEDURE_COLUMNSp1 MADB_SQL_DATATYPE_ODBC2U MADB_PROCEDURE_COLUMNSp3
#define MADB_PROCEDURE_COLUMNS_ODBC3A MADB_PROCEDURE_COLUMNSp1 MADB_SQL_DATATYPE_ODBC3A MADB_PROCEDURE_COLUMNSp3
#define MADB_PROCEDURE_COLUMNS_ODBC2A MADB_PROCEDURE_COLUMNSp1 MADB_SQL_DATATYPE_ODBC2A MADB_PROCEDURE_COLUMNSp3

#define MADB_PROCEDURE_COLUMNS(StmtHndl) (StmtHndl->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3 ?\
                                        (StmtHndl->Connection->IsAnsi ? MADB_PROCEDURE_COLUMNS_ODBC3A : MADB_PROCEDURE_COLUMNS_ODBC3U) : \
                                        (StmtHndl->Connection->IsAnsi ? MADB_PROCEDURE_COLUMNS_ODBC2A : MADB_PROCEDURE_COLUMNS_ODBC2U))
/************** SQLProcedureColumns **********/

#endif
