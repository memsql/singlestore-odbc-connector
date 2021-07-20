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

#undef MA_ODBC_DEBUG_ALL

#include <ma_odbc.h>
#include <ma_parse.h>
#include <odbc_3_api.h>

//#define TRACE_ANSI_ON
#ifdef TRACE_ANSI_ON
#define TRACE_ANSI printf("%s\n", __PRETTY_FUNCTION__);
#define TRACE_ANSI_INT(val) printf("%s %d\n", __PRETTY_FUNCTION__, val);
#else
#define TRACE_ANSI
#define TRACE_ANSI_INT(val)
#endif // TRACE_ANSI_ON

/* {{{ SQLColAttribute */
SQLRETURN SQL_API SQLColAttribute (SQLHSTMT StatementHandle,
                                   SQLUSMALLINT ColumnNumber,
                                   SQLUSMALLINT FieldIdentifier,
                                   SQLPOINTER CharacterAttributePtr,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT *StringLengthPtr,
#ifdef SQLCOLATTRIB_SQLPOINTER
        SQLPOINTER NumericAttributePtr
#else
                                   SQLLEN *NumericAttributePtr
#endif
)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLColAttribute");
    MDBUG_C_DUMP(Stmt->Connection, StatementHandle, 0x);
    MDBUG_C_DUMP(Stmt->Connection, ColumnNumber, u);
    MDBUG_C_DUMP(Stmt->Connection, FieldIdentifier, u);
    MDBUG_C_DUMP(Stmt->Connection, CharacterAttributePtr, 0x);
    MDBUG_C_DUMP(Stmt->Connection, BufferLength, d);
    MDBUG_C_DUMP(Stmt->Connection, StringLengthPtr, 0x);
    MDBUG_C_DUMP(Stmt->Connection, NumericAttributePtr, 0x);

    ret= Stmt->Methods->ColAttribute(Stmt, ColumnNumber, FieldIdentifier, CharacterAttributePtr,
                                     BufferLength, StringLengthPtr, NumericAttributePtr, FALSE);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLColAttributes */
SQLRETURN SQL_API SQLColAttributes(SQLHSTMT hstmt,
                                   SQLUSMALLINT icol,
                                   SQLUSMALLINT fDescType,
                                   SQLPOINTER rgbDesc,
                                   SQLSMALLINT cbDescMax,
                                   SQLSMALLINT * pcbDesc,
                                   SQLLEN * pfDesc)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)hstmt;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->ColAttribute(Stmt, icol, MapColAttributeDescType(fDescType), rgbDesc,
                                       cbDescMax, pcbDesc, pfDesc, FALSE);
}
/* }}} */

/* {{{ SQLColumnPrivileges */
SQLRETURN SQL_API SQLColumnPrivileges(SQLHSTMT StatementHandle,
                                      SQLCHAR *CatalogName,
                                      SQLSMALLINT NameLength1,
                                      SQLCHAR *SchemaName,
                                      SQLSMALLINT NameLength2,
                                      SQLCHAR *TableName,
                                      SQLSMALLINT NameLength3,
                                      SQLCHAR *ColumnName,
                                      SQLSMALLINT NameLength4)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLColumnPrivileges");
    ret= Stmt->Methods->ColumnPrivileges(Stmt, (char *)CatalogName, NameLength1, (char *)SchemaName, NameLength2,
                                         (char *)TableName, NameLength3, (char *)ColumnName, NameLength4);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLColumns */
SQLRETURN SQL_API SQLColumns(SQLHSTMT StatementHandle,
                             SQLCHAR *CatalogName,
                             SQLSMALLINT NameLength1,
                             SQLCHAR *SchemaName,
                             SQLSMALLINT NameLength2,
                             SQLCHAR *TableName,
                             SQLSMALLINT NameLength3,
                             SQLCHAR *ColumnName,
                             SQLSMALLINT NameLength4)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLColumns");

    ret= Stmt->Methods->Columns(Stmt, (char *)CatalogName,NameLength1, (char *)SchemaName, NameLength2,
                                (char *)TableName, NameLength3, (char *)ColumnName, NameLength4);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLConnect */
SQLRETURN SQL_API SQLConnect(SQLHDBC ConnectionHandle,
                             SQLCHAR *ServerName,
                             SQLSMALLINT NameLength1,
                             SQLCHAR *UserName,
                             SQLSMALLINT NameLength2,
                             SQLCHAR *Authentication,
                             SQLSMALLINT NameLength3)
{
    MADB_Dbc *Dbc= (MADB_Dbc*)ConnectionHandle;

    TRACE_ANSI

    if (!Dbc)
        return SQL_INVALID_HANDLE;

    Dbc->IsAnsi = 1;

    return SQLConnectCommon(ConnectionHandle, ServerName, NameLength1,
                            UserName, NameLength2, Authentication, NameLength3);
}
/* }}} */

/* {{{ SQLDataSources */
SQLRETURN SQL_API SQLDataSources(SQLHENV EnvironmentHandle,
                                 SQLUSMALLINT Direction,
                                 SQLCHAR *ServerName,
                                 SQLSMALLINT BufferLength1,
                                 SQLSMALLINT *NameLength1Ptr,
                                 SQLCHAR *Description,
                                 SQLSMALLINT BufferLength2,
                                 SQLSMALLINT *NameLength2Ptr)
{
    SQLRETURN ret= SQL_ERROR;

    TRACE_ANSI

    return ret;
}
/* }}} */

/* {{{ SQLDescribeCol */
SQLRETURN SQL_API SQLDescribeCol(SQLHSTMT StatementHandle,
                                 SQLUSMALLINT ColumnNumber,
                                 SQLCHAR *ColumnName,
                                 SQLSMALLINT BufferLength,
                                 SQLSMALLINT *NameLengthPtr,
                                 SQLSMALLINT *DataTypePtr,
                                 SQLULEN *ColumnSizePtr,
                                 SQLSMALLINT *DecimalDigitsPtr,
                                 SQLSMALLINT *NullablePtr)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLDescribeCol");
    MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);
    MDBUG_C_DUMP(Stmt->Connection, ColumnNumber, u);

    ret= Stmt->Methods->DescribeCol(Stmt, ColumnNumber, (void *)ColumnName, BufferLength,
                                    NameLengthPtr, DataTypePtr, ColumnSizePtr, DecimalDigitsPtr,
                                    NullablePtr, FALSE);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLDriverConnect */
SQLRETURN SQL_API SQLDriverConnect(SQLHDBC ConnectionHandle,
                                   SQLHWND WindowHandle,
                                   SQLCHAR *InConnectionString,
                                   SQLSMALLINT StringLength1,
                                   SQLCHAR *OutConnectionString,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT *StringLength2Ptr,
                                   SQLUSMALLINT DriverCompletion)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
    SQLRETURN ret;

    TRACE_ANSI

    if (!Dbc)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Dbc->Error);

    Dbc->IsAnsi = 1;

    MDBUG_C_ENTER(Dbc, "SQLDriverConnect");
    MDBUG_C_DUMP(Dbc, Dbc, 0x);
    MDBUG_C_DUMP(Dbc, InConnectionString, s);
    MDBUG_C_DUMP(Dbc, StringLength1, d);
    MDBUG_C_DUMP(Dbc, OutConnectionString, 0x);
    MDBUG_C_DUMP(Dbc, BufferLength, d);
    MDBUG_C_DUMP(Dbc, StringLength2Ptr, 0x);
    MDBUG_C_DUMP(Dbc, DriverCompletion, d);
    ret= Dbc->Methods->DriverConnect(Dbc, WindowHandle, InConnectionString, StringLength1, OutConnectionString,
                                     BufferLength, StringLength2Ptr, DriverCompletion);

    MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLDrivers */
SQLRETURN SQL_API SQLDrivers(SQLHENV EnvironmentHandle,
                             SQLUSMALLINT Direction,
                             SQLCHAR *DriverDescription,
                             SQLSMALLINT BufferLength1,
                             SQLSMALLINT *DescriptionLengthPtr,
                             SQLCHAR *DriverAttributes,
                             SQLSMALLINT BufferLength2,
                             SQLSMALLINT *AttributesLengthPtr)
{
    SQLRETURN ret= SQL_ERROR;

    TRACE_ANSI

    return ret;
}
/* }}} */

/* {{{ SQLExecDirect */
SQLRETURN SQL_API SQLExecDirect(SQLHSTMT StatementHandle,
                                SQLCHAR *StatementText,
                                SQLINTEGER TextLength)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;

    TRACE_ANSI

    if (!Stmt)
        ret= SQL_INVALID_HANDLE;
    else
        ret= Stmt->Methods->ExecDirect(Stmt, (char *)StatementText, TextLength);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

SQLRETURN SQL_API SQLGetConnectAttr(SQLHDBC ConnectionHandle,
                                    SQLINTEGER Attribute,
                                    SQLPOINTER ValuePtr,
                                    SQLINTEGER BufferLength,
                                    SQLINTEGER *StringLengthPtr)
{
    TRACE_ANSI_INT(Attribute)

    if (ConnectionHandle == SQL_NULL_HDBC)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&((MADB_Dbc *)ConnectionHandle)->Error);

    return MA_SQLGetConnectAttr(ConnectionHandle, Attribute, ValuePtr, BufferLength, StringLengthPtr);
}
/* }}} */

/* {{{ SQLGetConnectOption */
SQLRETURN SQL_API SQLGetConnectOption(SQLHDBC ConnectionHandle, SQLUSMALLINT Option, SQLPOINTER ValuePtr)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;

    TRACE_ANSI_INT(Option)

    if (!Dbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Dbc->Error);

    return MA_SQLGetConnectAttr(ConnectionHandle, Option, ValuePtr,
                                Option == SQL_ATTR_CURRENT_CATALOG ? SQL_MAX_OPTION_STRING_LENGTH : 0, NULL);
}
/* }}} */

/* {{{ SQLGetCursorName */
SQLRETURN SQL_API SQLGetCursorName(
        SQLHSTMT        StatementHandle,
        SQLCHAR *       CursorName,
        SQLSMALLINT     BufferLength,
        SQLSMALLINT *   NameLengthPtr)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->GetCursorName(Stmt, CursorName, BufferLength, NameLengthPtr, FALSE);
}
/* }}} */

/* {{{ SQLGetDescField */
SQLRETURN SQL_API SQLGetDescField(SQLHDESC DescriptorHandle,
                                  SQLSMALLINT RecNumber,
                                  SQLSMALLINT FieldIdentifier,
                                  SQLPOINTER ValuePtr,
                                  SQLINTEGER BufferLength,
                                  SQLINTEGER *StringLengthPtr)
{
    TRACE_ANSI

    if (!DescriptorHandle)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&((MADB_Desc*)DescriptorHandle)->Error);

    return MADB_DescGetField(DescriptorHandle, RecNumber, FieldIdentifier, ValuePtr, BufferLength, StringLengthPtr, FALSE);
}
/* }}} */

/* {{{ SQLGetDescRec */
SQLRETURN SQL_API SQLGetDescRec(SQLHDESC DescriptorHandle,
                                SQLSMALLINT RecNumber,
                                SQLCHAR *Name,
                                SQLSMALLINT BufferLength,
                                SQLSMALLINT *StringLengthPtr,
                                SQLSMALLINT *TypePtr,
                                SQLSMALLINT *SubTypePtr,
                                SQLLEN *LengthPtr,
                                SQLSMALLINT *PrecisionPtr,
                                SQLSMALLINT *ScalePtr,
                                SQLSMALLINT *NullablePtr)
{
    MADB_Desc *Desc= (MADB_Desc *)DescriptorHandle;

    TRACE_ANSI

    if (!Desc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Desc->Error);

    return MADB_DescGetRec(Desc, RecNumber, Name, BufferLength, StringLengthPtr, TypePtr, SubTypePtr,
                           LengthPtr, PrecisionPtr, ScalePtr, NullablePtr, FALSE);
}
/* }}} */

/* {{{ SQLGetDiagField */
SQLRETURN SQL_API SQLGetDiagField(SQLSMALLINT HandleType,
                                  SQLHANDLE Handle,
                                  SQLSMALLINT RecNumber,
                                  SQLSMALLINT DiagIdentifier,
                                  SQLPOINTER DiagInfoPtr,
                                  SQLSMALLINT BufferLength,
                                  SQLSMALLINT *StringLengthPtr)
{
    TRACE_ANSI

    if (!Handle)
        return SQL_INVALID_HANDLE;
    return MADB_GetDiagField(HandleType, Handle, RecNumber, DiagIdentifier, DiagInfoPtr, BufferLength, StringLengthPtr, FALSE);
}
/* }}} */

SQLRETURN SQL_API SQLGetDiagRec(SQLSMALLINT HandleType,
                                SQLHANDLE Handle,
                                SQLSMALLINT RecNumber,
                                SQLCHAR *SQLState,
                                SQLINTEGER *NativeErrorPtr,
                                SQLCHAR *MessageText,
                                SQLSMALLINT BufferLength,
                                SQLSMALLINT *TextLengthPtr)
{
    TRACE_ANSI

    return MA_SQLGetDiagRec(HandleType, Handle, RecNumber, SQLState, NativeErrorPtr,
                            MessageText, BufferLength, TextLengthPtr);
}
/* }}} */

/* {{{ SQLGetInfo */
SQLRETURN SQL_API SQLGetInfo(SQLHDBC ConnectionHandle,
                             SQLUSMALLINT InfoType,
                             SQLPOINTER InfoValuePtr,
                             SQLSMALLINT BufferLength,
                             SQLSMALLINT *StringLengthPtr)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
    SQLRETURN ret;

    TRACE_ANSI_INT(InfoType)

    if (!Dbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Dbc->Error);

    MDBUG_C_ENTER(Dbc, "SQLGetInfo");
    MDBUG_C_DUMP(Dbc, InfoType, d);
    /* Linux DM overflows output buffer when SQLGetInfoW() is called with ANSI driver.
     * The DM thinks that BufferLength is in SQLWCHARs, not in bytes.
     */
    ret= Dbc->Methods->GetInfo(Dbc, InfoType, InfoValuePtr, BufferLength, StringLengthPtr, FALSE);

    MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

SQLRETURN SQL_API SQLGetStmtAttr(SQLHSTMT StatementHandle,
                                 SQLINTEGER Attribute,
                                 SQLPOINTER ValuePtr,
                                 SQLINTEGER BufferLength,
                                 SQLINTEGER *StringLengthPtr)
{
    TRACE_ANSI_INT(Attribute)

    if (StatementHandle == SQL_NULL_HSTMT)
    {
        return SQL_INVALID_HANDLE;
    }
    MADB_CLEAR_ERROR(&((MADB_Stmt*)StatementHandle)->Error);

    return MA_SQLGetStmtAttr(StatementHandle, Attribute, ValuePtr, BufferLength, StringLengthPtr);
}
/* }}} */

/* {{{ SQLGetTypeInfo */
SQLRETURN SQL_API SQLGetTypeInfo(SQLHSTMT StatementHandle,
                                 SQLSMALLINT DataType)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI_INT(DataType)

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return MADB_GetTypeInfo(Stmt, DataType);
}
/* }}} */

/* {{{ SQLNativeSql */
SQLRETURN SQL_API SQLNativeSql(SQLHDBC ConnectionHandle,
                               SQLCHAR *InStatementText,
                               SQLINTEGER TextLength1,
                               SQLCHAR *OutStatementText,
                               SQLINTEGER BufferLength,
                               SQLINTEGER *TextLength2Ptr)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
    SQLINTEGER Length;
    char *InStatementStart, *InStatementEnd;
    char **InStatementIterator;
    MADB_DynString res;

    TRACE_ANSI

    if (!Dbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Dbc->Error);

    if (!TextLength2Ptr && (!OutStatementText || !BufferLength))
    {
        MADB_SetError(&Dbc->Error, MADB_ERR_01004, NULL, 0);
        return Dbc->Error.ReturnValue;
    }

    InStatementStart = (char *)InStatementText;
    ADJUST_LENGTH(InStatementStart, TextLength1);
    InStatementEnd = InStatementStart + TextLength1;
    InStatementIterator = &InStatementStart;
    if (MADB_UnescapeQuery(Dbc, &Dbc->Error, &res, InStatementIterator, &InStatementEnd, 0)) {
        return Dbc->Error.ReturnValue;
    }

    Length= (SQLINTEGER)MADB_SetString(0, OutStatementText, BufferLength, (char *)res.str, res.length, &Dbc->Error);
    MADB_DynstrFree(&res);
    if (TextLength2Ptr)
        *TextLength2Ptr= Length;

    return Dbc->Error.ReturnValue;
}
/* }}} */

SQLRETURN SQL_API SQLPrepare(SQLHSTMT StatementHandle,
                             SQLCHAR *StatementText,
                             SQLINTEGER TextLength)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (StatementHandle == SQL_NULL_HSTMT)
    {
        return SQL_INVALID_HANDLE;
    }

    MDBUG_C_ENTER(Stmt->Connection, "SQLPrepare");

    MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);
    MDBUG_C_DUMP(Stmt->Connection, StatementText, s);
    MDBUG_C_DUMP(Stmt->Connection, TextLength, d);

    /* Prepare method clears error */

    return Stmt->Methods->Prepare(Stmt, (char *)StatementText, TextLength, FALSE);
}
/* }}} */

/* {{{ SQLPrimaryKeys */
SQLRETURN SQL_API SQLPrimaryKeys(SQLHSTMT StatementHandle,
                                 SQLCHAR *CatalogName,
                                 SQLSMALLINT NameLength1,
                                 SQLCHAR *SchemaName,
                                 SQLSMALLINT NameLength2,
                                 SQLCHAR *TableName,
                                 SQLSMALLINT NameLength3)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;

    TRACE_ANSI

    MDBUG_C_ENTER(Stmt->Connection, "SQLPrimaryKeys");
    MDBUG_C_DUMP(Stmt->Connection, StatementHandle, 0x);
    MDBUG_C_DUMP(Stmt->Connection, CatalogName, s);
    MDBUG_C_DUMP(Stmt->Connection, NameLength1, d);
    MDBUG_C_DUMP(Stmt->Connection, SchemaName, s);
    MDBUG_C_DUMP(Stmt->Connection, NameLength2, d);
    MDBUG_C_DUMP(Stmt->Connection, TableName, s);
    MDBUG_C_DUMP(Stmt->Connection, NameLength3, d);

    if (!Stmt)
        ret= SQL_INVALID_HANDLE;
    else
    {
        MADB_CLEAR_ERROR(&Stmt->Error);
        ret= Stmt->Methods->PrimaryKeys(Stmt, (char *)CatalogName, NameLength1, (char *)SchemaName, NameLength2,
                                        (char *)TableName, NameLength3);
    }

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLProcedureColumns */
SQLRETURN SQL_API SQLProcedureColumns(SQLHSTMT StatementHandle,
                                      SQLCHAR *CatalogName,
                                      SQLSMALLINT NameLength1,
                                      SQLCHAR *SchemaName,
                                      SQLSMALLINT NameLength2,
                                      SQLCHAR *ProcName,
                                      SQLSMALLINT NameLength3,
                                      SQLCHAR *ColumnName,
                                      SQLSMALLINT NameLength4)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->ProcedureColumns(Stmt, (char *)CatalogName,NameLength1, (char *)SchemaName, NameLength2,
                                           (char *)ProcName, NameLength3, (char *)ColumnName, NameLength4);

}
/* }}} */

/* {{{ SQLProcedures */
SQLRETURN SQL_API SQLProcedures(SQLHSTMT StatementHandle,
                                SQLCHAR *CatalogName,
                                SQLSMALLINT NameLength1,
                                SQLCHAR *SchemaName,
                                SQLSMALLINT NameLength2,
                                SQLCHAR *ProcName,
                                SQLSMALLINT NameLength3)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->Procedures(Stmt, (char *)CatalogName, NameLength1, (char *)SchemaName,
                                     NameLength2, (char *)ProcName, NameLength3);
}
/* }}} */

SQLRETURN SQL_API SQLSetConnectAttr(SQLHDBC ConnectionHandle,
                                    SQLINTEGER Attribute,
                                    SQLPOINTER ValuePtr,
                                    SQLINTEGER StringLength)
{
    TRACE_ANSI_INT(Attribute)

    if (ConnectionHandle == SQL_NULL_HDBC)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&((MADB_Dbc *)ConnectionHandle)->Error);

    return MA_SQLSetConnectAttr(ConnectionHandle, Attribute, ValuePtr, StringLength);
}

/* {{{ SQLSetConnectOption */
SQLRETURN SQL_API SQLSetConnectOption(SQLHDBC Hdbc, SQLUSMALLINT Option, SQLULEN Param)
{
    SQLINTEGER StringLength= 0;
    SQLRETURN ret;

    TRACE_ANSI_INT(Option)

    if (!Hdbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&((MADB_Dbc*)Hdbc)->Error);

    /* todo: do we have more string options ? */
    if (Option == SQL_ATTR_CURRENT_CATALOG)
        StringLength= SQL_NTS;
    ret= MA_SQLSetConnectAttr(Hdbc, Option, (SQLPOINTER)Param, StringLength);
    return ret;
}
/* }}} */

/* {{{ SQLSetCursorName */
SQLRETURN SQL_API SQLSetCursorName(SQLHSTMT StatementHandle,
                                   SQLCHAR *CursorName,
                                   SQLSMALLINT NameLength)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->SetCursorName(Stmt, (char *)CursorName, NameLength);
}
/* }}} */

/* {{{ SQLSetDescField */
SQLRETURN SQL_API SQLSetDescField(SQLHDESC DescriptorHandle,
                                  SQLSMALLINT RecNumber,
                                  SQLSMALLINT FieldIdentifier,
                                  SQLPOINTER ValuePtr,
                                  SQLINTEGER BufferLength)
{
    MADB_Desc *Desc= (MADB_Desc *)DescriptorHandle;

    TRACE_ANSI

    if (!Desc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Desc->Error);

    return MADB_DescSetField(DescriptorHandle, RecNumber, FieldIdentifier, ValuePtr, BufferLength, FALSE);
}
/* }}} */

/* {{{ SQLSetDescRec */
// TODO PLAT-5516
SQLRETURN SQL_API SQLSetDescRec(SQLHDESC DescriptorHandle,
                                SQLSMALLINT RecNumber,
                                SQLSMALLINT Type,
                                SQLSMALLINT SubType,
                                SQLLEN Length,
                                SQLSMALLINT Precision,
                                SQLSMALLINT Scale,
                                SQLPOINTER DataPtr,
                                SQLLEN *StringLengthPtr,
                                SQLLEN *IndicatorPtr)
{
    MADB_Desc *Desc= (MADB_Desc *)DescriptorHandle;

    TRACE_ANSI

    MADB_NOT_IMPLEMENTED(Desc);
}
/* }}} */

/* {{{ SQLSetStmtAttr */
SQLRETURN SQL_API SQLSetStmtAttr(SQLHSTMT StatementHandle,
                                 SQLINTEGER Attribute,
                                 SQLPOINTER ValuePtr,
                                 SQLINTEGER StringLength)
{
    TRACE_ANSI_INT(Attribute)

    if (StatementHandle == SQL_NULL_HSTMT)
    {
        return SQL_INVALID_HANDLE;
    }
    MADB_CLEAR_ERROR(&((MADB_Stmt*)StatementHandle)->Error);

    return MA_SQLSetStmtAttr(StatementHandle, Attribute, ValuePtr, StringLength);
}
/* }}} */

/* {{{ SQLSpecialColumns */
SQLRETURN SQL_API SQLSpecialColumns(SQLHSTMT StatementHandle,
                                    SQLUSMALLINT IdentifierType,
                                    SQLCHAR *CatalogName,
                                    SQLSMALLINT NameLength1,
                                    SQLCHAR *SchemaName,
                                    SQLSMALLINT NameLength2,
                                    SQLCHAR *TableName,
                                    SQLSMALLINT NameLength3,
                                    SQLUSMALLINT Scope,
                                    SQLUSMALLINT Nullable)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->SpecialColumns(Stmt,IdentifierType, (char *)CatalogName, NameLength1,
                                         (char *)SchemaName, NameLength2,
                                         (char *)TableName, NameLength3, Scope, Nullable);
}
/* }}} */

/* {{{ SQLStatistics */
SQLRETURN SQL_API SQLStatistics(SQLHSTMT StatementHandle,
                                SQLCHAR *CatalogName,
                                SQLSMALLINT NameLength1,
                                SQLCHAR *SchemaName,
                                SQLSMALLINT NameLength2,
                                SQLCHAR *TableName,
                                SQLSMALLINT NameLength3,
                                SQLUSMALLINT Unique,
                                SQLUSMALLINT Reserved)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->Statistics(Stmt, (char *)CatalogName, NameLength1, (char *)SchemaName, NameLength2,
                                     (char *)TableName, NameLength3, Unique, Reserved);
}
/* }}} */

/* {{{ SQLTablePrivileges */
SQLRETURN SQL_API SQLTablePrivileges(SQLHSTMT StatementHandle,
                                     SQLCHAR *CatalogName,
                                     SQLSMALLINT NameLength1,
                                     SQLCHAR *SchemaName,
                                     SQLSMALLINT NameLength2,
                                     SQLCHAR *TableName,
                                     SQLSMALLINT NameLength3)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->TablePrivileges(Stmt, (char *)CatalogName, NameLength1, (char *)SchemaName, NameLength2,
                                          (char *)TableName, NameLength3);
}
/* }}} */

/* {{{ SQLTables */
SQLRETURN SQL_API SQLTables(SQLHSTMT StatementHandle,
                            SQLCHAR *CatalogName,
                            SQLSMALLINT NameLength1,
                            SQLCHAR *SchemaName,
                            SQLSMALLINT NameLength2,
                            SQLCHAR *TableName,
                            SQLSMALLINT NameLength3,
                            SQLCHAR *TableType,
                            SQLSMALLINT NameLength4)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    TRACE_ANSI

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->Tables(Stmt, (char *)CatalogName, NameLength1, (char *)SchemaName, NameLength2,
                                 (char *)TableName,NameLength3, (char *)TableType, NameLength4);
}
/* }}} */

/* {{{ SQLError */
SQLRETURN SQL_API SQLError(SQLHENV Env, SQLHDBC Dbc, SQLHSTMT Stmt,
                           SQLCHAR *Sqlstate, SQLINTEGER *NativeError,
                           SQLCHAR *Message, SQLSMALLINT MessageMax,
                           SQLSMALLINT *MessageLen)
{
    SQLSMALLINT HandleType= 0;
    SQLHANDLE   Handle=     NULL;
    MADB_Error *error;

    TRACE_ANSI

    if (Stmt)
    {
        MDBUG_C_ENTER(((MADB_Stmt*)Stmt)->Connection, "SQLError->SQLGetDiagRec");

        MDBUG_C_DUMP(((MADB_Stmt*)Stmt)->Connection, Env, 0x);
        MDBUG_C_DUMP(((MADB_Stmt*)Stmt)->Connection, Dbc, 0x);
        MDBUG_C_DUMP(((MADB_Stmt*)Stmt)->Connection, Stmt, 0x);

        Handle= Stmt;
        HandleType= SQL_HANDLE_STMT;
        error= &((MADB_Stmt*)Stmt)->Error;
    }
    else if (Dbc)
    {
        MDBUG_C_ENTER((MADB_Dbc*)Dbc, "SQLError->SQLGetDiagRec");

        MDBUG_C_DUMP((MADB_Dbc*)Dbc, Env, 0x);
        MDBUG_C_DUMP((MADB_Dbc*)Dbc, Dbc, 0x);
        MDBUG_C_DUMP((MADB_Dbc*)Dbc, Stmt, 0x);

        Handle= Dbc;
        HandleType= SQL_HANDLE_DBC;
        error= &((MADB_Dbc*)Dbc)->Error;
    }
    else
    {
        MDBUG_ENTER("SQLError->SQLGetDiagRec");
        MDBUG_DUMP(Env, 0x);
        MDBUG_DUMP(Dbc, 0x);
        MDBUG_DUMP(Stmt, 0x);

        Handle= Env;
        HandleType= SQL_HANDLE_ENV;
        error= &((MADB_Env*)Env)->Error;
    }

    return MA_SQLGetDiagRec(HandleType, Handle, ++error->ErrorNum, Sqlstate, NativeError, Message, MessageMax, MessageLen);
}
/* }}} */
