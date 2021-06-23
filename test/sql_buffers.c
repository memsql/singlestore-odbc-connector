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

#include "tap.h"

#define BUF_SIZE    1000

ODBC_TEST(buffers_native_sql) {
    SQLCHAR in_buf[BUF_SIZE];
    SQLCHAR out_buf[BUF_SIZE];
    SQLINTEGER out_len;
    const char* query;
    const char* long_query;

    // Normal run
    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), out_buf, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, query));
    IS(!strcmp(in_buf, out_buf));
    IS(out_len == strlen(query));

    query = "DELETE FROM thetable WHERE id = 1";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, query));
    IS(!strcmp(in_buf, out_buf));
    IS(out_len == strlen(query));

    // Omit length output
    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), out_buf, BUF_SIZE, NULL));
    IS(!strcmp(in_buf, query));
    IS(!strcmp(in_buf, out_buf));

    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, BUF_SIZE, NULL));
    IS(!strcmp(in_buf, query));
    IS(!strcmp(in_buf, out_buf));

    // Omit out buf but keep length output
    query = "DELETE FROM thetable WHERE id = 1";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), NULL, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, query));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), NULL, 0, &out_len));
    IS(!strcmp(in_buf, query));
    IS(out_len == strlen(query));

    // Don't provide out buf length
    strcpy(out_buf, "DEADBEEF");

    query = "DELETE FROM thetable WHERE id = 1";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, 0, &out_len));
    IS(!strcmp(in_buf, query));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_ERR(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, SQL_NTS, &out_len), "HY090", 0, "Invalid string or buffer length");

    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, SQL_NTS, out_buf, 0, NULL));
    IS(!strcmp(in_buf, query));

    IS(!strcmp("DEADBEEF", out_buf));

    // Shortcut in buff
    query = "SELECT (1, 2, 3)";
    long_query = "SELECT (1, 2, 3)BLAH";
    strcpy(in_buf, long_query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(query), out_buf, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, long_query));
    IS(!strcmp(query, out_buf));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, 0, out_buf, BUF_SIZE, &out_len));
    IS(!strcmp(in_buf, query));
    IS(out_buf[0] == 0);

    // Shortcut out buff
    query = "SELECT (1, 2, 3)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), out_buf, 10, &out_len));
    IS(!strcmp(in_buf, query));
    IS(strlen(out_buf) == 9)
    IS(!strncmp(in_buf, out_buf, 9));
    IS(out_len == strlen(query));   // The standard is not very clear there

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    strcpy(in_buf, query);
    CHECK_DBC_RC(Connection, SQLNativeSql(Connection, in_buf, strlen(in_buf), out_buf, 15, NULL));
    IS(!strcmp(in_buf, query));
    IS(strlen(out_buf) == 14)
    IS(!strncmp(in_buf, out_buf, 14));

    return OK;
}

ODBC_TEST(buffers_native_sql_w) {
    SQLWCHAR* in_buf;
    SQLWCHAR out_buf[BUF_SIZE];
    SQLINTEGER out_len;
    const char* query;
    const char* long_query;

    // Normal run
    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), out_buf, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharequal(in_buf, out_buf));
    IS(out_len == strlen(query));

    query = "DELETE FROM thetable WHERE id = 1";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharequal(in_buf, out_buf));
    IS(out_len == strlen(query));

    // Omit length output
    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), out_buf, BUF_SIZE, NULL));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharequal(in_buf, out_buf));

    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, BUF_SIZE, NULL));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharequal(in_buf, out_buf));

    // Omit out buf but keep length output
    query = "DELETE FROM thetable WHERE id = 1";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), NULL, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), NULL, 0, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(out_len == strlen(query));

    // Don't provide out buf length
    strcpy((char*)out_buf, "DEADBEEF");

    query = "DELETE FROM thetable WHERE id = 1";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, 0, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_ERR(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, SQL_NTS, &out_len), "HY090", 0, "Invalid string or buffer length");

    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, SQL_NTS, out_buf, 0, NULL));
    IS(sqlwcharequal(in_buf, CW(query)));

    IS(!strcmp("DEADBEEF", (char*)out_buf));

    // Shortcut in buff
    query = "SELECT (1, 2, 3)";
    long_query = "SELECT (1, 2, 3)BLAH";
    in_buf = CW(long_query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, strlen(query), out_buf, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(long_query)));
    IS(sqlwcharequal(out_buf, CW(query)));
    IS(out_len == strlen(query));

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, 0, out_buf, BUF_SIZE, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(out_buf[0] == 0);

    // Shortcut out buff
    query = "SELECT (1, 2, 3)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), out_buf, 10, &out_len));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharlen(out_buf) == 9)
    IS(!sqlwcharcmp(in_buf, out_buf, 9));
    IS(out_len == strlen(query));   // The standard is not very clear there

    query = "INSERT INTO thetable VALUES (1, 2, none)";
    in_buf = CW(query);
    CHECK_DBC_RC(Connection, SQLNativeSqlW(Connection, in_buf, sqlwcharlen(in_buf), out_buf, 15, NULL));
    IS(sqlwcharequal(in_buf, CW(query)));
    IS(sqlwcharlen(out_buf) == 14)
    IS(!sqlwcharcmp(in_buf, out_buf, 14));

    return OK;
}

MA_ODBC_TESTS my_tests[] = {
   {buffers_native_sql, "buffers_native_sql", NORMAL, ALL_DRIVERS},
   {buffers_native_sql_w, "buffers_native_sql_w", NORMAL, ALL_DRIVERS},
   {NULL, NULL, NORMAL, ALL_DRIVERS}
};


int main(int argc, char **argv) {
    int tests = LENGTHOF(my_tests) - 1;
    get_options(argc, argv);
    plan(tests);
    return run_tests(my_tests);
}
