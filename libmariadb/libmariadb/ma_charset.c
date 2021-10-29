/****************************************************************************
   Copyright (C) 2012, 2020, MariaDB Corporation.
                 2021 SingleStore, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not see <http://www.gnu.org/licenses>
   or write to the Free Software Foundation, Inc., 
   51 Franklin St., Fifth Floor, Boston, MA 02110, USA

   Part of this code includes code from the PHP project which
   is freely available from http://www.php.net
*****************************************************************************/

/* The implementation for character set support was ported from PHP's mysqlnd
   extension, written by Andrey Hristov, Georg Richter and Ulf Wendel 

   Original file header:
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Georg Richter <georg@mysql.com>                             |
  |          Andrey Hristov <andrey@mysql.com>                           |
  |          Ulf Wendel <uwendel@mysql.com>                              |
  +----------------------------------------------------------------------+
*/

#ifndef _WIN32
#include <strings.h>
#include <string.h>
#else
#include <string.h>
#endif
#include <ma_global.h>
#include <mariadb_ctype.h>
#include <ma_string.h>

#ifdef HAVE_ICONV
#ifdef _WIN32
#include "../win-iconv/iconv.h"
#else
#include <iconv.h>
#endif
#endif


#if defined(HAVE_NL_LANGINFO) && defined(HAVE_SETLOCALE)
#include <locale.h>
#include <langinfo.h>
#endif

/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2006-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Georg Richter <georg@mysql.com>                             |
  |          Andrey Hristov <andrey@mysql.com>                           |
  |          Ulf Wendel <uwendel@mysql.com>                              |
  +----------------------------------------------------------------------+
*/

/* {{{ utf8 functions */
static unsigned int check_mb_utf8mb3_sequence(const char *start, const char *end)
{
  uchar  c;

  if (start >= end) {
    return 0;
  }

  c = (uchar) start[0];

  if (c < 0x80) {
    return 1;    /* single byte character */
  }
  if (c < 0xC2) {
    return 0;    /* invalid mb character */
  }
  if (c < 0xE0) {
    if (start + 2 > end) {
      return 0;  /* too small */
    }
    if (!(((uchar)start[1] ^ 0x80) < 0x40)) {
      return 0;
    }
    return 2;
  }
  if (c < 0xF0) {
    if (start + 3 > end) {
      return 0;  /* too small */
    }
    if (!(((uchar)start[1] ^ 0x80) < 0x40 && ((uchar)start[2] ^ 0x80) < 0x40 &&
      (c >= 0xE1 || (uchar)start[1] >= 0xA0))) {
      return 0;  /* invalid utf8 character */
    }
    return 3;
  }
  return 0;
}


static unsigned int check_mb_utf8_sequence(const char *start, const char *end)
{
  uchar  c;

  if (start >= end) {
    return 0;
  }

  c = (uchar) start[0];

  if (c < 0x80) {
    return 1;    /* single byte character */
  }
  if (c < 0xC2) {
    return 0;    /* invalid mb character */
  }
  if (c < 0xE0) {
    if (start + 2 > end) {
      return 0;  /* too small */
    }
    if (!(((uchar)start[1] ^ 0x80) < 0x40)) {
      return 0;
    }
    return 2;
  }
  if (c < 0xF0) {
    if (start + 3 > end) {
      return 0;  /* too small */
    }
    if (!(((uchar)start[1] ^ 0x80) < 0x40 && ((uchar)start[2] ^ 0x80) < 0x40 &&
      (c >= 0xE1 || (uchar)start[1] >= 0xA0))) {
      return 0;  /* invalid utf8 character */
    }
    return 3;
  }
  if (c < 0xF5) {
    if (start + 4 > end) { /* We need 4 characters */
      return 0;  /* too small */
    }

    /*
      UTF-8 quick four-byte mask:
      11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      Encoding allows to encode U+00010000..U+001FFFFF

      The maximum character defined in the Unicode standard is U+0010FFFF.
      Higher characters U+00110000..U+001FFFFF are not used.

      11110000.10010000.10xxxxxx.10xxxxxx == F0.90.80.80 == U+00010000 (min)
      11110100.10001111.10111111.10111111 == F4.8F.BF.BF == U+0010FFFF (max)

      Valid codes:
      [F0][90..BF][80..BF][80..BF]
      [F1][80..BF][80..BF][80..BF]
      [F2][80..BF][80..BF][80..BF]
      [F3][80..BF][80..BF][80..BF]
      [F4][80..8F][80..BF][80..BF]
    */

    if (!(((uchar)start[1] ^ 0x80) < 0x40 &&
      ((uchar)start[2] ^ 0x80) < 0x40 &&
      ((uchar)start[3] ^ 0x80) < 0x40 &&
        (c >= 0xf1 || (uchar)start[1] >= 0x90) &&
        (c <= 0xf3 || (uchar)start[1] <= 0x8F)))
    {
      return 0;  /* invalid utf8 character */
    }
    return 4;
  }
  return 0;
}

static unsigned int check_mb_utf8mb3_valid(const char *start, const char *end)
{
  unsigned int len = check_mb_utf8mb3_sequence(start, end);
  return (len > 1)? len:0;
}

static unsigned int check_mb_utf8_valid(const char *start, const char *end)
{
  unsigned int len = check_mb_utf8_sequence(start, end);
  return (len > 1)? len:0;
}


static unsigned int mysql_mbcharlen_utf8mb3(unsigned int utf8)
{
  if (utf8 < 0x80) {
    return 1;    /* single byte character */
  }
  if (utf8 < 0xC2) {
    return 0;    /* invalid multibyte header */
  }
  if (utf8 < 0xE0) {
    return 2;    /* double byte character */
  }
  if (utf8 < 0xF0) {
    return 3;    /* triple byte character */
  }
  return 0;
}


static unsigned int mysql_mbcharlen_utf8(unsigned int utf8)
{
  if (utf8 < 0x80) {
    return 1;    /* single byte character */
  }
  if (utf8 < 0xC2) {
    return 0;    /* invalid multibyte header */
  }
  if (utf8 < 0xE0) {
    return 2;    /* double byte character */
  }
  if (utf8 < 0xF0) {
    return 3;    /* triple byte character */
  }
  if (utf8 < 0xF8) {
    return 4;    /* four byte character */
  }
  return 0;
}
/* }}} */


/* {{{ big5 functions */
#define valid_big5head(c)  (0xA1 <= (unsigned int)(c) && (unsigned int)(c) <= 0xF9)
#define valid_big5tail(c)  ((0x40 <= (unsigned int)(c) && (unsigned int)(c) <= 0x7E) || \
              (0xA1 <= (unsigned int)(c) && (unsigned int)(c) <= 0xFE))

#define isbig5code(c,d) (isbig5head(c) && isbig5tail(d))

static unsigned int check_mb_big5(const char *start, const char *end)
{
  return (valid_big5head(*((const uchar*) start)) && (end - start) > 1 && valid_big5tail(*((const uchar*) start + 1)) ? 2 : 0);
}


static unsigned int mysql_mbcharlen_big5(unsigned int big5)
{
  return (valid_big5head(big5)) ? 2 : 1;
}
/* }}} */


/* {{{ cp932 functions */
#define valid_cp932head(c) ((0x81 <= (c) && (c) <= 0x9F) || (0xE0 <= (c) && c <= 0xFC))
#define valid_cp932tail(c) ((0x40 <= (c) && (c) <= 0x7E) || (0x80 <= (c) && c <= 0xFC))


static unsigned int check_mb_cp932(const char *start, const char *end)
{
  return (valid_cp932head((uchar)start[0]) && (end - start >  1) &&
      valid_cp932tail((uchar)start[1])) ? 2 : 0;
}


static unsigned int mysql_mbcharlen_cp932(unsigned int cp932)
{
  return (valid_cp932head((uchar)cp932)) ? 2 : 1;
}
/* }}} */


/* {{{ euckr functions */
#define valid_euckr(c)  ((0xA1 <= (uchar)(c) && (uchar)(c) <= 0xFE))

static unsigned int check_mb_euckr(const char *start, const char *end)
{
  if (end - start <= 1) {
    return 0;  /* invalid length */
  }
  if (*(uchar *)start < 0x80) {
    return 0;  /* invalid euckr character */
  }
  if (valid_euckr(start[1])) {
    return 2;
  }
  return 0;
}


static unsigned int mysql_mbcharlen_euckr(unsigned int kr)
{
  return (valid_euckr(kr)) ? 2 : 1;
}
/* }}} */


/* {{{ eucjpms functions */
#define valid_eucjpms(c)     (((c) & 0xFF) >= 0xA1 && ((c) & 0xFF) <= 0xFE)
#define valid_eucjpms_kata(c)  (((c) & 0xFF) >= 0xA1 && ((c) & 0xFF) <= 0xDF)
#define valid_eucjpms_ss2(c)  (((c) & 0xFF) == 0x8E)
#define valid_eucjpms_ss3(c)  (((c) & 0xFF) == 0x8F)

static unsigned int check_mb_eucjpms(const char *start, const char *end)
{
  if (*((uchar *)start) < 0x80) {
    return 0;  /* invalid eucjpms character */
  }
  if (valid_eucjpms(start[0]) && (end - start) > 1 && valid_eucjpms(start[1])) {
    return 2;
  }
  if (valid_eucjpms_ss2(start[0]) && (end - start) > 1 && valid_eucjpms_kata(start[1])) {
    return 2;
  }
  if (valid_eucjpms_ss3(start[0]) && (end - start) > 2 && valid_eucjpms(start[1]) &&
    valid_eucjpms(start[2])) {
    return 2;
  }
  return 0;
}


static unsigned int mysql_mbcharlen_eucjpms(unsigned int jpms)
{
  if (valid_eucjpms(jpms) || valid_eucjpms_ss2(jpms)) {
    return 2;
  }
  if (valid_eucjpms_ss3(jpms)) {
    return 3;
  }
  return 1;
}
/* }}} */


/* {{{ gb2312 functions */
#define valid_gb2312_head(c)  (0xA1 <= (uchar)(c) && (uchar)(c) <= 0xF7)
#define valid_gb2312_tail(c)  (0xA1 <= (uchar)(c) && (uchar)(c) <= 0xFE)


static unsigned int check_mb_gb2312(const char *start, const char *end)
{
  return (valid_gb2312_head((unsigned int)start[0]) && end - start > 1 &&
      valid_gb2312_tail((unsigned int)start[1])) ? 2 : 0;
}


static unsigned int mysql_mbcharlen_gb2312(unsigned int gb)
{
  return (valid_gb2312_head(gb)) ? 2 : 1;
}
/* }}} */


/* {{{ gbk functions */
#define valid_gbk_head(c)  (0x81<=(uchar)(c) && (uchar)(c)<=0xFE)
#define valid_gbk_tail(c)  ((0x40<=(uchar)(c) && (uchar)(c)<=0x7E) || (0x80<=(uchar)(c) && (uchar)(c)<=0xFE))

static unsigned int check_mb_gbk(const char *start, const char *end)
{
  return (valid_gbk_head(start[0]) && (end) - (start) > 1 && valid_gbk_tail(start[1])) ? 2 : 0;
}

static unsigned int mysql_mbcharlen_gbk(unsigned int gbk)
{
  return (valid_gbk_head(gbk) ? 2 : 1);
}
/* }}} */


/* {{{ sjis functions */
#define valid_sjis_head(c)  ((0x81 <= (c) && (c) <= 0x9F) || (0xE0 <= (c) && (c) <= 0xFC))
#define valid_sjis_tail(c)  ((0x40 <= (c) && (c) <= 0x7E) || (0x80 <= (c) && (c) <= 0xFC))


static unsigned int check_mb_sjis(const char *start, const char *end)
{
  return (valid_sjis_head((uchar)start[0]) && (end - start) > 1 && valid_sjis_tail((uchar)start[1])) ? 2 : 0;
}


static unsigned int mysql_mbcharlen_sjis(unsigned int sjis)
{
  return (valid_sjis_head((uchar)sjis)) ? 2 : 1;
}
/* }}} */


/* {{{ ucs2 functions */
static unsigned int check_mb_ucs2(const char *start __attribute((unused)), const char *end __attribute((unused)))
{
  return 2; /* always 2 */
}

static unsigned int mysql_mbcharlen_ucs2(unsigned int ucs2 __attribute((unused)))
{
  return 2; /* always 2 */
}
/* }}} */


/* {{{ ujis functions */
#define valid_ujis(c)       ((0xA1 <= ((c)&0xFF) && ((c)&0xFF) <= 0xFE))
#define valid_ujis_kata(c)  ((0xA1 <= ((c)&0xFF) && ((c)&0xFF) <= 0xDF))
#define valid_ujis_ss2(c)   (((c)&0xFF) == 0x8E)
#define valid_ujis_ss3(c)   (((c)&0xFF) == 0x8F)

static unsigned int check_mb_ujis(const char *start, const char *end)
{
  if (*(uchar*)start < 0x80) {
    return 0;  /* invalid ujis character */
  }
  if (valid_ujis(*(start)) && valid_ujis(*((start)+1))) {
    return 2;
  }
  if (valid_ujis_ss2(*(start)) && valid_ujis_kata(*((start)+1))) {
    return 2;
  }
  if (valid_ujis_ss3(*(start)) && (end-start) > 2 && valid_ujis(*((start)+1)) && valid_ujis(*((start)+2))) {
    return 3;
  }
  return 0;
}


static unsigned int mysql_mbcharlen_ujis(unsigned int ujis)
{
  return (valid_ujis(ujis)? 2: valid_ujis_ss2(ujis)? 2: valid_ujis_ss3(ujis)? 3: 1);
}
/* }}} */



/* {{{ utf16 functions */
#define UTF16_HIGH_HEAD(x)  ((((uchar) (x)) & 0xFC) == 0xD8)
#define UTF16_LOW_HEAD(x)   ((((uchar) (x)) & 0xFC) == 0xDC)

static unsigned int check_mb_utf16(const char *start, const char *end)
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


static uint mysql_mbcharlen_utf16(unsigned int utf16)
{
  return UTF16_HIGH_HEAD(utf16) ? 4 : 2;
}
/* }}} */


/* {{{ utf32 functions */
static uint
check_mb_utf32(const char *start __attribute((unused)), const char *end __attribute((unused)))
{
  return 4;
}


static uint
mysql_mbcharlen_utf32(unsigned int utf32 __attribute((unused)))
{
  return 4;
}
/* }}} */

/* {{{ gb18030 functions */
#define is_gb18030_odd(c)          (0x81 <= (unsigned char) (c) && (unsigned char) (c) <= 0xFE)
#define is_gb18030_even_2(c)       ((0x40 <= (unsigned char) (c) && (unsigned char) (c) <= 0x7E) || (0x80 <= (unsigned char) (c) && (unsigned char) (c) <= 0xFE))
#define is_gb18030_even_4(c)       (0x30 <= (unsigned char) (c) && (unsigned char) (c) <= 0x39)


static unsigned int mysql_mbcharlen_gb18030(unsigned int c)
{
	if (c <= 0xFF) {
		return !is_gb18030_odd(c);
	}
	if (c > 0xFFFF || !is_gb18030_odd((c >> 8) & 0xFF)) {
		return 0;
	}
	if (is_gb18030_even_2((c & 0xFF))) {
	    return 2;
	}
	if (is_gb18030_even_4((c & 0xFF))) {
		return 4;
	}

	return 0;
}

static unsigned int check_mb_gb18030_valid(const char * start, const char * end)
{
	if (end - start <= 1 || !is_gb18030_odd(start[0])) {
		return 0;
	}

	if (is_gb18030_even_2(start[1])) {
		return 2;
	} else if (end - start > 3 && is_gb18030_even_4(start[1]) && is_gb18030_odd(start[2]) && is_gb18030_even_4(start[3])) {
		return 4;
	}

	return 0;
}
/* }}} */

/*
  The server compiles sometimes the full utf-8 (the mb4) as utf8m4, and the old as utf8,
  for BC reasons. Sometimes, utf8mb4 is just utf8 but the old charsets are utf8mb3.
  Change easily now, with a macro, could be made compilastion dependable.
*/

#define UTF8_MB4 "utf8mb4"
#define UTF8_MB3 "utf8"

/* {{{ mysql_charsets */
const MARIADB_CHARSET_INFO mariadb_compiled_charsets[] =
{
  {  33, 1, UTF8_MB3, UTF8_MB3"_general_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3,  check_mb_utf8mb3_valid},
  {  45, 1, UTF8_MB4, UTF8_MB4"_general_ci", "", 65001, "UTF-8", 1, 4, mysql_mbcharlen_utf8,  check_mb_utf8_valid},
  {  46, 1, UTF8_MB4, UTF8_MB4"_bin", "", 65001, "UTF-8", 1, 4, mysql_mbcharlen_utf8,  check_mb_utf8_valid},
  {  63, 1, "binary", "binary", "", 0, "ASCII", 1, 1, NULL, NULL},
  {  83, 1, UTF8_MB3, UTF8_MB3"_bin", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3,  check_mb_utf8mb3_valid},
  { 192, 1, UTF8_MB3, UTF8_MB3"_unicode_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 193, 1, UTF8_MB3, UTF8_MB3"_icelandic_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 194, 1, UTF8_MB3, UTF8_MB3"_latvian_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3,  check_mb_utf8mb3_valid},
  { 195, 1, UTF8_MB3, UTF8_MB3"_romanian_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 196, 1, UTF8_MB3, UTF8_MB3"_slovenian_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 197, 1, UTF8_MB3, UTF8_MB3"_polish_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 198, 1, UTF8_MB3, UTF8_MB3"_estonian_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 199, 1, UTF8_MB3, UTF8_MB3"_spanish_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 200, 1, UTF8_MB3, UTF8_MB3"_swedish_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 201, 1, UTF8_MB3, UTF8_MB3"_turkish_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 202, 1, UTF8_MB3, UTF8_MB3"_czech_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 203, 1, UTF8_MB3, UTF8_MB3"_danish_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid },
  { 204, 1, UTF8_MB3, UTF8_MB3"_lithuanian_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid },
  { 205, 1, UTF8_MB3, UTF8_MB3"_slovak_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 206, 1, UTF8_MB3, UTF8_MB3"_spanish2_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 207, 1, UTF8_MB3, UTF8_MB3"_roman_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 208, 1, UTF8_MB3, UTF8_MB3"_persian_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 209, 1, UTF8_MB3, UTF8_MB3"_esperanto_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 210, 1, UTF8_MB3, UTF8_MB3"_hungarian_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  { 211, 1, UTF8_MB3, UTF8_MB3"_sinhala_ci", "", 65001, "UTF-8", 1, 3, mysql_mbcharlen_utf8mb3, check_mb_utf8mb3_valid},
  {   0, 0, NULL, NULL, NULL, 0, NULL, 0, 0, NULL, NULL}
};
/* }}} */


/* {{{ mysql_find_charset_nr */
const MARIADB_CHARSET_INFO * mysql_find_charset_nr(unsigned int charsetnr)
{
  const MARIADB_CHARSET_INFO * c = mariadb_compiled_charsets;

  do {
    if (c->nr == charsetnr) {
      return(c);
    }
    ++c;
  } while (c[0].nr != 0);
  return(NULL);
}
/* }}} */


/* {{{ mysql_find_charset_name */
MARIADB_CHARSET_INFO * mysql_find_charset_name(const char *name)
{
  MARIADB_CHARSET_INFO *c = (MARIADB_CHARSET_INFO *)mariadb_compiled_charsets;
  const char *csname;

  if (!strcasecmp(name, MADB_AUTODETECT_CHARSET_NAME))
    csname= madb_get_os_character_set();
  else
    csname= (char *)name;

  do {
    if (!strcasecmp(c->csname, csname)) {
      return(c);
    }
    ++c;
  } while (c[0].nr != 0);
  return(NULL);
}
/* }}} */

/* {{{ mysql_find_charset_by_collation */
const MARIADB_CHARSET_INFO *mysql_find_charset_by_collation(const char *name)
{
  MARIADB_CHARSET_INFO *c = (MARIADB_CHARSET_INFO *)mariadb_compiled_charsets;

  do
  {
    if (!strcasecmp(c->name, name))
      return c;
    ++c;
  } while (c[0].nr != 0);
  return NULL;
}
/* }}} */

/* {{{ mysql_cset_escape_quotes */
size_t mysql_cset_escape_quotes(const MARIADB_CHARSET_INFO *cset, char *newstr,
                    const char * escapestr, size_t escapestr_len )
{
  const char   *newstr_s = newstr;
  const char   *newstr_e = newstr + 2 * escapestr_len;
  const char   *end = escapestr + escapestr_len;
  my_bool  escape_overflow = FALSE;

  for (;escapestr < end; escapestr++) {
    unsigned int len = 0;
    /* check unicode characters */

    if (cset->char_maxlen > 1 && (len = cset->mb_valid(escapestr, end))) {

      /* check possible overflow */
      if ((newstr + len) > newstr_e) {
        escape_overflow = TRUE;
        break;
      }
      /* copy mb char without escaping it */
      while (len--) {
        *newstr++ = *escapestr++;
      }
      escapestr--;
      continue;
    }
    if (*escapestr == '\'') {
      if (newstr + 2 > newstr_e) {
        escape_overflow = TRUE;
        break;
      }
      *newstr++ = '\'';
      *newstr++ = '\'';
    } else {
      if (newstr + 1 > newstr_e) {
        escape_overflow = TRUE;
        break;
      }
      *newstr++ = *escapestr;
    }
  }
  *newstr = '\0';

  if (escape_overflow) {
    return((size_t)~0);
  }
  return((size_t)(newstr - newstr_s));
}
/* }}} */


/* {{{ mysql_cset_escape_slashes */
size_t mysql_cset_escape_slashes(const MARIADB_CHARSET_INFO * cset, char *newstr,
                     const char * escapestr, size_t escapestr_len )
{
  const char   *newstr_s = newstr;
  const char   *newstr_e = newstr + 2 * escapestr_len;
  const char   *end = escapestr + escapestr_len;
  my_bool  escape_overflow = FALSE;

  for (;escapestr < end; escapestr++) {
    char esc = '\0';
    unsigned int len = 0;

    /* check unicode characters */
    if (cset->char_maxlen > 1 && (len = cset->mb_valid(escapestr, end))) {
      /* check possible overflow */
      if ((newstr + len) > newstr_e) {
        escape_overflow = TRUE;
        break;
      }
      /* copy mb char without escaping it */
      while (len--) {
        *newstr++ = *escapestr++;
      }
      escapestr--;
      continue;
    }
    if (cset->char_maxlen > 1 && cset->mb_charlen(*escapestr) > 1) {
      esc = *escapestr;
    } else {
      switch (*escapestr) {
        case 0:
          esc = '0';
          break;
        case '\n':
          esc = 'n';
          break;
        case '\r':
          esc = 'r';
          break;
        case '\\':
        case '\'':
        case '"':
          esc = *escapestr;
          break;
        case '\032':
          esc = 'Z';
          break;
      }
    }
    if (esc) {
      if (newstr + 2 > newstr_e) {
        escape_overflow = TRUE;
        break;
      }
      /* copy escaped character */
      *newstr++ = '\\';
      *newstr++ = esc;
    } else {
      if (newstr + 1 > newstr_e) {
        escape_overflow = TRUE;
        break;
      }
      /* copy non escaped character */
      *newstr++ = *escapestr;
    }
  }
  *newstr = '\0';

  if (escape_overflow) {
    return((size_t)~0);
  }
  return((size_t)(newstr - newstr_s));
}
/* }}} */

/* {{{ MADB_OS_CHARSET */
struct st_madb_os_charset {
  const char *identifier;
  const char *description;
  const char *charset;
  const char *iconv_cs;
  unsigned char supported;
};

#define MADB_CS_UNSUPPORTED 0
#define MADB_CS_APPROX 1
#define MADB_CS_EXACT 2

/* Please add new character sets at the end. */
struct st_madb_os_charset MADB_OS_CHARSET[]=
{
#ifdef _WIN32
  /* Windows code pages */
  {"037", "IBM EBCDIC US-Canada", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"437", "OEM United States", "cp850", NULL, MADB_CS_UNSUPPORTED},
  {"500", "IBM EBCDIC International", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"708", "Arabic (ASMO 708)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"709", "Arabic (ASMO-449+, BCON V4)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"710", "Transparent Arabic", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"720", "Arabic (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"737", "Greek (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"775", "Baltic (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"850", "Western European (DOS)", "cp850", NULL, MADB_CS_UNSUPPORTED},
  {"852", "Central European (DOS)", "cp852", NULL, MADB_CS_UNSUPPORTED},
  {"855", "Cyrillic (primarily Russian)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"857", "Turkish (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"858", "OEM Multilingual Latin 1 + Euro symbol", "cp850", NULL, MADB_CS_UNSUPPORTED},
  {"860", "Portuguese (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"861", "Icelandic (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"862", "Hebrew (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"863", "French Canadian (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"864", "Arabic (864)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"865", "Nordic (DOS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"866", "Cyrillic (DOS)", "cp866", NULL, MADB_CS_UNSUPPORTED},
  {"869", "Greek, Modern (DOS)", "greek", NULL, MADB_CS_UNSUPPORTED},
  {"870", "IBM EBCDIC Multilingual Latin 2", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"874", "Thai (Windows)", "tis620", NULL, MADB_CS_UNSUPPORTED},
  {"875", "Greek Modern", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"932", "Japanese (Shift-JIS)", "cp932", NULL, MADB_CS_UNSUPPORTED},
  {"936", "Chinese Simplified (GB2312)", "gbk", NULL, MADB_CS_UNSUPPORTED},
  {"949", "ANSI/OEM Korean (Unified Hangul Code)", "euckr", NULL, MADB_CS_UNSUPPORTED},
  {"950", "Chinese Traditional (Big5)", "big5", NULL, MADB_CS_UNSUPPORTED},
  {"1026", "EBCDIC Turkish (Latin 5)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1047", "EBCDIC Latin 1/Open System", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1140", "IBM EBCDIC (US-Canada-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1141", "IBM EBCDIC (Germany-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1142", "IBM EBCDIC (Denmark-Norway-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1143", "IBM EBCDIC (Finland-Sweden-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1144", "IBM EBCDIC (Italy-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1145", "IBM EBCDIC (Spain-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1146", "IBM EBCDIC (UK-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1147", "IBM EBCDIC (France-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1148", "IBM EBCDIC (International-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1149", "IBM EBCDIC (Icelandic-Euro)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1200", "UTF-16, little endian byte order", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1201", "UTF-16, big endian byte order", "utf16", NULL, MADB_CS_UNSUPPORTED},
  {"1250", "Central European (Windows)", "cp1250", NULL, MADB_CS_UNSUPPORTED},
  {"1251", "Cyrillic (Windows)", "cp1251", NULL, MADB_CS_UNSUPPORTED},
  {"1252", "Western European (Windows)", "latin1", NULL, MADB_CS_UNSUPPORTED},
  {"1253", "Greek (Windows)", "greek", NULL, MADB_CS_UNSUPPORTED},
  {"1254", "Turkish (Windows)", "latin5", NULL, MADB_CS_UNSUPPORTED},
  {"1255", "Hebrew (Windows)", "hewbrew", NULL, MADB_CS_UNSUPPORTED},
  {"1256", "Arabic (Windows)", "cp1256", NULL, MADB_CS_UNSUPPORTED},
  {"1257", "Baltic (Windows)","cp1257", NULL, MADB_CS_UNSUPPORTED},
  {"1258", "Vietnamese (Windows)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"1361", "Korean (Johab)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10000", "Western European (Mac)", "macroman", NULL, MADB_CS_UNSUPPORTED},
  {"10001", "Japanese (Mac)", "sjis", NULL, MADB_CS_UNSUPPORTED},
  {"10002", "Chinese Traditional (Mac)", "big5", NULL, MADB_CS_UNSUPPORTED},
  {"10003", "Korean (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10004", "Arabic (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10005", "Hebrew (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10006", "Greek (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10007", "Cyrillic (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10008", "Chinese Simplified (Mac)", "gb2312", NULL, MADB_CS_UNSUPPORTED},
  {"10010", "Romanian (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10017", "Ukrainian (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10021", "Thai (Mac)", "tis620", NULL, MADB_CS_UNSUPPORTED},
  {"10029", "Central European (Mac)", "macce", NULL, MADB_CS_UNSUPPORTED},
  {"10079", "Icelandic (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10081", "Turkish (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"10082", "Croatian (Mac)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"12000", "Unicode UTF-32, little endian byte order", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"12001", "Unicode UTF-32, big endian byte order", "utf32", NULL, MADB_CS_UNSUPPORTED},
  {"20000", "Chinese Traditional (CNS)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20001", "TCA Taiwan", NULL, NULL, MADB_CS_UNSUPPORTED}, 
  {"20002", "Chinese Traditional (Eten)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20003", "IBM5550 Taiwan", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20004", "TeleText Taiwan", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20005", "Wang Taiwan", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20105", "Western European (IA5)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20106", "IA5 German (7-bit)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20107", "Swedish (7-bit)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20108", "Norwegian (7-bit)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20127", "US-ASCII (7-bit)", "ascii", NULL, MADB_CS_UNSUPPORTED},
  {"20261", "T.61", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20269", "Non-Spacing Accent", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20273", "EBCDIC Germany", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20277", "EBCDIC Denmark-Norway", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20278", "EBCDIC Finland-Sweden", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20280", "EBCDIC Italy", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20284", "EBCDIC Latin America-Spain", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20285", "EBCDIC United Kingdom", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20290", "EBCDIC Japanese Katakana Extended", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20297", "EBCDIC France", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20420", "EBCDIC Arabic", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20423", "EBCDIC Greek", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20424", "EBCDIC Hebrew", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20833", "EBCDIC Korean Extended", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20838", "EBCDIC Thai", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20866", "Cyrillic (KOI8-R)", "koi8r", NULL, MADB_CS_UNSUPPORTED},
  {"20871", "EBCDIC Icelandic", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20880", "EBCDIC Cyrillic Russian", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20905", "EBCDIC Turkish", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20924", "EBCDIC Latin 1/Open System (1047 + Euro symbol)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"20932", "Japanese (JIS 0208-1990 and 0121-1990)", "ujis", NULL, MADB_CS_UNSUPPORTED},
  {"20936", "Chinese Simplified (GB2312-80)", "gb2312", NULL, MADB_CS_UNSUPPORTED},
  {"20949", "Korean Wansung", "euckr", NULL, MADB_CS_UNSUPPORTED},
  {"21025", "EBCDIC Cyrillic Serbian-Bulgarian", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"21866", "Cyrillic (KOI8-U)", "koi8u", NULL, MADB_CS_UNSUPPORTED},
  {"28591", "Western European (ISO)", "latin1", NULL, MADB_CS_UNSUPPORTED},
  {"28592", "Central European (ISO)", "latin2", NULL, MADB_CS_UNSUPPORTED},
  {"28593", "Latin 3", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"28594", "Baltic", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"28595", "ISO 8859-5 Cyrillic", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"28596", "ISO 8859-6 Arabic", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"28597", "ISO 8859-7 Greek", "greek", NULL, MADB_CS_UNSUPPORTED},
  {"28598", "Hebrew (ISO-Visual)", "hebrew", NULL, MADB_CS_UNSUPPORTED},
  {"28599", "ISO 8859-9 Turkish", "latin5", NULL, MADB_CS_UNSUPPORTED},
  {"28603", "ISO 8859-13 Estonian", "latin7", NULL, MADB_CS_UNSUPPORTED},
  {"28605", "8859-15 Latin 9", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"29001", "Europa 3", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"38598", "ISO 8859-8 Hebrew; Hebrew (ISO-Logical)", "hebrew", NULL, MADB_CS_UNSUPPORTED},
  {"50220", "ISO 2022 Japanese with no halfwidth Katakana", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50221", "ISO 2022 Japanese with halfwidth Katakana", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50222", "ISO 2022 Japanese JIS X 0201-1989", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50225", "ISO 2022 Korean", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50227", "ISO 2022 Simplified Chinese", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50229", "ISO 2022 Traditional Chinese", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50930", "EBCDIC Japanese (Katakana) Extended", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50931", "EBCDIC US-Canada and Japanese", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50933", "EBCDIC Korean Extended and Korean", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50935", "EBCDIC Simplified Chinese Extended and Simplified Chinese", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50936", "EBCDIC Simplified Chinese", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50937", "EBCDIC US-Canada and Traditional Chinese", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"50939", "EBCDIC Japanese (Latin) Extended and Japanese", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"51932", "EUC Japanese", "ujis", NULL, MADB_CS_UNSUPPORTED},
  {"51936", "EUC Simplified Chinese; Chinese Simplified (EUC)", "gb2312", NULL, MADB_CS_UNSUPPORTED},
  {"51949", "EUC Korean", "euckr", NULL, MADB_CS_UNSUPPORTED},
  {"51950", "EUC Traditional Chinese", "big5", NULL, MADB_CS_UNSUPPORTED},
  {"52936", "Chinese Simplified (HZ)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"54936", "Chinese Simplified (GB18030)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57002", "ISCII Devanagari", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57003", "ISCII Bengali", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57004", "ISCII Tamil", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57005", "ISCII Telugu", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57006", "ISCII Assamese", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57007", "ISCII Oriya", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57008", "ISCII Kannada", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57009", "ISCII Malayalam", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57010", "ISCII Gujarati", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"57011", "ISCII Punjabi", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"65000", "utf-7 Unicode (UTF-7)", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"65001", "utf-8 Unicode (UTF-8)", "utf8", NULL, MADB_CS_EXACT},
  /* non Windows */
#else
  /* iconv encodings */
  {"ASCII", "US-ASCII", "ascii", "ASCII", MADB_CS_UNSUPPORTED},
  {"US-ASCII", "US-ASCII", "ascii", "ASCII", MADB_CS_UNSUPPORTED},
  {"Big5", "Chinese for Taiwan Multi-byte set", "big5", "BIG5", MADB_CS_UNSUPPORTED},
  {"CP866", "IBM 866", "cp866", "CP866", MADB_CS_UNSUPPORTED},
  {"IBM-1252", "Catalan Spain", "cp1252", "CP1252", MADB_CS_UNSUPPORTED},
  {"ISCII-DEV", "Hindi", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"ISO-8859-1", "ISO-8859-1", "latin1", "ISO_8859-1", MADB_CS_UNSUPPORTED},
  {"ISO8859-1", "ISO-8859-1", "latin1", "ISO_8859-1", MADB_CS_UNSUPPORTED},
  {"ISO_8859-1", "ISO-8859-1", "latin1", "ISO_8859-1", MADB_CS_UNSUPPORTED},
  {"ISO88591", "ISO-8859-1", "latin1", "ISO_8859-1", MADB_CS_UNSUPPORTED},
  {"ISO-8859-13", "ISO-8859-13", "latin7", "ISO_8859-13", MADB_CS_UNSUPPORTED},
  {"ISO8859-13", "ISO-8859-13", "latin7", "ISO_8859-13", MADB_CS_UNSUPPORTED},
  {"ISO_8859-13", "ISO-8859-13", "latin7", "ISO_8859-13", MADB_CS_UNSUPPORTED},
  {"ISO885913", "ISO-8859-13", "latin7", "ISO_8859-13", MADB_CS_UNSUPPORTED},
  {"ISO-8859-15", "ISO-8859-15", "latin9", "ISO_8859-15", MADB_CS_UNSUPPORTED},
  {"ISO8859-15", "ISO-8859-15", "latin9", "ISO_8859-15", MADB_CS_UNSUPPORTED},
  {"ISO_8859-15", "ISO-8859-15", "latin9", "ISO_8859-15", MADB_CS_UNSUPPORTED},
  {"ISO885915", "ISO-8859-15", "latin9", "ISO_8859-15", MADB_CS_UNSUPPORTED},
  {"ISO-8859-2", "ISO-8859-2", "latin2", "ISO_8859-2", MADB_CS_UNSUPPORTED},
  {"ISO8859-2", "ISO-8859-2", "latin2", "ISO_8859-2", MADB_CS_UNSUPPORTED},
  {"ISO_8859-2", "ISO-8859-2", "latin2", "ISO_8859-2", MADB_CS_UNSUPPORTED},
  {"ISO88592", "ISO-8859-2", "latin2", "ISO_8859-2", MADB_CS_UNSUPPORTED},
  {"ISO-8859-7", "ISO-8859-7", "greek", "ISO_8859-7", MADB_CS_UNSUPPORTED},
  {"ISO8859-7", "ISO-8859-7", "greek", "ISO_8859-7", MADB_CS_UNSUPPORTED},
  {"ISO_8859-7", "ISO-8859-7", "greek", "ISO_8859-7", MADB_CS_UNSUPPORTED},
  {"ISO88597", "ISO-8859-7", "greek", "ISO_8859-7", MADB_CS_UNSUPPORTED},
  {"ISO-8859-8", "ISO-8859-8", "hebrew", "ISO_8859-8", MADB_CS_UNSUPPORTED},
  {"ISO8859-8", "ISO-8859-8", "hebrew", "ISO_8859-8", MADB_CS_UNSUPPORTED},
  {"ISO_8859-8", "ISO-8859-8", "hebrew", "ISO_8859-8", MADB_CS_UNSUPPORTED},
  {"ISO88598", "ISO-8859-8", "hebrew", "ISO_8859-8", MADB_CS_UNSUPPORTED},
  {"ISO-8859-9", "ISO-8859-9", "latin5", "ISO_8859-9", MADB_CS_UNSUPPORTED},
  {"ISO8859-9", "ISO-8859-9", "latin5", "ISO_8859-9", MADB_CS_UNSUPPORTED},
  {"ISO_8859-9", "ISO-8859-9", "latin5", "ISO_8859-9", MADB_CS_UNSUPPORTED},
  {"ISO88599", "ISO-8859-9", "latin5", "ISO_8859-9", MADB_CS_UNSUPPORTED},
  {"ISO-8859-4", "ISO-8859-4", NULL, "ISO_8859-4", MADB_CS_UNSUPPORTED},
  {"ISO8859-4", "ISO-8859-4", NULL, "ISO_8859-4", MADB_CS_UNSUPPORTED},
  {"ISO_8859-4", "ISO-8859-4", NULL, "ISO_8859-4", MADB_CS_UNSUPPORTED},
  {"ISO88594", "ISO-8859-4", NULL, "ISO_8859-4", MADB_CS_UNSUPPORTED},
  {"ISO-8859-5", "ISO-8859-5", NULL, "ISO_8859-5", MADB_CS_UNSUPPORTED},
  {"ISO8859-5", "ISO-8859-5", NULL, "ISO_8859-5", MADB_CS_UNSUPPORTED},
  {"ISO_8859-5", "ISO-8859-5", NULL, "ISO_8859-5", MADB_CS_UNSUPPORTED},
  {"ISO88595", "ISO-8859-5", NULL, "ISO_8859-5", MADB_CS_UNSUPPORTED},
  {"KOI8-R", "KOI8-R", "koi8r", "KOI8R", MADB_CS_UNSUPPORTED},
  {"koi8r", "KOI8-R", "koi8r", "KOI8R", MADB_CS_UNSUPPORTED},
  {"KOI8-U", "KOI8-U", "koi8u", "KOI8U", MADB_CS_UNSUPPORTED},
  {"koi8u", "KOI8-U", "koi8u", "KOI8U", MADB_CS_UNSUPPORTED},
  {"koi8t", "KOI8-T", NULL, "KOI8-T", MADB_CS_UNSUPPORTED},
  {"KOI8-T", "KOI8-T", NULL, "KOI8-T", MADB_CS_UNSUPPORTED},
  {"SJIS", "SHIFT_JIS", "sjis", "SJIS", MADB_CS_UNSUPPORTED},
  {"Shift-JIS", "SHIFT_JIS", "sjis", "SJIS", MADB_CS_UNSUPPORTED},
  {"ansi1251", "Cyrillic", "cp1251", "CP1251", MADB_CS_UNSUPPORTED},
  {"cp1251", "Cyrillic", "cp1251", "CP1251", MADB_CS_UNSUPPORTED},
  {"armscii8", "Armenian", "armscii8", "ASMSCII-8", MADB_CS_UNSUPPORTED},
  {"armscii-8", "Armenian", "armscii8", "ASMSCII-8", MADB_CS_UNSUPPORTED},
  {"big5hkscs", "Big5-HKSCS", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"cp1255", "Hebrew", "cp1255", "CP1255", MADB_CS_UNSUPPORTED},
  {"eucCN", "GB-2312", "gb2312", "GB2312", MADB_CS_UNSUPPORTED},
  {"eucJP", "UJIS", "ujis", "UJIS", MADB_CS_UNSUPPORTED},
  {"eucKR", "EUC-KR", "euckr", "EUCKR", MADB_CS_UNSUPPORTED},
  {"euctw", "EUC-TW", NULL, NULL, MADB_CS_UNSUPPORTED},
  {"gb18030", "GB 18030-2000", "gb18030", "GB18030", MADB_CS_UNSUPPORTED},
  {"gb2312", "GB2312", "gb2312", "GB2312", MADB_CS_UNSUPPORTED},
  {"gbk", "GBK", "gbk", "GBK", MADB_CS_UNSUPPORTED},
  {"georgianps", "Georgian", "geostd8", "GEORGIAN-PS", MADB_CS_UNSUPPORTED},
  {"utf8", "UTF8", "utf8", "UTF-8", MADB_CS_EXACT},
  {"utf-8", "UTF8", "utf8", "UTF-8", MADB_CS_EXACT},
#endif
  {NULL, NULL, NULL, NULL, 0}
};
/* }}} */

/* {{{ madb_get_os_character_set */
const char *madb_get_os_character_set()
{
  unsigned int i= 0;
  char *p= NULL;
#ifdef _WIN32
  char codepage[FN_REFLEN];
  snprintf(codepage, FN_REFLEN, "%u", GetConsoleCP() ? GetConsoleCP() : GetACP());
  p= codepage;
#elif defined(HAVE_NL_LANGINFO) && defined(HAVE_SETLOCALE)
  if (setlocale(LC_CTYPE, ""))
    p= nl_langinfo(CODESET);
#endif
  if (!p)
    return MADB_DEFAULT_CHARSET_NAME;
  while (MADB_OS_CHARSET[i].identifier)
  {
    if (MADB_OS_CHARSET[i].supported > MADB_CS_UNSUPPORTED &&
        strcasecmp(MADB_OS_CHARSET[i].identifier, p) == 0)
      return MADB_OS_CHARSET[i].charset;
    i++;
  }
  return MADB_DEFAULT_CHARSET_NAME;
}
/* }}} */

/* {{{ madb_get_code_page */
#ifdef _WIN32
int madb_get_windows_cp(const char *charset)
{
  unsigned int i= 0;
  while (MADB_OS_CHARSET[i].identifier)
  {
    if (MADB_OS_CHARSET[i].supported > MADB_CS_UNSUPPORTED &&
        strcmp(MADB_OS_CHARSET[i].charset, charset) == 0)
      return atoi(MADB_OS_CHARSET[i].identifier);
    i++;
  }
  return -1;
}
#endif
/* }}} */

#ifdef HAVE_ICONV
/* {{{ map_charset_name
   Changing charset name into something iconv understands, if necessary.
   Another purpose it to avoid BOMs in result string, adding BE if necessary
   e.g.UTF16 does not work form iconv, while UTF-16 does.
 */
static void map_charset_name(const char *cs_name, my_bool target_cs, char *buffer, size_t buff_len)
{
  char digits[3], endianness[3]= "BE";

  if (sscanf(cs_name, "UTF%2[0-9]%2[LBE]", digits, endianness))
  {
    /* We should have at least digits. Endianness we write either default(BE), or what we found in the string */
    snprintf(buffer, buff_len, "UTF-%s%s", digits, endianness);
  }
  else
  {
    /* Not our client - copy as is*/
    strncpy(buffer, cs_name, buff_len - 1);
    buffer[buff_len - 1]= '\0';
  }

  if (target_cs)
  {
    strncat(buffer, "//TRANSLIT", buff_len - strlen(buffer));
  }
}
/* }}} */
#endif

/* {{{ mariadb_convert_string
   Converts string from one charset to another, and writes converted string to given buffer
   @param[in]     from
   @param[in/out] from_len
   @param[in]     from_cs
   @param[out]    to
   @param[in/out] to_len
   @param[in]     to_cs
   @param[out]    errorcode

   @return -1 in case of error, bytes used in the "to" buffer, otherwise
 */
size_t STDCALL mariadb_convert_string(const char *from __attribute__((unused)),
                                      size_t *from_len __attribute__((unused)),
                                      MARIADB_CHARSET_INFO *from_cs __attribute__((unused)),
                                      char *to __attribute__((unused)),
                                      size_t *to_len __attribute__((unused)),
                                      MARIADB_CHARSET_INFO *to_cs __attribute__((unused)), int *errorcode)
{
#ifndef HAVE_ICONV
  *errorcode= ENOTSUP;
  return -1;
#else
  iconv_t conv= 0;
  size_t rc= -1;
  size_t save_len= *to_len;
  char to_encoding[128], from_encoding[128];

  *errorcode= 0;

  /* check if conversion is supported */
  if (!from_cs || !from_cs->encoding || !from_cs->encoding[0] ||
      !to_cs || !to_cs->encoding || !to_cs->encoding[0])
  {
    *errorcode= EINVAL;
    return rc;
  }

  map_charset_name(to_cs->encoding, 1, to_encoding, sizeof(to_encoding));
  map_charset_name(from_cs->encoding, 0, from_encoding, sizeof(from_encoding));

  if ((conv= iconv_open(to_encoding, from_encoding)) == (iconv_t)-1)
  {
    *errorcode= errno;
    goto error;
  }
  if ((rc= iconv(conv, IF_WIN(,IF_SOLARIS(,(char **)))&from, from_len, &to, to_len)) == (size_t)-1)
  {
    *errorcode= errno;
    goto error;
  }
  rc= save_len - *to_len;
error:
  if (conv != (iconv_t)-1)
    iconv_close(conv);
  return rc;
#endif
}
/* }}} */

