/************************************************************************************
   Copyright (C) 2014,2016 MariaDB Corporation AB
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


/* MariaDB ODBC driver Win32 specific helper functions */

/* NOTE If you change something in this program, please consider if other platform's version 
        of the function you are changing, needs to be changed accordingly */

#include "ma_odbc.h"
#include <pathcch.h>

extern Client_Charset utf8;
char LogFile[256];

char *strndup(const char *s, size_t n)
{
  char *res= NULL;

  if (s != NULL)
  {
    size_t len= MIN(strlen(s), n);
    res= (char*)malloc(len + 1);
    if (res != NULL)
    {
      memcpy(res, s, len);
      res[len]= '\0';
    }
  }

  return res;
}


const char* GetDefaultLogDir()
{
  const char *DefaultLogDir= "c:";
  char *tmp= getenv("USERPROFILE");
  if (tmp)
  {
    DefaultLogDir= tmp;
  }

  tmp= getenv("TMP");
  if (tmp)
  {
    DefaultLogDir= tmp;
  }

#ifdef MAODBC_UNICODEDRIVER
  _snprintf(LogFile, sizeof(LogFile), "%s\\SSODBCW.LOG", DefaultLogDir);
#else
  _snprintf(LogFile, sizeof(LogFile), "%s\\SSODBCA.LOG", DefaultLogDir);
#endif

  return LogFile;
}


/* Connection is needed to set custom error */
int DSNPrompt_Lookup(MADB_Prompt *prompt, const char * SetupLibName)
{
  if (!(prompt->LibraryHandle=(void*) LoadLibrary(SetupLibName)))
  {
    return MADB_PROMPT_COULDNT_LOAD;
  }
  if (!(prompt->Call= (PromptDSN)GetProcAddress((HMODULE)prompt->LibraryHandle, "DSNPrompt")))
  {
    return MADB_PROMPT_COULDNT_LOAD;
  }

  return SQL_SUCCESS;
}


int DSNPrompt_Free(MADB_Prompt *prompt)
{
  if (prompt->LibraryHandle != NULL)
  {
    FreeLibrary((HMODULE)prompt->LibraryHandle);
  }
  prompt->LibraryHandle= NULL;
  prompt->Call= NULL;

  return 0;
}


SQLWCHAR *MADB_ConvertToWchar(const char *Ptr, SQLLEN PtrLength, Client_Charset* cc)
{
  SQLWCHAR *WStr= NULL;
  int Length;

  if (!Ptr)
    return WStr;

  if (PtrLength == SQL_NTS)
    PtrLength= -1;

  if (!cc || !cc->CodePage)
    cc= &utf8;

  /* TODO: Check this
     In fact MultiByteToWideChar does not copy terminating character by default
     Thus +1 does not make sense
     "If the function succeeds and cchWideChar is 0, the return value is the required size,
      in characters, for the buffer indicated by lpWideCharStr...
      MultiByteToWideChar does not null-terminate an output string if the input string length
      is explicitly specified without a terminating null character. To null-terminate an output
      string for this function, the application should pass in -1 or explicitly count the
      terminating null character for the input string." */
  if ((Length= MultiByteToWideChar(cc->CodePage, 0, Ptr, (int)PtrLength, NULL, 0)))
    if ((WStr= (SQLWCHAR *)MADB_CALLOC(sizeof(SQLWCHAR) * Length + 1)))
      MultiByteToWideChar(cc->CodePage, 0, Ptr, (int)PtrLength, WStr, Length);
  return WStr;
}

  /* {{{ MADB_ConvertFromLatin1Char
  Length gets number of written bytes including TN (if StrCharLen == -1 or SQL_NTS or if StrCharLen includes
  TN in the Str) */
char *MADB_ConvertFromLatin1Char(const char *Str, SQLINTEGER StrCharLen, SQLULEN *Length/*Bytes*/, Client_Charset *cc, BOOL *Error)
{
  char *AscStr;
  int AscLen = 0, i, StrActualCharLen;
  
  if (Error)
    *Error= 0;
  printf("AAAA1 %d\n", StrCharLen);
  if (StrCharLen == SQL_NTS) 
  {
    StrActualCharLen = strlen(Str) + 1;
  } else 
  {
    StrActualCharLen = StrCharLen;
  }
  printf("AAAA2 %d\n", StrActualCharLen);

  if (!(AscStr = (char *)MADB_CALLOC(StrActualCharLen * 2)))
    return NULL;
  
  for (i = 0; i < StrActualCharLen; i++) 
  {
    char c = *(Str + i);
    printf("AAAA3 %c\n", c);
    if (c & 0x80) 
    {
      *(AscStr + AscLen++) = c;
    } else 
    {
      *(AscStr + AscLen++) = 0xc0 | (c >> 6);
      *(AscStr + AscLen++) = 0x80 | (c & 0x3f);
    }
    printf("AAAA4 %d\n", AscLen);
  }

  if (Length)
    *Length= (SQLINTEGER)AscLen;
  printf("AAAA5 %d\n", *Length);
  return AscStr;
}
/* }}} */

  /* {{{ MADB_ConvertFromWChar
  Length gets number of written bytes including TN (if WstrCharLen == -1 or SQL_NTS or if WstrCharLen includes
  TN in the Wstr) */
char *MADB_ConvertFromWChar(const SQLWCHAR *Wstr, SQLINTEGER WstrCharLen, SQLULEN *Length/*Bytes*/, Client_Charset *cc, BOOL *Error)
{
  char *AscStr;
  int AscLen, AllocLen;
  
  if (Error)
    *Error= 0;

  if (cc == NULL || cc->CodePage < 1)
  {
    cc= &utf8;
  }

  if (WstrCharLen == SQL_NTS)
    WstrCharLen= -1;
  else if (WstrCharLen > 0) /* normalize the input string */
    WstrCharLen = SqlwcsLen(Wstr, WstrCharLen);

  AllocLen= AscLen= WideCharToMultiByte(cc->CodePage, 0, Wstr, WstrCharLen, NULL, 0, NULL, NULL);
  if (WstrCharLen != -1)
    ++AllocLen;
  
  if (!(AscStr = (char *)MADB_CALLOC(AllocLen)))
    return NULL;

  AscLen= WideCharToMultiByte(cc->CodePage,  0, Wstr, WstrCharLen, AscStr, AscLen, NULL, (cc->CodePage != CP_UTF8) ? Error : NULL);
  if (AscLen && WstrCharLen == -1)
    --AscLen;

  if (Length)
    *Length= (SQLINTEGER)AscLen;
  return AscStr;
}
/* }}} */

/* Required Length without or with TN(if IsNull is TRUE, or AnsiLength == -1 or SQL_NTS) is put to LenghtIndicator*/
int MADB_ConvertAnsi2Unicode(Client_Charset *cc, const char *AnsiString, SQLLEN AnsiLength,
                             SQLWCHAR *UnicodeString, SQLLEN UnicodeLength, 
                             SQLLEN *LengthIndicator, BOOL IsNull, MADB_Error *Error)
{
  SQLLEN    RequiredLength;
  SQLWCHAR *Tmp= UnicodeString;

  if (LengthIndicator)
    *LengthIndicator= 0;

  if (Error)
    MADB_CLEAR_ERROR(Error);

  if (!AnsiLength || UnicodeLength < 0)
  {
    if (Error)
      MADB_SetError(Error, MADB_ERR_HY090, NULL, 0);
    return 1;
  }

  if (AnsiLength == SQL_NTS || AnsiLength == -1)
    IsNull= 1;

  if (!UnicodeLength)
  {
    // Return buffer is empty
    // Get WideChar length if needed and return
    if (LengthIndicator) {
      RequiredLength= MultiByteToWideChar(cc->CodePage, 0, AnsiString, IsNull ? -1 : (int)AnsiLength, NULL, 0);
      if (RequiredLength < 1)
      {
        if (Error)
          MADB_SetError(Error, MADB_ERR_HY000, "Ansi to Unicode conversion error occurred", GetLastError());
        return 1;
      }

      // Set LengthIndicator
      *LengthIndicator= RequiredLength - IsNull;
    }
    return 0;
  }

  RequiredLength= MultiByteToWideChar(cc->CodePage, 0, AnsiString, IsNull ? -1 : (int)AnsiLength, Tmp, (int)UnicodeLength);
  if (RequiredLength < 1 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
    // Buffer is too small

    RequiredLength= MultiByteToWideChar(cc->CodePage, 0, AnsiString, IsNull ? -1 : (int)AnsiLength, NULL, 0);

    // Set LengthIndicator
    if (LengthIndicator)
      *LengthIndicator= RequiredLength - IsNull;

    Tmp= (SQLWCHAR *)malloc(RequiredLength * sizeof(SQLWCHAR));

    RequiredLength= MultiByteToWideChar(cc->CodePage, 0, AnsiString, IsNull ? -1 : (int)AnsiLength, Tmp, (int)RequiredLength);
    if (RequiredLength < 1)
    {
      if (Error)
        MADB_SetError(Error, MADB_ERR_HY000, "Ansi to Unicode conversion error occurred", GetLastError());
      free(Tmp);
      return 1;
    }

    wcsncpy(UnicodeString, L"", 1);
    wcsncat(UnicodeString, Tmp, UnicodeLength- 1);
    free(Tmp);
    if (Error)
      MADB_SetError(Error, MADB_ERR_01004, NULL, 0);
  } else if (RequiredLength < 1)
  {
    // Conversion error occurred
    if (Error)
      MADB_SetError(Error, MADB_ERR_HY000, "Ansi to Unicode conversion error occurred", GetLastError());
    return 1;
  } else {
    // Successfully converted string
    // Set LengthIndicator
    if (LengthIndicator)
      *LengthIndicator= RequiredLength - IsNull;
  }
}

/* Returns required length for result string with(if dest and dest length are provided)
   or without terminating NULL(otherwise). If cc is NULL, or not initialized(CodePage is 0),
   then simply SrcLength is returned. 
   If Dest is not NULL, and DestLenth is 0, then error */
SQLLEN MADB_SetString(Client_Charset* cc, void *Dest, SQLULEN DestLength,
                      const char *Src, SQLLEN SrcLength/*bytes*/, MADB_Error *Error)
{
  char  *p= (char *)Dest;
  SQLLEN Length= 0;

  if (SrcLength == SQL_NTS)
  {
    if (Src != NULL)
    {
      SrcLength= strlen(Src);
    }
    else
    {
      SrcLength= 0;
    }
  }

  /* Not enough space */
  if (!DestLength || !Dest)
  {
    if (Dest)
      MADB_SetError(Error, MADB_ERR_01004, NULL, 0);
    if (!cc || !cc->CodePage)
      return SrcLength;
    else
    {
      Length= MultiByteToWideChar(cc->CodePage, 0, Src, (int)SrcLength, NULL, 0);
      return Length;
    }
  }

  if (!Src || !strlen(Src) || !SrcLength)
  {
    memset(p, 0, cc && cc->CodePage ? sizeof(SQLWCHAR) : sizeof(SQLCHAR));
    return 0;
  }

  if (!cc || !cc->CodePage)
  {
    strncpy_s((char *)Dest, DestLength, Src ? Src : "", _TRUNCATE);
    if (Error && (SQLULEN)SrcLength >= DestLength)
      MADB_SetError(Error, MADB_ERR_01004, NULL, 0);
    return SrcLength;
  }
  else
  {
    MADB_ConvertAnsi2Unicode(cc, Src, -1, (SQLWCHAR *)Dest, DestLength, &Length, 1, Error);
    return Length;
  }
}


int GetSourceAnsiCs(Client_Charset *cc)
{
  cc->CodePage= GetACP();

  /* We don't need cs_info for this */
  return cc->CodePage;
}

BOOL MADB_DirectoryExists(const char *Path)
{
  DWORD FileAttributes = GetFileAttributes(Path);

  return (FileAttributes != INVALID_FILE_ATTRIBUTES) && (FileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

char* MADB_GetDefaultPluginsDir(char* Buffer, size_t Size)
{
  HMODULE hModule = GetModuleHandle(MADB_DRIVER_NAME);
  wchar_t wOurLocation[_MAX_PATH];
  const char *PluginsSubDirName= "\\"MADB_DEFAULT_PLUGINS_SUBDIR;
  HRESULT hr;

  memset(Buffer, 0, Size);
  GetModuleFileNameW(hModule, wOurLocation, _MAX_PATH);
  hr= PathCchRemoveFileSpec(wOurLocation, _MAX_PATH);

  WideCharToMultiByte(GetACP(), 0, wOurLocation, -1, Buffer, (int)Size, NULL, NULL);
  if (strlen(Buffer) < Size - strlen(PluginsSubDirName))
  {
    strcpy(Buffer + strlen(Buffer), PluginsSubDirName);

    if (MADB_DirectoryExists(Buffer) != FALSE)
    {
      return Buffer;
    }
  }
  return NULL;
}

/* {{{ MADB_DSN_PossibleConnect() */
BOOL MADB_DSN_PossibleConnect(MADB_Dsn *Dsn)
{
  return Dsn->ServerName && (Dsn->IsNamedPipe || Dsn->Port > 0);
}
/* }}} */

