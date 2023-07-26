/************************************************************************************
   Copyright (C) 2016 MariaDB Corporation AB
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

/* Common functions used in both connector and setup library.
 * Moved to avoid redundant dependencies */

#include <ma_odbc.h>
#include <wctype.h>

static unsigned int ValidChar32(const char *start, const char *end)
{
  return 4;
}


static unsigned int CharOctetLen32(unsigned int utf32)
{
  return 4;
}

#define UTF16_HIGH_HEAD(x)  ((((unsigned char) (x)) & 0xFC) == 0xD8)
#define UTF16_LOW_HEAD(x)   ((((unsigned char) (x)) & 0xFC) == 0xDC)

static unsigned int CharOctetLen16(unsigned int utf16)
{
  return UTF16_HIGH_HEAD(utf16) ? 4 : 2;
}

static unsigned int ValidChar16(const char *start, const char *end)
{
  if (start + 2 > end) {
    return 0;
  }

  if (UTF16_HIGH_HEAD(*start)) {
    return (start + 4 <= end) && UTF16_LOW_HEAD(start[2]) ? 4 : 0;
  }

  if (UTF16_LOW_HEAD(*start)) {
    return 0;
  }
  return 2;
}

MARIADB_CHARSET_INFO dummyUtf16le= {56,1, "utf16le", "utf16_general_ci", "", 1200, "UTF16LE", 2, 4, CharOctetLen16, ValidChar16};
MARIADB_CHARSET_INFO dummyUtf16be= {54,1, "utf16", "utf16_general_ci", "", 0, "UTF16", 2, 4, CharOctetLen16, ValidChar16};
MARIADB_CHARSET_INFO dummyUtf32le= {0, 1, "utf32le", "utf32_general_ci", "", 0, "UTF-32LE", 4, 4, CharOctetLen32, ValidChar32};
MARIADB_CHARSET_INFO dummyUtf32be= {60,1, "utf32", "utf32_general_ci", "", 0, "UTF32", 4, 4, CharOctetLen32, ValidChar32};
MARIADB_CHARSET_INFO dummyLatin1=  {48,1, "latin1", "latin1_general_ci", "", 1250, "LATIN1", 1, 1, NULL, NULL};

MARIADB_CHARSET_INFO*  DmUnicodeCs= NULL;
Client_Charset utf8=  {CP_UTF8, NULL};

/* {{{ ltrim */
char* ltrim(char *Str)
{
  /* I am not sure using iswspace, and not isspace makes any sense here. But probably does not hurt either */
  while (Str && iswspace(Str[0]))
    ++Str;
  return Str;
}
/* }}} */

/* {{{ trim */
char* trim(char *Str)
{
  char *end;
  
  Str= ltrim(Str);

  end= Str + strlen(Str) - 1;
  while (end >= Str && iswspace(*end))
    *end--= 0;
  return Str;
}
/* }}} */

/* Windows only common functions */
#ifdef _WIN32

/* {{{ strcasestr() */
char* strcasestr(const char* HayStack, const char* Needle)
{
  return StrStrIA(HayStack, Needle);
}
/* }}} */

#endif
