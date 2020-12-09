/*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
                2013 MontyProgram AB

  The MySQL Connector/ODBC is licensed under the terms of the GPLv2
  <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
  MySQL Connectors. There are special exceptions to the terms and
  conditions of the GPLv2 as it is applied to this software, see the
  FLOSS License Exception
  <http://www.mysql.com/about/legal/licensing/foss-exception.html>.
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; version 2 of the License.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "tap.h"

#include <time.h>

/* UPDATE with no keys ...  */
ODBC_TEST(my_no_keys)
{
    SQLRETURN rc;
    SQLINTEGER nData;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS my_no_keys");
  OK_SIMPLE_STMT(Stmt, "create table my_no_keys(col1 int,\
                                                      col2 varchar(30),\
                                                      col3 int,\
                                                      col4 int)");

  OK_SIMPLE_STMT(Stmt, "insert into my_no_keys values(100,'MySQL1',1,3000)");
  OK_SIMPLE_STMT(Stmt, "insert into my_no_keys values(200,'MySQL1',2,3000)");
  OK_SIMPLE_STMT(Stmt, "insert into my_no_keys values(300,'MySQL1',3,3000)");
  OK_SIMPLE_STMT(Stmt, "insert into my_no_keys values(400,'MySQL1',4,3000)");

    rc = SQLFreeStmt(Stmt,SQL_CLOSE);
    CHECK_STMT_RC(Stmt,rc);     

    rc = SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE, (SQLPOINTER)SQL_CURSOR_DYNAMIC, 0);
    CHECK_STMT_RC(Stmt, rc);

    rc = SQLSetStmtAttr(Stmt, SQL_ATTR_CONCURRENCY ,(SQLPOINTER)SQL_CONCUR_ROWVER , 0);
    CHECK_STMT_RC(Stmt, rc);  

    rc = SQLSetStmtAttr(Stmt, SQL_ATTR_ROW_ARRAY_SIZE  ,(SQLPOINTER)1 , 0);
    CHECK_STMT_RC(Stmt, rc);  

    /* UPDATE ROW[2]COL[4] */
    OK_SIMPLE_STMT(Stmt, "select col4 from my_no_keys");
    CHECK_STMT_RC(Stmt,rc);    

    rc = SQLBindCol(Stmt,1,SQL_C_LONG,&nData,100,NULL);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLExtendedFetch(Stmt,SQL_FETCH_ABSOLUTE,2,NULL,NULL);
    CHECK_STMT_RC(Stmt,rc);

    nData = 999;

    rc = SQLFreeStmt(Stmt,SQL_UNBIND);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLFreeStmt(Stmt,SQL_CLOSE);
    CHECK_STMT_RC(Stmt,rc);

    OK_SIMPLE_STMT(Stmt,"select * from my_no_keys");

    IS(4 == myrowcount(Stmt));

    rc = SQLFreeStmt(Stmt,SQL_CLOSE);
    CHECK_STMT_RC(Stmt,rc);

    OK_SIMPLE_STMT(Stmt,"select * from my_no_keys");

    rc = SQLFetch(Stmt);
    CHECK_STMT_RC(Stmt,rc);
    IS(3000 == my_fetch_int(Stmt,4));

    rc = SQLFetch(Stmt);
    CHECK_STMT_RC(Stmt,rc);
    IS(3000 == my_fetch_int(Stmt,4));

    rc = SQLFetch(Stmt);
    CHECK_STMT_RC(Stmt,rc);
    IS(3000 == my_fetch_int(Stmt,4));

    SQLFreeStmt(Stmt,SQL_UNBIND);
    SQLFreeStmt(Stmt,SQL_CLOSE);

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS my_no_keys");

  return OK;
}

ODBC_TEST(foreign_keys_error)
{
    FAIL_IF(SQLForeignKeys(Stmt,
                           NULL, SQL_NTS,
                           NULL, SQL_NTS,
                           NULL, SQL_NTS,
                           NULL, SQL_NTS,
                           NULL, SQL_NTS,
                           (SQLCHAR *)"test_table_fk", SQL_NTS) != SQL_ERROR, "Error expected");

    FAIL_IF(SQLForeignKeysW(Stmt, NULL, 0, NULL, 0, NULL, 0, NULL, 0,
                            NULL, 0, W(L"test_table_fk"), SQL_NTS) != SQL_ERROR, "Error expected");

    return OK;
}

void t_strstr()
{
    char    *string = "'TABLE','VIEW','SYSTEM TABLE'";
    char    *str=",";
    char    *type;

    type = strstr((const char *)string,(const char *)str);
    while (type++)
    {
        int len = (int)(type - string);
        diag("\n Found '%s' at position '%d[%s]", str, len, type);
        type = strstr(type,str);
    }
}



MA_ODBC_TESTS my_tests[]=
{
  {my_no_keys, "my_no_keys"},
  {foreign_keys_error, "foreign_keys_error"},
  {NULL, NULL}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  mark_all_tests_normal(my_tests);
  return run_tests(my_tests);
}
