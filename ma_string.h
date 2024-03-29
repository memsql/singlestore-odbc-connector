/************************************************************************************
   Copyright (C) 2013,2016 MariaDB Corporation AB
   
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
#ifndef _ma_string_h_
#define _ma_string_h_

#define IN
#define OUT
#define INOUT

char *MADB_ConvertFromWChar(const SQLWCHAR *Ptr, SQLINTEGER PtrLength, SQLULEN *Length, Client_Charset* cc, BOOL *DefaultCharUsed);
int MADB_ConvertAnsi2Unicode(Client_Charset* cc, const char *AnsiString, SQLLEN AnsiLength, 
                             SQLWCHAR *UnicodeString, SQLLEN UnicodeLength, 
                             SQLLEN *LengthIndicator, BOOL IsNull, MADB_Error *Error);
char*     MADB_GetInsertStatement(MADB_Stmt *Stmt);
char*     MADB_GetTableName(MADB_Stmt *Stmt);
char*     MADB_GetCatalogName(MADB_Stmt *Stmt);
my_bool   MADB_DynStrUpdateSet(MADB_Stmt *Stmt, MADB_DynString *DynString);
my_bool   MADB_DynStrInsertSet(MADB_Stmt *Stmt, MADB_DynString *DynString);
my_bool   MADB_DynStrGetWhere(MADB_Stmt *Stmt, MADB_DynString *DynString, char *TableName, my_bool ParameterMarkers);
my_bool   MADB_DynStrAppendQuoted(MADB_DynString *DynString, char *String);
my_bool   MADB_DynStrGetColumns(MADB_Stmt *Stmt, MADB_DynString *DynString);
my_bool   MADB_DynStrGetValues(MADB_Stmt *Stmt, MADB_DynString *DynString);
SQLWCHAR* MADB_ConvertToWchar(const char *Ptr, SQLLEN PtrLength, Client_Charset* cc);
SQLLEN    MADB_SetString(Client_Charset* cc, void *Dest, SQLULEN DestLength,
                        const char *Src, SQLLEN SrcLength, MADB_Error *Error);
my_bool   MADB_ValidateStmt(MADB_QUERY *Query);

SQLLEN     MbstrOctetLen(const char *str, SQLLEN *CharLen, MARIADB_CHARSET_INFO *cs);
SQLLEN     MbstrCharLen(const char *str, SQLINTEGER OctetLen, MARIADB_CHARSET_INFO *cs);
SQLINTEGER SqlwcsCharLen(SQLWCHAR *str, SQLLEN octets);
SQLLEN     SqlwcsLen(SQLWCHAR *str, SQLLEN buff_length);
SQLLEN     SafeStrlen(SQLCHAR *str, SQLLEN buff_length);

#define IDENTIFIER_BUFFER_OVERHEAD  3
my_bool ProcessIdentifierString(INOUT char* out, const char* in, const size_t len);

/* Calculate string length (if needed) and check for some input error
 *
 * ptr should be a string without zero chars inside
 * len is string length or SQL_NTS for 0-terminated strings
 *
 */
#define ADJUST_LENGTH(ptr, len) ((len) = (ptr) ? SafeStrlen((ptr), (len)) : 0)

/* Fix SQLWCHAR buffer length to be multiple of sizeof(SQLWCHAR) */
#define ALIGN_WCHAR_LENGTH(len) ((len) &= ~(sizeof(SQLWCHAR) == 2 ? 0x01ul : 0x03ul))

#endif
