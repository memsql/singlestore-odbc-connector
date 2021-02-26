//
// Created by iblinov-ua on 2/4/21.
//

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
