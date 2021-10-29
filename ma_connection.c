/************************************************************************************
   Copyright (C) 2013,2019 MariaDB Corporation AB
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
#include <keywords/keywords.hpp>
#include <ma_odbc.h>
extern const char* DefaultPluginLocation;

struct st_madb_isolation MADB_IsolationLevel[] =
{
  {SQL_TRANSACTION_REPEATABLE_READ, "REPEATABLE READ"},
  {SQL_TRANSACTION_READ_COMMITTED, "READ COMMITTED"},
  {SQL_TRANSACTION_READ_UNCOMMITTED, "READ UNCOMMITTED"},
  {SQL_TRANSACTION_SERIALIZABLE, "SERIALIZABLE"},
  {0, 0}
};

/* used by SQLGetFunctions */
SQLUSMALLINT MADB_supported_api[]=
{
  SQL_API_SQLALLOCCONNECT,
  SQL_API_SQLALLOCENV,
  SQL_API_SQLALLOCHANDLE,
  SQL_API_SQLALLOCSTMT,
  SQL_API_SQLBINDCOL,
  SQL_API_SQLBINDPARAM,
  SQL_API_SQLCANCEL,
  SQL_API_SQLCLOSECURSOR,
  SQL_API_SQLCOLATTRIBUTE,
  SQL_API_SQLCOLUMNS,
  SQL_API_SQLCONNECT,
  SQL_API_SQLCOPYDESC,
  SQL_API_SQLDATASOURCES,
  SQL_API_SQLDESCRIBECOL,
  SQL_API_SQLDISCONNECT,
  SQL_API_SQLENDTRAN,
  SQL_API_SQLERROR,
  SQL_API_SQLEXECDIRECT,
  SQL_API_SQLEXECUTE,
  SQL_API_SQLFETCH,
  SQL_API_SQLFETCHSCROLL,
  SQL_API_SQLFREECONNECT,
  SQL_API_SQLFREEENV,
  SQL_API_SQLFREEHANDLE,
  SQL_API_SQLFREESTMT,
  SQL_API_SQLGETCONNECTATTR,
  SQL_API_SQLGETCONNECTOPTION,
  SQL_API_SQLGETCURSORNAME,
  SQL_API_SQLGETDATA,
  SQL_API_SQLGETDESCFIELD,
  SQL_API_SQLGETDESCREC,
  SQL_API_SQLGETDIAGFIELD,
  SQL_API_SQLGETDIAGREC,
  SQL_API_SQLGETENVATTR,
  SQL_API_SQLGETFUNCTIONS,
  SQL_API_SQLGETINFO,
  SQL_API_SQLGETSTMTATTR,
  SQL_API_SQLGETSTMTOPTION,
  SQL_API_SQLGETTYPEINFO,
  SQL_API_SQLNUMRESULTCOLS,
  SQL_API_SQLPARAMDATA,
  SQL_API_SQLPREPARE,
  SQL_API_SQLPUTDATA,
  SQL_API_SQLROWCOUNT,
  SQL_API_SQLSETCONNECTATTR,
  SQL_API_SQLSETCONNECTOPTION,
  SQL_API_SQLSETCURSORNAME,
  SQL_API_SQLSETDESCFIELD,
  // SQL_API_SQLSETDESCREC, TODO PLAT-5516
  SQL_API_SQLSETENVATTR,
  SQL_API_SQLSETPARAM,
  SQL_API_SQLSETSTMTATTR,
  SQL_API_SQLSETSTMTOPTION,
  SQL_API_SQLSPECIALCOLUMNS,
  SQL_API_SQLSTATISTICS,
  SQL_API_SQLTABLES,
  SQL_API_SQLTRANSACT,
  SQL_API_SQLBULKOPERATIONS,
  SQL_API_SQLBINDPARAMETER,
  // SQL_API_SQLBROWSECONNECT, TODO PLAT-5515
  SQL_API_SQLCOLATTRIBUTES,
  SQL_API_SQLCOLUMNPRIVILEGES ,
  SQL_API_SQLDESCRIBEPARAM,
  SQL_API_SQLDRIVERCONNECT,
  SQL_API_SQLDRIVERS,
  SQL_API_SQLEXTENDEDFETCH,
  SQL_API_SQLMORERESULTS,
  SQL_API_SQLNATIVESQL,
  SQL_API_SQLNUMPARAMS,
  SQL_API_SQLPARAMOPTIONS,
  SQL_API_SQLPRIMARYKEYS,
  SQL_API_SQLPROCEDURECOLUMNS,
  SQL_API_SQLPROCEDURES,
  SQL_API_SQLSETPOS,
  SQL_API_SQLSETSCROLLOPTIONS,
  SQL_API_SQLTABLEPRIVILEGES,
  // SQL_API_SQLCANCELHANDLE TODO PLAT-5514
};


struct st_ma_connection_methods MADB_Dbc_Methods; /* declared at the end of file */


my_bool CheckConnection(MADB_Dbc *Dbc)
{
  if (!Dbc->mariadb)
    return FALSE;
  if (mysql_get_socket(Dbc->mariadb) == MARIADB_INVALID_SOCKET)
  {
    /* Check if reconnect option was set */
    if (DSN_OPTION(Dbc, MADB_OPT_FLAG_AUTO_RECONNECT))
    {
      if (!mysql_ping(Dbc->mariadb))
        return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}

/* {{{ MADB_DbcSetAttr */
SQLRETURN MADB_DbcSetAttr(MADB_Dbc *Dbc, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER StringLength, my_bool isWChar)
{
  MADB_CLEAR_ERROR(&Dbc->Error);

  if (!Dbc)
  {
    /* Todo: check */
    if (Attribute != SQL_ATTR_TRACE &&
        Attribute != SQL_ATTR_TRACEFILE)
      return SQL_INVALID_HANDLE;
    return SQL_SUCCESS;
  } 

  switch(Attribute) {
  case SQL_ATTR_ACCESS_MODE:
    if ((SQLPOINTER)SQL_MODE_READ_WRITE != ValuePtr)
      MADB_SetError(&Dbc->Error, MADB_ERR_01S02, NULL, 0);
    Dbc->AccessMode= SQL_MODE_READ_WRITE;
    break;
#if (ODBCVER >= 0x0351)
  case SQL_ATTR_ANSI_APP:
    if (ValuePtr != NULL)
    {
      Dbc->IsAnsi= 1;
      Dbc->ConnOrSrcCharset= &SourceAnsiCs;
      CopyClientCharset(&SourceAnsiCs, &Dbc->Charset);
    }
    else
    {
      Dbc->IsAnsi= 0;
    }
    break;
#endif
  case SQL_ATTR_ASYNC_ENABLE:
     if ((SQLPOINTER)SQL_ASYNC_ENABLE_OFF != ValuePtr)
      MADB_SetError(&Dbc->Error, MADB_ERR_01S02, NULL, 0);
     Dbc->AsyncEnable= SQL_ASYNC_ENABLE_OFF;
    break;
  case SQL_ATTR_AUTO_IPD:
    /* read only */
    MADB_SetError(&Dbc->Error, MADB_ERR_HY092, NULL, 0);
    break;
  case SQL_ATTR_AUTOCOMMIT:
    {
      SQLULEN ValidAttrs[]= {2, SQL_AUTOCOMMIT_ON, SQL_AUTOCOMMIT_OFF};
      MADB_CHECK_ATTRIBUTE(Dbc, ValuePtr, ValidAttrs);
      /* if a connection is open, try to apply setting to the connection */
      if (Dbc->mariadb)
      {
        if (Dbc->EnlistInDtc) {
          return MADB_SetError(&Dbc->Error, MADB_ERR_25000, NULL, 0);
        }
        if (mysql_autocommit(Dbc->mariadb, (my_bool)(size_t)ValuePtr))
        {
          return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, mysql_error(Dbc->mariadb), mysql_errno(Dbc->mariadb));
        }
      }
      Dbc->AutoCommit= (SQLUINTEGER)(SQLULEN)ValuePtr;
    }
    break;
  case SQL_ATTR_CONNECTION_DEAD:
    /* read only! */
    return MADB_SetError(&Dbc->Error, MADB_ERR_HY092, NULL, 0);
  case SQL_ATTR_CURRENT_CATALOG:
    {
      MADB_FREE(Dbc->CatalogName);
      if (isWChar)
      {
        /* IsAnsi will be set before this, even if it is set before connection */
        Dbc->CatalogName= MADB_ConvertFromWChar((SQLWCHAR *)ValuePtr, StringLength, NULL, Dbc->ConnOrSrcCharset, NULL);
      }
      else {
          ADJUST_LENGTH(ValuePtr, StringLength);
          Dbc->CatalogName= strndup((char *)ValuePtr, StringLength);
      }
      if (Dbc->mariadb &&
          mysql_select_db(Dbc->mariadb, Dbc->CatalogName))
      {
        return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, mysql_error(Dbc->mariadb), mysql_errno(Dbc->mariadb));
      }
    }
    break;
  case SQL_ATTR_LOGIN_TIMEOUT:
    Dbc->LoginTimeout= (SQLUINTEGER)(SQLULEN)ValuePtr;
    break;
  case SQL_ATTR_METADATA_ID:
    Dbc->MetadataId= (SQLUINTEGER)(SQLULEN)ValuePtr;
    break;
  case SQL_ATTR_ODBC_CURSORS:
    {
      SQLULEN ValidAttrs[]= {3, SQL_CUR_USE_IF_NEEDED, SQL_CUR_USE_ODBC, SQL_CUR_USE_DRIVER};
      MADB_CHECK_ATTRIBUTE(Dbc, ValuePtr, ValidAttrs);
      if ((SQLULEN)ValuePtr != SQL_CUR_USE_ODBC)
        MADB_SetError(&Dbc->Error, MADB_ERR_01S02, NULL, 0);
      Dbc->OdbcCursors= SQL_CUR_USE_ODBC;
    }
    break;
  case SQL_ATTR_ENLIST_IN_DTC:
    /* MS Distributed Transaction Coordinator not supported */
    return MADB_SetError(&Dbc->Error, MADB_ERR_HYC00, NULL, 0);
  case SQL_ATTR_PACKET_SIZE:
    /* if connection was made, return HY001 */
    if (Dbc->mariadb)
    {
      return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
    }
    Dbc->PacketSize= (SQLUINTEGER)(SQLULEN)ValuePtr;
    break;
  case SQL_ATTR_QUIET_MODE:
    Dbc->QuietMode= (HWND)ValuePtr;
    break;
  case SQL_ATTR_TRACE:
    break;
  case SQL_ATTR_TRACEFILE:
    break;
  case SQL_ATTR_TRANSLATE_LIB:
    break;
  case SQL_ATTR_TRANSLATE_OPTION:
    break;
  case SQL_ATTR_TXN_ISOLATION:
    if (Dbc->mariadb)
    {
      my_bool ValidTx= FALSE;
      unsigned int i;
      for (i=0; i < 4; i++)
      {
        if (MADB_IsolationLevel[i].SqlIsolation == (SQLLEN)ValuePtr)
        {
          char StmtStr[128];
          _snprintf(StmtStr, sizeof(StmtStr), "SET SESSION TRANSACTION ISOLATION LEVEL %s",
                      MADB_IsolationLevel[i].StrIsolation);
          LOCK_MARIADB(Dbc);
          if (mysql_query(Dbc->mariadb, StmtStr))
          {
            UNLOCK_MARIADB(Dbc);
            return MADB_SetError(&Dbc->Error, MADB_ERR_HY001, mysql_error(Dbc->mariadb), mysql_errno(Dbc->mariadb));
          }
          UNLOCK_MARIADB(Dbc);
          ValidTx= TRUE;
          break;
        }
      }
      if (!ValidTx)
      {
        return MADB_SetError(&Dbc->Error, MADB_ERR_HY024, NULL, 0);
      }
    }
    Dbc->TxnIsolation= (SQLINTEGER)(SQLLEN)ValuePtr;
    break;
  default:
    break;
  }
  return Dbc->Error.ReturnValue;
}
/* }}} */

/* {{{ MADB_DbcHetAttr */
SQLRETURN MADB_DbcGetAttr(MADB_Dbc *Dbc, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER BufferLength, SQLINTEGER *StringLengthPtr, my_bool isWChar)
{
  MADB_CLEAR_ERROR(&Dbc->Error);

  if (!Dbc)
    return SQL_INVALID_HANDLE;

  if (!ValuePtr && Attribute != SQL_ATTR_CURRENT_CATALOG)
    return SQL_SUCCESS;
  if (Attribute == SQL_ATTR_CURRENT_CATALOG && !StringLengthPtr && 
      (!ValuePtr || !BufferLength))
  {
    return MADB_SetError(&Dbc->Error, MADB_ERR_01004, NULL, 0);
  }

  switch(Attribute) {
  case SQL_ATTR_ACCESS_MODE:
    *(SQLUINTEGER *)ValuePtr= SQL_MODE_READ_WRITE;
    break;
  case SQL_ATTR_ASYNC_ENABLE:
    *(SQLULEN *)ValuePtr= SQL_ASYNC_ENABLE_OFF;
    break;
  case SQL_ATTR_AUTO_IPD:
    *(SQLUINTEGER *)ValuePtr= SQL_FALSE;
    break;
  case SQL_ATTR_AUTOCOMMIT:
    *(SQLUINTEGER *)ValuePtr= Dbc->AutoCommit;
    break;
  case SQL_ATTR_CONNECTION_DEAD:
    /* ping may fail if status isn't ready, so we need to check errors */
    if (mysql_ping(Dbc->mariadb))
      *(SQLUINTEGER *)ValuePtr= (mysql_errno(Dbc->mariadb) == CR_SERVER_GONE_ERROR ||
                                 mysql_errno(Dbc->mariadb) == CR_SERVER_LOST) ? SQL_CD_TRUE : SQL_CD_FALSE;
    else
      *(SQLUINTEGER *)ValuePtr= SQL_CD_FALSE;
    break;
  case SQL_ATTR_CURRENT_CATALOG:
  {
    SQLSMALLINT StrLen;
    SQLRETURN   ret;

    ret= MADB_Dbc_GetCurrentDB(Dbc, ValuePtr, BufferLength, &StrLen, isWChar);
    /* if we weren't able to determine the current db, we will return the cached catalog name */
    if (!SQL_SUCCEEDED(ret) && Dbc->CatalogName)
    {
      MADB_CLEAR_ERROR(&Dbc->Error);
      StrLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : 0, ValuePtr, BufferLength, 
                                          Dbc->CatalogName, strlen(Dbc->CatalogName), &Dbc->Error);
      ret= SQL_SUCCESS;
    }
    if (StringLengthPtr != NULL)
    {
      *StringLengthPtr= (SQLINTEGER)StrLen;
    }
    return ret;
  }
  case SQL_ATTR_LOGIN_TIMEOUT:
    *(SQLUINTEGER *)ValuePtr= Dbc->LoginTimeout;
    break;
  case SQL_ATTR_CONNECTION_TIMEOUT:
    *(SQLUINTEGER *)ValuePtr= 0;
    break;
  case SQL_ATTR_METADATA_ID:
    *(SQLUINTEGER *)ValuePtr= Dbc->MetadataId;
  case SQL_ATTR_ODBC_CURSORS:
    *(SQLINTEGER *)ValuePtr= SQL_CUR_USE_ODBC;
    break;
  case SQL_ATTR_ENLIST_IN_DTC:
    /* MS Distributed Transaction Coordinator not supported */
    MADB_SetError(&Dbc->Error, MADB_ERR_HYC00, NULL, 0);
    break;
  case SQL_ATTR_PACKET_SIZE:
    {
      unsigned long packet_size= 0;
      mysql_get_option(Dbc->mariadb, MYSQL_OPT_NET_BUFFER_LENGTH, &packet_size);
      *(SQLINTEGER *)ValuePtr= (SQLINTEGER)packet_size;
    }
    break;
  case SQL_ATTR_QUIET_MODE:
    Dbc->QuietMode= (HWND)ValuePtr;
    break;
  case SQL_ATTR_TRACE:
    break;
  case SQL_ATTR_TRACEFILE:
    break;
  case SQL_ATTR_ANSI_APP:
    *(SQLUINTEGER *)ValuePtr= SQL_SUCCESS;
    break;
  case SQL_ATTR_TRANSLATE_LIB:
    break;
  case SQL_ATTR_TRANSLATE_OPTION:
    break;
  case SQL_ATTR_TXN_ISOLATION:
    /* TxnIsolation wasn't set before we retrieve it from open connection or
       assume a default of REPETABLE_READ */
    if (!Dbc->TxnIsolation)
    {
      *(SQLULEN *)ValuePtr= SQL_TRANSACTION_REPEATABLE_READ;
      if (Dbc->mariadb)
      {
        MYSQL_RES *result;
        MYSQL_ROW row;
        const char *StmtString= "SELECT VARIABLE_VALUE FROM INFORMATION_SCHEMA.SESSION_VARIABLES WHERE VARIABLE_NAME='TX_ISOLATION'";

        LOCK_MARIADB(Dbc);
        if (mysql_query(Dbc->mariadb, StmtString))
        {
          UNLOCK_MARIADB(Dbc);
          MADB_SetNativeError(&Dbc->Error, SQL_HANDLE_DBC, Dbc->mariadb);
          return Dbc->Error.ReturnValue;
        }
        result= mysql_store_result(Dbc->mariadb);
        UNLOCK_MARIADB(Dbc);
        if ((row = mysql_fetch_row(result)))
        {
          unsigned int i;
          for (i=0; i < 4; i++)
            if (!strcmp(row[0], MADB_IsolationLevel[i].StrIsolation))
            {
              *(SQLULEN *)ValuePtr= MADB_IsolationLevel[i].SqlIsolation;
              break;
            }
        }
        mysql_free_result(result);
      }
    }
    else 
      *(SQLULEN *)ValuePtr= Dbc->TxnIsolation;
    break;

  default:
    MADB_SetError(&Dbc->Error, MADB_ERR_HYC00, NULL, 0);
    break;
  }
  return Dbc->Error.ReturnValue;
}
/* }}} */


/* {{{ MADB_DbcInit() */
MADB_Dbc *MADB_DbcInit(MADB_Env *Env)
{
  MADB_Dbc *Connection= NULL;

  MADB_CLEAR_ERROR(&Env->Error);

  if (!(Connection = (MADB_Dbc *)MADB_CALLOC(sizeof(MADB_Dbc))))
    goto cleanup;

  Connection->AutoCommit= 4;
  Connection->Environment= Env;
  Connection->Methods= &MADB_Dbc_Methods;
  //CopyClientCharset(&SourceAnsiCs, &Connection->Charset);
  InitializeCriticalSection(&Connection->cs);
  InitializeCriticalSection(&Connection->ListsCs);
  /* Not sure that critical section is really needed here - this init routine is called when
     no one has the handle yet */
  EnterCriticalSection(&Connection->Environment->cs);

  /* Save connection in Environment list */
  Connection->ListItem.data= (void *)Connection;
  Connection->Environment->Dbcs= MADB_ListAdd(Connection->Environment->Dbcs, &Connection->ListItem);

  LeaveCriticalSection(&Connection->Environment->cs);

  MADB_PutErrorPrefix(NULL, &Connection->Error);

  return Connection;      
cleanup:
  if (Connection)
    free(Connection);
  else
    MADB_SetError(&Env->Error, MADB_ERR_HY001, NULL, 0);

  return NULL;
}
/* }}} */

/* {{{ MADB_DbcFree() */
SQLRETURN MADB_DbcFree(MADB_Dbc *Connection)
{
  MADB_Env *Env= NULL;

  if (!Connection)
    return SQL_ERROR;
  MDBUG_C_PRINT(Connection, "%sMADB_DbcFree", "\t->");
  MDBUG_C_DUMP(Connection, Connection, 0x);

  Env= Connection->Environment;

  /* TODO: If somebody uses connection it won't help if lock it here. At least it requires
           more fingers movements
    LOCK_MARIADB(Dbc);*/
  if (Connection->mariadb)
  {
    mysql_close(Connection->mariadb);
    Connection->mariadb= NULL;
  }
  /*UNLOCK_MARIADB(Dbc);*/

  /* todo: delete all descriptors */

  EnterCriticalSection(&Env->cs);
  Connection->Environment->Dbcs= MADB_ListDelete(Connection->Environment->Dbcs, &Connection->ListItem);
  LeaveCriticalSection(&Env->cs);

  MADB_FREE(Connection->CatalogName);
  CloseClientCharset(&Connection->Charset);
  MADB_FREE(Connection->DataBase);
  MADB_DSN_Free(Connection->Dsn);
  DeleteCriticalSection(&Connection->cs);

  free(Connection);
  return SQL_SUCCESS;
}
/* }}} */

/* {{{ MADB_Dbc_GetCurrentDB */
SQLRETURN MADB_Dbc_GetCurrentDB(MADB_Dbc *Connection, SQLPOINTER CurrentDB, SQLINTEGER CurrentDBLength, 
                                SQLSMALLINT *StringLengthPtr, my_bool isWChar) 
{
  MADB_Stmt *Stmt;
  SQLRETURN ret;
  SQLLEN Size;
  char Buffer[65 * sizeof(SQLWCHAR)];

  MADB_CLEAR_ERROR(&Connection->Error);
  ret= MA_SQLAllocHandle(SQL_HANDLE_STMT, (SQLHANDLE) Connection, (SQLHANDLE*)&Stmt);
  if (!SQL_SUCCEEDED(ret))
    return ret;

  // Currently we have to check if DATABASE() != '' because SingleStore returns an empty string (rather than NULL)
  // for a connection with no database.
  char* currentDbQuery = "SELECT IF(DATABASE() IS NOT NULL AND DATABASE() != '',DATABASE(),'null')";
  if (!SQL_SUCCEEDED(Stmt->Methods->ExecDirect(Stmt, currentDbQuery, SQL_NTS)) ||
      !SQL_SUCCEEDED(Stmt->Methods->Fetch(Stmt)))
  {
    MADB_CopyError(&Connection->Error, &Stmt->Error);
    goto end;
  }
  
  ret= Stmt->Methods->GetData(Stmt, 1, SQL_CHAR, Buffer, 65, &Size, TRUE);

  Size= (SQLSMALLINT)MADB_SetString(isWChar ? & Connection->Charset : 0, 
                                     (void *)CurrentDB, BUFFER_CHAR_LEN(CurrentDBLength, isWChar), Buffer,
                                     SQL_NTS, &Connection->Error);
  if (StringLengthPtr)
    *StringLengthPtr= isWChar ? (SQLSMALLINT)Size * sizeof(SQLWCHAR) : (SQLSMALLINT)Size;
  
end:
  MA_SQLFreeStmt(Stmt, SQL_DROP);
  return Connection->Error.ReturnValue;
}
/* }}} */

/* {{{ MADB_DbcEndTran */
SQLRETURN MADB_DbcEndTran(MADB_Dbc *Dbc, SQLSMALLINT CompletionType)
{
  MADB_CLEAR_ERROR(&Dbc->Error);
  if (!Dbc)
    return SQL_INVALID_HANDLE;

  LOCK_MARIADB(Dbc);
  switch (CompletionType) {
  case SQL_ROLLBACK:
    if (Dbc->mariadb && mysql_rollback(Dbc->mariadb))
      MADB_SetNativeError(&Dbc->Error, SQL_HANDLE_DBC, Dbc->mariadb);
    break;
  case SQL_COMMIT:
    if (Dbc->mariadb && mysql_commit(Dbc->mariadb))
      MADB_SetNativeError(&Dbc->Error, SQL_HANDLE_DBC, Dbc->mariadb);
    break;
  default:
    MADB_SetError(&Dbc->Error, MADB_ERR_HY012, NULL, 0);
  }
  UNLOCK_MARIADB(Dbc);

  return Dbc->Error.ReturnValue;
}
/* }}} */

/* {{{ MADB_Dbc_ConnectDB
       Mind that this function is used for establishing connection from the setup lib
*/
SQLRETURN MADB_DbcConnectDB(MADB_Dbc *Connection,
    MADB_Dsn *Dsn)
{
  char StmtStr[128];
  unsigned ReportDataTruncation= 1;
  unsigned int i;
  unsigned long client_flags= 0L;
  my_bool my_reconnect= 1;
  
  if (!Connection || !Dsn)
    return SQL_ERROR;

  MADB_CLEAR_ERROR(&Connection->Error);

  if (Connection->mariadb == NULL)
  {
    if (!(Connection->mariadb= mysql_init(NULL)))
    {
      MADB_SetError(&Connection->Error, MADB_ERR_HY001, NULL, 0);
      goto end;
    }
  }

  if( !MADB_IS_EMPTY(Dsn->ConnCPluginsDir))
  {
    mysql_optionsv(Connection->mariadb, MYSQL_PLUGIN_DIR, Dsn->ConnCPluginsDir);
  }
  else
  {
    if (DefaultPluginLocation != NULL)
    {
      mysql_optionsv(Connection->mariadb, MYSQL_PLUGIN_DIR, DefaultPluginLocation);
    }
  }

  if (Dsn->ReadMycnf != '\0')
  {
    mysql_optionsv(Connection->mariadb, MYSQL_READ_DEFAULT_GROUP, (void *)"odbc");
  }
  /* If a client character set was specified in DSN, we will always use it.
     Otherwise for ANSI applications we will use the current character set,
     for unicode connections we use utf8
  */
  {
    const char* cs_name= NULL;

    if (!MADB_IS_EMPTY(Dsn->CharacterSet))
    {
     cs_name= Dsn->CharacterSet;
    }
    else if (Connection->IsAnsi)
    {
      MARIADB_CHARSET_INFO *cs= mariadb_get_charset_by_name("auto");
      cs_name= cs->csname;
    }

    // Use utf8mb3 by default if no specific charset was provided by the client.
    if (InitClientCharset(&Connection->Charset, MADB_IS_EMPTY(cs_name) ? "utf8" : cs_name))
    {
      /* Memory allocation error */
      MADB_SetError(&Connection->Error, MADB_ERR_HY001, "Charset is not supported", 0);
      goto end;
    }
    if (iOdbc() && strcmp(Connection->Charset.cs_info->csname, "swe7") == 0)
    {
      MADB_SetError(&Connection->Error, MADB_ERR_HY001, "Charset SWE7 is not supported with iODBC", 0);
      goto end;

    }
    if (!Connection->IsAnsi || iOdbc())
    {
      /* If application is not ansi, we should convert wchar into connection string */
      Connection->ConnOrSrcCharset= &Connection->Charset;
    }
  }

  /* todo: error handling */
  mysql_optionsv(Connection->mariadb, MYSQL_SET_CHARSET_NAME, Connection->Charset.cs_info->csname);

  if (Dsn->InitCommand && Dsn->InitCommand[0])
    mysql_optionsv(Connection->mariadb, MYSQL_INIT_COMMAND, Dsn->InitCommand);
 
  if (Dsn->ConnectionTimeout)
    mysql_optionsv(Connection->mariadb, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&Dsn->ConnectionTimeout);

  Connection->Options= Dsn->Options;

  if (DSN_OPTION(Connection, MADB_OPT_FLAG_AUTO_RECONNECT))
    mysql_optionsv(Connection->mariadb, MYSQL_OPT_RECONNECT, &my_reconnect);

  if (Dsn->IsNamedPipe) /* DSN_OPTION(Connection, MADB_OPT_FLAG_NAMED_PIPE) */
    mysql_optionsv(Connection->mariadb, MYSQL_OPT_NAMED_PIPE, (void *)Dsn->ServerName);

  if (DSN_OPTION(Connection, MADB_OPT_FLAG_NO_SCHEMA))
    client_flags|= CLIENT_NO_SCHEMA;
  if (DSN_OPTION(Connection, MADB_OPT_FLAG_IGNORE_SPACE))
    client_flags|= CLIENT_IGNORE_SPACE;

  if (DSN_OPTION(Connection, MADB_OPT_FLAG_FOUND_ROWS))
    client_flags|= CLIENT_FOUND_ROWS;
  if (DSN_OPTION(Connection, MADB_OPT_FLAG_COMPRESSED_PROTO))
    client_flags|= CLIENT_COMPRESS;
  if (DSN_OPTION(Connection, MADB_OPT_FLAG_MULTI_STATEMENTS))
    client_flags|= CLIENT_MULTI_STATEMENTS;

  if (Dsn->InteractiveClient)
  {
    mysql_optionsv(Connection->mariadb, MARIADB_OPT_INTERACTIVE, 1);
  }
  /* enable truncation reporting */
  mysql_optionsv(Connection->mariadb, MYSQL_REPORT_DATA_TRUNCATION, &ReportDataTruncation);

  if (Dsn->Socket)
  {
    int protocol= MYSQL_PROTOCOL_SOCKET;
    mysql_optionsv(Connection->mariadb, MYSQL_OPT_PROTOCOL, (void*)&protocol);
  }

  {
    /* I don't think it's possible to have empty strings or only spaces in the string here, but I prefer
       to have this paranoid check to make sure we dont' them */
    const char *SslKey=    ltrim(Dsn->SslKey);
    const char *SslCert=   ltrim(Dsn->SslCert);
    const char *SslCa=     ltrim(Dsn->SslCa);
    const char *SslCaPath= ltrim(Dsn->SslCaPath);
    const char *SslCipher= ltrim(Dsn->SslCipher);

    if (!MADB_IS_EMPTY(SslCa)
     || !MADB_IS_EMPTY(SslCaPath)
     || !MADB_IS_EMPTY(SslCipher)
     || !MADB_IS_EMPTY(SslCert)
     || !MADB_IS_EMPTY(SslKey))
    {
      char Enable= 1;
      mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_ENFORCE, &Enable);

      if (!MADB_IS_EMPTY(SslKey))
      {
        mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_KEY, SslKey);
      }
      if (!MADB_IS_EMPTY(SslCert))
      {
        mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_CERT, SslCert);
      }
      if (!MADB_IS_EMPTY(SslCa))
      {
        mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_CA, SslCa);
      }
      if (!MADB_IS_EMPTY(SslCaPath))
      {
        mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_CAPATH, SslCaPath);
      }
      if (!MADB_IS_EMPTY(SslCipher))
      {
        mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_CIPHER, SslCipher);
      }

      if (Dsn->TlsVersion > 0)
      {
        char TlsVersion[sizeof(TlsVersionName) + sizeof(TlsVersionBits) - 1], *Ptr= TlsVersion; /* All names + (n-1) comma */
        unsigned int i, NeedComma= 0;

        for (i= 0; i < sizeof(TlsVersionBits); ++i)
        {
          if (Dsn->TlsVersion & TlsVersionBits[i])
          {
            if (NeedComma != 0)
            {
              *Ptr++= ',';
            }
            else
            {
              NeedComma= 1;
            }
            strcpy(Ptr, TlsVersionName[i]);
            Ptr += strlen(TlsVersionName[i]);
          }
        }
        mysql_optionsv(Connection->mariadb, MARIADB_OPT_TLS_VERSION, (void *)TlsVersion);
      }
    }
  
    if (Dsn->SslVerify)
    {
      const unsigned int verify= 0x01010101;
      mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, (const char*)&verify);
    }
    else
    {
      const unsigned int verify= 0;
      mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, (const char*)&verify);
    }
  }
  
  if (Dsn->ForceTls != '\0')
  {
    const unsigned int ForceTls= 0x01010101;
    mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_ENFORCE, (const char*)&ForceTls);
  }

  if (!MADB_IS_EMPTY(Dsn->SslCrl))
  {
    mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_CRL, Dsn->SslCrl);
  }
  if (!MADB_IS_EMPTY(Dsn->SslCrlPath))
  {
    mysql_optionsv(Connection->mariadb, MYSQL_OPT_SSL_CRLPATH, Dsn->SslCrlPath);
  }

  if (!MADB_IS_EMPTY(Dsn->ServerKey))
  {
    mysql_optionsv(Connection->mariadb, MYSQL_SERVER_PUBLIC_KEY, Dsn->ServerKey);
  }

  if (!MADB_IS_EMPTY(Dsn->TlsPeerFp))
  {
    mysql_optionsv(Connection->mariadb, MARIADB_OPT_TLS_PEER_FP, (void*)Dsn->TlsPeerFp);
  }
  if (!MADB_IS_EMPTY(Dsn->TlsPeerFpList))
  {
    mysql_optionsv(Connection->mariadb, MARIADB_OPT_TLS_PEER_FP_LIST, (void*)Dsn->TlsPeerFpList);
  }

  if (!MADB_IS_EMPTY(Dsn->TlsKeyPwd))
  {
    mysql_optionsv(Connection->mariadb, MARIADB_OPT_TLS_PASSPHRASE, (void*)Dsn->TlsKeyPwd);
  }

  if (!mysql_real_connect(Connection->mariadb,
      Dsn->Socket ? "localhost" : Dsn->ServerName, Dsn->UserName, Dsn->Password,
        Dsn->Catalog && Dsn->Catalog[0] ? Dsn->Catalog : NULL, Dsn->Port, Dsn->Socket, client_flags))
  {
    goto err;
  }
  
  /* I guess it is better not to do that at all. Besides SQL_ATTR_PACKET_SIZE is actually not for max packet size */
  if (Connection->PacketSize)
  {
    /*_snprintf(StmtStr, 128, "SET GLOBAL max_allowed_packet=%ld", Connection-> PacketSize);
    if (mysql_query(Connection->mariadb, StmtStr))
      goto err;*/
  }

  /* set default catalog */
  if (Connection->CatalogName && Connection->CatalogName[0])
  {
    if (mysql_select_db(Connection->mariadb, Connection->CatalogName))
      goto err;
  }

  /* Turn sql_auto_is_null behavior off.
     For more details see: http://bugs.mysql.com/bug.php?id=47005 */
  if (mysql_query(Connection->mariadb, "SET SESSION SQL_AUTO_IS_NULL=0"))
    goto err;

  /* set autocommit behavior */
  if (mysql_autocommit(Connection->mariadb, (my_bool)Connection->AutoCommit))
    goto err;

  if (!Dsn->CompatMode) {
    MYSQL_RES *result;
    MYSQL_ROW row;
    const char *StmtString= "SELECT @@memsql_version";

    LOCK_MARIADB(Connection);
    if (mysql_query(Connection->mariadb, StmtString))
    {
      UNLOCK_MARIADB(Connection);
      return MADB_SetNativeError(&Connection->Error, SQL_HANDLE_DBC, Connection->mariadb);
    }
    result= mysql_store_result(Connection->mariadb);
    UNLOCK_MARIADB(Connection);
    if ((row = mysql_fetch_row(result)))
    {
      char* ss_version = (char*) row[0];
      mysql_optionsv(Connection->mariadb, MYSQL_SS_VERSION, ss_version);
    }
    mysql_free_result(result);
  }
  SetDBCharsetnr(Connection);
  /* Set isolation level */
  if (Connection->IsolationLevel)
    for (i=0; i < 4; i++)
    {
      if (MADB_IsolationLevel[i].SqlIsolation == Connection->IsolationLevel)
      {
        _snprintf(StmtStr, 128, "SET SESSION TRANSACTION ISOLATION LEVEL %s",
                    MADB_IsolationLevel[i].StrIsolation);
        if (mysql_query(Connection->mariadb, StmtStr))
          goto err;
        break;
      }
    }

  goto end;

err:
  MADB_SetNativeError(&Connection->Error, SQL_HANDLE_DBC, Connection->mariadb);
      
end:
  if (Connection->Error.ReturnValue == SQL_ERROR && Connection->mariadb)
  {
    mysql_close(Connection->mariadb);
    Connection->mariadb= NULL;
  }

  return Connection->Error.ReturnValue;
}
/* }}} */

/* {{{ MADB_DbcGetFunctions */
SQLRETURN MADB_DbcGetFunctions(MADB_Dbc *Dbc, SQLUSMALLINT FunctionId, SQLUSMALLINT *SupportedPtr)
{
  unsigned int i, Elements= sizeof(MADB_supported_api) / sizeof(SQLUSMALLINT);
  
  switch(FunctionId) {
  case SQL_API_ODBC3_ALL_FUNCTIONS:
    /* clear ptr */ 
    memset(SupportedPtr, 0, sizeof(SQLUSMALLINT) * SQL_API_ODBC3_ALL_FUNCTIONS_SIZE);
    for (i=0; i < Elements; ++i)
    {
      SQLUSMALLINT function= MADB_supported_api[i]; 
      SupportedPtr[function >> 4]|= (1 << (function & 0x000F));
    }
    return SQL_SUCCESS;
  case SQL_API_ALL_FUNCTIONS:
    /* Set all to SQL_FALSE (0) */
    memset(SupportedPtr, 0, sizeof(SQLUSMALLINT) * 100);
    for (i=0; i < Elements; i++)
      if (MADB_supported_api[i] < 100)
        SupportedPtr[MADB_supported_api[i]]= SQL_TRUE;
    return SQL_SUCCESS;
  default:
    *SupportedPtr= SQL_FALSE;
    for (i=0; i < Elements; i++)
      if (MADB_supported_api[i] == FunctionId)
      {
        *SupportedPtr= SQL_TRUE;
        return SQL_SUCCESS;
      }
    break;
  }
  return MADB_SetError(&Dbc->Error, MADB_ERR_HY095, "Function type out of range",
                  mysql_errno(Dbc->mariadb));
}
/* }}} */

/* {{{ IsStringInfoType */
int IsStringInfoType(SQLSMALLINT InfoType)
{
  switch (InfoType)
  {
    case SQL_ACCESSIBLE_PROCEDURES:
    case SQL_ACCESSIBLE_TABLES:
    case SQL_CATALOG_NAME:
    case SQL_CATALOG_NAME_SEPARATOR:
    case SQL_CATALOG_TERM:
    case SQL_COLLATION_SEQ:
    case SQL_COLUMN_ALIAS:
    case SQL_DATA_SOURCE_NAME:
    case SQL_DATABASE_NAME:
    case SQL_DBMS_NAME:
    case SQL_DBMS_VER:
    case SQL_DESCRIBE_PARAMETER:
    case SQL_DRIVER_NAME:
    case SQL_DRIVER_ODBC_VER:
    case SQL_DRIVER_VER:
    case SQL_EXPRESSIONS_IN_ORDERBY:
    case SQL_INTEGRITY:
    case SQL_KEYWORDS:
    case SQL_LIKE_ESCAPE_CLAUSE:
    case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
    case SQL_MULT_RESULT_SETS:
    case SQL_MULTIPLE_ACTIVE_TXN:
    case SQL_NEED_LONG_DATA_LEN:
    case SQL_OUTER_JOINS:
    case SQL_ORDER_BY_COLUMNS_IN_SELECT:
    case SQL_PROCEDURE_TERM:
    case SQL_PROCEDURES:
    case SQL_ROW_UPDATES:
    case SQL_SCHEMA_TERM:
    case SQL_SEARCH_PATTERN_ESCAPE:
    case SQL_SERVER_NAME:
    case SQL_SPECIAL_CHARACTERS:
    case SQL_TABLE_TERM:
    case SQL_USER_NAME:
    case SQL_XOPEN_CLI_YEAR:
    case SQL_DATA_SOURCE_READ_ONLY:
    case SQL_IDENTIFIER_QUOTE_CHAR:
      return 1;
  }

  return 0;
}
/* }}} */

/* {{{ MADB_DbcGetInfo */
SQLRETURN MADB_DbcGetInfo(MADB_Dbc *Dbc, SQLUSMALLINT InfoType, SQLPOINTER InfoValuePtr,
                          SQLSMALLINT BufferLength, SQLSMALLINT *StringLengthPtr, my_bool isWChar)
{
  SQLSMALLINT SLen= 0;
  extern Client_Charset utf8;

  if (!InfoValuePtr && !StringLengthPtr)
    return SQL_SUCCESS;

  /* Prety special case - on Windows DM passes NULL instead of InfoValuePtr and own pointer instead of StringLengthPtr.
     The logic here is not quite clear - I would imagine that truncated status is more appropriate.
     But UnixODBC does not do so, and we are making connector's behavior consistent */
  if (InfoValuePtr != NULL && BufferLength == 0 && StringLengthPtr == NULL && IsStringInfoType(InfoType))
  {
    return SQL_SUCCESS;
  }
 
  MADB_CLEAR_ERROR(&Dbc->Error);
  switch(InfoType) {
  case SQL_ACCESSIBLE_PROCEDURES:
    // SQL_ACCESSIBLE_PROCEDURES is a character string:
    // "Y" if the user can execute all procedures returned by SQLProcedures;
    // "N" if there may be procedures returned that the user cannot execute.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_ACCESSIBLE_TABLES:
    // SQL_ACCESSIBLE_TABLES is a character string:
    // "Y" if the user is guaranteed SELECT privileges to all tables returned by SQLTables;
    // "N" if there may be tables returned that the user cannot access.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_ACTIVE_ENVIRONMENTS:
    // SQL_ACTIVE_ENVIRONMENTS specifies the maximum number of active environments that the driver can support.
    // We don't have a limit for maximum number of active environments
    // therefore we are returning 0
    //
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_AGGREGATE_FUNCTIONS:
    // SQL_AGGREGATE_FUNCTIONS enumerates support for aggregation functions.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_AF_AVG | SQL_AF_COUNT |
                                                  SQL_AF_MAX | SQL_AF_MIN | SQL_AF_SUM |
                                                  SQL_AF_ALL | SQL_AF_DISTINCT, StringLengthPtr);
    break;
  case SQL_ALTER_DOMAIN:
    // SQL_ALTER_DOMAIN enumerates the clauses in the ALTER DOMAIN statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the ALTER DOMAIN statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_ALTER_TABLE:
    // SQL_ALTER_TABLE enumerates the clauses in the ALTER TABLE statement supported by the data source.
    // SQL_AT_ADD_COLUMN_COLLATION = <add column> clause is supported, with facility to specify column collation (Full level) (ODBC 3.0)
    // SQL_AT_ADD_COLUMN_DEFAULT = <add column> clause is supported, with facility to specify column defaults (FIPS Transitional level) (ODBC 3.0)
    // SQL_AT_ADD_COLUMN_SINGLE = <add column> is supported (FIPS Transitional level) (ODBC 3.0)
    // SQL_AT_ADD_CONSTRAINT = <add column> clause is supported, with facility to specify column constraints (FIPS Transitional level) (ODBC 3.0)
    // SQL_AT_ADD_TABLE_CONSTRAINT = <add table constraint> clause is supported (FIPS Transitional level) (ODBC 3.0)
    // SQL_AT_CONSTRAINT_NAME_DEFINITION = <constraint name definition> is supported for naming column and table constraints (Intermediate level) (ODBC 3.0)
    // SQL_AT_DROP_COLUMN_CASCADE = <drop column> CASCADE is supported (FIPS Transitional level) (ODBC 3.0)
    // SQL_AT_DROP_COLUMN_DEFAULT = <alter column> <drop column default clause> is supported (Intermediate level) (ODBC 3.0)
    // SQL_AT_DROP_COLUMN_RESTRICT = <drop column> RESTRICT is supported (FIPS Transitional level) (ODBC 3.0)
    // SQL_AT_DROP_TABLE_CONSTRAINT_CASCADE (ODBC 3.0)
    // SQL_AT_DROP_TABLE_CONSTRAINT_RESTRICT = <drop column> RESTRICT is supported (FIPS Transitional level) (ODBC 3.0)
    // SQL_AT_SET_COLUMN_DEFAULT = <alter column> <set column default clause> is supported (Intermediate level) (ODBC 3.0)
    //
    // The following bits specify the support <constraint attributes> if specifying column or table constraints is supported (the SQL_AT_ADD_CONSTRAINT bit is set):
    // SQL_AT_CONSTRAINT_INITIALLY_DEFERRED (Full level) (ODBC 3.0)
    // SQL_AT_CONSTRAINT_INITIALLY_IMMEDIATE (Full level) (ODBC 3.0)
    // SQL_AT_CONSTRAINT_DEFERRABLE (Full level) (ODBC 3.0)
    // SQL_AT_CONSTRAINT_NON_DEFERRABLE (Full level) (ODBC 3.0)
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_AT_ADD_COLUMN | SQL_AT_DROP_COLUMN | SQL_AT_ADD_CONSTRAINT |
                     SQL_AT_ADD_COLUMN_SINGLE | SQL_AT_ADD_COLUMN_DEFAULT, StringLengthPtr);
    break;
#ifdef SQL_ASYNC_DBC_FUNCTIONS
  case SQL_ASYNC_DBC_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_ASYNC_DBC_NOT_CAPABLE, StringLengthPtr);
    break;
#endif
#ifdef SQL_ASYNC_MODE
  case SQL_ASYNC_MODE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_AM_NONE, StringLengthPtr);
    break;
#endif
#ifdef SQL_ASYNC_NOTIFICATION
  case SQL_ASYNC_NOTIFICATION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_ASYNC_NOTIFICATION_NOT_CAPABLE, StringLengthPtr);
    break;
#endif
  case SQL_BATCH_ROW_COUNT:
    // SQL_BATCH_ROW_COUNT enumerates the behavior of the driver with respect to the availability of row counts.
    // SQL_BRC_ROLLED_UP = row counts for consecutive INSERT, DELETE, or UPDATE statements are rolled up into one.
    // In this driver, row counts are available for each statement.
    // SQL_BRC_PROCEDURES = row counts, if any, are available when a batch is executed in a stored procedure.
    // SQL_BRC_EXPLICIT = row counts, if any, are available when a batch is executed directly by calling SQLExecute or SQLExecDirect.
    //
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_BRC_EXPLICIT, StringLengthPtr);
    break;
  case SQL_BATCH_SUPPORT:
    // SQL_BATCH_SUPPORT enumerating the driver's support for batches.
    // This driver supports all types of batches.
    // SQL_BS_SELECT_EXPLICIT = the driver supports explicit batches that can have result-set generating statements.
    // SQL_BS_ROW_COUNT_EXPLICIT = the driver supports explicit batches that can have row-count generating statements.
    // SQL_BS_SELECT_PROC = the driver supports explicit procedures that can have result-set generating statements.
    // SQL_BS_ROW_COUNT_PROC = the driver supports explicit procedures that can have row-count generating statements.
    //
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_BS_SELECT_EXPLICIT | SQL_BS_ROW_COUNT_EXPLICIT |
                                                SQL_BS_SELECT_PROC | SQL_BS_ROW_COUNT_PROC,
                     StringLengthPtr);
    break;
  case SQL_BOOKMARK_PERSISTENCE:
    // SQL_BOOKMARK_PERSISTENCE enumerates the operations through which bookmarks persist.
    // TODO PLAT-5471
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CATALOG_LOCATION:
    // SQL_CATALOG_LOCATION indicates the position of the catalog in a qualified table name
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CL_START, StringLengthPtr);
    break;
  case SQL_CATALOG_NAME:
    // SQL_CATALOG_NAME is a character string: "Y" if the server supports catalog names, or "N" if it does not.
    /* Todo: MyODBC Driver has a DSN configuration for disabling catalog usage:
       but it's not implemented in MAODBC */
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_CATALOG_NAME_SEPARATOR:
    // SQL_CATALOG_NAME_SEPARATOR is a character string: the character or characters that the data source defines as the separator
    // between a catalog name and the qualified name element that follows or precedes it.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), ".", SQL_NTS, &Dbc->Error);
    break;
  case SQL_CATALOG_TERM:
    // SQL_CATALOG_TERM is a character string with the data source vendor's name for a catalog.
    /* todo: See comment for SQL_CATALOG_NAME */
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "database", SQL_NTS, &Dbc->Error);
    break;
  case SQL_CATALOG_USAGE:
    // SQL_CATALOG_USAGE enumerates the statements in which catalogs can be used.
    // SQL_CU_DML_STATEMENTS = Catalogs are supported in all Data Manipulation Language statements: SELECT, INSERT, UPDATE, DELETE, and if supported, SELECT FOR UPDATE and positioned update and delete statements.
    // SQL_CU_PROCEDURE_INVOCATION = Catalogs are supported in the ODBC procedure invocation statement.
    // SQL_CU_TABLE_DEFINITION = Catalogs are supported in all table definition statements: CREATE TABLE, CREATE VIEW, ALTER TABLE, DROP TABLE, and DROP VIEW.
    // SQL_CU_INDEX_DEFINITION = Catalogs are supported in all index definition statements: CREATE INDEX and DROP INDEX.
    // SQL_CU_PRIVILEGE_DEFINITION = Catalogs are supported in all privilege definition statements: GRANT and REVOKE.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CU_DML_STATEMENTS | 
                                                SQL_CU_INDEX_DEFINITION |
                                                SQL_CU_PROCEDURE_INVOCATION | 
                                                SQL_CU_PRIVILEGE_DEFINITION |
                                                SQL_CU_TABLE_DEFINITION,
                      StringLengthPtr);
    break;
  case SQL_COLLATION_SEQ:
  {
    // SQL_COLLATION_SEQ is the name of the collation sequence.
    MY_CHARSET_INFO cs;
    mariadb_get_infov(Dbc->mariadb, MARIADB_CONNECTION_MARIADB_CHARSET_INFO, (void*)&cs);
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL,
      (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
      cs.name, SQL_NTS, &Dbc->Error);
    break;
  }
  case SQL_COLUMN_ALIAS:
    // SQL_COLUMN_ALIAS is a character string: "Y" if the data source supports column aliases; otherwise, "N".
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, 
                           (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_CONCAT_NULL_BEHAVIOR:
    // SQL_CONCAT_NULL_BEHAVIOR  indicates how the data source handles the concatenation of NULL valued character data type columns
    // with non-NULL valued character data type columns.
    // SQL_CB_NULL = Result is NULL valued.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CB_NULL, StringLengthPtr);
    break;
  case SQL_CONVERT_BIGINT:
    // SQL_CONVERT_BIGINT indicates the conversions supported by the data source with the CONVERT scalar function for data of the type BIGINT
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_BINARY:
    // SQL_CONVERT_BINARY indicates the conversions supported by the data source with the CONVERT scalar function for data of the type BINARY
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_BIT:
    // SQL_CONVERT_BIT indicates the conversions supported by the data source with the CONVERT scalar function for data of the type BIT
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_CHAR:
    // SQL_CONVERT_CHAR indicates the conversions supported by the data source with the CONVERT scalar function for data of the type CHAR
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_WCHAR:
    // SQL_CONVERT_WCHAR indicates the conversions supported by the data source with the CONVERT scalar function for data of the type WCHAR
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
#ifdef SQL_CONVERT_GUID
  case SQL_CONVERT_GUID:
    // SQL_CONVERT_GUID indicates the conversions supported by the data source with the CONVERT scalar function for data of the type GUID
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
#endif
  case SQL_CONVERT_DATE:
    // SQL_CONVERT_DATE indicates the conversions supported by the data source with the CONVERT scalar function for data of the type DATE
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_DECIMAL:
    // SQL_CONVERT_DECIMAL indicates the conversions supported by the data source with the CONVERT scalar function for data of the type DECIMAL
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_DOUBLE:
    // SQL_CONVERT_DOUBLE indicates the conversions supported by the data source with the CONVERT scalar function for data of the type DOUBLE
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_FLOAT:
    // SQL_CONVERT_FLOAT indicates the conversions supported by the data source with the CONVERT scalar function for data of the type FLOAT
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_INTEGER:
    // SQL_CONVERT_INTEGER indicates the conversions supported by the data source with the CONVERT scalar function for data of the type INTEGER
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_INTERVAL_YEAR_MONTH:
    // SQL_CONVERT_INTERVAL_YEAR_MONTH indicates the conversions supported by the data source with the CONVERT scalar function for data of the type INTERVAL_YEAR_MONTH
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CONVERT_INTERVAL_DAY_TIME:
    // SQL_CONVERT_INTERVAL_DAY_TIME indicates the conversions supported by the data source with the CONVERT scalar function for data of the type INTERVAL_DAY_TIME
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CONVERT_LONGVARBINARY:
    // SQL_CONVERT_LONGVARBINARY indicates the conversions supported by the data source with the CONVERT scalar function for data of the type LONGVARBINARY
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_LONGVARCHAR:
    // SQL_CONVERT_LONGVARCHAR indicates the conversions supported by the data source with the CONVERT scalar function for data of the type LONGVARCHAR
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_WLONGVARCHAR:
    // SQL_CONVERT_WLONGVARCHAR indicates the conversions supported by the data source with the CONVERT scalar function for data of the type WLONGVARCHAR
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_NUMERIC:
    // SQL_CONVERT_NUMERIC indicates the conversions supported by the data source with the CONVERT scalar function for data of the type NUMERIC
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_REAL:
    // SQL_CONVERT_REAL indicates the conversions supported by the data source with the CONVERT scalar function for data of the type REAL
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_SMALLINT:
    // SQL_CONVERT_SMALLINT indicates the conversions supported by the data source with the CONVERT scalar function for data of the type SMALLINT
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_TIME:
    // SQL_CONVERT_TIME indicates the conversions supported by the data source with the CONVERT scalar function for data of the type TIME
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_TIMESTAMP:
    // SQL_CONVERT_TIMESTAMP indicates the conversions supported by the data source with the CONVERT scalar function for data of the type TIMESTAMP
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_TINYINT:
    // SQL_CONVERT_TINYINT indicates the conversions supported by the data source with the CONVERT scalar function for data of the type TINYINT
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_VARBINARY:
    // SQL_CONVERT_VARBINARY indicates the conversions supported by the data source with the CONVERT scalar function for data of the type VARBINARY
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_VARCHAR:
    // SQL_CONVERT_VARCHAR indicates the conversions supported by the data source with the CONVERT scalar function for data of the type VARCHAR
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_WVARCHAR:
    // SQL_CONVERT_WVARCHAR indicates the conversions supported by the data source with the CONVERT scalar function for data of the type WVARCHAR
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_FUNCTIONS:
    // SQL_CONVERT_FUNCTIONS enumerates the scalar conversion functions supported by the driver and associated data source.
    // CAST is not supported for all datatypes, while CONVERT is expected to arrive in the escape syntax form,
    // in which case we fully support it.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_CVT_CONVERT, StringLengthPtr);
    break;
  case SQL_CORRELATION_NAME:
    // SQL_CORRELATION_NAME indicates whether table correlation names are supported.
    // SQL_CN_ANY = Correlation names are supported and can be any valid user-defined name.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CN_ANY, StringLengthPtr);
    break;
  case SQL_CREATE_ASSERTION:
    // SQL_CREATE_ASSERTION enumerates the clauses in the CREATE ASSERTION statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the CREATE ASSERTION statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_CHARACTER_SET:
    // SQL_CREATE_CHARACTER_SET enumerates the clauses in the CREATE CHARACTER SET statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the CREATE CHARACTER SET statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_COLLATION:
    // SQL_CREATE_COLLATION enumerates the clauses in the CREATE DOMAIN statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the CREATE DOMAIN statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_DOMAIN:
    // SQL_CREATE_DOMAIN enumerates the clauses in the CREATE DOMAIN statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the CREATE DOMAIN statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_SCHEMA:
    // SQL_CREATE_SCHEMA enumerates the clauses in the CREATE SCHEMA statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the CREATE SCHEMA statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_TABLE:
    // SQL_CREATE_TABLE enumerates the clauses in the CREATE TABLE statement, as defined in SQL-92, supported by the data source.
    // The following bitmasks are used to determine which clauses are supported:
    // SQL_CT_CREATE_TABLE = The CREATE TABLE statement is supported. (Entry level)
    // SQL_CT_TABLE_CONSTRAINT = Specifying table constraints is supported (FIPS Transitional level)
    // SQL_CT_CONSTRAINT_NAME_DEFINITION = The <constraint name definition> clause is supported for naming column and table constraints (Intermediate level)
    //
    // The following bits specify the ability to create temporary tables:
    // SQL_CT_COMMIT_PRESERVE = Deleted rows are preserved on commit. (Full level)
    // SQL_CT_COMMIT_DELETE = Deleted rows are deleted on commit. (Full level)
    // SQL_CT_GLOBAL_TEMPORARY = Global temporary tables can be created. (Full level)
    // SQL_CT_LOCAL_TEMPORARY = Local temporary tables can be created. (Full level)
    //
    // The following bits specify the ability to create column constraints:
    // SQL_CT_COLUMN_CONSTRAINT = Specifying column constraints is supported (FIPS Transitional level)
    // SQL_CT_COLUMN_DEFAULT = Specifying column defaults is supported (FIPS Transitional level)
    // SQL_CT_COLUMN_COLLATION = Specifying column collation is supported (Full level)
    //
    // The following bits specify the supported constraint attributes if specifying column or table constraints is supported:
    // SQL_CT_CONSTRAINT_INITIALLY_DEFERRED (Full level)
    // SQL_CT_CONSTRAINT_INITIALLY_IMMEDIATE (Full level)
    // SQL_CT_CONSTRAINT_DEFERRABLE (Full level)
    // SQL_CT_CONSTRAINT_NON_DEFERRABLE (Full level)
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CT_COLUMN_COLLATION | SQL_CT_COLUMN_DEFAULT | SQL_CT_COLUMN_CONSTRAINT |
                                                SQL_CT_COMMIT_DELETE | SQL_CT_CREATE_TABLE | SQL_CT_TABLE_CONSTRAINT | SQL_CT_GLOBAL_TEMPORARY |
                                                SQL_CT_LOCAL_TEMPORARY | SQL_CT_CONSTRAINT_NAME_DEFINITION,
                     StringLengthPtr);
    break;
  case SQL_CREATE_TRANSLATION:
    // SQL_CREATE_TRANSLATION enumerates the clauses in the CREATE TRANSLATION statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the CREATE TRANSLATION statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_VIEW:
    // SQL_CREATE_VIEW enumerates the clauses in the CREATE VIEW statement, as defined in SQL-92, supported by the data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CV_CREATE_VIEW, StringLengthPtr);
    break;
  case SQL_CURSOR_COMMIT_BEHAVIOR:
    // SQL_CURSOR_COMMIT_BEHAVIOR indicates how a COMMIT operation affects cursors and prepared statements in the data source
    // SQL_CB_PRESERVE = Preserve cursors in the same position as before the COMMIT operation.
    // The application can continue to fetch data, or it can close the cursor and re-execute the statement without repreparing it.
    // TODO PLAT-5414
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CB_PRESERVE, StringLengthPtr);
    break;
  case SQL_CURSOR_ROLLBACK_BEHAVIOR:
    // SQL_CURSOR_ROLLBACK_BEHAVIOR indicates how a ROLLBACK operation affects cursors and prepared statements in the data source.
    // SQL_CB_PRESERVE = Preserve cursors in the same position as before the ROLLBACK operation.
    // The application can continue to fetch data, or it can close the cursor and re-execute the statement without repreparing it.
    // TODO PLAT-5414
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CB_PRESERVE, StringLengthPtr);
    break;
  case SQL_CURSOR_SENSITIVITY:
    // SQL_CURSOR_SENSITIVITY indicates the support for cursor sensitivity.
    // SQL_UNSPECIFIED = It is unspecified whether cursors on the statement handle make visible the changes
    // that were made to a result set by another cursor within the same transaction.
    // Cursors on the statement handle may make visible none, some, or all such changes.
    // TODO PLAT-5414
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_UNSPECIFIED, StringLengthPtr);
    break;
  case SQL_DATA_SOURCE_NAME:
    // SQL_DATA_SOURCE_NAME is a character string with the data source name that was used during connection
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     Dbc->Dsn ? Dbc->Dsn->DSNName : "", SQL_NTS, &Dbc->Error);
    break;
  case SQL_DATABASE_NAME:
    // SQL_DATABASE_NAME is a character string with the name of the current database in use
    return MADB_Dbc_GetCurrentDB(Dbc, InfoValuePtr, BufferLength, (SQLSMALLINT *)StringLengthPtr, isWChar);
    break;
  case SQL_DATETIME_LITERALS:
    // SQL_DATETIME_LITERALS enumerating the SQL-92 datetime literals supported by the data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DL_SQL92_DATE | SQL_DL_SQL92_TIME |
                                                SQL_DL_SQL92_TIMESTAMP, StringLengthPtr);
    break;
  case SQL_DBMS_NAME:
    // SQL_DBMS_NAME is a character string with the name of the DBMS product accessed by the driver.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                      !Dbc->Dsn->CompatMode ? "SingleStore" : "MySQL",
                                     SQL_NTS, &Dbc->Error);
    break;
  case SQL_DBMS_VER:
    {
      // SQL_DBMS_VER is a character string that indicates the version of the DBMS product accessed by the driver.
      // The version is of the form ##.##.####, where the first two digits are the major version, the next two digits are the minor version, and the last four digits are the release version.
      char Version[13];
      unsigned long ServerVersion= 0L;
      if (Dbc->mariadb)
      {
        ServerVersion = !Dbc->Dsn->CompatMode ? single_store_get_server_version(Dbc->mariadb) : mysql_get_server_version(Dbc->mariadb);
        if (ServerVersion < 50600)
        {
            ServerVersion = 50600;
        }
        _snprintf(Version, sizeof(Version), "%02u.%02u.%04u", ServerVersion / 10000,
                    (ServerVersion % 10000) / 100, ServerVersion % 100);
      }
      else
        Version[0]= 0;
      SLen= (SQLSMALLINT)MADB_SetString(isWChar ?  &utf8 : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                       Version[0] ? Version : "", SQL_NTS, &Dbc->Error);
    }
    break;
  case SQL_DDL_INDEX:
    // SQL_DDL_INDEX indicates support for creation and dropping of indexes:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DI_CREATE_INDEX | SQL_DI_DROP_INDEX, StringLengthPtr);
    break;
  case SQL_DEFAULT_TXN_ISOLATION:
    // SQL_DEFAULT_TXN_ISOLATION ndicates the default transaction isolation level supported by the driver or data source.
    // SQL_TXN_READ_COMMITTED = Dirty reads are not possible. Non-repeatable reads and phantoms are possible.
    // TODO PLAT-5414
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_TXN_READ_COMMITTED, StringLengthPtr);
    break;
  case SQL_DESCRIBE_PARAMETER:
    // SQL_DESCRIBE_PARAMETER is a character string: "Y" if parameters can be described; "N", if not.
    // TODO PLAT-5483
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "N", SQL_NTS, &Dbc->Error);
    break;
#ifdef SQL_DRIVER_AWARE_POOLING_SUPPORTED
  case SQL_DRIVER_AWARE_POOLING_SUPPORTED:
    // SQL_DRIVER_AWARE_POOLING_SUPPORTED indicates if the driver support driver-aware pooling.
    // SQL_DRIVER_AWARE_POOLING_NOT_CAPABLE indicates that the driver cannot support driver-aware pooling mechanism.
    // TODO PLAT-5459 investigate if we can/need to support this
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DRIVER_AWARE_POOLING_NOT_CAPABLE, StringLengthPtr);
    break;
#endif
    /* Handled by driver manager */
  case SQL_DRIVER_HDBC:
    break;
  case SQL_DRIVER_HENV:
    break;
  case SQL_DRIVER_HLIB:
    break;
  case SQL_DRIVER_HSTMT:
    break;
  case SQL_DRIVER_NAME:
    // A character string with the file name of the driver used to access the data source
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     MADB_DRIVER_NAME, SQL_NTS, &Dbc->Error);
    break;
  case SQL_DRIVER_ODBC_VER:
    // A character string with the version of ODBC that the driver supports.
    {
      char *OdbcVersion = "03.51";
      /* DM requests this info before Dbc->Charset initialized. Thus checking if it is, and use utf8 by default
         The other way would be to use utf8 when Dbc initialized */
      SLen= (SQLSMALLINT)MADB_SetString(isWChar ? (Dbc->Charset.cs_info ? &Dbc->Charset : &utf8 ): NULL,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     OdbcVersion, SQL_NTS, &Dbc->Error);
    }
    break;
  case SQL_DRIVER_VER:
     // A character string with the version of the driver and optionally, a description of the driver
     SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     MARIADB_ODBC_VERSION, SQL_NTS, &Dbc->Error);
    break;
    /*******************************/
  case SQL_DROP_ASSERTION:
    // SQL_DROP_ASSERTION enumerates the clauses in the DROP ASSERTION statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the DROP ASSERTION statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_CHARACTER_SET:
    // SQL_DROP_CHARACTER_SET enumerates the clauses in the DROP CHARACTER SET statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the DROP CHARACTER_SET statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_COLLATION:
    // SQL_DROP_COLLATION enumerating the clauses in the DROP COLLATION statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the DROP COLLATION statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_DOMAIN:
    // SQL_DROP_COLLATION enumerates the clauses in the DROP DOMAIN statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the DROP DOMAIN statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_SCHEMA:
    // SQL_DROP_SCHEMA enumerates the clauses in the DROP SCHEMA statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the DROP SCHEMA statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_TABLE:
    // SQL_DROP_TABLE enumerates the clauses in the DROP TABLE statement, as defined in SQL-92, supported by the data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DT_DROP_TABLE, StringLengthPtr);
    break;
  case SQL_DROP_TRANSLATION:
    // SQL_DROP_TRANSLATION enumerates the clauses in the DROP TRANSLATION statement, as defined in SQL-92, supported by the data source.
    // A return value of "0" means that the DROP TRANSLATION statement is not supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_VIEW:
    // SQL_DROP_VIEW enumerating the clauses in the DROP VIEW statement, as defined in SQL-92, supported by the data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DV_DROP_VIEW, StringLengthPtr);
    break;
  case SQL_DYNAMIC_CURSOR_ATTRIBUTES1:
    // SQL_DYNAMIC_CURSOR_ATTRIBUTES1 describes the attributes of a dynamic cursor that are supported by the driver.
    // SQL_CA1_NEXT = A FetchOrientation argument of SQL_FETCH_NEXT is supported in a call to SQLFetchScroll when the cursor is a dynamic cursor.
    // SQL_CA1_ABSOLUTE = FetchOrientation arguments of SQL_FETCH_FIRST, SQL_FETCH_LAST, and SQL_FETCH_ABSOLUTE are supported in a call to SQLFetchScroll when the cursor is a dynamic cursor. (The rowset that will be fetched is independent of the current cursor position.)
    // SQL_CA1_RELATIVE = FetchOrientation arguments of SQL_FETCH_PRIOR and SQL_FETCH_RELATIVE are supported in a call to SQLFetchScroll when the cursor is a dynamic cursor. (The rowset that will be fetched depends on the current cursor position. Note that this is separated from SQL_FETCH_NEXT because in a forward-only cursor, only SQL_FETCH_NEXT is supported.)
    // SQL_CA1_BOOKMARK = A FetchOrientation argument of SQL_FETCH_BOOKMARK is supported in a call to SQLFetchScroll when the cursor is a dynamic cursor.
    // SQL_CA1_LOCK_EXCLUSIVE = A LockType argument of SQL_LOCK_EXCLUSIVE is supported in a call to SQLSetPos when the cursor is a dynamic cursor.
    // SQL_CA1_LOCK_NO_CHANGE = A LockType argument of SQL_LOCK_NO_CHANGE is supported in a call to SQLSetPos when the cursor is a dynamic cursor.
    // SQL_CA1_LOCK_UNLOCK = A LockType argument of SQL_LOCK_UNLOCK is supported in a call to SQLSetPos when the cursor is a dynamic cursor.
    // SQL_CA1_POS_POSITION = An Operation argument of SQL_POSITION is supported in a call to SQLSetPos when the cursor is a dynamic cursor.
    // SQL_CA1_POS_UPDATE = An Operation argument of SQL_UPDATE is supported in a call to SQLSetPos when the cursor is a dynamic cursor.
    // SQL_CA1_POS_DELETE = An Operation argument of SQL_DELETE is supported in a call to SQLSetPos when the cursor is a dynamic cursor.
    // SQL_CA1_POS_REFRESH = An Operation argument of SQL_REFRESH is supported in a call to SQLSetPos when the cursor is a dynamic cursor.
    // SQL_CA1_POSITIONED_UPDATE = An UPDATE WHERE CURRENT OF SQL statement is supported when the cursor is a dynamic cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_POSITIONED_DELETE = A DELETE WHERE CURRENT OF SQL statement is supported when the cursor is a dynamic cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_SELECT_FOR_UPDATE = A SELECT FOR UPDATE SQL statement is supported when the cursor is a dynamic cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_BULK_ADD = An Operation argument of SQL_ADD is supported in a call to SQLBulkOperations when the cursor is a dynamic cursor.
    // SQL_CA1_BULK_UPDATE_BY_BOOKMARK = An Operation argument of SQL_UPDATE_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a dynamic cursor.
    // SQL_CA1_BULK_DELETE_BY_BOOKMARK = An Operation argument of SQL_DELETE_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a dynamic cursor.
    // SQL_CA1_BULK_FETCH_BY_BOOKMARK = An Operation argument of SQL_FETCH_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a dynamic cursor.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA1_NEXT |
                                                  SQL_CA1_ABSOLUTE |
                                                  SQL_CA1_RELATIVE |
                                                  // SQL_CA1_BOOKMARK | TODO PLAT-5471
                                                  SQL_CA1_LOCK_NO_CHANGE |
                                                  SQL_CA1_POS_POSITION |
                                                  // SQL_CA1_POS_UPDATE | TODO PLAT-5080
                                                  SQL_CA1_POS_DELETE |
                                                  // SQL_CA1_POS_REFRESH | TODO PLAT-5466
                                                  // SQL_CA1_POSITIONED_UPDATE | TODO PLAT-5080
                                                  SQL_CA1_POSITIONED_DELETE |
                                                  SQL_CA1_BULK_ADD,
                     StringLengthPtr);
    break;
  case SQL_DYNAMIC_CURSOR_ATTRIBUTES2:
    // SQL_DYNAMIC_CURSOR_ATTRIBUTES2 describes the attributes of a dynamic cursor that are supported by the driver.
    // SQL_CA2_READ_ONLY_CONCURRENCY = A read-only dynamic cursor, in which no updates are allowed, is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_READ_ONLY for a dynamic cursor).
    // SQL_CA2_LOCK_CONCURRENCY = A dynamic cursor that uses the lowest level of locking sufficient to make sure that the row can be updated is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_LOCK for a dynamic cursor.) These locks must be consistent with the transaction isolation level set by the SQL_ATTR_TXN_ISOLATION connection attribute.
    // SQL_CA2_OPT_ROWVER_CONCURRENCY = A dynamic cursor that uses the optimistic concurrency control comparing row versions is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_ROWVER for a dynamic cursor.)
    // SQL_CA2_OPT_VALUES_CONCURRENCY = A dynamic cursor that uses the optimistic concurrency control comparing values is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_VALUES for a dynamic cursor.)
    // SQL_CA2_SENSITIVITY_ADDITIONS = Added rows are visible to a dynamic cursor; the cursor can scroll to those rows. (Where these rows are added to the cursor is driver-dependent.)
    // SQL_CA2_SENSITIVITY_DELETIONS = Deleted rows are no longer available to a dynamic cursor, and do not leave a "hole" in the result set; after the dynamic cursor scrolls from a deleted row, it cannot return to that row.
    // SQL_CA2_SENSITIVITY_UPDATES = Updates to rows are visible to a dynamic cursor; if the dynamic cursor scrolls from and returns to an updated row, the data returned by the cursor is the updated data, not the original data.
    // SQL_CA2_MAX_ROWS_SELECT = The SQL_ATTR_MAX_ROWS statement attribute affects SELECT statements when the cursor is a dynamic cursor.
    // SQL_CA2_MAX_ROWS_INSERT = The SQL_ATTR_MAX_ROWS statement attribute affects INSERT statements when the cursor is a dynamic cursor.
    // SQL_CA2_MAX_ROWS_DELETE = The SQL_ATTR_MAX_ROWS statement attribute affects DELETE statements when the cursor is a dynamic cursor.
    // SQL_CA2_MAX_ROWS_UPDATE = The SQL_ATTR_MAX_ROWS statement attribute affects UPDATE statements when the cursor is a dynamic cursor.
    // SQL_CA2_MAX_ROWS_CATALOG = The SQL_ATTR_MAX_ROWS statement attribute affects CATALOG result sets when the cursor is a dynamic cursor.
    // SQL_CA2_MAX_ROWS_AFFECTS_ALL = The SQL_ATTR_MAX_ROWS statement attribute affects SELECT, INSERT, DELETE, and UPDATE statements, and CATALOG result sets, when the cursor is a dynamic cursor.
    // SQL_CA2_CRC_EXACT = The exact row count is available in the SQL_DIAG_CURSOR_ROW_COUNT diagnostic field when the cursor is a dynamic cursor.
    // SQL_CA2_CRC_APPROXIMATE = An approximate row count is available in the SQL_DIAG_CURSOR_ROW_COUNT diagnostic field when the cursor is a dynamic cursor.
    // SQL_CA2_SIMULATE_NON_UNIQUE = The driver does not guarantee that simulated positioned update or delete statements will affect only one row when the cursor is a dynamic cursor; it is the application's responsibility to guarantee this. (If a statement affects more than one row, SQLExecute or SQLExecDirect returns SQLSTATE 01001 [Cursor operation conflict].) To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_NON_UNIQUE.
    // SQL_CA2_SIMULATE_TRY_UNIQUE = The driver tries to guarantee that simulated positioned update or delete statements will affect only one row when the cursor is a dynamic cursor. The driver always executes such statements, even if they might affect more than one row, such as when there is no unique key. (If a statement affects more than one row, SQLExecute or SQLExecDirect returns SQLSTATE 01001 [Cursor operation conflict].) To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_TRY_UNIQUE.
    // SQL_CA2_SIMULATE_UNIQUE = The driver guarantees that simulated positioned update or delete statements will affect only one row when the cursor is a dynamic cursor. If the driver cannot guarantee this for a given statement, SQLExecDirect or SQLPrepare return SQLSTATE 01001 (Cursor operation conflict). To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_UNIQUE.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA2_READ_ONLY_CONCURRENCY |
                                                  SQL_CA2_SENSITIVITY_ADDITIONS |
                                                  SQL_CA2_SENSITIVITY_DELETIONS |
                                                  SQL_CA2_SENSITIVITY_UPDATES |
                                                  SQL_CA2_MAX_ROWS_SELECT |
                                                  SQL_CA2_MAX_ROWS_INSERT |
                                                  SQL_CA2_MAX_ROWS_DELETE |
                                                  SQL_CA2_MAX_ROWS_UPDATE |
                                                  SQL_CA2_CRC_EXACT |
                                                  SQL_CA2_SIMULATE_TRY_UNIQUE, StringLengthPtr);
    break;
  case SQL_EXPRESSIONS_IN_ORDERBY:
    // SQL_EXPRESSIONS_IN_ORDERBY is a character string: "Y" if the data source supports expressions in the ORDER BY list; "N" if it does not.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_FILE_USAGE:
    // SQL_FILE_USAGE indicates how a single-tier driver directly treats files in a data source:
    //SQL_FILE_NOT_SUPPORTED = The driver is not a single-tier driver. For example, an ORACLE driver is a two-tier driver.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_FILE_NOT_SUPPORTED, StringLengthPtr);
    break;
  case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1:
    // SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1 describes the attributes of a forward-only cursor that are supported by the driver.
    // SQL_CA1_NEXT = A FetchOrientation argument of SQL_FETCH_NEXT is supported in a call to SQLFetchScroll when the cursor is a forward-only cursor.
    // SQL_CA1_ABSOLUTE = FetchOrientation arguments of SQL_FETCH_FIRST, SQL_FETCH_LAST, and SQL_FETCH_ABSOLUTE are supported in a call to SQLFetchScroll when the cursor is a forward-only cursor. (The rowset that will be fetched is independent of the current cursor position.)
    // SQL_CA1_RELATIVE = FetchOrientation arguments of SQL_FETCH_PRIOR and SQL_FETCH_RELATIVE are supported in a call to SQLFetchScroll when the cursor is a forward-only cursor. (The rowset that will be fetched depends on the current cursor position. Note that this is separated from SQL_FETCH_NEXT because in a forward-only cursor, only SQL_FETCH_NEXT is supported.)
    // SQL_CA1_BOOKMARK = A FetchOrientation argument of SQL_FETCH_BOOKMARK is supported in a call to SQLFetchScroll when the cursor is a forward-only cursor.
    // SQL_CA1_LOCK_EXCLUSIVE = A LockType argument of SQL_LOCK_EXCLUSIVE is supported in a call to SQLSetPos when the cursor is a forward-only cursor.
    // SQL_CA1_LOCK_NO_CHANGE = A LockType argument of SQL_LOCK_NO_CHANGE is supported in a call to SQLSetPos when the cursor is a forward-only cursor.
    // SQL_CA1_LOCK_UNLOCK = A LockType argument of SQL_LOCK_UNLOCK is supported in a call to SQLSetPos when the cursor is a forward-only cursor.
    // SQL_CA1_POS_POSITION = An Operation argument of SQL_POSITION is supported in a call to SQLSetPos when the cursor is a forward-only cursor.
    // SQL_CA1_POS_UPDATE = An Operation argument of SQL_UPDATE is supported in a call to SQLSetPos when the cursor is a forward-only cursor.
    // SQL_CA1_POS_DELETE = An Operation argument of SQL_DELETE is supported in a call to SQLSetPos when the cursor is a forward-only cursor.
    // SQL_CA1_POS_REFRESH = An Operation argument of SQL_REFRESH is supported in a call to SQLSetPos when the cursor is a forward-only cursor.
    // SQL_CA1_POSITIONED_UPDATE = An UPDATE WHERE CURRENT OF SQL statement is supported when the cursor is a forward-only cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_POSITIONED_DELETE = A DELETE WHERE CURRENT OF SQL statement is supported when the cursor is a forward-only cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_SELECT_FOR_UPDATE = A SELECT FOR UPDATE SQL statement is supported when the cursor is a forward-only cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_BULK_ADD = An Operation argument of SQL_ADD is supported in a call to SQLBulkOperations when the cursor is a forward-only cursor.
    // SQL_CA1_BULK_UPDATE_BY_BOOKMARK = An Operation argument of SQL_UPDATE_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a forward-only cursor.
    // SQL_CA1_BULK_DELETE_BY_BOOKMARK = An Operation argument of SQL_DELETE_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a forward-only cursor.
    // SQL_CA1_BULK_FETCH_BY_BOOKMARK = An Operation argument of SQL_FETCH_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a forward-only cursor.
    // TODO PLAT-5473 make these limitations stricter
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, Dbc->Dsn && Dbc->Dsn->NoCache ?
                                                SQL_CA1_NEXT:
                                                (SQL_CA1_NEXT |
                                                  SQL_CA1_LOCK_NO_CHANGE |
                                                  SQL_CA1_POS_POSITION |
                                                  // SQL_CA1_POS_UPDATE | TODO PLAT-5080
                                                  SQL_CA1_POS_DELETE |
                                                  // SQL_CA1_POS_REFRESH | TODO PLAT-5466
                                                  // SQL_CA1_POSITIONED_UPDATE | TODO PLAT-5466
                                                  SQL_CA1_POSITIONED_DELETE |
                                                  SQL_CA1_BULK_ADD), StringLengthPtr);
    break;
  case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2:
    // An SQLUINTEGER bitmask that describes the attributes of a forward-only cursor that are supported by the driver.
    // SQL_CA2_READ_ONLY_CONCURRENCY = A read-only forward-only cursor, in which no updates are allowed, is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_READ_ONLY for a forward-only cursor).
    // SQL_CA2_LOCK_CONCURRENCY = A forward-only cursor that uses the lowest level of locking sufficient to make sure that the row can be updated is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_LOCK for a forward-only cursor.) These locks must be consistent with the transaction isolation level set by the SQL_ATTR_TXN_ISOLATION connection attribute.
    // SQL_CA2_OPT_ROWVER_CONCURRENCY = A forward-only cursor that uses the optimistic concurrency control comparing row versions is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_ROWVER for a forward-only cursor.)
    // SQL_CA2_OPT_VALUES_CONCURRENCY = A forward-only cursor that uses the optimistic concurrency control comparing values is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_VALUES for a forward-only cursor.)
    // SQL_CA2_SENSITIVITY_ADDITIONS = Added rows are visible to a forward-only cursor; the cursor can scroll to those rows. (Where these rows are added to the cursor is driver-dependent.)
    // SQL_CA2_SENSITIVITY_DELETIONS = Deleted rows are no longer available to a forward-only cursor, and do not leave a "hole" in the result set; after the forward-only cursor scrolls from a deleted row, it cannot return to that row.
    // SQL_CA2_SENSITIVITY_UPDATES = Updates to rows are visible to a forward-only cursor; if the forward-only cursor scrolls from and returns to an updated row, the data returned by the cursor is the updated data, not the original data.
    // SQL_CA2_MAX_ROWS_SELECT = The SQL_ATTR_MAX_ROWS statement attribute affects SELECT statements when the cursor is a forward-only cursor.
    // SQL_CA2_MAX_ROWS_INSERT = The SQL_ATTR_MAX_ROWS statement attribute affects INSERT statements when the cursor is a forward-only cursor.
    // SQL_CA2_MAX_ROWS_DELETE = The SQL_ATTR_MAX_ROWS statement attribute affects DELETE statements when the cursor is a forward-only cursor.
    // SQL_CA2_MAX_ROWS_UPDATE = The SQL_ATTR_MAX_ROWS statement attribute affects UPDATE statements when the cursor is a forward-only cursor.
    // SQL_CA2_MAX_ROWS_CATALOG = The SQL_ATTR_MAX_ROWS statement attribute affects CATALOG result sets when the cursor is a forward-only cursor.
    // SQL_CA2_MAX_ROWS_AFFECTS_ALL = The SQL_ATTR_MAX_ROWS statement attribute affects SELECT, INSERT, DELETE, and UPDATE statements, and CATALOG result sets, when the cursor is a forward-only cursor.
    // SQL_CA2_CRC_EXACT = The exact row count is available in the SQL_DIAG_CURSOR_ROW_COUNT diagnostic field when the cursor is a forward-only cursor.
    // SQL_CA2_CRC_APPROXIMATE = An approximate row count is available in the SQL_DIAG_CURSOR_ROW_COUNT diagnostic field when the cursor is a forward-only cursor.
    // SQL_CA2_SIMULATE_NON_UNIQUE = The driver does not guarantee that simulated positioned update or delete statements will affect only one row when the cursor is a forward-only cursor; it is the application's responsibility to guarantee this. (If a statement affects more than one row, SQLExecute or SQLExecDirect returns SQLSTATE 01001 [Cursor operation conflict].) To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_NON_UNIQUE.
    // SQL_CA2_SIMULATE_TRY_UNIQUE = The driver tries to guarantee that simulated positioned update or delete statements will affect only one row when the cursor is a forward-only cursor. The driver always executes such statements, even if they might affect more than one row, such as when there is no unique key. (If a statement affects more than one row, SQLExecute or SQLExecDirect returns SQLSTATE 01001 [Cursor operation conflict].) To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_TRY_UNIQUE.
    // SQL_CA2_SIMULATE_UNIQUE = The driver guarantees that simulated positioned update or delete statements will affect only one row when the cursor is a forward-only cursor. If the driver cannot guarantee this for a given statement, SQLExecDirect or SQLPrepare return SQLSTATE 01001 (Cursor operation conflict). To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_UNIQUE.
    // TODO PLAT-5473 make these limitations stricter
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, (Dbc->Dsn && Dbc->Dsn->NoCache ? 0: SQL_CA2_CRC_EXACT) |
                                                  SQL_CA2_READ_ONLY_CONCURRENCY |
                                                  SQL_CA2_MAX_ROWS_SELECT |
                                                  SQL_CA2_MAX_ROWS_INSERT |
                                                  SQL_CA2_MAX_ROWS_DELETE |
                                                  SQL_CA2_MAX_ROWS_UPDATE, StringLengthPtr);
    break;
  case SQL_GETDATA_EXTENSIONS:
  {
    // SQL_GETDATA_EXTENSIONS enumerates extensions to SQLGetData.
    // SQL_GD_ANY_COLUMN = SQLGetData can be called for any unbound column, including those before the last bound column. Note that the columns must be called in order of ascending column number unless SQL_GD_ANY_ORDER is also returned.
    // SQL_GD_ANY_ORDER = SQLGetData can be called for unbound columns in any order. Note that SQLGetData can be called only for columns after the last bound column unless SQL_GD_ANY_COLUMN is also returned.
    // SQL_GD_BLOCK = SQLGetData can be called for an unbound column in any row in a block (where the rowset size is greater than 1) of data after positioning to that row with SQLSetPos.
    // SQL_GD_BOUND = SQLGetData can be called for bound columns in addition to unbound columns. A driver cannot return this value unless it also returns SQL_GD_ANY_COLUMN.
    // SQL_GD_OUTPUT_PARAMS = SQLGetData can be called to return output parameter values. (SingleStore doesn't support output parameters, so this feature is disabled)
    // TODO PLAT-5418 check that these values are correct
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER |
                                                SQL_GD_BLOCK | SQL_GD_BOUND, StringLengthPtr);
    break;
  }
  case SQL_GROUP_BY:
    // SQL_GROUP_BY specifies the relationship between the columns in the GROUP BY clause and the nonaggregated columns in the select list:
    // SQL_GB_NO_RELATION = The columns in the GROUP BY clause and the select list are not related.
    // The meaning of nongrouped, nonaggregated columns in the select list is data source-dependent.
    // For example, SELECT DEPT, SALARY FROM EMPLOYEE GROUP BY DEPT, AGE. (ODBC 2.0)
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_GB_NO_RELATION, StringLengthPtr);
    break;
  case SQL_IDENTIFIER_CASE:
    // SQL_IC_SENSITIVE = Identifiers in SQL are case sensitive and are stored in mixed case in system catalog.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_IC_SENSITIVE, StringLengthPtr);
    break;
  case SQL_IDENTIFIER_QUOTE_CHAR:
    // SQL_IDENTIFIER_QUOTE_CHAR is the character string that is used as the starting and ending delimiter of a quoted (delimited) identifier in SQL statements.
    // TODO DB-46842
    // If this is ever changed please update MADB_DynStrAppendQuoted() and ProcessIdentifierString()
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),"`", SQL_NTS, &Dbc->Error);
    break;
  case SQL_INDEX_KEYWORDS:
    // SQL_INDEX_KEYWORDS enumerates keywords in the CREATE INDEX statement that are supported by the driver.
    // SQL_IK_ALL = All keywords are supported.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_IK_ALL, StringLengthPtr);
    break;
  case SQL_INFO_SCHEMA_VIEWS:
    // SQL_INFO_SCHEMA_VIEWS enumerates the views in the INFORMATION_SCHEMA that are supported by the driver.
    // SQL_ISV_ASSERTIONS = Identifies the catalog's assertions that are owned by a given user. (Full level)
    // SQL_ISV_CHARACTER_SETS = Identifies the catalog's character sets that can be accessed by a given user. (Intermediate level)
    // SQL_ISV_CHECK_CONSTRAINTS = Identifies the CHECK constraints that are owned by a given user. (Intermediate level)
    // SQL_ISV_COLLATIONS = Identifies the character collations for the catalog that can be accessed by a given user. (Full level)
    // SQL_ISV_COLUMN_DOMAIN_USAGE = Identifies columns for the catalog that depend on domains defined in the catalog and are owned by a given user. (Intermediate level)
    // SQL_ISV_COLUMN_PRIVILEGES = Identifies the privileges on columns of persistent tables that are available to or granted by a given user. (FIPS Transitional level)
    // SQL_ISV_COLUMNS = Identifies the columns of persistent tables that can be accessed by a given user. (FIPS Transitional level)
    // SQL_ISV_CONSTRAINT_COLUMN_USAGE = Similar to CONSTRAINT_TABLE_USAGE view, columns are identified for the various constraints that are owned by a given user. (Intermediate level)
    // SQL_ISV_CONSTRAINT_TABLE_USAGE = Identifies the tables that are used by constraints (referential, unique, and assertions), and are owned by a given user. (Intermediate level)
    // SQL_ISV_DOMAIN_CONSTRAINTS = Identifies the domain constraints (of the domains in the catalog) that can be accessed by a given user. (Intermediate level)
    // SQL_ISV_DOMAINS = Identifies the domains defined in a catalog that can be accessed by the user. (Intermediate level)
    // SQL_ISV_KEY_COLUMN_USAGE = Identifies columns defined in the catalog that are constrained as keys by a given user. (Intermediate level)
    // SQL_ISV_REFERENTIAL_CONSTRAINTS = Identifies the referential constraints that are owned by a given user. (Intermediate level)
    // SQL_ISV_SCHEMATA = Identifies the schemas that are owned by a given user. (Intermediate level)
    // SQL_ISV_SQL_LANGUAGES = Identifies the SQL conformance levels, options, and dialects supported by the SQL implementation. (Intermediate level)
    // SQL_ISV_TABLE_CONSTRAINTS = Identifies the table constraints that are owned by a given user. (Intermediate level)
    // SQL_ISV_TABLE_PRIVILEGES = Identifies the privileges on persistent tables that are available to or granted by a given user. (FIPS Transitional level)
    // SQL_ISV_TABLES = Identifies the persistent tables defined in a catalog that can be accessed by a given user. (FIPS Transitional level)
    // SQL_ISV_TRANSLATIONS = Identifies character translations for the catalog that can be accessed by a given user. (Full level)
    // SQL_ISV_USAGE_PRIVILEGES = Identifies the USAGE privileges on catalog objects that are available to or owned by a given user. (FIPS Transitional level)
    // SQL_ISV_VIEW_COLUMN_USAGE = Identifies the columns on which the catalog's views that are owned by a given user are dependent. (Intermediate level)
    // SQL_ISV_VIEW_TABLE_USAGE = Identifies the tables on which the catalog's views that are owned by a given user are dependent. (Intermediate level)
    // SQL_ISV_VIEWS = Identifies the viewed tables defined in this catalog that can be accessed by a given user. (FIPS Transitional level)
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_ISV_CHARACTER_SETS | SQL_ISV_COLLATIONS |
                                                SQL_ISV_COLUMNS | SQL_ISV_COLUMN_PRIVILEGES |
                                                SQL_ISV_KEY_COLUMN_USAGE | SQL_ISV_REFERENTIAL_CONSTRAINTS |
                                                SQL_ISV_SCHEMATA | SQL_ISV_TABLES | SQL_ISV_TABLE_PRIVILEGES |
                                                SQL_ISV_TABLE_CONSTRAINTS | SQL_ISV_VIEWS, StringLengthPtr);
    break;
  case SQL_INSERT_STATEMENT:
    // SQL_INSERT_STATEMENT indicates support for INSERT statements.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_IS_INSERT_LITERALS | SQL_IS_INSERT_SEARCHED |
                                                SQL_IS_SELECT_INTO, StringLengthPtr);
    break;
  case SQL_INTEGRITY:
    // SQL_INTEGRITY is a character string: "Y" if the data source supports the Integrity Enhancement Facility; "N" if it does not.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_KEYSET_CURSOR_ATTRIBUTES1:
    // SQL_KEYSET_CURSOR_ATTRIBUTES1 describes the attributes of a keyset cursor that are supported by the driver.
    // This driver doesn't support keyset cursor, so we are returning 0
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_KEYSET_CURSOR_ATTRIBUTES2:
    // SQL_KEYSET_CURSOR_ATTRIBUTES2 describes the attributes of a keyset cursor that are supported by the driver.
    // This driver doesn't support keyset cursor, so we are returning 0
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_KEYWORDS:
    // SQL_KEYWORDS is a character string that contains a comma-separated list of all data source-specific keywords.
    // This list does not contain keywords specific to ODBC or keywords used by both the data source and ODBC.
    // This list represents all the reserved keywords; interoperable applications should not use these words in object names.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                        SINGLESTORE_KEYWORDS, SQL_NTS, &Dbc->Error);
    break;
  case SQL_LIKE_ESCAPE_CLAUSE:
    // SQL_LIKE_ESCAPE_CLAUSE is a character string: "Y" if the data source supports an escape character for the percent character (%)
    // and underscore character (_) in a LIKE predicate and the driver supports the ODBC syntax for defining a LIKE predicate escape character.
    // "N" otherwise.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_MAX_ASYNC_CONCURRENT_STATEMENTS:
    // SQL_MAX_ASYNC_CONCURRENT_STATEMENTS  specifies the maximum number of active concurrent statements in asynchronous mode that the driver can support on a given connection.
    // We are returning 0. It means that we don't have a specific limit or limit is unknown.
    // TODO PLAT-5472
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_BINARY_LITERAL_LEN:
    // SQL_MAX_BINARY_LITERAL_LEN specifies the maximum length (number of hexadecimal characters, excluding the literal prefix and suffix returned by SQLGetTypeInfo)
    // of a binary literal in an SQL statement.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_CATALOG_NAME_LEN:
    // SQL_MAX_CATALOG_NAME_LEN specifies the maximum length of a catalog name in the data source.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 60 * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_CHAR_LITERAL_LEN:
    // SQL_MAX_CHAR_LITERAL_LEN specifies the maximum length (number of characters, excluding the literal prefix and suffix returned by SQLGetTypeInfo)
    // of a character literal in an SQL statement.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_COLUMN_NAME_LEN:
    // SQL_MAX_COLUMN_NAME_LEN pecifies the maximum length of a column name in the data source.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_GROUP_BY:
    // SQL_MAX_COLUMNS_IN_GROUP_BY pecifies the maximum number of columns allowed in a GROUP BY clause.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_INDEX:
    // SQL_MAX_COLUMNS_IN_INDEX specifies the maximum number of columns allowed in an index.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 32, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_ORDER_BY:
    // SQL_MAX_COLUMNS_IN_ORDER_BY specifies the maximum number of columns allowed in an ORDER BY clause.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_SELECT:
    // SQL_MAX_COLUMNS_IN_SELECT specifies the maximum number of columns allowed in a select list.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_TABLE:
    // SQL_MAX_COLUMNS_IN_TABLE specifies the maximum number of columns allowed in a table.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 4096, StringLengthPtr);
    break;
  case SQL_MAX_CONCURRENT_ACTIVITIES:
    // SQL_MAX_CONCURRENT_ACTIVITIES specifies the maximum number of active statements that the driver can support for a connection.
    // We are returning 0. It means that we don't have specific limit or limit is unknown.
    // TODO PLAT-5472
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_CURSOR_NAME_LEN:
    // SQL_MAX_CURSOR_NAME_LEN specifies the maximum length of a cursor name in the data source.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, MADB_MAX_CURSOR_NAME, StringLengthPtr);
    break;
  case SQL_MAX_DRIVER_CONNECTIONS:
    // SQL_MAX_DRIVER_CONNECTIONS specifies the maximum number of active connections that the driver can support for an environment.
    // We are returning 0. It means that we don't have specific limit or limit is unknown.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_IDENTIFIER_LEN:
    // SQL_MAX_IDENTIFIER_LEN indicates the maximum size in characters that the data source supports for user-defined names.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_INDEX_SIZE:
    // SQL_MAX_INDEX_SIZE specifies the maximum number of bytes allowed in the combined fields of an index.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_PROCEDURE_NAME_LEN:
    // SQL_MAX_PROCEDURE_NAME_LEN specifies the maximum length of a procedure name in the data source.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_ROW_SIZE:
    // SQL_MAX_ROW_SIZE specifies the maximum length of a single row in a table.
    // In SingleStore this limit is 64KB
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 1<<26, StringLengthPtr);
    break;
    // SQL_MAX_ROW_SIZE_INCLUDES_LONG is a character string: "Y" if the maximum row size returned for the SQL_MAX_ROW_SIZE information type
    // includes the length of all SQL_LONGVARCHAR and SQL_LONGVARBINARY columns in the row; "N" otherwise.
  case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_MAX_SCHEMA_NAME_LEN:
    // SQL_MAX_SCHEMA_NAME_LEN specifies the maximum length of a schema name in the data source.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_STATEMENT_LEN:
    {
      // SQL_MAX_STATEMENT_LEN specifies the maximum length (number of characters, including white space) of an SQL statement.
      size_t max_packet_size;
      mariadb_get_infov(Dbc->mariadb, MARIADB_MAX_ALLOWED_PACKET, (void*)&max_packet_size);
      MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, (SQLUINTEGER)max_packet_size, StringLengthPtr);
    }
    break;
  case SQL_MAX_TABLE_NAME_LEN:
    // SQL_MAX_TABLE_NAME_LEN specifies the maximum length of a table name in the data source.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_TABLES_IN_SELECT:
    // SQL_MAX_TABLES_IN_SELECT specifies the maximum number of tables allowed in the FROM clause of a SELECT statement.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 253, StringLengthPtr);
    break;
  case SQL_MAX_USER_NAME_LEN:
    // SQL_MAX_USER_NAME_LEN specifies the maximum length of a user name in the data source.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 32 * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MULT_RESULT_SETS:
    // SQL_MULT_RESULT_SETS is a character string: "Y" if the data source supports multiple result sets, "N" if it does not.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_MULTIPLE_ACTIVE_TXN:
    // SQL_MULTIPLE_ACTIVE_TXN 	is a character string: "Y" if the driver supports more than one active transaction at the same time,
    // "N" if only one transaction can be active at any time.
    // TODO PLAT-5414
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_NEED_LONG_DATA_LEN:
    // SQL_NEED_LONG_DATA_LEN is a character string: "Y" if the data source needs the length of a long data value
    // (the data type is SQL_LONGVARCHAR, SQL_LONGVARBINARY, or a long data source-specific data type)
    // before that value is sent to the data source, "N" if it does not.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_NON_NULLABLE_COLUMNS:
    // SQL_NON_NULLABLE_COLUMNS specifies whether the data source supports NOT NULL in columns.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_NNC_NON_NULL, StringLengthPtr);
    break;
  case SQL_NULL_COLLATION:
    // SQL_NULL_COLLATION specifies where NULLs are sorted in a result set
    // SQL_NC_LOW = NULLs are sorted at the low end of the result set, depending on the ASC or DESC keywords.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_NC_LOW, StringLengthPtr);
    break;
  case SQL_NUMERIC_FUNCTIONS:
    // SQL_NUMERIC_FUNCTIONS enumerates the scalar numeric functions supported by the driver and associated data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_NUM_ABS | SQL_FN_NUM_ACOS | SQL_FN_NUM_ASIN |
                                                SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2 | SQL_FN_NUM_CEILING |
                                                SQL_FN_NUM_COS | SQL_FN_NUM_COT | SQL_FN_NUM_EXP |
                                                SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_MOD |
                                                SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT |
                                                SQL_FN_NUM_TAN | SQL_FN_NUM_PI | SQL_FN_NUM_RAND |
                                                SQL_FN_NUM_DEGREES | SQL_FN_NUM_LOG10 | SQL_FN_NUM_POWER |
                                                SQL_FN_NUM_RADIANS | SQL_FN_NUM_ROUND | SQL_FN_NUM_TRUNCATE,
                     StringLengthPtr);
    break;
  case SQL_ODBC_API_CONFORMANCE:
    MADB_SET_NUM_VAL(SQLSMALLINT, InfoValuePtr, SQL_OAC_LEVEL1, StringLengthPtr);
    break;
  case SQL_ODBC_INTERFACE_CONFORMANCE:
    // SQL_ODBC_INTERFACE_CONFORMANCE indicates the level of the ODBC 3*.x* interface that the driver complies with.
    // SQL_OIC_CORE: The minimum level that all ODBC drivers are expected to comply with.
    // This level includes basic interface elements such as connection functions, functions for preparing and executing an SQL statement, basic result set metadata functions, basic catalog functions, and so on.
    // We don't support bookmarks and positioned updates, so we can return only SQL_OIC_CORE
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_OIC_CORE, StringLengthPtr);
    break;
  case SQL_ODBC_SQL_CONFORMANCE:
    MADB_SET_NUM_VAL(SQLSMALLINT, InfoValuePtr, SQL_OSC_CORE, StringLengthPtr);
    break;
  case SQL_ODBC_VER:
    break;
  case SQL_OUTER_JOINS:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_OJ_CAPABILITIES:
    // SQL_OJ_CAPABILITIES enumerates the types of outer joins supported by the driver and data source.
    // SQL_OJ_LEFT = Left outer joins are supported.
    // SQL_OJ_RIGHT = Right outer joins are supported.
    // SQL_OJ_FULL = Full outer joins are supported.
    // SQL_OJ_NESTED = Nested outer joins are supported.
    // SQL_OJ_NOT_ORDERED = The column names in the ON clause of the outer join do not have to be in the same order as their respective table names in the OUTER JOIN clause.
    // SQL_OJ_INNER = The inner table (the right table in a left outer join or the left table in a right outer join) can also be used in an inner join. This does not apply to full outer joins, which do not have an inner table.
    // SQL_OJ_ALL_COMPARISON_OPS = The comparison operator in the ON clause can be any of the ODBC comparison operators. If this bit is not set, only the equals (=) comparison operator can be used in outer joins.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr,  SQL_OJ_LEFT | SQL_OJ_RIGHT | SQL_OJ_FULL | SQL_OJ_NESTED |
        SQL_OJ_NOT_ORDERED | SQL_OJ_INNER | SQL_OJ_ALL_COMPARISON_OPS, StringLengthPtr);
    break;
  case SQL_ORDER_BY_COLUMNS_IN_SELECT:
    // SQL_ORDER_BY_COLUMNS_IN_SELECT is a character string: "Y" if the columns in the ORDER BY clause must be in the select list; otherwise, "N".
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_PARAM_ARRAY_ROW_COUNTS:
    // SQL_PARAM_ARRAY_ROW_COUNTS enumerates the driver's properties regarding the availability of row counts in a parameterized execution.
    // SQL_PARC_NO_BATCH = There is only one row count available, which is the cumulative row count resulting from the execution of the statement for the entire array of parameters.
    // This is conceptually equivalent to treating the statement together with the complete parameter array as one atomic unit.
    // Errors are handled the same as if one statement were executed.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_PARC_NO_BATCH, StringLengthPtr);
    break;
  case SQL_PARAM_ARRAY_SELECTS:
    // SQL_PARAM_ARRAY_SELECTS enumerates the driver's properties regarding the availability of result sets in a parameterized execution.
    // SQL_PAS_NO_BATCH = There is only one result set available, which represents the cumulative result set resulting from the execution of the statement for the complete array of parameters.
    // This is conceptually equivalent to treating the statement together with the complete parameter array as one atomic unit.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_PAS_NO_BATCH, StringLengthPtr);
    break;
  case SQL_PROCEDURE_TERM:
    // SQL_PROCEDURE_TERM is a character string with the data source vendor's name for a procedure.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "procedure", SQL_NTS, &Dbc->Error);
    break;
  case SQL_PROCEDURES:
    // SQL_PROCEDURES is a character string: "Y" if the data source supports procedures and the driver supports the ODBC procedure invocation syntax; "N" otherwise.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_QUOTED_IDENTIFIER_CASE:
    // SQL_IC_SENSITIVE = Quoted identifiers in SQL are case sensitive and are stored in mixed case in the system catalog.
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_IC_SENSITIVE, StringLengthPtr);
    break;
  case SQL_ROW_UPDATES:
    // SQL_ROW_UPDATES is "Y" if a keyset-driven or mixed cursor maintains row versions or values for all fetched rows
    // and therefore can detect any updates that were made to a row by any user since the row was last fetched.
    // This driver doesn't support keyset-driven or mixed cursor, so we are returning "N".
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_SCHEMA_TERM:
    // SQL_SCHEMA_TERM is a character string with the data source vendor's name for a schema.
    // TODO PLAT-5484
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "", SQL_NTS, &Dbc->Error);
    break;
  case SQL_SCHEMA_USAGE:
    // SQL_SCHEMA_USAGE enumerates the statements in which schemas can be used:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_SCROLL_OPTIONS:
    {
      // SQL_SCROLL_OPTIONS enumerates the scroll options supported for scrollable cursors.
      // SQL_SO_FORWARD_ONLY = The cursor only scrolls forward. (ODBC 1.0)
      // SQL_SO_STATIC = The data in the result set is static. (ODBC 2.0)
      // SQL_SO_KEYSET_DRIVEN = The driver saves and uses the keys for every row in the result set. (ODBC 1.0)
      // SQL_SO_DYNAMIC = The driver keeps the keys for every row in the rowset (the keyset size is the same as the rowset size). (ODBC 1.0)
      // SQL_SO_MIXED = The driver keeps the keys for every row in the keyset, and the keyset size is greater than the rowset size. The cursor is keyset-driven inside the keyset and dynamic outside the keyset. (ODBC 1.0)
      // TODO PLAT-5418
      SQLUINTEGER Options= SQL_SO_FORWARD_ONLY;
      if (!MA_ODBC_CURSOR_FORWARD_ONLY(Dbc))
        Options|= SQL_SO_STATIC;
      if (MA_ODBC_CURSOR_DYNAMIC(Dbc))
        Options|= SQL_SO_DYNAMIC;
      MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, Options, StringLengthPtr);
    }
    break;
  case SQL_SEARCH_PATTERN_ESCAPE:
    // SQL_SEARCH_PATTERN_ESCAPE specifies what the driver supports as an escape character that allows the use of the
    // pattern match metacharacters underscore (_) and percent sign (%) as valid characters in search patterns.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "\\", SQL_NTS, &Dbc->Error);
    break;
  case SQL_SERVER_NAME:
  {
    // SQL_SERVER_NAME is the actual data source-specific server name.
    const char *Host= "";
    if (Dbc->mariadb)
    {
      mariadb_get_infov(Dbc->mariadb, MARIADB_CONNECTION_HOST, (void*)&Host);
    }
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
      BUFFER_CHAR_LEN(BufferLength, isWChar),
      Host, SQL_NTS, &Dbc->Error);
    break;
  }
  case SQL_SPECIAL_CHARACTERS:
    // SQL_SPECIAL_CHARACTERS A character string that contains all special characters
    // (that is, all characters except a through z, A through Z, 0 through 9, and underscore)
    // that can be used in an identifier name, such as a table name, column name, or index name, on the data source.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "\"\\/!@#$%^&*()-=+<>|:;,.%?", SQL_NTS, &Dbc->Error);
    break;
  case SQL_SQL_CONFORMANCE:
    // SQL_SQL_CONFORMANCE indicates the level of SQL-92 supported by the driver
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SC_SQL92_INTERMEDIATE, StringLengthPtr);
    break;
  case SQL_SQL92_DATETIME_FUNCTIONS:
    // SQL_SQL92_DATETIME_FUNCTIONS enumerates the datetime scalar functions that are supported by the driver and the associated data source, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SDF_CURRENT_DATE | SQL_SDF_CURRENT_TIME |
                                                 SQL_SDF_CURRENT_TIMESTAMP, StringLengthPtr);
    break;
  case SQL_SQL92_FOREIGN_KEY_DELETE_RULE:
    // SQL_SQL92_FOREIGN_KEY_DELETE_RULE enumerates the rules supported for a foreign key in a DELETE statement, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_SQL92_FOREIGN_KEY_UPDATE_RULE:
    // SQL_SQL92_FOREIGN_KEY_UPDATE_RULE enumerates the rules supported for a foreign key in an UPDATE statement, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_SQL92_GRANT:
    // SQL_SQL92_GRANT enumerates the clauses supported in the GRANT statement, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SG_DELETE_TABLE | SQL_SG_INSERT_COLUMN |
                                                SQL_SG_INSERT_TABLE | SQL_SG_SELECT_TABLE |
                                                SQL_SG_UPDATE_COLUMN | SQL_SG_UPDATE_TABLE |
                                                SQL_SG_WITH_GRANT_OPTION, StringLengthPtr);
    break;
  case SQL_SQL92_NUMERIC_VALUE_FUNCTIONS:
    // SQL_SQL92_NUMERIC_VALUE_FUNCTIONS enumerates the numeric value scalar functions that are supported by the driver and the associated data source, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SNVF_BIT_LENGTH | SQL_SNVF_CHARACTER_LENGTH |
                                                SQL_SNVF_CHAR_LENGTH | SQL_SNVF_EXTRACT |
                                                SQL_SNVF_OCTET_LENGTH | SQL_SNVF_POSITION,
                     StringLengthPtr);
    break;
  case SQL_SQL92_PREDICATES:
    // SQL_SQL92_PREDICATES enumerates the predicates supported in a SELECT statement, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SP_BETWEEN | SQL_SP_COMPARISON |
                                                SQL_SP_EXISTS | SQL_SP_IN | SQL_SP_ISNOTNULL |
                                                SQL_SP_ISNULL | SQL_SP_LIKE | SQL_SP_QUANTIFIED_COMPARISON,
                     StringLengthPtr);
    break;
  case SQL_SQL92_RELATIONAL_JOIN_OPERATORS:
    // SQL_SQL92_RELATIONAL_JOIN_OPERATORS enumerates the relational join operators supported in a SELECT statement, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SRJO_CROSS_JOIN | SQL_SRJO_FULL_OUTER_JOIN | SQL_SRJO_INNER_JOIN |
                                                SQL_SRJO_LEFT_OUTER_JOIN | SQL_SRJO_RIGHT_OUTER_JOIN |
                                                SQL_SRJO_NATURAL_JOIN,
                     StringLengthPtr);
    break;
  case SQL_SQL92_REVOKE:
    // SQL_SQL92_REVOKE enumerates the clauses supported in the REVOKE statement, as defined in SQL-92, supported by the data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SR_DELETE_TABLE | SQL_SR_INSERT_COLUMN |
                                                SQL_SR_INSERT_TABLE | SQL_SR_SELECT_TABLE |
                                                SQL_SR_UPDATE_COLUMN | SQL_SR_UPDATE_TABLE,
                     StringLengthPtr);
    break;
  case SQL_SQL92_ROW_VALUE_CONSTRUCTOR:
    // SQL_SQL92_ROW_VALUE_CONSTRUCTOR enumerates the row value constructor expressions supported in a SELECT statement, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SRVC_DEFAULT | SQL_SRVC_NULL |
                                                 SQL_SRVC_ROW_SUBQUERY | SQL_SRVC_VALUE_EXPRESSION,
                     StringLengthPtr);
    break;
  case SQL_SQL92_STRING_FUNCTIONS:
    // SQL_SQL92_STRING_FUNCTIONS enumerates the string scalar functions that are supported by the driver and the associated data source, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SSF_CONVERT | SQL_SSF_LOWER |
                                                  SQL_SSF_SUBSTRING | SQL_SSF_TRIM_BOTH |
                                                  SQL_SSF_TRIM_LEADING | SQL_SSF_TRIM_TRAILING |
                                                  SQL_SSF_UPPER,
                     StringLengthPtr);
    break;
  case SQL_SQL92_VALUE_EXPRESSIONS:
    // SQL_SQL92_VALUE_EXPRESSIONS enumerates the value expressions supported, as defined in SQL-92.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SVE_CASE | SQL_SVE_CAST | SQL_SVE_COALESCE |
                                                SQL_SVE_NULLIF, StringLengthPtr);
    break;
  case SQL_STANDARD_CLI_CONFORMANCE:
    // SQL_STANDARD_CLI_CONFORMANCE enumerates the CLI standard or standards to which the driver conforms.
    // SQL_SCC_ISO92_CLI: The driver complies with the ISO 92 CLI.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SCC_ISO92_CLI, StringLengthPtr);
    break;
  case SQL_STATIC_CURSOR_ATTRIBUTES1:
    // SQL_STATIC_CURSOR_ATTRIBUTES1 describes the attributes of a static cursor that are supported by the driver.
    // SQL_CA1_NEXT = A FetchOrientation argument of SQL_FETCH_NEXT is supported in a call to SQLFetchScroll when the cursor is a static cursor.
    // SQL_CA1_ABSOLUTE = FetchOrientation arguments of SQL_FETCH_FIRST, SQL_FETCH_LAST, and SQL_FETCH_ABSOLUTE are supported in a call to SQLFetchScroll when the cursor is a static cursor. (The rowset that will be fetched is independent of the current cursor position.)
    // SQL_CA1_RELATIVE = FetchOrientation arguments of SQL_FETCH_PRIOR and SQL_FETCH_RELATIVE are supported in a call to SQLFetchScroll when the cursor is a static cursor. (The rowset that will be fetched depends on the current cursor position. Note that this is separated from SQL_FETCH_NEXT because in a forward-only cursor, only SQL_FETCH_NEXT is supported.)
    // SQL_CA1_BOOKMARK = A FetchOrientation argument of SQL_FETCH_BOOKMARK is supported in a call to SQLFetchScroll when the cursor is a static cursor.
    // SQL_CA1_LOCK_EXCLUSIVE = A LockType argument of SQL_LOCK_EXCLUSIVE is supported in a call to SQLSetPos when the cursor is a static cursor.
    // SQL_CA1_LOCK_NO_CHANGE = A LockType argument of SQL_LOCK_NO_CHANGE is supported in a call to SQLSetPos when the cursor is a static cursor.
    // SQL_CA1_LOCK_UNLOCK = A LockType argument of SQL_LOCK_UNLOCK is supported in a call to SQLSetPos when the cursor is a static cursor.
    // SQL_CA1_POS_POSITION = An Operation argument of SQL_POSITION is supported in a call to SQLSetPos when the cursor is a static cursor.
    // SQL_CA1_POS_UPDATE = An Operation argument of SQL_UPDATE is supported in a call to SQLSetPos when the cursor is a static cursor.
    // SQL_CA1_POS_DELETE = An Operation argument of SQL_DELETE is supported in a call to SQLSetPos when the cursor is a static cursor.
    // SQL_CA1_POS_REFRESH = An Operation argument of SQL_REFRESH is supported in a call to SQLSetPos when the cursor is a static cursor.
    // SQL_CA1_POSITIONED_UPDATE = An UPDATE WHERE CURRENT OF SQL statement is supported when the cursor is a static cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_POSITIONED_DELETE = A DELETE WHERE CURRENT OF SQL statement is supported when the cursor is a static cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_SELECT_FOR_UPDATE = A SELECT FOR UPDATE SQL statement is supported when the cursor is a static cursor. (An SQL-92 Entry level-conformant driver will always return this option as supported.)
    // SQL_CA1_BULK_ADD = An Operation argument of SQL_ADD is supported in a call to SQLBulkOperations when the cursor is a static cursor.
    // SQL_CA1_BULK_UPDATE_BY_BOOKMARK = An Operation argument of SQL_UPDATE_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a static cursor.
    // SQL_CA1_BULK_DELETE_BY_BOOKMARK = An Operation argument of SQL_DELETE_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a static cursor.
    // SQL_CA1_BULK_FETCH_BY_BOOKMARK = An Operation argument of SQL_FETCH_BY_BOOKMARK is supported in a call to SQLBulkOperations when the cursor is a static cursor.
    // TODO PLAT-5473 make these limitations stricter
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA1_NEXT |
                                                SQL_CA1_LOCK_NO_CHANGE |
                                                SQL_CA1_POS_POSITION |
                                                // SQL_CA1_POS_UPDATE | TODO PLAT-5080
                                                SQL_CA1_POS_DELETE |
                                                // SQL_CA1_POS_REFRESH | TODO PLAT-5466
                                                // SQL_CA1_POSITIONED_UPDATE | TODO PLAT-5466
                                                SQL_CA1_POSITIONED_DELETE |
                                                SQL_CA1_BULK_ADD,
                     StringLengthPtr);
    break;
  case SQL_STATIC_CURSOR_ATTRIBUTES2:
    // SQL_STATIC_CURSOR_ATTRIBUTES2 describes the attributes of a static cursor that are supported by the driver.
    // SQL_CA2_READ_ONLY_CONCURRENCY = A read-only static cursor, in which no updates are allowed, is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_READ_ONLY for a static cursor).
    // SQL_CA2_LOCK_CONCURRENCY = A static cursor that uses the lowest level of locking sufficient to make sure that the row can be updated is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_LOCK for a static cursor.) These locks must be consistent with the transaction isolation level set by the SQL_ATTR_TXN_ISOLATION connection attribute.
    // SQL_CA2_OPT_ROWVER_CONCURRENCY = A static cursor that uses the optimistic concurrency control comparing row versions is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_ROWVER for a static cursor.)
    // SQL_CA2_OPT_VALUES_CONCURRENCY = A static cursor that uses the optimistic concurrency control comparing values is supported. (The SQL_ATTR_CONCURRENCY statement attribute can be SQL_CONCUR_VALUES for a static cursor.)
    // SQL_CA2_SENSITIVITY_ADDITIONS = Added rows are visible to a static cursor; the cursor can scroll to those rows. (Where these rows are added to the cursor is driver-dependent.)
    // SQL_CA2_SENSITIVITY_DELETIONS = Deleted rows are no longer available to a static cursor, and do not leave a "hole" in the result set; after the static cursor scrolls from a deleted row, it cannot return to that row.
    // SQL_CA2_SENSITIVITY_UPDATES = Updates to rows are visible to a static cursor; if the static cursor scrolls from and returns to an updated row, the data returned by the cursor is the updated data, not the original data.
    // SQL_CA2_MAX_ROWS_SELECT = The SQL_ATTR_MAX_ROWS statement attribute affects SELECT statements when the cursor is a static cursor.
    // SQL_CA2_MAX_ROWS_INSERT = The SQL_ATTR_MAX_ROWS statement attribute affects INSERT statements when the cursor is a static cursor.
    // SQL_CA2_MAX_ROWS_DELETE = The SQL_ATTR_MAX_ROWS statement attribute affects DELETE statements when the cursor is a static cursor.
    // SQL_CA2_MAX_ROWS_UPDATE = The SQL_ATTR_MAX_ROWS statement attribute affects UPDATE statements when the cursor is a static cursor.
    // SQL_CA2_MAX_ROWS_CATALOG = The SQL_ATTR_MAX_ROWS statement attribute affects CATALOG result sets when the cursor is a static cursor.
    // SQL_CA2_MAX_ROWS_AFFECTS_ALL = The SQL_ATTR_MAX_ROWS statement attribute affects SELECT, INSERT, DELETE, and UPDATE statements, and CATALOG result sets, when the cursor is a static cursor.
    // SQL_CA2_CRC_EXACT = The exact row count is available in the SQL_DIAG_CURSOR_ROW_COUNT diagnostic field when the cursor is a static cursor.
    // SQL_CA2_CRC_APPROXIMATE = An approximate row count is available in the SQL_DIAG_CURSOR_ROW_COUNT diagnostic field when the cursor is a static cursor.
    // SQL_CA2_SIMULATE_NON_UNIQUE = The driver does not guarantee that simulated positioned update or delete statements will affect only one row when the cursor is a static cursor; it is the application's responsibility to guarantee this. (If a statement affects more than one row, SQLExecute or SQLExecDirect returns SQLSTATE 01001 [Cursor operation conflict].) To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_NON_UNIQUE.
    // SQL_CA2_SIMULATE_TRY_UNIQUE = The driver tries to guarantee that simulated positioned update or delete statements will affect only one row when the cursor is a static cursor. The driver always executes such statements, even if they might affect more than one row, such as when there is no unique key. (If a statement affects more than one row, SQLExecute or SQLExecDirect returns SQLSTATE 01001 [Cursor operation conflict].) To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_TRY_UNIQUE.
    // SQL_CA2_SIMULATE_UNIQUE = The driver guarantees that simulated positioned update or delete statements will affect only one row when the cursor is a static cursor. If the driver cannot guarantee this for a given statement, SQLExecDirect or SQLPrepare return SQLSTATE 01001 (Cursor operation conflict). To set this behavior, the application calls SQLSetStmtAttr with the SQL_ATTR_SIMULATE_CURSOR attribute set to SQL_SC_UNIQUE.
    // TODO PLAT-5473 make these limitations stricter
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA2_CRC_EXACT |
                                                SQL_CA2_READ_ONLY_CONCURRENCY |
                                                SQL_CA2_MAX_ROWS_SELECT |
                                                SQL_CA2_MAX_ROWS_INSERT |
                                                SQL_CA2_MAX_ROWS_DELETE |
                                                SQL_CA2_MAX_ROWS_UPDATE, StringLengthPtr);
    break;
  case SQL_STRING_FUNCTIONS:
    // SQL_STRING_FUNCTIONS enumerates the scalar string functions supported by the driver and associated data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_STR_ASCII | SQL_FN_STR_BIT_LENGTH |
                                                SQL_FN_STR_CHAR | SQL_FN_STR_CHAR_LENGTH | SQL_FN_STR_CHARACTER_LENGTH |
                                                SQL_FN_STR_CONCAT | SQL_FN_STR_INSERT | 
                                                SQL_FN_STR_LCASE | SQL_FN_STR_LEFT | 
                                                SQL_FN_STR_LENGTH | SQL_FN_STR_LOCATE |
                                                SQL_FN_STR_LOCATE_2 | SQL_FN_STR_LTRIM |
                                                SQL_FN_STR_OCTET_LENGTH | SQL_FN_STR_POSITION |
                                                SQL_FN_STR_REPEAT | SQL_FN_STR_REPLACE |
                                                SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM | SQL_FN_STR_SPACE |
                                                SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE,
                     StringLengthPtr);
    break;
  case SQL_SUBQUERIES:
    // SQL_SUBQUERIES enumerates the predicates that support subqueries.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SQ_COMPARISON | SQL_SQ_CORRELATED_SUBQUERIES |
                                                SQL_SQ_EXISTS | SQL_SQ_IN,
                     StringLengthPtr);
    break;
  case SQL_SYSTEM_FUNCTIONS:
    // SQL_SYSTEM_FUNCTIONS enumerates the scalar system functions supported by the driver and associated data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_SYS_DBNAME | SQL_FN_SYS_IFNULL |
                                                SQL_FN_SYS_USERNAME, StringLengthPtr);
    break;
  case SQL_TABLE_TERM:
    // SQL_TABLE_TERM is a character string with the data source vendor's name for a table.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
                                      BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "table", SQL_NTS, &Dbc->Error);
    break;
  case SQL_TIMEDATE_ADD_INTERVALS:
    // SQL_TIMEDATE_ADD_INTERVALS enumerates the timestamp intervals supported by the driver and associated data source for the TIMESTAMPADD scalar function.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE |
                     SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER |
                     SQL_FN_TSI_YEAR, StringLengthPtr);
    break;
  case SQL_TIMEDATE_DIFF_INTERVALS:
    // SQL_TIMEDATE_DIFF_INTERVALS enumerating the timestamp intervals supported by the driver and associated data source for the TIMESTAMPDIFF scalar function.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_TSI_FRAC_SECOND | SQL_FN_TSI_SECOND | SQL_FN_TSI_MINUTE |
                     SQL_FN_TSI_HOUR | SQL_FN_TSI_DAY | SQL_FN_TSI_WEEK | SQL_FN_TSI_MONTH | SQL_FN_TSI_QUARTER |
                     SQL_FN_TSI_YEAR, StringLengthPtr);
    break;
  case SQL_TIMEDATE_FUNCTIONS:
    // SQL_TIMEDATE_FUNCTIONS enumerates the scalar date and time functions supported by the driver and associated data source.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_TD_CURDATE | SQL_FN_TD_CURRENT_DATE | 
                                                SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_CURRENT_TIMESTAMP | 
                                                SQL_FN_TD_CURTIME | SQL_FN_TD_DAYNAME |
                                                SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK |
                                                SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_EXTRACT | 
                                                SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE | 
                                                SQL_FN_TD_MONTH | SQL_FN_TD_MONTHNAME | 
                                                SQL_FN_TD_NOW | SQL_FN_TD_QUARTER | 
                                                SQL_FN_TD_SECOND | SQL_FN_TD_TIMESTAMPADD |
                                                SQL_FN_TD_TIMESTAMPDIFF | SQL_FN_TD_WEEK | SQL_FN_TD_YEAR,
                      StringLengthPtr);
    break;
  case SQL_TXN_CAPABLE:
    // SQL_TXN_CAPABLE describes the transaction support in the driver or data source.
    // SQL_TC_DDL_COMMIT = Transactions can contain only DML statements.
    // DDL statements (CREATE TABLE, DROP INDEX, and so on) encountered in a transaction cause the transaction to be committed.
    // TODO PLAT-5414
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_TC_DDL_COMMIT, StringLengthPtr);
    break;
  case SQL_TXN_ISOLATION_OPTION:
    // SQL_TXN_ISOLATION_OPTION enumerates the transaction isolation levels available from the driver or data source.
    // SQL_TXN_READ_COMMITTED = Dirty reads are not possible. Non-repeatable reads and phantoms are possible.
    // TODO PLAT-5414
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_TXN_READ_COMMITTED, StringLengthPtr);
    break;
  case SQL_UNION:
    // SQL_UNION enumerates the support for the UNION clause
    // SQL_U_UNION = The data source supports the UNION clause.
    // SQL_U_UNION_ALL = The data source supports the ALL keyword in the UNION clause.
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_U_UNION | SQL_U_UNION_ALL, StringLengthPtr);
    break;
  case SQL_USER_NAME:
  {
    // SQL_USER_NAME is a character string with the name used in a particular database, which can be different from the login name.
    const char *User= "";
    if (Dbc->mariadb)
    {
      mariadb_get_infov(Dbc->mariadb, MARIADB_CONNECTION_USER, (void *)&User);
    }
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
                                     BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                      User, SQL_NTS, &Dbc->Error);
    break;
  }
  case SQL_XOPEN_CLI_YEAR:
    // SQL_XOPEN_CLI_YEAR is a character string that indicates the year of publication
    // of the Open Group specification with which the version of the ODBC Driver Manager fully complies.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
                                     BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "1992", SQL_NTS, &Dbc->Error);
    break;
  case SQL_DATA_SOURCE_READ_ONLY:
    // SQL_DATA_SOURCE_READ_ONLY is a character string "Y" if the data source is set to READ ONLY mode, "N" if it is otherwise.
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? &Dbc->Charset : NULL, (void *)InfoValuePtr,
                                     BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  /* 2.0 types */
  case SQL_POS_OPERATIONS:
    MADB_SET_NUM_VAL(SQLINTEGER, InfoValuePtr, SQL_POS_POSITION | SQL_POS_REFRESH | SQL_POS_UPDATE | SQL_POS_DELETE | SQL_POS_ADD,
                     StringLengthPtr);
    break;
  case SQL_STATIC_SENSITIVITY:
    MADB_SET_NUM_VAL(SQLINTEGER, InfoValuePtr, SQL_SS_DELETIONS | SQL_SS_UPDATES, StringLengthPtr);
    break;
  case SQL_LOCK_TYPES:
    MADB_SET_NUM_VAL(SQLINTEGER, InfoValuePtr, SQL_LCK_NO_CHANGE, StringLengthPtr);
    break;
  case SQL_SCROLL_CONCURRENCY:
    MADB_SET_NUM_VAL(SQLINTEGER, InfoValuePtr, SQL_SCCO_READ_ONLY | SQL_SCCO_OPT_VALUES, StringLengthPtr);
    break;
default:
    MADB_SetError(&Dbc->Error, MADB_ERR_HY096, NULL, 0);
    return Dbc->Error.ReturnValue;
  }
  if (isWChar && SLen)
  {
    SLen*= sizeof(SQLWCHAR);
  }
  if (IsStringInfoType(InfoType) && StringLengthPtr)
  {
    *StringLengthPtr= SLen;
  }
  
  return SQL_SUCCESS;
}
/* }}} */


/* {{{ MADB_DriverSideMemalocate */
char * MADB_DriverSideAllocate(size_t size)
{
  return (char *)MADB_CALLOC(size);
}
/* }}} */


/* {{{ MADB_DriverSideFree */
void MADB_DriverSideFree(void *ptr)
{
  MADB_FREE(ptr);
}
/* }}} */


/* {{{ MADB_DriverConnect */
SQLRETURN MADB_DriverConnect(MADB_Dbc *Dbc, SQLHWND WindowHandle, SQLCHAR *InConnectionString,
                             SQLULEN StringLength1, SQLCHAR *OutConnectionString,
                             SQLULEN BufferLength, SQLSMALLINT *StringLength2Ptr,
                             SQLUSMALLINT DriverCompletion)
{
  MADB_Dsn   *Dsn;
  MADB_Drv   *Drv=       NULL;
  SQLRETURN   ret=       SQL_SUCCESS;
  MADB_Prompt DSNPrompt= { NULL, NULL };
  SQLULEN     Length;

  if (!Dbc)
    return SQL_INVALID_HANDLE;

  MADB_CLEAR_ERROR(&Dbc->Error);

  Dsn= MADB_DSN_Init();

  if (!MADB_ReadConnString(Dsn, (char *)InConnectionString, StringLength1, ';'))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY000, "Error while parsing DSN", 0);
    goto error;
  }

  /* if DSN prompt is off, adjusting DriverCompletion */
  if (Dsn->ConnectPrompt)
    DriverCompletion= SQL_DRIVER_NOPROMPT;

  switch (DriverCompletion) {
  case SQL_DRIVER_COMPLETE_REQUIRED:
  case SQL_DRIVER_COMPLETE:
  case SQL_DRIVER_NOPROMPT:

    if (SQL_SUCCEEDED(MADB_DbcConnectDB(Dbc, Dsn)))
    {
      goto end;
    }
    else if (DriverCompletion == SQL_DRIVER_NOPROMPT)
    {
      /* For SQL_DRIVER_COMPLETE(_REQUIRED) this is not the end - will show prompt for user */
      goto error;
    }
    /* If we got here, it means that we had unsuccessful connect attempt with SQL_DRIVER_COMPLETE(_REQUIRED) completion
       Have to clean that error */
    MADB_CLEAR_ERROR(&Dbc->Error);
    break;

  case SQL_DRIVER_PROMPT:
    break;
  default:
    MADB_SetError(&Dbc->Error, MADB_ERR_HY110, NULL, 0);
    goto error;
    break;
  }

  /* Without window handle we can't show a dialog */
  if (DriverCompletion != SQL_DRIVER_NOPROMPT && !WindowHandle)
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_IM008, NULL, 0);
    goto error;
  }
  
  if (DriverCompletion == SQL_DRIVER_COMPLETE_REQUIRED)
    Dsn->isPrompt= MAODBC_PROMPT_REQUIRED;
  else
    Dsn->isPrompt= MAODBC_PROMPT;

  /* We need to obtain the driver name to load ssodbcs.dll, if it's not stored inside DSN,
     error IM007 (dialog prohibited) will be returned */
  if (!Dsn->Driver)
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_IM007, NULL, 0);
    goto error;
  }
  
  if (!(Drv= MADB_DriverGet(Dsn->Driver)))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_IM003, NULL, 0);
    goto error;
  }
  if (!Drv->SetupLibrary)
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY000, "Couldn't determine setup library", 0);
    goto error;
  }
 
  switch (DSNPrompt_Lookup(&DSNPrompt, Drv->SetupLibrary))
  {
  case 0: break;
  case MADB_PROMPT_NOT_SUPPORTED:
    MADB_SetError(&Dbc->Error, MADB_ERR_HY000, "Prompting is not supported on this platform", 0);
    goto error;
  case MADB_PROMPT_COULDNT_LOAD:
    MADB_SetError(&Dbc->Error, MADB_ERR_HY000, "Couldn't load the setup library", 0);
    goto error;
  }

  Dsn->allocator= MADB_DriverSideAllocate;
  Dsn->free=      MADB_DriverSideFree;

  if (DSNPrompt.Call((HWND)WindowHandle, Dsn) == FALSE)
  {
    /* If user cancels prompt, SQLDriverConnect should return SQL_NO_DATA */
    Dbc->Error.ReturnValue= SQL_NO_DATA;
    goto error;
  }

  DSNPrompt_Free(&DSNPrompt);

  ret= MADB_DbcConnectDB(Dbc, Dsn);
  if (!SQL_SUCCEEDED(ret))
  {
    goto error;
  }

end:
  Dbc->Dsn= Dsn;
  /* Dialog returns bitmap - syncing corresponding properties */
  MADB_DsnUpdateOptionsFields(Dsn);
  if (Dsn->isPrompt)
  {
    char *PreservePwd;

    /* DM should do that on its own, but we still better also remove pwd from the string being saved in the file DSN */
    if (Dsn->SaveFile != NULL)
    {
      PreservePwd= Dsn->Password;
      Dsn->Password= NULL;
    }
    /* If prompt/complete(_required), and dialog was succusefully showed - we generate string from the result DSN */
    Length= MADB_DsnToString(Dsn, (char *)OutConnectionString, BufferLength);

    if (Dsn->SaveFile != NULL)
    {
      Dsn->Password= PreservePwd;
    }
  }
  else
  {
    if (StringLength1 == SQL_NTS)
    {
      StringLength1= (SQLSMALLINT)strlen((const char*)InConnectionString);
    }
    if (OutConnectionString && BufferLength)
    {
      /* Otherwise we are supposed to simply copy incoming connection string */
      strncpy_s((char *)OutConnectionString, BufferLength, (const char*)InConnectionString, StringLength1);
    }
    Length= StringLength1;
  }
  if (StringLength2Ptr)
    *StringLength2Ptr= (SQLSMALLINT)Length;

  if (OutConnectionString && BufferLength && Length > BufferLength)
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_01004, NULL, 0);
    return Dbc->Error.ReturnValue;
  }
  return ret;
error:
  DSNPrompt_Free(&DSNPrompt);
  MADB_DSN_Free(Dsn);
  MADB_DriverFree(Drv);
  return Dbc->Error.ReturnValue;
}
/* }}} */

struct st_ma_connection_methods MADB_Dbc_Methods =
{ 
  MADB_DbcSetAttr,
  MADB_DbcGetAttr,
  MADB_DbcConnectDB,
  MADB_DbcEndTran,
  MADB_DbcGetFunctions,
  MADB_DbcGetInfo,
  MADB_DriverConnect
};
