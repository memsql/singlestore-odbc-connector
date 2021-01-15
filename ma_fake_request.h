#ifndef MARIADB_CONNECTOR_ODBC_MA_FAKE_REQUEST_H
#define MARIADB_CONNECTOR_ODBC_MA_FAKE_REQUEST_H

SQLRETURN MADB_FakeRequest(MADB_Stmt *Stmt, char **fields, enum enum_field_types *fieldTypes, unsigned long fieldsLength,
                       char ***data, unsigned long dataLength);

#endif //MARIADB_CONNECTOR_ODBC_MA_FAKE_REQUEST_H
