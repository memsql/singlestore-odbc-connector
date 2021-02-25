#undef MA_ODBC_DEBUG_ALL

#include <ma_odbc.h>
#include <ma_parse.h>
#include <odbc_3_api.h>

extern Client_Charset utf8;

/* {{{ SQLBrowseConnectW */
SQLRETURN SQL_API SQLBrowseConnectW(SQLHDBC ConnectionHandle,
                                    SQLWCHAR *InConnectionString,
                                    SQLSMALLINT StringLength1,
                                    SQLWCHAR *OutConnectionString,
                                    SQLSMALLINT BufferLength,
                                    SQLSMALLINT *StringLength2Ptr)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
    SQLRETURN ret;
    MDBUG_C_ENTER(Dbc, SQLBrowseConnectW);
    MADB_SetError(&Dbc->Error, MADB_ERR_IM001, NULL, 0);
    ret= Dbc->Error.ReturnValue;

    MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLColAttributeW */
SQLRETURN SQL_API SQLColAttributeW (SQLHSTMT StatementHandle,
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
    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLColAttributeW");
    MDBUG_C_DUMP(Stmt->Connection, StatementHandle, 0x);
    MDBUG_C_DUMP(Stmt->Connection, ColumnNumber, u);
    MDBUG_C_DUMP(Stmt->Connection, FieldIdentifier, u);
    MDBUG_C_DUMP(Stmt->Connection, CharacterAttributePtr, 0x);
    MDBUG_C_DUMP(Stmt->Connection, BufferLength, d);
    MDBUG_C_DUMP(Stmt->Connection, StringLengthPtr, 0x);
    MDBUG_C_DUMP(Stmt->Connection, NumericAttributePtr, 0x);

    ret= Stmt->Methods->ColAttribute(Stmt, ColumnNumber, FieldIdentifier, CharacterAttributePtr,
                                     BufferLength, StringLengthPtr, NumericAttributePtr, TRUE);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLColAttributesW */
SQLRETURN SQL_API SQLColAttributesW(SQLHSTMT hstmt,
                                    SQLUSMALLINT icol,
                                    SQLUSMALLINT fDescType,
                                    SQLPOINTER rgbDesc,
                                    SQLSMALLINT cbDescMax,
                                    SQLSMALLINT * pcbDesc,
                                    SQLLEN * pfDesc)
{
    /* TODO: use internal function, not api */
    return SQLColAttributeW(hstmt, icol, MapColAttributeDescType(fDescType), rgbDesc, cbDescMax, pcbDesc, pfDesc);
}
/* }}} */

/* {{{ SQLColumnPrivilegesW */
SQLRETURN SQL_API SQLColumnPrivilegesW(SQLHSTMT StatementHandle,
                                       SQLWCHAR *CatalogName,
                                       SQLSMALLINT NameLength1,
                                       SQLWCHAR *SchemaName,
                                       SQLSMALLINT NameLength2,
                                       SQLWCHAR *TableName,
                                       SQLSMALLINT NameLength3,
                                       SQLWCHAR *ColumnName,
                                       SQLSMALLINT NameLength4)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLULEN CpLength1, CpLength2, CpLength3, CpLength4;
    char *CpCatalog= NULL,
            *CpSchema= NULL,
            *CpTable= NULL,
            *CpColumn= NULL;
    SQLRETURN ret;

    if (!StatementHandle)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLColumnPrivilegesW");

    CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpSchema= MADB_ConvertFromWChar(SchemaName, NameLength2, &CpLength2, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpTable= MADB_ConvertFromWChar(TableName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpColumn= MADB_ConvertFromWChar(ColumnName, NameLength4, &CpLength4, Stmt->Connection->ConnOrSrcCharset, NULL);

    ret= Stmt->Methods->ColumnPrivileges(Stmt, CpCatalog, (SQLSMALLINT)CpLength1, CpSchema, (SQLSMALLINT)CpLength2,
                                         CpTable, (SQLSMALLINT)CpLength3, CpColumn, (SQLSMALLINT)CpLength4);

    MADB_FREE(CpCatalog);
    MADB_FREE(CpSchema);
    MADB_FREE(CpTable);
    MADB_FREE(CpColumn);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLColumnsW */
SQLRETURN SQL_API SQLColumnsW(SQLHSTMT StatementHandle,
                              SQLWCHAR *CatalogName,
                              SQLSMALLINT NameLength1,
                              SQLWCHAR *SchemaName,
                              SQLSMALLINT NameLength2,
                              SQLWCHAR *TableName,
                              SQLSMALLINT NameLength3,
                              SQLWCHAR *ColumnName,
                              SQLSMALLINT NameLength4)
{
    char *CpCatalog= NULL,
            *CpSchema= NULL,
            *CpTable= NULL,
            *CpColumn= NULL;
    SQLULEN CpLength1= 0, CpLength2= 0, CpLength3= 0, CpLength4= 0;
    SQLRETURN ret;
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLColumns");

    if (CatalogName != NULL)
    {
        CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (SchemaName != NULL)
    {
        CpSchema=  MADB_ConvertFromWChar(SchemaName, NameLength2, &CpLength2, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (TableName != NULL)
    {
        CpTable=   MADB_ConvertFromWChar(TableName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (ColumnName != NULL)
    {
        CpColumn=  MADB_ConvertFromWChar(ColumnName, NameLength4, &CpLength4, Stmt->Connection->ConnOrSrcCharset, NULL);
    }

    ret= Stmt->Methods->Columns(Stmt, CpCatalog, (SQLSMALLINT)CpLength1, CpSchema, (SQLSMALLINT)CpLength2,
                                CpTable, (SQLSMALLINT)CpLength3, CpColumn, (SQLSMALLINT)CpLength4);

    MADB_FREE(CpCatalog);
    MADB_FREE(CpSchema);
    MADB_FREE(CpTable);
    MADB_FREE(CpColumn);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLConnectW */
SQLRETURN SQL_API SQLConnectW(SQLHDBC ConnectionHandle,
        SQLWCHAR *ServerName,
        SQLSMALLINT NameLength1,
        SQLWCHAR *UserName,
        SQLSMALLINT NameLength2,
        SQLWCHAR *Authentication,
        SQLSMALLINT NameLength3)
{
    char *MBServerName= NULL, *MBUserName= NULL, *MBAuthentication= NULL;
    SQLRETURN ret;
    MADB_Dbc *Dbc= (MADB_Dbc*)ConnectionHandle;

    if (!Dbc)
    return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Dbc->Error);

    Dbc->IsAnsi = 0;
    /* Convert parameters to Cp */
    if (ServerName)
    MBServerName= MADB_ConvertFromWChar(ServerName, NameLength1, 0, &utf8, NULL);
    if (UserName) {
        MBUserName= MADB_ConvertFromWChar(UserName, NameLength2, 0, &utf8, NULL);
    }
    if (Authentication) {
        MBAuthentication= MADB_ConvertFromWChar(Authentication, NameLength3, 0, &utf8, NULL);
    }
    ret= SQLConnectCommon(ConnectionHandle, (SQLCHAR *)MBServerName, SQL_NTS, (SQLCHAR *)MBUserName, SQL_NTS,
                          (SQLCHAR *)MBAuthentication, SQL_NTS);
    MADB_FREE(MBServerName);
    MADB_FREE(MBUserName);
    MADB_FREE(MBAuthentication);
    return ret;
}
/* }}} */

/* {{{ SQLDataSourcesW */
SQLRETURN SQL_API SQLDataSourcesW(SQLHENV EnvironmentHandle,
                                  SQLUSMALLINT Direction,
                                  SQLWCHAR *ServerName,
                                  SQLSMALLINT BufferLength1,
                                  SQLSMALLINT *NameLength1Ptr,
                                  SQLWCHAR *Description,
                                  SQLSMALLINT BufferLength2,
                                  SQLSMALLINT *NameLength2Ptr)
{
    SQLRETURN ret= SQL_ERROR;

    return ret;
}
/* }}} */

/* {{{ SQLDescribeColW */
SQLRETURN SQL_API SQLDescribeColW(SQLHSTMT StatementHandle,
                                  SQLUSMALLINT ColumnNumber,
                                  SQLWCHAR *ColumnName,
                                  SQLSMALLINT BufferLength,
                                  SQLSMALLINT *NameLengthPtr,
                                  SQLSMALLINT *DataTypePtr,
                                  SQLULEN *ColumnSizePtr,
                                  SQLSMALLINT *DecimalDigitsPtr,
                                  SQLSMALLINT *NullablePtr)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;

    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLDescribeColW");
    MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);
    MDBUG_C_DUMP(Stmt->Connection, ColumnNumber, u);

    ret= Stmt->Methods->DescribeCol(Stmt, ColumnNumber, (void *)ColumnName, BufferLength,
                                    NameLengthPtr, DataTypePtr, ColumnSizePtr, DecimalDigitsPtr,
                                    NullablePtr, TRUE);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLDriverConnectW */
SQLRETURN SQL_API SQLDriverConnectW(SQLHDBC      ConnectionHandle,
                                    SQLHWND      WindowHandle,
                                    SQLWCHAR    *InConnectionString,
                                    SQLSMALLINT  StringLength1,
                                    SQLWCHAR    *OutConnectionString,
                                    SQLSMALLINT  BufferLength,
                                    SQLSMALLINT *StringLength2Ptr,
                                    SQLUSMALLINT DriverCompletion)
{
    SQLRETURN   ret=          SQL_ERROR;
    SQLULEN     Length=       0; /* Since we need bigger(in bytes) buffer for utf8 string, the length may be > max SQLSMALLINT */
    char        *InConnStrA=  NULL;
    SQLULEN     InStrAOctLen= 0;
    char        *OutConnStrA= NULL;
    MADB_Dbc    *Dbc=         (MADB_Dbc *)ConnectionHandle;

    if (!ConnectionHandle)
    {
        return SQL_INVALID_HANDLE;
    }

    MDBUG_C_ENTER(Dbc, "SQLDriverConnectW");

    MADB_CLEAR_ERROR(&Dbc->Error);

    Dbc->IsAnsi = 0;

    InConnStrA= MADB_ConvertFromWChar(InConnectionString, StringLength1, &InStrAOctLen, &utf8, NULL);
    MDBUG_C_DUMP(Dbc, Dbc, 0x);
    MDBUG_C_DUMP(Dbc, InConnStrA, s);
    MDBUG_C_DUMP(Dbc, StringLength1, d);
    MDBUG_C_DUMP(Dbc, OutConnectionString, 0x);
    MDBUG_C_DUMP(Dbc, BufferLength, d);
    MDBUG_C_DUMP(Dbc, StringLength2Ptr, 0x);
    MDBUG_C_DUMP(Dbc, DriverCompletion, d);

    /* Allocate buffer for Asc OutConnectionString */
    if (OutConnectionString && BufferLength)
    {
        Length= BufferLength*4 /*Max bytes per utf8 character */;
        OutConnStrA= (char *)MADB_CALLOC(Length);

        if (OutConnStrA == NULL)
        {
            ret= MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
            goto end;
        }
    }

    ret= Dbc->Methods->DriverConnect(Dbc, WindowHandle, (SQLCHAR *)InConnStrA, InStrAOctLen, (SQLCHAR *)OutConnStrA,
                                     Length, StringLength2Ptr, DriverCompletion);
    MDBUG_C_DUMP(Dbc, ret, d);
    if (!SQL_SUCCEEDED(ret))
        goto end;

    if (OutConnectionString)
    {
        Length= MADB_SetString(&utf8, OutConnectionString, BufferLength,
                               OutConnStrA, SQL_NTS, &((MADB_Dbc *)ConnectionHandle)->Error);
        if (StringLength2Ptr)
            *StringLength2Ptr= (SQLSMALLINT)Length;
    }

    end:
    MADB_FREE(OutConnStrA);
    MADB_FREE(InConnStrA);
    MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLDriversW */
SQLRETURN SQL_API SQLDriversW(SQLHENV EnvironmentHandle,
                              SQLUSMALLINT Direction,
                              SQLWCHAR *DriverDescription,
                              SQLSMALLINT BufferLength1,
                              SQLSMALLINT *DescriptionLengthPtr,
                              SQLWCHAR *DriverAttributes,
                              SQLSMALLINT BufferLength2,
                              SQLSMALLINT *AttributesLengthPtr)
{
    SQLRETURN ret= SQL_ERROR;

    return ret;
}
/* }}} */

/* {{{ SQLExecDirectW */
SQLRETURN SQL_API SQLExecDirectW(SQLHSTMT StatementHandle,
                                 SQLWCHAR *StatementText,
                                 SQLINTEGER TextLength)
{
    char      *CpStmt;
    SQLULEN   StmtLength;
    SQLRETURN ret;
    BOOL      ConversionError;

    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    if (!Stmt)
        return SQL_INVALID_HANDLE;

    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLExecDirectW");
    MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);

    CpStmt= MADB_ConvertFromWChar(StatementText, TextLength, &StmtLength, Stmt->Connection->ConnOrSrcCharset, &ConversionError);
    MDBUG_C_DUMP(Stmt->Connection, CpStmt, s);
    if (ConversionError)
    {
        MADB_SetError(&Stmt->Error, MADB_ERR_22018, NULL, 0);
        ret= Stmt->Error.ReturnValue;
    }
    else
        ret= Stmt->Methods->ExecDirect(Stmt, CpStmt, (SQLINTEGER)StmtLength);
    MADB_FREE(CpStmt);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLForeignKeysW */
SQLRETURN SQL_API SQLForeignKeysW(SQLHSTMT StatementHandle,
                                  SQLWCHAR *PKCatalogName,
                                  SQLSMALLINT NameLength1,
                                  SQLWCHAR *PKSchemaName,
                                  SQLSMALLINT NameLength2,
                                  SQLWCHAR *PKTableName,
                                  SQLSMALLINT NameLength3,
                                  SQLWCHAR *FKCatalogName,
                                  SQLSMALLINT NameLength4,
                                  SQLWCHAR *FKSchemaName,
                                  SQLSMALLINT NameLength5,
                                  SQLWCHAR *FKTableName,
                                  SQLSMALLINT NameLength6)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

    if(!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return MADB_SetError(&Stmt->Error,  MADB_ERR_IM001, "Foreign keys are not supported", 0);
}
/* }}} */

/* {{{ SQLGetConnectAttrW */
SQLRETURN SQL_API SQLGetConnectAttrW(SQLHDBC ConnectionHandle,
                                     SQLINTEGER Attribute,
                                     SQLPOINTER ValuePtr,
                                     SQLINTEGER BufferLength,
                                     SQLINTEGER *StringLengthPtr)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
    SQLRETURN ret;

    if (!Dbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Dbc->Error);

    MDBUG_C_ENTER(Dbc, "SQLGetConnectAttr");
    MDBUG_C_DUMP(Dbc, Attribute, d);
    MDBUG_C_DUMP(Dbc, ValuePtr, 0x);
    MDBUG_C_DUMP(Dbc, BufferLength, d);
    MDBUG_C_DUMP(Dbc, StringLengthPtr, 0x);

    ret= Dbc->Methods->GetAttr(Dbc, Attribute, ValuePtr, BufferLength, StringLengthPtr, TRUE);

    MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLGetConnectOptionW */
SQLRETURN SQL_API SQLGetConnectOptionW(SQLHDBC ConnectionHandle, SQLUSMALLINT Option, SQLPOINTER ValuePtr)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
    if (!Dbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Dbc->Error);
    return SQLGetConnectAttrW(ConnectionHandle, Option, ValuePtr,
                              Option == SQL_ATTR_CURRENT_CATALOG ? SQL_MAX_OPTION_STRING_LENGTH : 0, NULL);
}
/* }}} */

/* {{{ SQLGetCursorNameW */
SQLRETURN SQL_API SQLGetCursorNameW(
        SQLHSTMT        StatementHandle,
        SQLWCHAR *      CursorName,
        SQLSMALLINT     BufferLength,
        SQLSMALLINT *   NameLengthPtr)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->GetCursorName(Stmt, CursorName, BufferLength, NameLengthPtr, TRUE);
}
/* }}} */

/* {{{ SQLGetDescFieldW */
SQLRETURN SQL_API SQLGetDescFieldW(SQLHDESC DescriptorHandle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT FieldIdentifier,
                                   SQLPOINTER ValuePtr,
                                   SQLINTEGER BufferLength,
                                   SQLINTEGER *StringLengthPtr)
{
    if (!DescriptorHandle)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&((MADB_Desc*)DescriptorHandle)->Error);

    return MADB_DescGetField(DescriptorHandle, RecNumber, FieldIdentifier, ValuePtr, BufferLength, StringLengthPtr, TRUE);
}
/* }}} */

/* {{{ SQLGetDescRecW */
SQLRETURN SQL_API SQLGetDescRecW(SQLHDESC DescriptorHandle,
                                 SQLSMALLINT RecNumber,
                                 SQLWCHAR *Name,
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
    if (!Desc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Desc->Error);

    return MADB_DescGetRec(Desc, RecNumber, (SQLCHAR *)Name, BufferLength, StringLengthPtr, TypePtr, SubTypePtr,
                           LengthPtr, PrecisionPtr, ScalePtr, NullablePtr, TRUE);
}
/* }}} */

/* {{{ SQLGetDiagFieldW */
SQLRETURN SQL_API SQLGetDiagFieldW(SQLSMALLINT HandleType,
                                   SQLHANDLE Handle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT DiagIdentifier,
                                   SQLPOINTER DiagInfoPtr,
                                   SQLSMALLINT BufferLength,
                                   SQLSMALLINT *StringLengthPtr)
{
    if (!Handle)
        return SQL_INVALID_HANDLE;
    return MADB_GetDiagField(HandleType, Handle, RecNumber, DiagIdentifier, DiagInfoPtr, BufferLength, StringLengthPtr, TRUE);
}
/* }}} */

/* {{{ MA_SQLGetDiagRecW */
SQLRETURN SQL_API MA_SQLGetDiagRecW(SQLSMALLINT HandleType,
                                    SQLHANDLE Handle,
                                    SQLSMALLINT RecNumber,
                                    SQLWCHAR *SQLState,
                                    SQLINTEGER *NativeErrorPtr,
                                    SQLWCHAR *MessageText,
                                    SQLSMALLINT BufferLength,
                                    SQLSMALLINT *TextLengthPtr)
{
    if (!Handle)
        return SQL_INVALID_HANDLE;

    /* Maria ODBC driver doesn't support error lists, so only the first record can be retrieved */
    if (RecNumber != 1)
        return SQL_NO_DATA_FOUND;

    switch (HandleType) {
        case SQL_HANDLE_DBC:
        {
            MADB_Dbc *Dbc= (MADB_Dbc *)Handle;
            return MADB_GetDiagRec(&Dbc->Error, RecNumber, (void *)SQLState, NativeErrorPtr,
                                   (void *)MessageText, BufferLength, TextLengthPtr, TRUE,
                                   Dbc->Environment->OdbcVersion);
        }
            break;
        case SQL_HANDLE_STMT:
        {
            MADB_Stmt *Stmt= (MADB_Stmt *)Handle;
            return MADB_GetDiagRec(&Stmt->Error, RecNumber, (void *)SQLState, NativeErrorPtr,
                                   (void *)MessageText, BufferLength, TextLengthPtr, TRUE,
                                   Stmt->Connection->Environment->OdbcVersion);
        }
            break;
        case SQL_HANDLE_DESC:
        {
            MADB_Desc *Desc= (MADB_Desc *)Handle;
            return MADB_GetDiagRec(&Desc->Error, RecNumber, (void *)SQLState, NativeErrorPtr,
                                   (void *)MessageText, BufferLength, TextLengthPtr, TRUE,
                                   SQL_OV_ODBC3);
        }
            break;
        case SQL_HANDLE_ENV:
        {
            MADB_Env *Env= (MADB_Env *)Handle;
            return MADB_GetDiagRec(&Env->Error, RecNumber, (void *)SQLState, NativeErrorPtr,
                                   (void *)MessageText, BufferLength, TextLengthPtr, TRUE,
                                   Env->OdbcVersion);
        }
        default:
            return SQL_ERROR;
            break;
    }
}
/* }}} */

/*{{{ SQLErrorW */
SQLRETURN SQL_API SQLErrorW(SQLHENV Env,
                            SQLHDBC Dbc,
                            SQLHSTMT Stmt,
                            SQLWCHAR *Sqlstate,
                            SQLINTEGER *NativeError,
                            SQLWCHAR *Message,
                            SQLSMALLINT MessageMax,
                            SQLSMALLINT *MessageLen)

{
    SQLSMALLINT HandleType= 0;
    SQLHANDLE   Handle=     NULL;
    MADB_Error *error;

    if (Stmt)
    {
        Handle= Stmt;
        HandleType= SQL_HANDLE_STMT;
        error= &((MADB_Stmt*)Stmt)->Error;
    }
    else if (Dbc)
    {
        Handle= Dbc;
        HandleType= SQL_HANDLE_DBC;
        error= &((MADB_Dbc*)Dbc)->Error;
    }
    else
    {
        Handle= Env;
        HandleType= SQL_HANDLE_ENV;
        error= &((MADB_Env*)Env)->Error;
    }

    return MA_SQLGetDiagRecW(HandleType, Handle, ++error->ErrorNum, Sqlstate, NativeError, Message, MessageMax, MessageLen);
}
/* }}} */

/* {{{ SQLGetDiagRecW */
SQLRETURN SQL_API SQLGetDiagRecW(SQLSMALLINT HandleType,
                                 SQLHANDLE Handle,
                                 SQLSMALLINT RecNumber,
                                 SQLWCHAR *SQLState,
                                 SQLINTEGER *NativeErrorPtr,
                                 SQLWCHAR *MessageText,
                                 SQLSMALLINT BufferLength,
                                 SQLSMALLINT *TextLengthPtr)
{
    return MA_SQLGetDiagRecW(HandleType, Handle, RecNumber, SQLState, NativeErrorPtr, MessageText,
                             BufferLength, TextLengthPtr);
}
/* }}} */

/* {{{ SQLGetInfoW */
SQLRETURN SQL_API SQLGetInfoW(SQLHDBC ConnectionHandle,
                              SQLUSMALLINT InfoType,
                              SQLPOINTER InfoValuePtr,
                              SQLSMALLINT BufferLength,
                              SQLSMALLINT *StringLengthPtr)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
    SQLRETURN ret;
    if (!Dbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Dbc->Error);

    MDBUG_C_ENTER(Dbc, "SQLGetInfo");
    MDBUG_C_DUMP(Dbc, InfoType, d);
    MDBUG_C_DUMP(Dbc, InfoValuePtr, 0x);
    MDBUG_C_DUMP(Dbc, StringLengthPtr, 0x);
    ret= Dbc->Methods->GetInfo(Dbc, InfoType, InfoValuePtr, BufferLength, StringLengthPtr, TRUE);

    MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLGetStmtAttrW */
SQLRETURN SQL_API SQLGetStmtAttrW(SQLHSTMT StatementHandle,
                                  SQLINTEGER Attribute,
                                  SQLPOINTER ValuePtr,
                                  SQLINTEGER BufferLength,
                                  SQLINTEGER *StringLengthPtr)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return Stmt->Methods->GetAttr(Stmt, Attribute, ValuePtr, BufferLength, StringLengthPtr);
}
/* }}} */

/* {{{ SQLGetTypeInfoW */
SQLRETURN SQL_API SQLGetTypeInfoW(SQLHSTMT StatementHandle,
                                  SQLSMALLINT DataType)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    return MADB_GetTypeInfo(Stmt, DataType);}
/* }}} */

/* {{{ SQLNativeSqlW */
SQLRETURN SQL_API SQLNativeSqlW(SQLHDBC ConnectionHandle,
                                SQLWCHAR *InStatementText,
                                SQLINTEGER TextLength1,
                                SQLWCHAR *OutStatementText,
                                SQLINTEGER BufferLength,
                                SQLINTEGER *TextLength2Ptr)
{
    MADB_Dbc  *Conn=   (MADB_Dbc *)ConnectionHandle;
    SQLINTEGER Length= (TextLength1 == SQL_NTS) ? SqlwcsCharLen(InStatementText, (SQLLEN)-1) : TextLength1;

    if (!Conn)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Conn->Error);

    if (TextLength2Ptr)
        *TextLength2Ptr= Length;

    if(OutStatementText && BufferLength < Length)
        MADB_SetError(&Conn->Error, MADB_ERR_01004, NULL, 0);

    if(OutStatementText && BufferLength < Length)
        MADB_SetError(&Conn->Error, MADB_ERR_01004, NULL, 0);
    Length= MIN(Length, BufferLength - 1);

    if (OutStatementText && BufferLength)
    {
        memcpy(OutStatementText, InStatementText, Length * sizeof(SQLWCHAR));
        OutStatementText[Length]= 0;
    }
    return Conn->Error.ReturnValue;
}
/* }}} */

/* {{{ SQLPrepareW */
SQLRETURN SQL_API SQLPrepareW(SQLHSTMT StatementHandle,
                              SQLWCHAR *StatementText,
                              SQLINTEGER TextLength)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    char *StmtStr;
    SQLULEN StmtLength;
    SQLRETURN ret;
    BOOL ConversionError;

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    MDBUG_C_ENTER(Stmt->Connection, "SQLPrepareW");

    StmtStr= MADB_ConvertFromWChar(StatementText, TextLength, &StmtLength, Stmt->Connection->ConnOrSrcCharset, &ConversionError);

    MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);
    MDBUG_C_DUMP(Stmt->Connection, StmtStr, s);
    MDBUG_C_DUMP(Stmt->Connection, TextLength, d);

    if (ConversionError)
    {
        MADB_SetError(&Stmt->Error, MADB_ERR_22018, NULL, 0);
        ret= Stmt->Error.ReturnValue;
    }
    else
        ret= Stmt->Methods->Prepare(Stmt, StmtStr, (SQLINTEGER)StmtLength, FALSE);
    MADB_FREE(StmtStr);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLPrimaryKeysW */
SQLRETURN SQL_API SQLPrimaryKeysW(SQLHSTMT StatementHandle,
                                  SQLWCHAR *CatalogName,
                                  SQLSMALLINT NameLength1,
                                  SQLWCHAR *SchemaName,
                                  SQLSMALLINT NameLength2,
                                  SQLWCHAR *TableName,
                                  SQLSMALLINT NameLength3)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    char *CpCatalog= NULL,
            *CpSchema= NULL,
            *CpTable= NULL;
    SQLULEN CpLength1, CpLength2, CpLength3;
    SQLRETURN ret;

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpSchema= MADB_ConvertFromWChar(SchemaName, NameLength2, &CpLength2, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpTable= MADB_ConvertFromWChar(TableName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);

    MDBUG_C_ENTER(Stmt->Connection, "SQLPrimaryKeysW");
    MDBUG_C_DUMP(Stmt->Connection, StatementHandle, 0x);
    MDBUG_C_DUMP(Stmt->Connection, CpCatalog, s);
    MDBUG_C_DUMP(Stmt->Connection, CpLength1, d);
    MDBUG_C_DUMP(Stmt->Connection, CpSchema, s);
    MDBUG_C_DUMP(Stmt->Connection, CpLength2, d);
    MDBUG_C_DUMP(Stmt->Connection, CpTable, s);
    MDBUG_C_DUMP(Stmt->Connection, CpLength3, d);

    ret= Stmt->Methods->PrimaryKeys(Stmt, CpCatalog, (SQLSMALLINT)CpLength1, CpSchema, (SQLSMALLINT)CpLength2,
                                    CpTable, (SQLSMALLINT)CpLength3);
    MADB_FREE(CpCatalog);
    MADB_FREE(CpSchema);
    MADB_FREE(CpTable);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLProcedureColumnsW */
SQLRETURN SQL_API SQLProcedureColumnsW(SQLHSTMT StatementHandle,
                                       SQLWCHAR *CatalogName,
                                       SQLSMALLINT NameLength1,
                                       SQLWCHAR *SchemaName,
                                       SQLSMALLINT NameLength2,
                                       SQLWCHAR *ProcName,
                                       SQLSMALLINT NameLength3,
                                       SQLWCHAR *ColumnName,
                                       SQLSMALLINT NameLength4)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;
    char *CpCatalog= NULL,
            *CpSchema= NULL,
            *CpProc= NULL,
            *CpColumn= NULL;
    SQLULEN CpLength1= 0, CpLength2= 0, CpLength3= 0, CpLength4= 0;

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    if (CatalogName != NULL)
    {
        CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (SchemaName != NULL)
    {
        CpSchema= MADB_ConvertFromWChar(SchemaName, NameLength2, &CpLength2, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (ProcName != NULL)
    {
        CpProc= MADB_ConvertFromWChar(ProcName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (ColumnName != NULL)
    {
        CpColumn= MADB_ConvertFromWChar(ColumnName, NameLength4, &CpLength4, Stmt->Connection->ConnOrSrcCharset, NULL);
    }

    ret= Stmt->Methods->ProcedureColumns(Stmt, CpCatalog, (SQLSMALLINT)CpLength1, CpSchema, (SQLSMALLINT)CpLength2,
                                         CpProc, (SQLSMALLINT)CpLength3, CpColumn, (SQLSMALLINT)CpLength4);
    MADB_FREE(CpCatalog);
    MADB_FREE(CpSchema);
    MADB_FREE(CpProc);
    MADB_FREE(CpColumn);

    return ret;
}
/* }}} */

/* {{{ SQLProceduresW */
SQLRETURN SQL_API SQLProceduresW(SQLHSTMT StatementHandle,
                                 SQLWCHAR *CatalogName,
                                 SQLSMALLINT NameLength1,
                                 SQLWCHAR *SchemaName,
                                 SQLSMALLINT NameLength2,
                                 SQLWCHAR *ProcName,
                                 SQLSMALLINT NameLength3)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;
    char *CpCatalog= NULL,
            *CpSchema= NULL,
            *CpProc= NULL;
    SQLULEN CpLength1, CpLength2, CpLength3;

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpSchema= MADB_ConvertFromWChar(SchemaName, NameLength2, &CpLength2, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpProc= MADB_ConvertFromWChar(ProcName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);

    ret= Stmt->Methods->Procedures(Stmt, CpCatalog, (SQLSMALLINT)CpLength1, CpSchema, (SQLSMALLINT)CpLength2,
                                   CpProc, (SQLSMALLINT)CpLength3);
    MADB_FREE(CpCatalog);
    MADB_FREE(CpSchema);
    MADB_FREE(CpProc);
    return ret;
}
/* }}} */

/* {{{ SQLSetConnectAttrW */
SQLRETURN SQL_API SQLSetConnectAttrW(SQLHDBC ConnectionHandle,
                                     SQLINTEGER Attribute,
                                     SQLPOINTER ValuePtr,
                                     SQLINTEGER StringLength)
{
    MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
    SQLRETURN ret;
    if (!Dbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Dbc->Error);

    MDBUG_C_ENTER(Dbc, "SetConnectAttrW");
    MDBUG_C_DUMP(Dbc, Dbc, 0x);
    MDBUG_C_DUMP(Dbc, Attribute, d);
    MDBUG_C_DUMP(Dbc, ValuePtr, 0x);
    MDBUG_C_DUMP(Dbc, StringLength, d);
    ret= Dbc->Methods->SetAttr(Dbc, Attribute, ValuePtr, StringLength, TRUE);

    MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLSetConnectOptionW */
SQLRETURN SQL_API SQLSetConnectOptionW(SQLHDBC Hdbc, SQLUSMALLINT Option, SQLULEN Param)
{
    SQLINTEGER StringLength= 0;
    SQLRETURN ret;
    MADB_Dbc *Dbc= (MADB_Dbc *)Hdbc;

    if (!Dbc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Dbc->Error);

    MDBUG_C_ENTER(Dbc, "SetSetConnectOptionW");
    MDBUG_C_DUMP(Dbc, Option, d);
    MDBUG_C_DUMP(Dbc, Param, u);
    /* todo: do we have more string options ? */
    if (Option == SQL_ATTR_CURRENT_CATALOG)
        StringLength= SQL_NTS;

    ret= Dbc->Methods->SetAttr(Dbc, Option, (SQLPOINTER)Param, StringLength, TRUE);

    MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLSetCursorNameW */
SQLRETURN SQL_API SQLSetCursorNameW(SQLHSTMT StatementHandle,
                                    SQLWCHAR *CursorName,
                                    SQLSMALLINT NameLength)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    char *CpName= NULL;
    SQLULEN Length;
    SQLRETURN rc;

    if (!Stmt)
    {
        return SQL_INVALID_HANDLE;
    }
    MADB_CLEAR_ERROR(&Stmt->Error);

    CpName= MADB_ConvertFromWChar(CursorName, NameLength, &Length, Stmt->Connection->ConnOrSrcCharset, NULL);
    rc= Stmt->Methods->SetCursorName(Stmt, (char *)CpName, (SQLINTEGER)Length);

    MADB_FREE(CpName);

    return rc;
}
/* }}} */

/* {{{ SQLSetDescFieldW */
SQLRETURN SQL_API SQLSetDescFieldW(SQLHDESC DescriptorHandle,
                                   SQLSMALLINT RecNumber,
                                   SQLSMALLINT FieldIdentifier,
                                   SQLPOINTER ValuePtr,
                                   SQLINTEGER BufferLength)
{
    MADB_Desc *Desc= (MADB_Desc *)DescriptorHandle;
    if (!Desc)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Desc->Error);

    return MADB_DescSetField(DescriptorHandle, RecNumber, FieldIdentifier, ValuePtr, BufferLength, TRUE);
}
/* }}} */

/* {{{ SQLSetDescRecW */
SQLRETURN SQL_API SQLSetDescRecW(SQLHDESC DescriptorHandle,
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
    MADB_NOT_IMPLEMENTED(Desc);
}
/* }}} */

/* {{{ SQLSetStmtAttrW */
SQLRETURN SQL_API SQLSetStmtAttrW(SQLHSTMT StatementHandle,
                                  SQLINTEGER Attribute,
                                  SQLPOINTER ValuePtr,
                                  SQLINTEGER StringLength)
{
    if (StatementHandle == SQL_NULL_HSTMT)
    {
        return SQL_INVALID_HANDLE;
    }
    MADB_CLEAR_ERROR(&((MADB_Stmt*)StatementHandle)->Error);

    return MA_SQLSetStmtAttr(StatementHandle, Attribute, ValuePtr, StringLength);
}
/* }}} */

/* {{{ SQLSpecialColumnsW */
SQLRETURN SQL_API SQLSpecialColumnsW(SQLHSTMT StatementHandle,
                                     SQLUSMALLINT IdentifierType,
                                     SQLWCHAR *CatalogName,
                                     SQLSMALLINT NameLength1,
                                     SQLWCHAR *SchemaName,
                                     SQLSMALLINT NameLength2,
                                     SQLWCHAR *TableName,
                                     SQLSMALLINT NameLength3,
                                     SQLUSMALLINT Scope,
                                     SQLUSMALLINT Nullable)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;
    char *CpCatalog= NULL,
            *CpSchema= NULL,
            *CpTable= NULL;
    SQLULEN CpLength1, CpLength2, CpLength3;

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    if (CatalogName != NULL)
    {
        CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (SchemaName != NULL)
    {
        CpSchema= MADB_ConvertFromWChar(SchemaName, NameLength2, &CpLength2, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (TableName != NULL)
    {
        CpTable= MADB_ConvertFromWChar(TableName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);
    }

    ret= Stmt->Methods->SpecialColumns(Stmt,IdentifierType, CpCatalog, (SQLSMALLINT)CpLength1, CpSchema,
                                       (SQLSMALLINT)CpLength2, CpTable, (SQLSMALLINT)CpLength3, Scope, Nullable);
    MADB_FREE(CpCatalog);
    MADB_FREE(CpSchema);
    MADB_FREE(CpTable);
    return ret;
}
/* }}} */

/* {{{ SQLStatisticsW */
SQLRETURN SQL_API SQLStatisticsW(SQLHSTMT StatementHandle,
                                 SQLWCHAR *CatalogName,
                                 SQLSMALLINT NameLength1,
                                 SQLWCHAR *SchemaName,
                                 SQLSMALLINT NameLength2,
                                 SQLWCHAR *TableName,
                                 SQLSMALLINT NameLength3,
                                 SQLUSMALLINT Unique,
                                 SQLUSMALLINT Reserved)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;
    char *CpCatalog= NULL,
            *CpSchema= NULL,
            *CpTable= NULL;
    SQLULEN CpLength1, CpLength2, CpLength3;

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpSchema= MADB_ConvertFromWChar(SchemaName, NameLength2, &CpLength2, Stmt->Connection->ConnOrSrcCharset, NULL);
    CpTable= MADB_ConvertFromWChar(TableName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    ret= Stmt->Methods->Statistics(Stmt, CpCatalog, (SQLSMALLINT)CpLength1, CpSchema, (SQLSMALLINT)CpLength2,
                                   CpTable, (SQLSMALLINT)CpLength3, Unique, Reserved);
    MADB_FREE(CpCatalog);
    MADB_FREE(CpSchema);
    MADB_FREE(CpTable);
    return ret;
}
/* }}} */

/* {{{ SQLTablePrivilegesW */
SQLRETURN SQL_API SQLTablePrivilegesW(SQLHSTMT StatementHandle,
                                      SQLWCHAR *CatalogName,
                                      SQLSMALLINT NameLength1,
                                      SQLWCHAR *SchemaName,
                                      SQLSMALLINT NameLength2,
                                      SQLWCHAR *TableName,
                                      SQLSMALLINT NameLength3)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    SQLRETURN ret;
    char *CpCatalog= NULL,
            *CpTable= NULL;
    SQLULEN CpLength1= 0, CpLength3= 0;

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    if (CatalogName != NULL)
    {
        CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (TableName != NULL)
    {
        CpTable=   MADB_ConvertFromWChar(TableName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);
    }

    ret= Stmt->Methods->TablePrivileges(Stmt, CpCatalog, (SQLSMALLINT)CpLength1, NULL, 0, CpTable, (SQLSMALLINT)CpLength3);

    MADB_FREE(CpCatalog);
    MADB_FREE(CpTable);
    return ret;
}
/* }}} */

/* {{{ SQLTablesW */
SQLRETURN SQL_API SQLTablesW(SQLHSTMT StatementHandle,
                             SQLWCHAR *CatalogName,
                             SQLSMALLINT NameLength1,
                             SQLWCHAR *SchemaName,
                             SQLSMALLINT NameLength2,
                             SQLWCHAR *TableName,
                             SQLSMALLINT NameLength3,
                             SQLWCHAR *TableType,
                             SQLSMALLINT NameLength4)
{
    MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
    char *CpCatalog= NULL,
            *CpSchema= NULL,
            *CpTable= NULL,
            *CpType= NULL;
    SQLULEN CpLength1= 0, CpLength2= 0, CpLength3= 0, CpLength4= 0;
    SQLRETURN ret;

    if (!Stmt)
        return SQL_INVALID_HANDLE;
    MADB_CLEAR_ERROR(&Stmt->Error);

    if (CatalogName)
    {
        CpCatalog= MADB_ConvertFromWChar(CatalogName, NameLength1, &CpLength1, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (SchemaName)
    {
        CpSchema= MADB_ConvertFromWChar(SchemaName, NameLength2, &CpLength2, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (TableName)
    {
        CpTable= MADB_ConvertFromWChar(TableName, NameLength3, &CpLength3, Stmt->Connection->ConnOrSrcCharset, NULL);
    }
    if (TableType)
    {
        CpType= MADB_ConvertFromWChar(TableType, NameLength4, &CpLength4, Stmt->Connection->ConnOrSrcCharset, NULL);
    }

    ret= Stmt->Methods->Tables(Stmt, CpCatalog, (SQLSMALLINT)CpLength1, CpSchema, (SQLSMALLINT)CpLength2,
                               CpTable, (SQLSMALLINT)CpLength3, CpType, (SQLSMALLINT)CpLength4);
    MADB_FREE(CpCatalog);
    MADB_FREE(CpSchema);
    MADB_FREE(CpTable);
    MADB_FREE(CpType);
    return ret;
}
/* }}} */
