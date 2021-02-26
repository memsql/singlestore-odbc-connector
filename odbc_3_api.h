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

#ifndef MARIADB_CONNECTOR_ODBC_ODBC_3_API_H
#define MARIADB_CONNECTOR_ODBC_ODBC_3_API_H

#endif //MARIADB_CONNECTOR_ODBC_ODBC_3_API_H

#include <ma_odbc.h>
#include <ma_parse.h>

SQLRETURN SQLConnectCommon(SQLHDBC ConnectionHandle,
                           SQLCHAR *ServerName,
                           SQLSMALLINT NameLength1,
                           SQLCHAR *UserName,
                           SQLSMALLINT NameLength2,
                           SQLCHAR *Authentication,
                           SQLSMALLINT NameLength3);
