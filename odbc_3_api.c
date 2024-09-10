/************************************************************************************
   Copyright (C) 2013,2016 MariaDB Corporation AB
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

extern Client_Charset utf8;

/* {{{ SQLAllocHandle */
SQLRETURN MA_SQLAllocHandle(SQLSMALLINT HandleType,
    SQLHANDLE InputHandle,
    SQLHANDLE *OutputHandlePtr)
{
  SQLRETURN ret= SQL_ERROR;

  switch(HandleType) {
    case SQL_HANDLE_DBC:
      EnterCriticalSection(&((MADB_Env *)InputHandle)->cs);
      MADB_CLEAR_ERROR(&((MADB_Env *)InputHandle)->Error);
      if ((*OutputHandlePtr= (SQLHANDLE)MADB_DbcInit((MADB_Env *)InputHandle)) != NULL)
      {
        ret= SQL_SUCCESS;
      }
      LeaveCriticalSection(&((MADB_Env *)InputHandle)->cs);
      break;
    case SQL_HANDLE_DESC:
      EnterCriticalSection(&((MADB_Dbc *)InputHandle)->cs);
      MADB_CLEAR_ERROR(&((MADB_Dbc *)InputHandle)->Error);
      if ((*OutputHandlePtr= (SQLHANDLE)MADB_DescInit((MADB_Dbc *)InputHandle, MADB_DESC_UNKNOWN, TRUE)) != NULL)
      {
        ret= SQL_SUCCESS;
      }
      LeaveCriticalSection(&((MADB_Dbc *)InputHandle)->cs);
      break;
    case SQL_HANDLE_ENV:
      if ((*OutputHandlePtr= (SQLHANDLE)MADB_EnvInit()) != NULL)
      {
        ret= SQL_SUCCESS;
      }
      break;
    case SQL_HANDLE_STMT:
      {
        MADB_Dbc *Connection= (MADB_Dbc *)InputHandle;
        MDBUG_C_ENTER(InputHandle, "MA_SQLAllocHandle(Stmt)");
        MDBUG_C_DUMP(InputHandle, InputHandle, 0x);
        MDBUG_C_DUMP(InputHandle, OutputHandlePtr, 0x);

        MADB_CLEAR_ERROR(&Connection->Error);
       
        if (!CheckConnection(Connection))
        {
          MADB_SetError(&Connection->Error, MADB_ERR_08003, NULL, 0);
          break;
        }

        ret= MADB_StmtInit(Connection, OutputHandlePtr);
        MDBUG_C_DUMP(InputHandle, *OutputHandlePtr, 0x);
        MDBUG_C_RETURN(InputHandle,ret, &Connection->Error);
      }
      break;
    default:
      /* todo: set error message */
      break;
  }
  return ret;
}
/* }}} */

/* {{{ SQLAllocHandle */
SQLRETURN SQL_API SQLAllocHandle(SQLSMALLINT HandleType,
    SQLHANDLE InputHandle,
    SQLHANDLE *OutputHandlePtr)
{
  SQLRETURN ret;
  MDBUG_ENTER("SQLAllocHandle");
  MDBUG_DUMP(HandleType, d);
  MDBUG_DUMP(InputHandle, 0x);
  MDBUG_DUMP(OutputHandlePtr, 0x);
  
  ret= MA_SQLAllocHandle(HandleType, InputHandle, OutputHandlePtr);

  MDBUG_DUMP(ret,d);
  MDBUG_RETURN(ret);
}
/* }}} */

/* {{{ SQLAllocConnect */
SQLRETURN MA_SQLAllocConnect(SQLHANDLE InputHandle,
                             SQLHANDLE *OutputHandlePtr)
{
  return MA_SQLAllocHandle(SQL_HANDLE_DBC, InputHandle, OutputHandlePtr);
}
SQLRETURN SQL_API SQLAllocConnect(SQLHANDLE InputHandle,
                                  SQLHANDLE *OutputHandlePtr)
{
  return MA_SQLAllocConnect(InputHandle, OutputHandlePtr);
}
/* }}} */

/* {{{ SQLAllocStmt */
SQLRETURN SQL_API SQLAllocStmt(SQLHANDLE InputHandle,
                               SQLHANDLE *OutputHandlePtr)
{
  MDBUG_C_ENTER(InputHandle, "SQLAllocStmt");
  MDBUG_C_DUMP(InputHandle, InputHandle, 0x);
  MDBUG_C_DUMP(InputHandle, OutputHandlePtr, 0x);

  return MA_SQLAllocHandle(SQL_HANDLE_STMT, InputHandle, OutputHandlePtr);
}
/* }}} */

/* {{{ SQLAllocEnv */
SQLRETURN SQL_API SQLAllocEnv(SQLHANDLE *OutputHandlePtr)
{
  return MA_SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, OutputHandlePtr);
}
/* }}} */

/* {{{ SQLBindCol */
SQLRETURN SQL_API SQLBindCol(SQLHSTMT StatementHandle,
    SQLUSMALLINT ColumnNumber,
    SQLSMALLINT TargetType,
    SQLPOINTER TargetValuePtr,
    SQLLEN BufferLength,
    SQLLEN *StrLen_or_Ind)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  SQLRETURN ret;
  MADB_CLEAR_ERROR(&Stmt->Error);
  MADB_CHECK_STMT_HANDLE(Stmt,stmt);  

  MDBUG_C_ENTER(Stmt->Connection, "SQLBindCol");
  MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);
  MDBUG_C_DUMP(Stmt->Connection, ColumnNumber, u);
  MDBUG_C_DUMP(Stmt->Connection, TargetType, d);
  MDBUG_C_DUMP(Stmt->Connection, BufferLength, d);
  MDBUG_C_DUMP(Stmt->Connection, StrLen_or_Ind, 0x);

  ret= Stmt->Methods->BindColumn(Stmt, ColumnNumber, TargetType, TargetValuePtr, BufferLength, StrLen_or_Ind);

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ MA_SQLBindParameter */
SQLRETURN MA_SQLBindParameter(SQLHSTMT StatementHandle,
    SQLUSMALLINT ParameterNumber,
    SQLSMALLINT InputOutputType,
    SQLSMALLINT ValueType,
    SQLSMALLINT ParameterType,
    SQLULEN ColumnSize,
    SQLSMALLINT DecimalDigits,
    SQLPOINTER ParameterValuePtr,
    SQLLEN BufferLength,
    SQLLEN *StrLen_or_IndPtr)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  SQLRETURN ret;

  if (!Stmt)
    return SQL_INVALID_HANDLE;

  MDBUG_C_ENTER(Stmt->Connection, "SQLBindParameter");
  MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);
  MDBUG_C_DUMP(Stmt->Connection, ParameterNumber, u);
  MDBUG_C_DUMP(Stmt->Connection, InputOutputType, d);
  MDBUG_C_DUMP(Stmt->Connection, ValueType, d);
  MDBUG_C_DUMP(Stmt->Connection, ParameterType, d);
  MDBUG_C_DUMP(Stmt->Connection, ColumnSize, u);
  MDBUG_C_DUMP(Stmt->Connection, DecimalDigits, d);
  MDBUG_C_DUMP(Stmt->Connection, ParameterValuePtr, 0x);
  MDBUG_C_DUMP(Stmt->Connection, BufferLength, d);
  MDBUG_C_DUMP(Stmt->Connection, StrLen_or_IndPtr, 0x);
      
  MADB_CHECK_STMT_HANDLE(Stmt,stmt);
  ret= Stmt->Methods->BindParam(Stmt, ParameterNumber, InputOutputType, ValueType, ParameterType, ColumnSize, DecimalDigits,
                                  ParameterValuePtr, BufferLength, StrLen_or_IndPtr);

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLBindParameter */
SQLRETURN SQL_API SQLBindParameter(SQLHSTMT StatementHandle,
    SQLUSMALLINT ParameterNumber,
    SQLSMALLINT InputOutputType,
    SQLSMALLINT ValueType,
    SQLSMALLINT ParameterType,
    SQLULEN ColumnSize,
    SQLSMALLINT DecimalDigits,
    SQLPOINTER ParameterValuePtr,
    SQLLEN BufferLength,
    SQLLEN *StrLen_or_IndPtr)
{
  if (!StatementHandle)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&((MADB_Stmt*)StatementHandle)->Error);
  return MA_SQLBindParameter(StatementHandle, ParameterNumber, InputOutputType, ValueType, ParameterType,
                             ColumnSize, DecimalDigits, ParameterValuePtr, BufferLength, StrLen_or_IndPtr);
}
/* }}} */




/* {{{ SQLBulkOperations */
SQLRETURN SQL_API SQLBulkOperations(SQLHSTMT StatementHandle,
    SQLSMALLINT Operation)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  SQLRETURN ret;
  if (!Stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  MDBUG_C_ENTER(Stmt->Connection, "SQLBulkOperations");
  MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);
  MDBUG_C_DUMP(Stmt->Connection, Operation, d);

  ret= Stmt->Methods->BulkOperations(Stmt, Operation);

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLCancel */
SQLRETURN MA_SQLCancel(SQLHSTMT StatementHandle)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  SQLRETURN ret= SQL_ERROR;

  if (!Stmt)
    return SQL_INVALID_HANDLE;

  MADB_CLEAR_ERROR(&Stmt->Error);

  MDBUG_C_ENTER(Stmt->Connection, "SQLCancel");
  MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);

  if (TryEnterCriticalSection(&Stmt->Connection->cs))
  {
    LeaveCriticalSection(&Stmt->Connection->cs);
    ret= Stmt->Methods->StmtFree(Stmt, SQL_CLOSE);

    MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
  } else
  {
    MYSQL *MariaDb, *Kill=Stmt->Connection->mariadb;
    
    char StmtStr[50];

    if (!(MariaDb= mysql_init(NULL)))
    {
      ret= SQL_ERROR;
      goto end;
    }
    if (!(mysql_real_connect(MariaDb, Kill->host, Kill->user, Kill->passwd,
                             "", Kill->port, Kill->unix_socket, 0)))
    {
      mysql_close(MariaDb);
      goto end;
    }
    
    _snprintf(StmtStr, 50, "KILL QUERY %ld %d", mysql_thread_id(Kill), Kill->ss_aggregator_id);
    if (mysql_query(MariaDb, StmtStr))
    {
      mysql_close(MariaDb);
      goto end;
    }
    mysql_close(MariaDb);
    ret= SQL_SUCCESS;
  }
end:
  LeaveCriticalSection(&Stmt->Connection->cs);

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}

SQLRETURN SQL_API SQLCancel(SQLHSTMT StatementHandle)
{
  return MA_SQLCancel(StatementHandle);
}
/* }}} */

/* {{{ SQLCancelHandle */
/* ODBC version 3.8 */
SQLRETURN SQL_API SQLCancelHandle(SQLSMALLINT HandleType,
    SQLHANDLE Handle)
{
  if (Handle)
    return SQL_INVALID_HANDLE;

  switch(HandleType) {
  case SQL_HANDLE_DBC:
    {
      MADB_Stmt Stmt;
      Stmt.Connection= (MADB_Dbc *)Handle;
      return MA_SQLCancel((SQLHSTMT)&Stmt);
    }
    break;
  case SQL_HANDLE_STMT:
    return MA_SQLCancel((SQLHSTMT)Handle);
    break;
  }
  return SQL_INVALID_HANDLE;
}
/* }}} */

/* {{{ SQLCloseCursor */
SQLRETURN SQL_API SQLCloseCursor(SQLHSTMT StatementHandle)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  SQLRETURN ret;
  if (!Stmt)
    return SQL_INVALID_HANDLE;

  MADB_CLEAR_ERROR(&Stmt->Error);

  MDBUG_C_ENTER(Stmt->Connection, "SQLCloseCursor");
  MDBUG_C_DUMP(Stmt->Connection, StatementHandle, 0x);

  if (!Stmt->stmt || 
     (!mysql_stmt_field_count(Stmt->stmt) && 
       Stmt->Connection->Environment->OdbcVersion >= SQL_OV_ODBC3))
  {
    MADB_SetError(&Stmt->Error, MADB_ERR_24000, NULL, 0);
    ret= Stmt->Error.ReturnValue;
  }
  else
    ret= MA_SQLFreeStmt(StatementHandle, SQL_CLOSE);

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLCompleteAsync */
/* ODBC 3.8 */
SQLRETURN SQL_API SQLCompleteAsync(SQLSMALLINT HandleType,
    SQLHANDLE Handle,
    RETCODE *AsyncRetCodePtr)
{
  SQLRETURN ret= SQL_ERROR;
  return ret;
}

/* }}} */

/* {{{ SQLConnectCommon */
SQLRETURN SQLConnectCommon(SQLHDBC ConnectionHandle,
    SQLCHAR *ServerName,
    SQLSMALLINT NameLength1,
    SQLCHAR *UserName,
    SQLSMALLINT NameLength2,
    SQLCHAR *Authentication,
    SQLSMALLINT NameLength3)
{
  MADB_Dbc *Connection= (MADB_Dbc *)ConnectionHandle;
  MADB_Dsn *Dsn;
  SQLRETURN ret;
  my_bool DsnFound;

  if (!Connection)
    return SQL_INVALID_HANDLE;

  MADB_CLEAR_ERROR(&Connection->Error);

  MDBUG_C_ENTER(Connection, "SQLConnect");
  MDBUG_C_DUMP(Connection, Connection, 0x);
  MDBUG_C_DUMP(Connection, ServerName, s);
  MDBUG_C_DUMP(Connection, NameLength1, d);
  MDBUG_C_DUMP(Connection, UserName, s);
  MDBUG_C_DUMP(Connection, NameLength2, d);
  MDBUG_C_DUMP(Connection, Authentication, s);
  MDBUG_C_DUMP(Connection, NameLength3, d);

  if (CheckConnection(Connection))
  {
    MADB_SetError(&Connection->Error, MADB_ERR_08002, NULL, 0);
    return SQL_ERROR;
  }

  if (!(Dsn= MADB_DSN_Init()))
  {
    MADB_SetError(&Connection->Error, MADB_ERR_HY001, NULL, 0);
    return SQL_ERROR; 
  }

  if (ServerName && !ServerName[0])
  {
    MADB_SetError(&Connection->Error, MADB_ERR_HY000, "Invalid DSN", 0);
    MADB_DSN_Free(Dsn);
    return Connection->Error.ReturnValue;
  }

  MADB_DSN_SET_STR(Dsn, DSNName, (char *)ServerName, NameLength1);
  DsnFound= MADB_ReadDSN(Dsn, NULL, TRUE);

  MADB_DSN_SET_STR(Dsn, UserName, (char *)UserName, NameLength2);
  MADB_DSN_SET_STR(Dsn, Password, (char *)Authentication, NameLength3);

  ret= Connection->Methods->Connect(Connection, Dsn);

  if (SQL_SUCCEEDED(ret))
  {
    MADB_DSN_Free(Connection->Dsn);
    Connection->Dsn= Dsn;
  }
  else
  {
    MADB_DSN_Free(Dsn);
  }

  MDBUG_C_RETURN(Connection, ret, &Connection->Error);
}
/* }}} */

/* {{{ SQLCopyDesc */
SQLRETURN SQL_API SQLCopyDesc(SQLHDESC SourceDescHandle,
    SQLHDESC TargetDescHandle)
{
  /*TODO: clear error */
  return MADB_DescCopyDesc((MADB_Desc *)SourceDescHandle, (MADB_Desc *)TargetDescHandle);
}
/* }}} */

/* {{{ SQLDescribeParam */
SQLRETURN SQL_API SQLDescribeParam(SQLHSTMT StatementHandle,
    SQLUSMALLINT ParameterNumber,
    SQLSMALLINT *DataTypePtr,
    SQLULEN *ParameterSizePtr,
    SQLSMALLINT *DecimalDigitsPtr,
    SQLSMALLINT *NullablePtr)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

  if (!Stmt)
    return SQL_INVALID_HANDLE;

  MADB_CLEAR_ERROR(&Stmt->Error);

  /* MariaDB doesn't support metadata for parameters,
     so we return default values */
  if (DataTypePtr)
    *DataTypePtr= SQL_VARCHAR;
  if (ParameterSizePtr)
    *ParameterSizePtr= 1024 * 1024 * 24;
  if (NullablePtr)
    *NullablePtr= SQL_NULLABLE_UNKNOWN;
  return SQL_SUCCESS;
}
/* }}} */

/* {{{ SQLDisconnect */
SQLRETURN SQL_API SQLDisconnect(SQLHDBC ConnectionHandle)
{
  SQLRETURN ret= SQL_ERROR;
  MADB_Dbc *Connection = (MADB_Dbc *)ConnectionHandle;
  MADB_List *Element, *NextElement;

  if (!Connection)
    return SQL_INVALID_HANDLE;

  MADB_CLEAR_ERROR(&Connection->Error);

  MDBUG_C_ENTER(Connection, "SQLDisconnect");
  MDBUG_C_DUMP(Connection, ConnectionHandle, 0x);

  /* Close all statements */
  for (Element= Connection->Stmts; Element; Element= NextElement)
  {
    NextElement= Element->next;
    MA_SQLFreeStmt((SQLHSTMT)Element->data, SQL_DROP);
  }

  /* Close all explicitly allocated descriptors */
  for (Element= Connection->Descrs; Element; Element= NextElement)
  {
    NextElement= Element->next;
    MADB_DescFree((MADB_Desc*)Element->data, FALSE);
  }

  if (Connection->mariadb)
  {
    mysql_close(Connection->mariadb);
    Connection->mariadb= NULL;
    ret= SQL_SUCCESS;
  }
  else
  {
    MADB_SetError(&Connection->Error, MADB_ERR_08003, NULL, 0);
    ret= Connection->Error.ReturnValue;
  }
  Connection->ConnOrSrcCharset= NULL;

  MDBUG_C_RETURN(Connection, ret, &Connection->Error);
}
/* }}} */

/* {{{ MA_SQLEndTran */
SQLRETURN MA_SQLEndTran(SQLSMALLINT HandleType,
    SQLHANDLE Handle,
    SQLSMALLINT CompletionType)
{
  SQLRETURN ret= SQL_SUCCESS;
  switch (HandleType) {
  case SQL_HANDLE_ENV:
    {
      MADB_Env *Env= (MADB_Env *)Handle;
      MADB_List *List= Env->Dbcs;

      for (List= Env->Dbcs; List; List= List->next)
        ((MADB_Dbc *)List->data)->Methods->EndTran((MADB_Dbc *)List->data, CompletionType);
    }
    break;
  case SQL_HANDLE_DBC:
    {
      MADB_Dbc *Dbc= (MADB_Dbc *)Handle;
      if (!Dbc->mariadb)
        MADB_SetError(&Dbc->Error, MADB_ERR_08002, NULL, 0);
      else
        Dbc->Methods->EndTran(Dbc, CompletionType);
      ret= Dbc->Error.ReturnValue;
    }
    break;
  default:
    /* todo: Do we need to set an error ?! */
    break;
  }

  return ret;
}

SQLRETURN SQL_API SQLEndTran(SQLSMALLINT HandleType,
    SQLHANDLE Handle,
    SQLSMALLINT CompletionType)
{
  MADB_CHECK_HANDLE_CLEAR_ERROR(HandleType, Handle);
  return MA_SQLEndTran(HandleType, Handle, CompletionType);
}
/* }}} */

/* {{{ SQLTransact */
SQLRETURN SQL_API SQLTransact(SQLHENV Env, SQLHDBC Dbc, SQLUSMALLINT CompletionType)
{
  if (Env != SQL_NULL_HENV)
  {
    MADB_CLEAR_ERROR(&((MADB_Env*)Env)->Error);
    return MA_SQLEndTran(SQL_HANDLE_ENV, Env, CompletionType);
  }
  else if (Dbc != SQL_NULL_HDBC)
  {
    MADB_CLEAR_ERROR(&((MADB_Dbc*)Dbc)->Error);
    return MA_SQLEndTran(SQL_HANDLE_DBC, Dbc, CompletionType);
  }
  else
    return SQL_INVALID_HANDLE;
}
/* }}} */

SQLRETURN SQL_API SQLExecute(SQLHSTMT StatementHandle)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

  if (StatementHandle == SQL_NULL_HSTMT)
    return SQL_INVALID_HANDLE;

  MADB_CLEAR_ERROR(&Stmt->Error);

  MDBUG_C_ENTER(Stmt->Connection, "SQLExecute");
  MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);

  return Stmt->Methods->Execute(Stmt, FALSE);
}
/* }}} */

/* {{{ SQLExtendedFetch */
SQLRETURN SQL_API SQLExtendedFetch(SQLHSTMT StatementHandle,
    SQLUSMALLINT FetchOrientation,
    SQLLEN FetchOffset,
    SQLULEN *RowCountPtr,
    SQLUSMALLINT *RowStatusArray)
{
  SQLRETURN ret;
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

  SQLULEN *SaveRowsProcessedPtr= Stmt->Ird->Header.RowsProcessedPtr;
  SQLUSMALLINT *SaveArrayStatusPtr= Stmt->Ird->Header.ArrayStatusPtr;

  if (!Stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  MDBUG_C_ENTER(Stmt->Connection, "SQLExtendedFetch");
  MDBUG_C_DUMP(Stmt->Connection, FetchOrientation, u);
  MDBUG_C_DUMP(Stmt->Connection, FetchOffset, d);
  MDBUG_C_DUMP(Stmt->Connection, RowCountPtr, 0x);
  MDBUG_C_DUMP(Stmt->Connection, RowStatusArray, 0x);
   
  Stmt->Ird->Header.RowsProcessedPtr= RowCountPtr;
  Stmt->Ird->Header.ArrayStatusPtr= RowStatusArray;
  ret=  Stmt->Methods->FetchScroll(Stmt, FetchOrientation, FetchOffset);

  if (RowStatusArray && SaveArrayStatusPtr)
  {
    SQLUINTEGER i;
    for (i=0; i < Stmt->Ard->Header.ArraySize; i++)
      SaveArrayStatusPtr[i]= RowStatusArray[i];
  }

  Stmt->Ird->Header.RowsProcessedPtr= SaveRowsProcessedPtr;
  Stmt->Ird->Header.ArrayStatusPtr= SaveArrayStatusPtr;

  if (ret == SQL_NO_DATA)
  {
    if (RowCountPtr)
      *RowCountPtr= 0;
  }
  if (ret == SQL_ERROR)
    if (strcmp(Stmt->Error.SqlState, "22002") == 0)
      ret= SQL_SUCCESS_WITH_INFO;

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLFetch */
SQLRETURN SQL_API SQLFetch(SQLHSTMT StatementHandle)
{
  MADB_Stmt *Stmt;
  
  if (StatementHandle == SQL_NULL_HSTMT)
    return SQL_INVALID_HANDLE;

  Stmt= (MADB_Stmt *)StatementHandle;

  MDBUG_C_ENTER(Stmt->Connection, "SQLFetch");
  MADB_CLEAR_ERROR(&Stmt->Error);

  /* SQLFetch is equivalent of SQLFetchScroll(SQL_FETCH_NEXT), 3rd parameter is ignored for SQL_FETCH_NEXT */
  MDBUG_C_RETURN(Stmt->Connection, Stmt->Methods->FetchScroll(Stmt, SQL_FETCH_NEXT, 1), &Stmt->Error);
}
/* }}} */

/* {{{ SQLFetchScroll */
SQLRETURN SQL_API SQLFetchScroll(SQLHSTMT StatementHandle,
    SQLSMALLINT FetchOrientation,
    SQLLEN FetchOffset)
{
  MADB_Stmt *Stmt;

  if (StatementHandle == SQL_NULL_HSTMT)
    return SQL_INVALID_HANDLE;

  Stmt= (MADB_Stmt *)StatementHandle;

  MDBUG_C_ENTER(Stmt->Connection, "SQLFetchScroll");
  MDBUG_C_DUMP(Stmt->Connection, FetchOrientation, d);

  MADB_CLEAR_ERROR(&Stmt->Error);

  MDBUG_C_RETURN(Stmt->Connection, Stmt->Methods->FetchScroll(Stmt, FetchOrientation, FetchOffset), &Stmt->Error);
}
/* }}} */

/* {{{ SQLFreeHandle */
SQLRETURN SQL_API SQLFreeHandle(SQLSMALLINT HandleType,
                                SQLHANDLE Handle)
{
  SQLRETURN ret= SQL_INVALID_HANDLE;
  MADB_CHECK_HANDLE_CLEAR_ERROR(HandleType, Handle);

  switch (HandleType)
  {
  case SQL_HANDLE_ENV:
      MDBUG_ENTER("SQLFreeHandle");
      MDBUG_DUMP(HandleType, d);
      MDBUG_DUMP(Handle, 0x);

      ret= MADB_EnvFree((MADB_Env *)Handle);
      break;
  case SQL_HANDLE_DBC:
    {
      MADB_Dbc *Dbc= (MADB_Dbc *)Handle;

      MDBUG_C_ENTER(Dbc, "SQLFreeHandle");
      MDBUG_C_DUMP(Dbc, HandleType, d);
      MDBUG_C_DUMP(Dbc, Handle, 0x);

      ret= MADB_DbcFree(Dbc);
      return ret;
      /*MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);*/
    }
  case SQL_HANDLE_DESC:
    {
      MADB_Desc *Desc= (MADB_Desc *)Handle;
      MADB_Dbc  *Dbc=  Desc->Dbc;

      MDBUG_C_ENTER(Dbc, "SQLFreeHandle");
      MDBUG_C_DUMP(Dbc, HandleType, d);
      MDBUG_C_DUMP(Dbc, Handle, 0x);

      /* Error if the descriptor does not belong to application(was automatically alliocated by the driver)
         Basically DM is supposed to take care of this. Keeping in mind direct linking */
      if (!Desc->AppType)
      {
        MADB_SetError(&Desc->Error, MADB_ERR_HY017, NULL, 0);
        MDBUG_C_RETURN(Dbc, Desc->Error.ReturnValue, &Desc->Error);
      }
      ret= MADB_DescFree(Desc, FALSE);
      MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
    }
  case SQL_HANDLE_STMT:
    {
      MADB_Stmt *Stmt= (MADB_Stmt *)Handle;
      MADB_Dbc  *Dbc=  Stmt->Connection;

      MDBUG_C_ENTER(Dbc, "SQLFreeHandle");
      MDBUG_C_DUMP(Dbc, HandleType, d);
      MDBUG_C_DUMP(Dbc, Handle, 0x);

      ret= MA_SQLFreeStmt(Stmt, SQL_DROP);

      MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
    }
  }

  MDBUG_RETURN(ret);
}
/* }}} */

/* {{{ SQLFreeEnv */
SQLRETURN SQL_API SQLFreeEnv(SQLHANDLE henv)
{
  if (henv == SQL_NULL_HENV)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&((MADB_Env*)henv)->Error);

  return MADB_EnvFree((MADB_Env *)henv);
}
/* }}} */

/* {{{ SQLFreeConnect */
SQLRETURN SQL_API SQLFreeConnect(SQLHANDLE hdbc)
{
  if (hdbc == SQL_NULL_HDBC)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&((MADB_Dbc*)hdbc)->Error);

  return MADB_DbcFree((MADB_Dbc*)hdbc);
}
/* }}} */

/* {{{ SQLFreeStmt */
SQLRETURN MA_SQLFreeStmt(SQLHSTMT StatementHandle,
           SQLUSMALLINT Option)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  MDBUG_C_PRINT(Stmt->Connection, "%sMA_SQLFreeStmt","\t->");
  MDBUG_C_DUMP(Stmt->Connection, Stmt, 0x);
  MDBUG_C_DUMP(Stmt->Connection, Option, d);

  return Stmt->Methods->StmtFree(Stmt, Option);
}

SQLRETURN SQL_API SQLFreeStmt(SQLHSTMT StatementHandle,
           SQLUSMALLINT Option)
{
  if (StatementHandle== SQL_NULL_HSTMT)
    return SQL_INVALID_HANDLE;
  MDBUG_C_ENTER(((MADB_Stmt*)StatementHandle)->Connection, "SQLFreeStmt");
  MADB_CLEAR_ERROR(&((MADB_Stmt*)StatementHandle)->Error);

  return MA_SQLFreeStmt(StatementHandle, Option);
}
/* }}} */

/* {{{ SQLGetConnectAttr */
SQLRETURN MA_SQLGetConnectAttr(SQLHDBC ConnectionHandle,
    SQLINTEGER Attribute,
    SQLPOINTER ValuePtr,
    SQLINTEGER BufferLength,
    SQLINTEGER *StringLengthPtr)
{
  MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
  SQLRETURN ret;

  if (!Dbc)
    return SQL_INVALID_HANDLE;

  MDBUG_C_ENTER(Dbc, "SQLGetConnectAttr");
  MDBUG_C_DUMP(Dbc, Attribute, d);
  MDBUG_C_DUMP(Dbc, ValuePtr, 0x);
  MDBUG_C_DUMP(Dbc, BufferLength, d);
  MDBUG_C_DUMP(Dbc, StringLengthPtr, 0x);

  ret= Dbc->Methods->GetConnectAttr(Dbc, Attribute, ValuePtr, BufferLength, StringLengthPtr, FALSE);

  MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}

/* {{{ SQLGetData */
SQLRETURN SQL_API SQLGetData(SQLHSTMT StatementHandle,
    SQLUSMALLINT Col_or_Param_Num,
    SQLSMALLINT TargetType,
    SQLPOINTER TargetValuePtr,
    SQLLEN BufferLength,
    SQLLEN *StrLen_or_IndPtr)
{
  MADB_Stmt *Stmt= (MADB_Stmt*)StatementHandle;
  MADB_DescRecord *IrdRec;

  if (StatementHandle== SQL_NULL_HSTMT)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  /* In case we don't have DM(it check for that) */
  if (TargetValuePtr == NULL)
  {
    return MADB_SetError(&Stmt->Error, MADB_ERR_HY009, NULL, 0);
  }

  /* Bookmark */
  if (Col_or_Param_Num == 0)
  {
    return MADB_GetBookmark(Stmt, TargetType, TargetValuePtr, BufferLength, StrLen_or_IndPtr);
  }

  /* We don't need this to be checked in case of "internal" use of the GetData, i.e. for internal needs we should always get the data */
  if ( Stmt->CharOffset[Col_or_Param_Num - 1] > 0
    && Stmt->CharOffset[Col_or_Param_Num - 1] >= Stmt->Lengths[Col_or_Param_Num - 1])
  {
    return SQL_NO_DATA;
  }

  if (BufferLength < 0)
  {
    return MADB_SetError(&Stmt->Error, MADB_ERR_HY090, NULL, 0);
  }

  /* reset offsets for column. Doing that here since "internal" calls should not do that */
  if (Stmt->LastSQLGetDataColumn != Col_or_Param_Num)
  {
    IrdRec= MADB_DescGetInternalRecord(Stmt->Ird, Col_or_Param_Num - 1, MADB_DESC_READ);
    if (IrdRec)
    {
      MADB_FREE(IrdRec->InternalBuffer);
    }
    Stmt->CharOffset[Col_or_Param_Num - 1]= 0;
  }
  Stmt->LastSQLGetDataColumn= Col_or_Param_Num;

  return Stmt->Methods->GetData(StatementHandle, Col_or_Param_Num, TargetType, TargetValuePtr, BufferLength, StrLen_or_IndPtr, FALSE);
}
/* }}} */

/* {{{ SQLGetDiagRec */
SQLRETURN MA_SQLGetDiagRec(SQLSMALLINT HandleType,
    SQLHANDLE Handle,
    SQLSMALLINT RecNumber,
    SQLCHAR *SQLState,
    SQLINTEGER *NativeErrorPtr,
    SQLCHAR *MessageText,
    SQLSMALLINT BufferLength,
    SQLSMALLINT *TextLengthPtr)
{
  SQLRETURN ret= SQL_ERROR;

  if (!Handle)
    MDBUG_RETURN(SQL_INVALID_HANDLE);

  if (RecNumber < 1 || BufferLength < 0)
    MDBUG_RETURN(SQL_ERROR);

  /* Maria ODBC driver doesn't support error lists, so only the first record can be retrieved */
  if (RecNumber != 1)
    MDBUG_RETURN(SQL_NO_DATA_FOUND);
  
  switch (HandleType) {
    case SQL_HANDLE_DBC:
      {
        MADB_Dbc *Dbc= (MADB_Dbc *)Handle;

        MDBUG_C_ENTER(Dbc, "SQLGetDiagRec");
        MDBUG_C_DUMP(Dbc, HandleType, d);
        MDBUG_C_DUMP(Dbc, Handle, 0x);
        MDBUG_C_DUMP(Dbc, MessageText, 0x);
        MDBUG_C_DUMP(Dbc, BufferLength, d);
        MDBUG_C_DUMP(Dbc, TextLengthPtr, 0x);

        ret= MADB_GetDiagRec(&Dbc->Error, RecNumber, (void *)SQLState, NativeErrorPtr,
                              (void *) MessageText, BufferLength, TextLengthPtr, FALSE,
                              Dbc->Environment->OdbcVersion);
      }
      break;
    case SQL_HANDLE_STMT:
      {
        MADB_Stmt *Stmt= (MADB_Stmt *)Handle;

        MDBUG_C_ENTER(Stmt->Connection, "SQLGetDiagRec");
        MDBUG_C_DUMP(Stmt->Connection, HandleType, d);
        MDBUG_C_DUMP(Stmt->Connection, Handle, 0x);
        MDBUG_C_DUMP(Stmt->Connection, MessageText, 0x);
        MDBUG_C_DUMP(Stmt->Connection, BufferLength, d);
        MDBUG_C_DUMP(Stmt->Connection, TextLengthPtr, 0x);

        ret= MADB_GetDiagRec(&Stmt->Error, RecNumber, (void *)SQLState, NativeErrorPtr,
                               (void *)MessageText, BufferLength, TextLengthPtr, FALSE,
                               Stmt->Connection->Environment->OdbcVersion);
      }
      break;
    case SQL_HANDLE_DESC:
      {
        MADB_Desc *Desc= (MADB_Desc *)Handle;

        MDBUG_C_ENTER(Desc->Dbc, "SQLGetDiagRec");
        MDBUG_C_DUMP(Desc->Dbc, HandleType, d);
        MDBUG_C_DUMP(Desc->Dbc, Handle, 0x);
        MDBUG_C_DUMP(Desc->Dbc, MessageText, 0x);
        MDBUG_C_DUMP(Desc->Dbc, BufferLength, d);
        MDBUG_C_DUMP(Desc->Dbc, TextLengthPtr, 0x);

        ret= MADB_GetDiagRec(&Desc->Error, RecNumber, (void *)SQLState, NativeErrorPtr,
                               (void *)MessageText, BufferLength, TextLengthPtr, FALSE,
                               SQL_OV_ODBC3);
      }
      break;
    case SQL_HANDLE_ENV:
      {
        MADB_Env *Env= (MADB_Env *)Handle;
        ret= MADB_GetDiagRec(&Env->Error, RecNumber, (void *)SQLState, NativeErrorPtr,
                               (void *)MessageText, BufferLength, TextLengthPtr, FALSE,
                               Env->OdbcVersion);
      }
      break;
  }

  MDBUG_RETURN(ret);
}

/* {{{ SQLGetEnvAttr */
SQLRETURN SQL_API SQLGetEnvAttr(SQLHENV EnvironmentHandle,
    SQLINTEGER Attribute,
    SQLPOINTER ValuePtr,
    SQLINTEGER BufferLength,
    SQLINTEGER *StringLengthPtr)
{
  MADB_Env *Env= (MADB_Env *)EnvironmentHandle;
  SQLRETURN ret;

  MDBUG_ENTER("SQLGetEnvAttr");
  MDBUG_DUMP(Attribute, d);
  MDBUG_DUMP(ValuePtr, 0x);
  MDBUG_DUMP(BufferLength, d);
  MDBUG_DUMP(StringLengthPtr, 0x);

  if (!Env)
    ret= SQL_INVALID_HANDLE;
  else
  {
    MADB_CLEAR_ERROR(&Env->Error);
    ret= MADB_EnvGetAttr(Env, Attribute, ValuePtr, BufferLength, StringLengthPtr);
  }

  MDBUG_DUMP(ret, d);
  MDBUG_RETURN(ret);
}
/* {{{ SQLGetFunctions */
SQLRETURN SQL_API SQLGetFunctions(SQLHDBC ConnectionHandle,
    SQLUSMALLINT FunctionId,
    SQLUSMALLINT *SupportedPtr)
{
  MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
  SQLRETURN ret;

  if (!Dbc)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Dbc->Error);

  MDBUG_C_ENTER(Dbc, "SQLGetFunctions");
  MDBUG_C_DUMP(Dbc, FunctionId, d);
  MDBUG_C_DUMP(Dbc, SupportedPtr, 0x);
  ret= Dbc->Methods->GetFunctions(Dbc, FunctionId, SupportedPtr);

  MDBUG_C_RETURN(Dbc,ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLGetStmtAttr */
SQLRETURN MA_SQLGetStmtAttr(SQLHSTMT StatementHandle,
    SQLINTEGER Attribute,
    SQLPOINTER ValuePtr,
    SQLINTEGER BufferLength,
    SQLINTEGER *StringLengthPtr)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  if (!Stmt)
    return SQL_INVALID_HANDLE;
  return Stmt->Methods->GetStmtAttr(Stmt, Attribute, ValuePtr, BufferLength, StringLengthPtr);
}

/* {{{ SQLGetStmtOption */
SQLRETURN SQL_API SQLGetStmtOption(SQLHSTMT StatementHandle,
                                    SQLUSMALLINT Option, SQLPOINTER Value)
{
  if (StatementHandle == SQL_NULL_HSTMT)
  {
    return SQL_INVALID_HANDLE;
  }
  MADB_CLEAR_ERROR(&((MADB_Stmt*)StatementHandle)->Error);

  return MA_SQLGetStmtAttr(StatementHandle, Option, Value, SQL_NTS, (SQLINTEGER *)NULL);
}

/* }}} */

/* {{{ SQLMoreResults */
SQLRETURN SQL_API SQLMoreResults(SQLHSTMT StatementHandle)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  if (!Stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  return MADB_StmtMoreResults(Stmt);
}
/* }}} */

/* {{{ SQLNumParams */
SQLRETURN SQL_API SQLNumParams(SQLHSTMT StatementHandle,
    SQLSMALLINT *ParameterCountPtr)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

  MADB_CHECK_STMT_HANDLE(Stmt, stmt);
  MADB_CLEAR_ERROR(&Stmt->Error);

  return Stmt->Methods->ParamCount(Stmt, ParameterCountPtr);
}
/* }}} */

/* {{{ SQLNumResultCols */
SQLRETURN SQL_API SQLNumResultCols(SQLHSTMT StatementHandle,
    SQLSMALLINT *ColumnCountPtr)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  MADB_CHECK_STMT_HANDLE(Stmt, stmt);
  MADB_CLEAR_ERROR(&Stmt->Error);

  return Stmt->Methods->ColumnCount(Stmt, ColumnCountPtr);
}
/* }}} */

/* {{{ SQLParamData */
SQLRETURN SQL_API SQLParamData(SQLHSTMT StatementHandle,
    SQLPOINTER *ValuePtrPtr)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  if (!Stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  return Stmt->Methods->ParamData(Stmt, ValuePtrPtr);
}
/* }}} */

/* {{{ SQLPutData */
SQLRETURN SQL_API SQLPutData(SQLHSTMT StatementHandle,
    SQLPOINTER DataPtr,
    SQLLEN StrLen_or_Ind)
{
  MADB_Stmt *Stmt=(MADB_Stmt *)StatementHandle;
  SQLRETURN ret;

  if (!Stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  MDBUG_C_ENTER(Stmt->Connection, "SQLPutData");
  MDBUG_C_DUMP(Stmt->Connection, DataPtr, 0x);
  MDBUG_C_DUMP(Stmt->Connection, StrLen_or_Ind, d);

  ret= Stmt->Methods->PutData(Stmt, DataPtr, StrLen_or_Ind);

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLRowCount */
SQLRETURN SQL_API SQLRowCount(SQLHSTMT StatementHandle,
    SQLLEN *RowCountPtr)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;

  MADB_CHECK_STMT_HANDLE(Stmt, stmt);
  MADB_CLEAR_ERROR(&Stmt->Error);

  return Stmt->Methods->RowCount(Stmt, RowCountPtr);
}
/* }}} */

/* {{{ SQLSetConnectAttr */
SQLRETURN MA_SQLSetConnectAttr(SQLHDBC ConnectionHandle,
    SQLINTEGER Attribute,
    SQLPOINTER ValuePtr,
    SQLINTEGER StringLength)
{
  MADB_Dbc *Dbc= (MADB_Dbc *)ConnectionHandle;
  SQLRETURN ret;

  if (!Dbc)
    return SQL_INVALID_HANDLE;

  MDBUG_C_ENTER(Dbc, "SQLSetConnectAttr");
  MDBUG_C_DUMP(Dbc, Attribute, d);
  MDBUG_C_DUMP(Dbc, ValuePtr, 0x);
  MDBUG_C_DUMP(Dbc, StringLength, d);

  ret= Dbc->Methods->SetConnectAttr(Dbc, Attribute, ValuePtr, StringLength, FALSE);

  MDBUG_C_RETURN(Dbc, ret, &Dbc->Error);
}
/* }}} */

/* {{{ SQLSetEnvAttr */
SQLRETURN SQL_API SQLSetEnvAttr(SQLHENV EnvironmentHandle,
    SQLINTEGER Attribute,
    SQLPOINTER ValuePtr,
    SQLINTEGER StringLength)
{
  MADB_Env *Env= (MADB_Env *)EnvironmentHandle;
  SQLRETURN ret;
  MDBUG_ENTER("SQLSetEnvAttr");
  MDBUG_DUMP(Attribute, d);
  MDBUG_DUMP(ValuePtr, 0x);
  if (!Env)
    ret= SQL_INVALID_HANDLE;
  else
  {
    MADB_CLEAR_ERROR(&Env->Error);
    ret= MADB_EnvSetAttr(Env, Attribute, ValuePtr, StringLength);
  }
  MDBUG_DUMP(ret, d);
  MDBUG_RETURN(ret);
}
/* }}} */


/* {{{ SQLSetPos */
SQLRETURN SQL_API SQLSetPos(SQLHSTMT StatementHandle,
    SQLSETPOSIROW RowNumber,
    SQLUSMALLINT Operation,
    SQLUSMALLINT LockType)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  SQLRETURN ret;
  if (!Stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  MDBUG_C_ENTER(Stmt->Connection, "SQLSetPos");
  MDBUG_C_DUMP(Stmt->Connection, RowNumber, d);
  MDBUG_C_DUMP(Stmt->Connection, Operation, u);
  MDBUG_C_DUMP(Stmt->Connection, LockType, d);

  ret= Stmt->Methods->SetPos(Stmt, RowNumber, Operation, LockType, 0);

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}
/* }}} */

/* {{{ SQLSetParam */
SQLRETURN SQL_API SQLSetParam(SQLHSTMT stmt,
	                            SQLUSMALLINT par,
	                            SQLSMALLINT type,
	                            SQLSMALLINT sqltype,
	                            SQLULEN coldef,
	                            SQLSMALLINT scale,
	                            SQLPOINTER val,
	                            SQLLEN *nval)
{
  if (!stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&((MADB_Stmt*)stmt)->Error);
  return MA_SQLBindParameter(stmt, par, SQL_PARAM_INPUT_OUTPUT, type, sqltype, coldef,
                             scale, val, SQL_SETPARAM_VALUE_MAX, nval);
}
/* }}} */

/* {{{ SQLBindParam - we need it for direct linking mainly */
SQLRETURN  SQL_API SQLBindParam(SQLHSTMT StatementHandle,
                                SQLUSMALLINT ParameterNumber, SQLSMALLINT ValueType,
                                SQLSMALLINT ParameterType, SQLULEN LengthPrecision,
                                SQLSMALLINT ParameterScale, SQLPOINTER ParameterValue,
                                SQLLEN *StrLen_or_Ind)
{
  if (!StatementHandle)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&((MADB_Stmt*)StatementHandle)->Error);

  return MA_SQLBindParameter(StatementHandle, ParameterNumber, SQL_PARAM_INPUT, ValueType, ParameterType, LengthPrecision, ParameterScale,
                      ParameterValue, SQL_SETPARAM_VALUE_MAX, StrLen_or_Ind);

}
/* }}} */

/* {{{ SQLSetStmtAttr */
SQLRETURN MA_SQLSetStmtAttr(SQLHSTMT StatementHandle,
    SQLINTEGER Attribute,
    SQLPOINTER ValuePtr,
    SQLINTEGER StringLength)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)StatementHandle;
  SQLRETURN ret;

  if (!Stmt)
    return SQL_INVALID_HANDLE;

  MDBUG_C_ENTER(Stmt->Connection, "SQLSetStmtAttr");
  MDBUG_C_DUMP(Stmt->Connection, Attribute, d);
  MDBUG_C_DUMP(Stmt->Connection, ValuePtr, 0x);
  MDBUG_C_DUMP(Stmt->Connection, StringLength, d);

  ret= Stmt->Methods->SetStmtAttr(Stmt, Attribute, ValuePtr, StringLength);

  MDBUG_C_RETURN(Stmt->Connection, ret, &Stmt->Error);
}

/* {{{ SQLSetStmtOption */
SQLRETURN SQL_API SQLSetStmtOption(SQLHSTMT StatementHandle,
                                    SQLUSMALLINT Option, SQLULEN Value)
{
  if (StatementHandle == SQL_NULL_HSTMT)
  {
    return SQL_INVALID_HANDLE;
  }
  MADB_CLEAR_ERROR(&((MADB_Stmt*)StatementHandle)->Error);
  return MA_SQLSetStmtAttr(StatementHandle, Option, (SQLPOINTER)Value, SQL_NTS);
}
/* }}} */

/* {{{ SQLSetScrollOptions */
SQLRETURN SQL_API SQLSetScrollOptions(SQLHSTMT     hstmt,
                                      SQLUSMALLINT Concurrency,
                                      SQLLEN       crowKeySet,
                                      SQLUSMALLINT crowRowSet)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)hstmt;
  if (!Stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  return MADB_DescSetField(Stmt->Ard, 0, SQL_DESC_ARRAY_SIZE, (SQLPOINTER)(SQLULEN)crowKeySet, SQL_IS_USMALLINT, 0);
}
/* }}} */

/* {{{ SQLParamOptions */
SQLRETURN SQL_API SQLParamOptions(
    SQLHSTMT hstmt,
    SQLULEN  crow,
    SQLULEN  *pirow)
{
  MADB_Stmt *Stmt= (MADB_Stmt *)hstmt;
  SQLRETURN result;
  if (!Stmt)
    return SQL_INVALID_HANDLE;
  MADB_CLEAR_ERROR(&Stmt->Error);

  result= MADB_DescSetField(Stmt->Apd, 0, SQL_DESC_ARRAY_SIZE, (SQLPOINTER)crow, SQL_IS_UINTEGER, 0);

  if (SQL_SUCCEEDED(result))
  {
    result= MADB_DescSetField(Stmt->Ipd, 0, SQL_DESC_ROWS_PROCESSED_PTR, (SQLPOINTER)pirow, SQL_IS_POINTER, 0);
  }

  return result;
}
/* }}} */
