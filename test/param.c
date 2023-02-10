  /*
  Copyright (c) 2001, 2012, Oracle and/or its affiliates. All rights reserved.
                2013, 2018 MariaDB Corporation AB
                2021 SingleStore, Inc.

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

/********************************************************
* initialize tables                                     *
*********************************************************/
ODBC_TEST(my_init_table)
{
  SQLRETURN   rc;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE if exists my_demo_param");

  /* commit the transaction */
  rc = SQLEndTran(SQL_HANDLE_DBC, Connection, SQL_COMMIT);
  CHECK_DBC_RC(Connection,rc);

  /* create the table 'my_demo_param' */
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE my_demo_param(\
                            id   int,\
                            auto int primary key auto_increment,\
                            name varchar(20),\
                            timestamp timestamp)");

  return OK;
}

ODBC_TEST(num_params)
{
  SQLRETURN   rc;
  SQLSMALLINT param_count;

  rc = SQLPrepare(Stmt, (SQLCHAR *)"INSERT INTO my_demo_param(id,name) VALUES(?,?)",SQL_NTS);
  CHECK_STMT_RC(Stmt,rc);
  SQLFreeStmt(Stmt, 0);

  rc = SQLNumParams(Stmt, &param_count);
  CHECK_STMT_RC(Stmt,rc);
  FAIL_IF_NE_INT(param_count, 2, "SQLNumParams returns wrong result");

  rc = SQLPrepare(Stmt, (SQLCHAR *)"INSERT INTO my_demo_param(id) VALUES(?)",SQL_NTS);
  CHECK_STMT_RC(Stmt,rc);

  rc = SQLNumParams(Stmt, &param_count);
  CHECK_STMT_RC(Stmt,rc);
  FAIL_IF_NE_INT(param_count, 1, "Wrong SQLNumParams result");

  return OK;
}

ODBC_TEST(my_param_insert)
{
  SQLRETURN   rc;
  SQLINTEGER  id;
  char        name[50];

  /* prepare the insert statement with parameters */
  rc = SQLPrepare(Stmt, (SQLCHAR *)"INSERT INTO my_demo_param(id,name) VALUES(?,?)",SQL_NTS);
  CHECK_STMT_RC(Stmt,rc);

  /* now supply data to parameter 1 and 2 */
  rc = SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT,
                        SQL_C_LONG, SQL_INTEGER, 0,0,
                        &id, 0, NULL);
  CHECK_STMT_RC(Stmt,rc);

  rc = SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT,
                        SQL_C_CHAR, SQL_CHAR, 0,0,
                        name, sizeof(name), NULL);
  CHECK_STMT_RC(Stmt,rc);

  /* now insert 10 rows of data */
  for (id = 0; id < 10; id++)
  {
      sprintf(name,"MySQL%d",id);

      rc = SQLExecute(Stmt);
      CHECK_STMT_RC(Stmt,rc);
  }

  /* Free statement param resorces */
  rc = SQLFreeStmt(Stmt, SQL_RESET_PARAMS);
  CHECK_STMT_RC(Stmt,rc);

  /* Free statement cursor resorces */
  rc = SQLFreeStmt(Stmt, SQL_CLOSE);
  CHECK_STMT_RC(Stmt,rc);

  /* commit the transaction */
  rc = SQLEndTran(SQL_HANDLE_DBC, Connection, SQL_COMMIT);
  CHECK_DBC_RC(Connection,rc);

  /* Now fetch and verify the data */
  OK_SIMPLE_STMT(Stmt, "SELECT * FROM my_demo_param");

  FAIL_IF(10 != myrowcount(Stmt), "expected 10 rows");

  return OK;
}

ODBC_TEST(test_numeric)
{
  SQLRETURN rc;
  unsigned long num= 2;

  OK_SIMPLE_STMT(Stmt, "SET GLOBAL GENERAL_LOG=1");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_numeric");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_numeric (`Id` int(11) NOT NULL AUTO_INCREMENT, "\
                        "a varchar(255) NOT NULL, b int(11) NOT NULL, c int(11) NOT NULL, "\
                        "d int(11) NOT NULL, PRIMARY KEY (`Id`)) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=latin1");
  OK_SIMPLE_STMT(Stmt, "INSERT INTO t_numeric VALUES (NULL, 'test', 1, 2, 3)");

  SQLBindParameter(Stmt,  1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_NUMERIC, 0, 0, &num, 0, NULL);

  OK_SIMPLE_STMT(Stmt, "SELECT * FROM t_numeric WHERE c=?");

  rc= SQLFetch(Stmt);
  FAIL_IF(rc== SQL_NO_DATA, "unexpected eof");

  return OK;
}

ODBC_TEST(unbuffered_result)
{
  SQLRETURN rc;
  SQLHSTMT Stmt1, Stmt2;

  SQLAllocHandle(SQL_HANDLE_STMT, Connection, &Stmt1);
  SQLSetStmtAttr(Stmt1, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY, SQL_IS_INTEGER);
  OK_SIMPLE_STMT(Stmt1, "DROP TABLE IF EXISTS t1");
  OK_SIMPLE_STMT(Stmt1, "CREATE TABLE t1 (a int)");
  OK_SIMPLE_STMT(Stmt1, "INSERT INTO t1 VALUES (1),(2),(3)");
  OK_SIMPLE_STMT(Stmt1, "SELECT * from t1");

  rc= SQLFetch(Stmt1);
  FAIL_IF(rc == SQL_NO_DATA, "unexpected eof");

  SQLAllocHandle(SQL_HANDLE_STMT, Connection, &Stmt2);
  SQLSetStmtAttr(Stmt2, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY, SQL_IS_INTEGER);
  OK_SIMPLE_STMT(Stmt2, "SELECT * from t1");

  SQLFreeStmt(Stmt1, SQL_DROP);
  SQLFreeStmt(Stmt2, SQL_DROP);
  return OK;
}


ODBC_TEST(my_param_update)
{
    SQLRETURN  rc;
    SQLLEN nRowCount;
    SQLINTEGER id=9;
    char name[]="update";

    /* prepare the insert statement with parameters */
    rc = SQLPrepare(Stmt, (SQLCHAR *)"UPDATE my_demo_param set name = ? WHERE id = ?",SQL_NTS);
    CHECK_STMT_RC(Stmt,rc);

    /* now supply data to parameter 1 and 2 */
    rc = SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_CHAR, SQL_CHAR, 0,0,
                          name, sizeof(name), NULL);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    CHECK_STMT_RC(Stmt,rc);

    /* now execute the update statement */
    rc = SQLExecute(Stmt);
    CHECK_STMT_RC(Stmt,rc);

    /* check the rows affected by the update statement */
    rc = SQLRowCount(Stmt, &nRowCount);
    CHECK_STMT_RC(Stmt,rc);
    diag("\n total rows updated:%d\n",nRowCount);
    IS( nRowCount == 1);

    /* Free statement param resorces */
    rc = SQLFreeStmt(Stmt, SQL_RESET_PARAMS);
    CHECK_STMT_RC(Stmt,rc);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(Stmt, SQL_CLOSE);
    CHECK_STMT_RC(Stmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, Connection, SQL_COMMIT);
    CHECK_DBC_RC(Connection,rc);

    /* Now fetch and verify the data */
    OK_SIMPLE_STMT(Stmt, "SELECT * FROM my_demo_param");
    CHECK_STMT_RC(Stmt,rc);

    IS(10 == myrowcount(Stmt));

  return OK;
}


ODBC_TEST(my_param_delete)
{
    SQLRETURN  rc;
    SQLINTEGER id;
    SQLLEN nRowCount;

    /* supply data to parameter 1 */
    rc = SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    CHECK_STMT_RC(Stmt,rc);

    /* execute the DELETE STATEMENT to delete 5th row  */
    id = 5;
    OK_SIMPLE_STMT(Stmt,"DELETE FROM my_demo_param WHERE id = ?");

    /* check the rows affected by the update statement */
    rc = SQLRowCount(Stmt, &nRowCount);
    CHECK_STMT_RC(Stmt,rc);
    diag(" total rows deleted:%d\n",nRowCount);
    IS( nRowCount == 1);

    SQLFreeStmt(Stmt, SQL_RESET_PARAMS);
    SQLFreeStmt(Stmt, SQL_CLOSE);

    /* execute the DELETE STATEMENT to delete 8th row  */
    rc = SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT,
                          SQL_C_LONG, SQL_INTEGER, 0,0,
                          &id, 0, NULL);
    CHECK_STMT_RC(Stmt,rc);

    id = 8;
    OK_SIMPLE_STMT(Stmt,"DELETE FROM my_demo_param WHERE id = ?");

    /* check the rows affected by the update statement */
    rc = SQLRowCount(Stmt, &nRowCount);
    CHECK_STMT_RC(Stmt,rc);
    diag(" total rows deleted:%d\n",nRowCount);
    IS( nRowCount == 1);

    /* Free statement param resorces */
    rc = SQLFreeStmt(Stmt, SQL_RESET_PARAMS);
    CHECK_STMT_RC(Stmt,rc);

    /* Free statement cursor resorces */
    rc = SQLFreeStmt(Stmt, SQL_CLOSE);
    CHECK_STMT_RC(Stmt,rc);

    /* commit the transaction */
    rc = SQLEndTran(SQL_HANDLE_DBC, Connection, SQL_COMMIT);
    CHECK_DBC_RC(Connection,rc);

    /* Now fetch and verify the data */
    OK_SIMPLE_STMT(Stmt, "SELECT * FROM my_demo_param");

    IS(8 == myrowcount(Stmt));

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    /* drop the table */
    OK_SIMPLE_STMT(Stmt,"DROP TABLE my_demo_param");

    rc = SQLFreeStmt(Stmt,SQL_CLOSE);
    CHECK_STMT_RC(Stmt,rc);

  return OK;
}


/*I really wonder what is this test about */
ODBC_TEST(tmysql_fix)
{
  SQLRETURN rc;

  diag("SQLDescribeParam not supported yet");
  return SKIP;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS tmysql_err");

  OK_SIMPLE_STMT(Stmt,"CREATE TABLE tmysql_err (\
                  td date NOT NULL default '0000-00-00',\
                  node varchar(8) NOT NULL default '',\
                  tag varchar(10) NOT NULL default '',\
                  sqlname varchar(8) default NULL,\
                  fix_err varchar(100) default NULL,\
                  sql_err varchar(255) default NULL,\
                  prog_err varchar(100) default NULL\
                ) ENGINE=MyISAM");

  OK_SIMPLE_STMT(Stmt,"INSERT INTO tmysql_err VALUES\
                  ('0000-00-00','0','0','0','0','0','0'),\
                  ('2001-08-29','FIX','SQLT2','ins1',\
                  NULL,NULL, 'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2','ins1',\
                  NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000!-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.'),\
                  ('0000-00-00','0','0','0','0','0','0'),('2001-08-29','FIX','SQLT2',\
                  'ins1',NULL,NULL,'Error.  SQL cmd %s is not terminated or too long.')");

  /* trace based */
  {
    SQLSMALLINT pcpar,pccol,pfSqlType,pibScale,pfNullable;
    SQLSMALLINT index;
    SQLCHAR     td[30]="20010830163225";
    SQLCHAR     node[30]="FIX";
    SQLCHAR     tag[30]="SQLT2";
    SQLCHAR     sqlname[30]="ins1";
    SQLCHAR     sqlerr[30]="error";
    SQLCHAR     fixerr[30]= "fixerr";
    SQLCHAR     progerr[30]="progerr";
    SQLULEN     pcbParamDef;

    SQLFreeStmt(Stmt,SQL_CLOSE);
    rc = SQLPrepare(Stmt,
      (SQLCHAR *)"insert into tmysql_err (TD, NODE, TAG, SQLNAME, SQL_ERR,"
                 "FIX_ERR, PROG_ERR) values (?, ?, ?, ?, ?, ?, ?)", 103);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLNumParams(Stmt,&pcpar);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLNumResultCols(Stmt,&pccol);
    CHECK_STMT_RC(Stmt,rc);

    for (index=1; index <= pcpar; index++)
    {
      rc = SQLDescribeParam(Stmt,index,&pfSqlType,&pcbParamDef,&pibScale,&pfNullable);
      CHECK_STMT_RC(Stmt,rc);

      diag("descparam[%d]:%d,%d,%d,%d\n",index,pfSqlType,pcbParamDef,pibScale,pfNullable);
    }

    /* TODO: C and SQL types as numeric consts. Splendid.*/
    rc = SQLBindParameter(Stmt,1,SQL_PARAM_INPUT,11,12,0,0,td,100,0);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLBindParameter(Stmt,2,SQL_PARAM_INPUT,1,12,0,0,node,100,0);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLBindParameter(Stmt,3,SQL_PARAM_INPUT,1,12,0,0,tag,100,0);
    CHECK_STMT_RC(Stmt,rc);
    rc = SQLBindParameter(Stmt,4,SQL_PARAM_INPUT,1,12,0,0,sqlname,100,0);
    CHECK_STMT_RC(Stmt,rc);
    rc = SQLBindParameter(Stmt,5,SQL_PARAM_INPUT,1,12,0,0,sqlerr,0,0);
    CHECK_STMT_RC(Stmt,rc);
    rc = SQLBindParameter(Stmt,6,SQL_PARAM_INPUT,1,12,0,0,fixerr,0,0);
    CHECK_STMT_RC(Stmt,rc);
    rc = SQLBindParameter(Stmt,7,SQL_PARAM_INPUT,1,12,0,0,progerr,0,0);
    CHECK_STMT_RC(Stmt,rc);

    rc = SQLExecute(Stmt);
    CHECK_STMT_RC(Stmt,rc);
  }

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS tmysql_err");

  return OK;
}


/*
  Test basic handling of SQL_ATTR_PARAM_BIND_OFFSET_PTR
*/
ODBC_TEST(t_param_offset)
{
  const SQLINTEGER rowcnt= 5;
  SQLINTEGER i;
  struct {
    SQLINTEGER id;
    SQLINTEGER x;
  } rows[25];
  size_t row_size= (sizeof(rows) / 25);
  SQLINTEGER out_id, out_x;
  SQLULEN bind_offset= 20 * row_size;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_param_offset");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_param_offset (id int not null, x int)");

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_BIND_OFFSET_PTR,
                                &bind_offset, 0));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &rows[0].id, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
                                  SQL_INTEGER, 0, 0, &rows[0].x, 0, NULL));

  for (i= 0; i < rowcnt; ++i)
  {
    rows[20+i].id= i * 10;
    rows[20+i].x= (i * 1000) % 97;
    OK_SIMPLE_STMT(Stmt, "insert into t_param_offset values (?,?)");
    bind_offset+= row_size;
  }

  /* verify the data */

  OK_SIMPLE_STMT(Stmt, "select id, x from t_param_offset order by 1");

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &out_id, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_LONG, &out_x, 0, NULL));

  for (i= 0; i < rowcnt; ++i)
  {
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(out_id, rows[20+i].id);
    is_num(out_id, i * 10);
    is_num(out_x, rows[20+i].x);
    is_num(out_x, (i * 1000) % 97);
  }

  return OK;
}


/*
Bug 48310 - parameters array support request.
Binding by row test
*/
ODBC_TEST(paramarray_by_row)
{
#define ROWS_TO_INSERT 3
#define STR_FIELD_LENGTH 255
  typedef struct DataBinding
  {
    SQLCHAR     bData[5];
    SQLINTEGER  intField;
    SQLCHAR     strField[STR_FIELD_LENGTH];
    SQLLEN      indBin;
    SQLLEN      indInt;
    SQLLEN      indStr;
  } DATA_BINDING;

   const SQLCHAR *str[]= {"nothing for 1st", "longest string for row 2", "shortest"  };

  SQLCHAR       buff[50];
  DATA_BINDING  dataBinding[ROWS_TO_INSERT];
  SQLUSMALLINT  paramStatusArray[ROWS_TO_INSERT];
  SQLULEN       paramsProcessed, i, nLen;
  SQLLEN        rowsCount;

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "DROP TABLE IF EXISTS t_bug48310", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "CREATE TABLE t_bug48310 (id int primary key auto_increment,"\
    "bData binary(5) NULL, intField int not null, strField varchar(255) not null)", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER)sizeof(DATA_BINDING), 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ROWS_TO_INSERT, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, dataBinding[0].bData, 0, &dataBinding[0].indBin));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
    0, 0, &dataBinding[0].intField, 0, &dataBinding[0].indInt));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
    0, 0, dataBinding[0].strField, 0, &dataBinding[0].indStr ));

  memcpy(dataBinding[0].bData, "\x01\x80\x00\x80\x00", 5);
  dataBinding[0].intField= 1;
 
  memcpy(dataBinding[1].bData, "\x02\x80\x00\x80", 4);
  dataBinding[1].intField= 0;
 
  memcpy(dataBinding[2].bData, "\x03\x80\x00", 3);
  dataBinding[2].intField= 223322;
 
  for (i= 0; i < ROWS_TO_INSERT; ++i)
  {
    strcpy(dataBinding[i].strField, str[i]);
    dataBinding[i].indBin= 5 - i;
    dataBinding[i].indInt= 0;
    dataBinding[i].indStr= SQL_NTS;
  }

  /* We don't expect errors in paramsets processing, thus we should get SQL_SUCCESS only*/
  FAIL_IF(SQLExecDirect(Stmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS) != SQL_SUCCESS, "success expected");

  is_num(paramsProcessed, ROWS_TO_INSERT);

  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowsCount));
  is_num(rowsCount, ROWS_TO_INSERT);

  for (i= 0; i < paramsProcessed; ++i)
    if ( paramStatusArray[i] != SQL_PARAM_SUCCESS
      && paramStatusArray[i] != SQL_PARAM_SUCCESS_WITH_INFO )
    {
      diag("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArray[i]);
      return FAIL;
    }

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)1, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "SELECT bData, intField, strField\
                                      FROM t_bug48310\
                                      ORDER BY id", SQL_NTS));

  /* Just to make sure RowCount isn't broken */
  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowsCount));
  FAIL_IF(rowsCount != 0 && rowsCount != ROWS_TO_INSERT, "Wrong row count");

  for (i= 0; i < paramsProcessed; ++i)
  {
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_BINARY, (SQLPOINTER)buff, 50, &nLen));
    IS(memcmp((const void*) buff, (const void*)dataBinding[i].bData, 5 - i)==0);
    is_num(my_fetch_int(Stmt, 2), dataBinding[i].intField);
    IS_STR(my_fetch_str(Stmt, buff, 3), dataBinding[i].strField, strlen(str[i]));
  }

  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "eof expected");
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  /* One more check that RowCount isn't broken. check may get broken if input data
     changes */
  OK_SIMPLE_STMT(Stmt, "update t_bug48310 set strField='changed' where intField > 1");
  CHECK_STMT_RC(Stmt, SQLRowCount(Stmt, &rowsCount));
  is_num(rowsCount, 1);

  /* Clean-up */
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "DROP TABLE IF EXISTS bug48310", SQL_NTS));

  return OK;

#undef ROWS_TO_INSERT
#undef STR_FIELD_LENGTH
}


/*
Bug 48310 - parameters array support request.
Binding by column test
*/
ODBC_TEST(paramarray_by_column)
{
#define ROWS_TO_INSERT 3
#define STR_FIELD_LENGTH 5
  SQLCHAR       buff[50];

  SQLCHAR       bData[ROWS_TO_INSERT][STR_FIELD_LENGTH]={{0x01, 0x80, 0x00, 0x80, 0x03},
                                          {0x02, 0x80, 0x00, 0x02},
                                          {0x03, 0x80, 0x01}};
  SQLLEN        bInd[ROWS_TO_INSERT]= {5,4,3};

  const SQLCHAR strField[ROWS_TO_INSERT][STR_FIELD_LENGTH]= {{'\0'}, {'x','\0'}, {'x','x','x','\0'} };
  SQLLEN        strInd[ROWS_TO_INSERT]= {SQL_NTS, SQL_NTS, SQL_NTS};

  SQLINTEGER    intField[ROWS_TO_INSERT] = {123321, 1, 0};
  SQLLEN        intInd[ROWS_TO_INSERT]= {5,4,3};

  SQLUSMALLINT  paramStatusArray[ROWS_TO_INSERT];
  SQLULEN       paramsProcessed, i, nLen;

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "DROP TABLE IF EXISTS t_bug48310", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "CREATE TABLE t_bug48310 (id int primary key auto_increment,"\
    "bData binary(5) NULL, intField int not null, strField varchar(255) not null)", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ROWS_TO_INSERT, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, bData, STR_FIELD_LENGTH, bInd));
  is_num(iOdbcSetParamBufferSize(Stmt, 1, STR_FIELD_LENGTH), OK);
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
    0, 0, intField, 0, intInd));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
    0, 0, (SQLPOINTER)strField, STR_FIELD_LENGTH, strInd ));
  is_num(iOdbcSetParamBufferSize(Stmt, 3, STR_FIELD_LENGTH), OK);

  /* We don't expect errors in paramsets processing, thus we should get SQL_SUCCESS only*/
  FAIL_IF(SQLExecDirect(Stmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS) != SQL_SUCCESS, "success expected");

  is_num(paramsProcessed, ROWS_TO_INSERT);

  for (i= 0; i < paramsProcessed; ++i)
    if ( paramStatusArray[i] != SQL_PARAM_SUCCESS
      && paramStatusArray[i] != SQL_PARAM_SUCCESS_WITH_INFO )
    {
      diag("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArray[i]);
      return FAIL;
    }

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)1, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "SELECT bData, intField, strField\
                                       FROM t_bug48310\
                                       ORDER BY id", SQL_NTS));

  for (i= 0; i < paramsProcessed; ++i)
  {
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_BINARY, (SQLPOINTER)buff, 50, &nLen));
    if (memcmp((const void*) buff, bData[i], 5 - i)!=0)
    {
      diag("Bin data inserted wrongly. Read: 0x%02X%02X%02X%02X%02X Had to be: 0x%02X%02X%02X%02X%02X"
        , buff[0], buff[1], buff[2], buff[3], buff[4]
        , bData[i][0], bData[i][1], bData[i][2], bData[i][3], bData[i][4]);
      return FAIL;
    }
    is_num(my_fetch_int(Stmt, 2), intField[i]);
    IS_STR(my_fetch_str(Stmt, buff, 3), strField[i], strlen(strField[i]));
  }

  /* Clean-up */
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "DROP TABLE IF EXISTS bug48310", SQL_NTS));

  return OK;

#undef ROWS_TO_INSERT
#undef STR_FIELD_LENGTH
}


/*
Bug 48310 - parameters array support request.
Ignore paramset test
*/
ODBC_TEST(paramarray_ignore_paramset)
{
#define ROWS_TO_INSERT 4
#define STR_FIELD_LENGTH 5
  SQLCHAR       buff[50];

  SQLCHAR       bData[ROWS_TO_INSERT][STR_FIELD_LENGTH]={{0x01, 0x80, 0x00, 0x80, 0x03},
                                                        {0x02, 0x80, 0x00, 0x02},
                                                        {0x03, 0x80, 0x01}};
  SQLLEN        bInd[ROWS_TO_INSERT]= {5,4,3};

  const SQLCHAR strField[ROWS_TO_INSERT][STR_FIELD_LENGTH]= {{'\0'}, {'x','\0'}, {'x','x','x','\0'} };
  SQLLEN        strInd[ROWS_TO_INSERT]= {SQL_NTS, SQL_NTS, SQL_NTS};

  SQLINTEGER    intField[ROWS_TO_INSERT] = {123321, 1, 0};
  SQLLEN        intInd[ROWS_TO_INSERT]= {5,4,3};

  SQLUSMALLINT  paramOperationArr[ROWS_TO_INSERT]={0,SQL_PARAM_IGNORE,0,SQL_PARAM_IGNORE};
  SQLUSMALLINT  paramStatusArr[ROWS_TO_INSERT];
  SQLULEN       paramsProcessed, i, nLen, rowsInserted= 0;

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "DROP TABLE IF EXISTS t_bug48310", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "CREATE TABLE t_bug48310 (id int primary key auto_increment,"\
    "bData binary(5) NULL, intField int not null, strField varchar(255) not null)", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ROWS_TO_INSERT, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArr, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_OPERATION_PTR, paramOperationArr, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, bData, STR_FIELD_LENGTH, bInd));
  is_num(iOdbcSetParamBufferSize(Stmt, 1, STR_FIELD_LENGTH), OK);
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
    0, 0, intField, 0, intInd));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
    0, 0, (SQLPOINTER)strField, STR_FIELD_LENGTH, strInd ));
  is_num(iOdbcSetParamBufferSize(Stmt, 3, STR_FIELD_LENGTH), OK);

  /* We don't expect errors in paramsets processing, thus we should get SQL_SUCCESS only*/
  FAIL_IF(SQLExecDirect(Stmt, "INSERT INTO t_bug48310 (bData, intField, strField) " \
    "VALUES (?,?,?)", SQL_NTS) != SQL_SUCCESS, "success expected");

  is_num(paramsProcessed, ROWS_TO_INSERT);

  for (i= 0; i < paramsProcessed; ++i)
  {
    if (paramOperationArr[i] == SQL_PARAM_IGNORE)
    {
      is_num(paramStatusArr[i], SQL_PARAM_UNUSED);
    }
    else if ( paramStatusArr[i] != SQL_PARAM_SUCCESS
      && paramStatusArr[i] != SQL_PARAM_SUCCESS_WITH_INFO )
    {
      diag("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArr[i]);
      return FAIL;
    }
  }

  /* Resetting statements attributes */
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)1, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, NULL, 0));

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "SELECT bData, intField, strField\
                                      FROM t_bug48310\
                                      ORDER BY id", SQL_NTS));

  i= 0;
  while(i < paramsProcessed)
  {
    if (paramStatusArr[i] == SQL_PARAM_UNUSED)
    {
      ++i;
      continue;
    }

    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

    CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_BINARY, (SQLPOINTER)buff, 50, &nLen));

    if (memcmp((const void*) buff, bData[i], 5 - i)!=0)
    {
      diag("Wrong Bin data to has been inserted to the row #%d. Read: 0x%02X%02X%02X%02X%02X Expected: 0x%02X%02X%02X%02X%02X"
        , i + 1, buff[0], buff[1], buff[2], buff[3], buff[4]
      , bData[i][0], bData[i][1], bData[i][2], bData[i][3], bData[i][4]);
      return FAIL;
    }
    is_num(my_fetch_int(Stmt, 2), intField[i]);
    IS_STR(my_fetch_str(Stmt, buff, 3), strField[i], strlen(strField[i]));

    ++rowsInserted;
    ++i;
  }

  /* Making sure that there is nothing else to fetch ... */
  FAIL_IF(SQLFetch(Stmt) != SQL_NO_DATA_FOUND, "eof expected");

  /* ... and that inserted was less than SQL_ATTR_PARAMSET_SIZE rows */
  IS( rowsInserted < ROWS_TO_INSERT);
  
  /* Clean-up */
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "DROP TABLE IF EXISTS bug48310", SQL_NTS));

  return OK;

#undef ROWS_TO_INSERT
#undef STR_FIELD_LENGTH
}


/*
  Bug 48310 - parameters array support request.
  Select statement.
*/
ODBC_TEST(paramarray_select)
{
#define STMTS_TO_EXEC 3

  SQLINTEGER    intField[STMTS_TO_EXEC] = {3, 1, 2};
  SQLLEN        intInd[STMTS_TO_EXEC]= {5,4,3};

  SQLUSMALLINT  paramStatusArray[STMTS_TO_EXEC];
  SQLULEN       paramsProcessed, i;

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)STMTS_TO_EXEC, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray, 0));
  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
    0, 0, intField, 0, intInd));

  /* We don't expect errors in paramsets processing, thus we should get SQL_SUCCESS only*/
  FAIL_IF(SQLExecDirect(Stmt, "SELect ?,'So what'", SQL_NTS) != SQL_SUCCESS, "success expected");
  is_num(paramsProcessed, STMTS_TO_EXEC);

  for (i= 0; i < paramsProcessed; ++i)
  {
    if ( paramStatusArray[i] != SQL_PARAM_SUCCESS
      && paramStatusArray[i] != SQL_PARAM_SUCCESS_WITH_INFO )
    {
      diag("Parameter #%u status isn't successful(0x%X)", i+1, paramStatusArray[i]);
      return FAIL;
    }
  }

  // SELECT statement with several sets of parameters returns result only for the last set
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), intField[STMTS_TO_EXEC-1]);
  EXPECT_STMT(Stmt, SQLFetch(Stmt), SQL_NO_DATA);

  /* Clean-up */
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;

#undef STMTS_TO_EXEC
}


/*
  Bug #49029 - Server with sql mode NO_BACKSLASHES_ESCAPE obviously
  can work incorrectly (at least) with binary parameters
*/
ODBC_TEST(t_bug49029)
{
  const SQLCHAR bData[6]= "\x01\x80\x00\x80\x01";
  SQLCHAR buff[6];
  SQLULEN len= 5;

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "set @@session.sql_mode='NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION,NO_BACKSLASH_ESCAPES'", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,
    0, 0, (SQLPOINTER)bData, 0, &len));

  CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "select ?", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_BINARY, (SQLPOINTER)buff, 6, &len));

  FAIL_IF(memcmp((const void*) buff, (const void*)bData, 5)!=0, "comparison failed");

  return OK;
}

/*
  INSERT queries of various shape to test insert rewrite mechanisms
*/
ODBC_TEST(t_insert_rewrites)
{
#define PARAMSET_SIZE 5
  int i, ret;

  SQLINTEGER	c1[PARAMSET_SIZE] = {0, 1, 2, 3, 4};
  SQLLEN      d1[PARAMSET_SIZE] = {4, 4, 4, 4, 4};
  SQLINTEGER	c2[PARAMSET_SIZE] = {9, 8, 7, 6, 5};
  SQLLEN      d2[PARAMSET_SIZE] = {4, 4, 4, 4, 4};

  SQLLEN	    paramset_size	= PARAMSET_SIZE;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_insert_rewrites");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_insert_rewrites (c1 BIGINT PRIMARY KEY, c2 INT)");

  int overall_result = OK;
  SQLCHAR *queries_to_test[5] = {
    "insert into t_insert_rewrites values (?, ?)",
    "insert into t_insert_rewrites (c1, c2) values (?, ?)",
    "insert into t_insert_rewrites (c1, c2) values (?, ?) on duplicate key update c2 = -c2",
    "insert into t_insert_rewrites select * from t_insert_rewrites; insert into t_insert_rewrites values (?, ?)",
    "replace into t_insert_rewrites (c1, c2) values (?, ?)",
  };

  for (i = 0; i < 5; ++i)
  {
    CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, queries_to_test[i], SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_PARAMSET_SIZE,
      (SQLPOINTER)paramset_size, SQL_IS_UINTEGER ));

    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,
      SQL_DECIMAL, 4, 0, c1, 4, d1));
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,
      SQL_DECIMAL, 4, 0, c2, 4, d2));
    ret = SQLExecute(Stmt);
    if (ret != SQL_SUCCESS) {
      diag("Query failed: %s", queries_to_test[i]);
      odbc_print_error(SQL_HANDLE_STMT, Stmt);
      overall_result = FAIL;
    }

    OK_SIMPLE_STMT(Stmt, "SELECT COUNT(*) FROM t_insert_rewrites");
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(my_fetch_int(Stmt, 1), PARAMSET_SIZE);
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

    OK_SIMPLE_STMT(Stmt, "TRUNCATE TABLE t_insert_rewrites");
  }

  // OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_insert_rewrites");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return overall_result;
#undef PARAMSET_SIZE
}

/*
  multi statement query with array param
*/
ODBC_TEST(t_multi_statement_multi_param)
{
#define PARAMSET_SIZE		10

  SQLINTEGER	len 	= 1;
  int i;

  SQLINTEGER	b1[PARAMSET_SIZE]=      {14, 14, 14, 14, 14, 14, 14, 14, 14, 14};
  SQLINTEGER	c1[PARAMSET_SIZE]=      {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  SQLINTEGER	c2[PARAMSET_SIZE]=      {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  SQLLEN      d1[PARAMSET_SIZE]=      {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
  SQLLEN      d2[PARAMSET_SIZE]=      {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
  SQLUSMALLINT status[PARAMSET_SIZE]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  SQLUSMALLINT ExpectedStatus[PARAMSET_SIZE];

  SQLLEN	    paramset_size	= PARAMSET_SIZE;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS multi_param_1");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS multi_param_2");

  OK_SIMPLE_STMT(Stmt, "create table multi_param_1 (c1 bigint primary key, c2 int)");
  OK_SIMPLE_STMT(Stmt, "create table multi_param_2 (b1 int)");

  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *)"insert into multi_param_1 values (?, ?); insert into multi_param_2 values (?)", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr( Stmt, SQL_ATTR_PARAMSET_SIZE,
    (SQLPOINTER)paramset_size, SQL_IS_UINTEGER ));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr( Stmt, SQL_ATTR_PARAM_STATUS_PTR,
    status, SQL_IS_POINTER ));

  CHECK_STMT_RC(Stmt, SQLBindParameter( Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,
    SQL_DECIMAL, 4, 0, c1, 4, d1));

  CHECK_STMT_RC(Stmt, SQLBindParameter( Stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,
    SQL_DECIMAL, 4, 0, c2, 4, d2));

  CHECK_STMT_RC(Stmt, SQLBindParameter( Stmt, 3, SQL_PARAM_INPUT, SQL_C_SLONG,
    SQL_DECIMAL, 4, 0, b1, 4, d2));

  EXPECT_STMT(Stmt, SQLExecute(Stmt), SQL_SUCCESS);

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS multi_param_1");
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS multi_param_2");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
#undef PARAMSET_SIZE
}

/*
  Bug #56804 - Server with sql mode NO_BACKSLASHES_ESCAPE obviously
  can work incorrectly (at least) with binary parameters
*/
ODBC_TEST(t_bug56804)
{
#define PARAMSET_SIZE		10

  SQLINTEGER	len 	= 1;
  int i;

  SQLINTEGER	c1[PARAMSET_SIZE]=      {0, 1, 2, 3, 4, 5, 1, 7, 8, 9};
  SQLINTEGER	c2[PARAMSET_SIZE]=      {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  SQLLEN      d1[PARAMSET_SIZE]=      {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
  SQLLEN      d2[PARAMSET_SIZE]=      {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
  SQLUSMALLINT status[PARAMSET_SIZE]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  SQLUSMALLINT ExpectedStatus[PARAMSET_SIZE];

  SQLLEN	    paramset_size	= PARAMSET_SIZE;

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS bug56804");
  OK_SIMPLE_STMT(Stmt, "create table bug56804 (c1 int primary key not null, c2 int)");
  OK_SIMPLE_STMT(Stmt, "insert into bug56804 values( 1, 1 ), (9, 9009)");

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, (SQLCHAR *)"insert into bug56804 values( ?,? )", SQL_NTS));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr( Stmt, SQL_ATTR_PARAMSET_SIZE,
    (SQLPOINTER)paramset_size, SQL_IS_UINTEGER ));

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr( Stmt, SQL_ATTR_PARAM_STATUS_PTR,
    status, SQL_IS_POINTER ));

  CHECK_STMT_RC(Stmt, SQLBindParameter( Stmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,
    SQL_DECIMAL, 4, 0, c1, 4, d1));

  CHECK_STMT_RC(Stmt, SQLBindParameter( Stmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG,
    SQL_DECIMAL, 4, 0, c2, 4, d2));

  if (NoSsps)
  {
    EXPECT_STMT(Stmt, SQLExecute(Stmt), SQL_ERROR);
  }
  else
  {
    EXPECT_STMT(Stmt, SQLExecute(Stmt), SQL_SUCCESS_WITH_INFO);
  }

  if (NoSsps)
  {
    int i;
    for (i = 0; i < PARAMSET_SIZE; i++)
    {
      ExpectedStatus[i] = SQL_PARAM_ERROR;
    }
  }
  else
  /* all errors but last have SQL_PARAM_DIAG_UNAVAILABLE */
  {
    memset(ExpectedStatus, 0x00ff & SQL_PARAM_SUCCESS, sizeof(ExpectedStatus));
    ExpectedStatus[1] = ExpectedStatus[6]= SQL_PARAM_DIAG_UNAVAILABLE;
    ExpectedStatus[9]= SQL_PARAM_ERROR;
  }

  /* Following tests are here to ensure that driver works how it is currently
     expected to work, and they need to be changed if driver changes smth in the
     way how it reports errors in paramsets and diagnostics */
  for(i = 0; i < PARAMSET_SIZE; ++i )
  {
    diag("Paramset #%d (%d, %d)", i, c1[i], c2[i]);
    is_num(status[i], ExpectedStatus[i]);
  }

  {
    SQLCHAR     sqlstate[6]= {0};
    SQLCHAR     message[255]= {0};
    SQLINTEGER  native_err= 0;
    SQLSMALLINT msglen= 0;

    i= 0;
    while(SQL_SUCCEEDED(SQLGetDiagRec(SQL_HANDLE_STMT, Stmt, ++i, sqlstate,
      &native_err, message, sizeof(message), &msglen)))
    {
      diag("%d) [%s] %s %d", i, sqlstate, message, native_err);
    }

    /* just to make sure we got 1 diagnostics record ... */
    is_num(i, 2);
    /* ... and what the record is for the last error */
    // the error is "Duplicate entry '1' for key 'PRIMARY'" in CSPS mode
    // and "Duplicate entry '9' for key 'PRIMARY'" in SSPS mode
    FAIL_IF(strstr(message, "Duplicate entry ") == NULL, "comparison failed");
  }

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS bug56804");

  return OK;
#undef PARAMSET_SIZE
}


/*
  Bug 59772 - Column parameter binding makes SQLExecute not to return
  SQL_ERROR on disconnect
*/
ODBC_TEST(t_bug59772)
{
#define ROWS_TO_INSERT 3

    SQLRETURN rc;
    SQLCHAR   buf_kill[64], buf_get_node_id[128];

    SQLINTEGER    intField[ROWS_TO_INSERT] = {123321, 1, 0};
    SQLLEN        intInd[ROWS_TO_INSERT]= {5,4,3};

    SQLUSMALLINT  paramStatusArray[ROWS_TO_INSERT];
    SQLULEN       paramsProcessed, i;

    SQLINTEGER connection_id, node_id;

    SQLHENV henv2;
    SQLHDBC  hdbc2;
    SQLHSTMT hstmt2;

    int overall_result= OK;

    /* Create a new connection that we deliberately will kill */
    ODBC_Connect(&henv2, &hdbc2, &hstmt2);

    OK_SIMPLE_STMT(hstmt2, "SELECT connection_id()");
    CHECK_STMT_RC(hstmt2, SQLFetch(hstmt2));
    connection_id = my_fetch_int(hstmt2, 1);
    CHECK_STMT_RC(hstmt2, SQLFreeStmt(hstmt2, SQL_CLOSE));

    sprintf(buf_get_node_id, "SELECT node_id FROM INFORMATION_SCHEMA.MV_PROCESSLIST where id = %d", connection_id);
    OK_SIMPLE_STMT(hstmt2, buf_get_node_id);
    CHECK_STMT_RC(hstmt2, SQLFetch(hstmt2));
    node_id = my_fetch_int(hstmt2, 1);
    CHECK_STMT_RC(hstmt2, SQLFreeStmt(hstmt2, SQL_CLOSE));

    CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "DROP TABLE IF EXISTS t_bug59772", SQL_NTS));
    CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "CREATE TABLE t_bug59772 (id int primary key auto_increment,"\
      "intField int)", SQL_NTS));

    CHECK_STMT_RC(hstmt2, SQLSetStmtAttr(hstmt2, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0));
    CHECK_STMT_RC(hstmt2, SQLSetStmtAttr(hstmt2, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER)ROWS_TO_INSERT, 0));
    CHECK_STMT_RC(hstmt2, SQLSetStmtAttr(hstmt2, SQL_ATTR_PARAM_STATUS_PTR, paramStatusArray, 0));
    CHECK_STMT_RC(hstmt2, SQLSetStmtAttr(hstmt2, SQL_ATTR_PARAMS_PROCESSED_PTR, &paramsProcessed, 0));

    CHECK_STMT_RC(hstmt2, SQLPrepare(hstmt2, "INSERT INTO t_bug59772 (intField) VALUES (?)", SQL_NTS));

    CHECK_STMT_RC(hstmt2, SQLBindParameter(hstmt2, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER,
      0, 0, intField, 0, intInd));

    /* From another connection, kill the connection created above */
    sprintf(buf_kill, "KILL CONNECTION %d %d", connection_id, node_id);
    CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, (SQLCHAR *)buf_kill, SQL_NTS));

    FAIL_IF(SQLExecute(hstmt2) != SQL_ERROR, "Statement is expected to fail after connection is killed");

    SQLUSMALLINT expectedStatus;

    FAIL_IF(paramsProcessed != ROWS_TO_INSERT, "Got incorrect number of rows in SQL_ATTR_PARAMS_PROCESSED_PTR");
    for (i = 0; i < paramsProcessed; ++i)
    {
      if (NoSsps)
      {
        expectedStatus = SQL_PARAM_ERROR;
      }
      else
      {
        expectedStatus = (i + 1 < ROWS_TO_INSERT) ? SQL_PARAM_DIAG_UNAVAILABLE : SQL_PARAM_ERROR;
      }
      if (paramStatusArray[i] != expectedStatus)
      {
        diag("Parameter #%u status is wrong: expected %u, got %u", i + 1, expectedStatus, paramStatusArray[i]);
        overall_result = FAIL;
      }
    }

    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    CHECK_STMT_RC(Stmt, SQLExecDirect(Stmt, "DROP TABLE IF EXISTS t_bug59772", SQL_NTS));

    SQLFreeHandle(SQL_HANDLE_STMT, hstmt2);
    SQLDisconnect(hdbc2);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc2);
    SQLFreeHandle(SQL_HANDLE_ENV, henv2);

    return overall_result;
#undef ROWS_TO_INSERT
}

/* Test of inserting of previously fetched values, including NULL values, empty values etc
   Also covers ODBC-51 and 52(duplicate) */
ODBC_TEST(insert_fetched_null)
{
  SQLLEN      len[6];
  SQLWCHAR    val[50], empty[50];
  SQLINTEGER  id, mask;
  SQLDOUBLE   double_val;
  HSTMT       Stmt1;
  const char     *str= "Text val";
  const SQLWCHAR *wstr= CW(str);

  //TODO https://memsql.atlassian.net/jira/software/c/projects/PLAT/issues/PLAT-5348
  if (iOdbc()) return OK;

  CHECK_DBC_RC(Connection, SQLAllocHandle(SQL_HANDLE_STMT, Connection, &Stmt1));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_insert_fetched_null");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_insert_fetched_null (id int not null primary key, double_val double not null,\
                                                            val varchar(50) CHARACTER SET utf8 not null,\
                                                            nullable_val varchar(50) CHARACTER SET utf8, mask int unsigned not null,\
                                                            empty_val varchar(50) CHARACTER SET utf8)");

  OK_SIMPLE_STMT(Stmt, "SELECT 1, 0.001, 'Text val', NULL, 127, ''");

  memset(len, 0, sizeof(len));

  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 1, SQL_C_LONG, &id, 0, &len[0] ));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 2, SQL_C_DOUBLE, &double_val, 0, &len[1]));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 3, SQL_C_WCHAR, &val, sizeof(val), &len[2]));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 4, SQL_C_WCHAR, &val, sizeof(val), &len[3]));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 5, SQL_C_LONG, &mask, 0, &len[4]));
  CHECK_STMT_RC(Stmt, SQLBindCol(Stmt, 6, SQL_C_WCHAR, &empty, sizeof(empty), &len[5]));

  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  is_num(id, 1);
  IS_WSTR(val, wstr, len[2] / sizeof(SQLWCHAR))
  is_num(len[3], SQL_NULL_DATA);
  is_num(mask, 127);
  is_num(len[5], 0);
  IS_WSTR(empty, sqlwchar_empty, len[5]);

  CHECK_STMT_RC(Stmt1, SQLBindParameter(Stmt1, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &id, 0, &len[0]));
  CHECK_STMT_RC(Stmt1, SQLBindParameter(Stmt1, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &double_val, 0, &len[1]));
  CHECK_STMT_RC(Stmt1, SQLBindParameter(Stmt1, 3, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 0, 0, &val, sizeof(val), &len[2]));
  CHECK_STMT_RC(Stmt1, SQLBindParameter(Stmt1, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 0, 0, &val, sizeof(val), &len[3]));
  CHECK_STMT_RC(Stmt1, SQLBindParameter(Stmt1, 5, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &mask, 0, &len[4]));
  CHECK_STMT_RC(Stmt1, SQLBindParameter(Stmt1, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_VARCHAR, 0, 0, &empty, sizeof(empty), &len[5]));

  CHECK_STMT_RC(Stmt1, SQLPrepare(Stmt1, "INSERT INTO t_insert_fetched_null(id, double_val, val, nullable_val, mask, empty_val)\
                                          VALUES(?, ?, ?, ?, ?, ?)", SQL_NTS));
  CHECK_STMT_RC(Stmt1, SQLExecute(Stmt1));
  
  OK_SIMPLE_STMT(Stmt1, "SELECT id, double_val, val, nullable_val, mask, empty_val FROM t_insert_fetched_null");

  val[0]= 0;
  empty[0]= 'a';
  id= mask= 0;
  memset(len, 0, sizeof(len));

  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 1, SQL_C_LONG, &id, 0, &len[0]));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 2, SQL_C_DOUBLE, &double_val, 0, &len[1]));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 3, SQL_C_WCHAR, &val, 0, &len[2]));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 4, SQL_C_WCHAR, &val, sizeof(val), &len[3]));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 5, SQL_C_LONG, &mask, 0, &len[4]));
  CHECK_STMT_RC(Stmt1, SQLBindCol(Stmt1, 6, SQL_C_WCHAR, &empty, sizeof(empty), &len[5]));

  CHECK_STMT_RC(Stmt1, SQLFetch(Stmt1));

  is_num(id, 1);
  is_num(len[2], strlen(str)*sizeof(SQLWCHAR));
  is_num(len[3], SQL_NULL_DATA);
  is_num(mask, 127);
  is_num(len[5], 0);
  CHECK_STMT_RC(Stmt1, SQLGetData(Stmt1, 3, SQL_C_WCHAR, &val, len[2]+sizeof(SQLWCHAR), &len[2]));
  IS_WSTR(val, wstr, len[2]/sizeof(SQLWCHAR));

  /* Testing also that with SQLGetData everything is fine */
  len[3]= 0;
  len[5]= 1;

  CHECK_STMT_RC(Stmt1, SQLGetData(Stmt1, 4, SQL_C_WCHAR, &val, sizeof(val), &len[3]));
  /* val len should not be changed, indicator should be NULL */
  IS_WSTR(val, wstr, len[2]/sizeof(SQLWCHAR));
  is_num(len[3], SQL_NULL_DATA);

  CHECK_STMT_RC(Stmt1, SQLGetData(Stmt1, 6, SQL_C_WCHAR, &empty, sizeof(empty), &len[5]));
  is_num(len[5], 0);
  IS_WSTR(empty, sqlwchar_empty, len[5]);

  CHECK_STMT_RC(Stmt1, SQLFreeStmt(Stmt1, SQL_DROP));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE t_insert_fetched_null");

  return OK;
}


ODBC_TEST(odbc45)
{
  SQLSMALLINT i;
  SQLLEN      len= SQL_NTS;
  SQLCHAR     val[][4]=        {"\0",            "\1"};//, "4", "-1", "0.5", "z"},
  SQLWCHAR    valw[][4]=       { { '0', '\0' }, { '1', '\0' }, { '4', '\0' }, { '-', '1', '\0' }, { '0', '.', '5', '\0' }, { 'z', '\0' } };
  SQLRETURN   XpctdRc[]=       {SQL_SUCCESS,    SQL_SUCCESS, SQL_ERROR, SQL_ERROR, SQL_ERROR, SQL_ERROR};
  SQLCHAR     XpctdState[][6]= { "",            "",          "22003",   "22003",   "22001",   "22018" };
  SQLCHAR     XpctdValue[]=    { 0,             1 };// , 0, 1};

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS odbc45");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE odbc45 (id int not null primary key auto_increment, val bit(1) not null)");

  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, "INSERT INTO odbc45(val) VALUES(?)", SQL_NTS)); 
  for (i = 0; i<sizeof(val)/sizeof(val[0]); ++i)
  {
    diag("SQLCHAR binding #%d(%s)", i + 1, val[i]);
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_BIT, 0, 0, val[i], sizeof(val[0]), &len));
    EXPECT_STMT(Stmt, SQLExecute(Stmt), XpctdRc[i]);
    if (XpctdRc[i] != SQL_SUCCESS)
    {
      CHECK_SQLSTATE(Stmt, XpctdState[i]);
    }
  }

  for (i = 0; i < 0*sizeof(valw)/sizeof(valw[0]); ++i)
  {
    diag("SQLWCHAR binding #%d(%s)", i + 1, val[i]);
    CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_BIT, 0, 0, valw[i], sizeof(valw[0]), &len));
    EXPECT_STMT(Stmt, SQLExecute(Stmt), XpctdRc[i]);
    if (XpctdRc[i]!=SQL_SUCCESS)
    {
      CHECK_SQLSTATE(Stmt, XpctdState[i]);
    }
  }

  OK_SIMPLE_STMT(Stmt, "SELECT val FROM odbc45 ORDER BY id");

  for (i= 0; i<sizeof(XpctdValue); ++i)
  {
    CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
    is_num(my_fetch_int(Stmt, 1), XpctdValue[i]);
  }

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE odbc45");

  return OK;
}


ODBC_TEST(odbc151)
{
  SQLINTEGER  Val;
  SQLLEN      Len= 2, OctetLength= 0;
  SQLHANDLE   Apd;

  CHECK_STMT_RC(Stmt, SQLPrepare(Stmt, "SELECT ?", SQL_NTS));
  CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_APP_PARAM_DESC, &Apd, SQL_IS_POINTER, NULL));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &Val, Len, &Len));
  CHECK_DESC_RC(Apd, SQLGetDescField(Apd, 1, SQL_DESC_OCTET_LENGTH,   &OctetLength, SQL_IS_INTEGER, NULL));
  is_num(OctetLength, sizeof(SQLINTEGER));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &Val, Len, NULL));
  CHECK_DESC_RC(Apd, SQLGetDescField(Apd, 1, SQL_DESC_OCTET_LENGTH, &OctetLength, SQL_IS_INTEGER, NULL));
  is_num(OctetLength, sizeof(SQLINTEGER));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &Val, 0, &Len));
  CHECK_DESC_RC(Apd, SQLGetDescField(Apd, 1, SQL_DESC_OCTET_LENGTH, &OctetLength, SQL_IS_INTEGER, NULL));
  is_num(OctetLength, sizeof(SQLINTEGER));
 
  return OK;
}


ODBC_TEST(odbc182)
{
  char buffer[128];
  SQL_TIMESTAMP_STRUCT ts= {0/*year*/, 0, 3, 12/*hour*/, 34, 56, 777/*fractional*/};

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_odbc182");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_odbc182(col1 time)");

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TIME, 8, 3, &ts, 0, NULL));
  EXPECT_STMT(Stmt, SQLExecDirect(Stmt, "INSERT INTO t_odbc182 VALUES(?)", SQL_NTS), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "22008");

  ts.fraction= 0;
  ts.hour= 24;
  EXPECT_STMT(Stmt, SQLExecDirect(Stmt, "INSERT INTO t_odbc182 VALUES(?)", SQL_NTS), SQL_ERROR);
  CHECK_SQLSTATE(Stmt, "22007");

  ts.hour= 12;
  OK_SIMPLE_STMT(Stmt, "INSERT INTO t_odbc182 VALUES(?)");

  OK_SIMPLE_STMT(Stmt, "SELECT col1 FROM t_odbc182");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, buffer, 1), "12:34:56", 8);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE t_odbc182");

  return OK;
}


ODBC_TEST(odbc212)
{
  SQLSMALLINT a;
  SQLHDESC Ipd;
#pragma warning(disable: 4996)
#pragma warning(push)
  CHECK_STMT_RC(Stmt, SQLBindParam(Stmt, 1, SQL_C_SHORT, SQL_SMALLINT, 0, 0, &a, NULL));
#pragma warning(pop)
  CHECK_STMT_RC(Stmt, SQLGetStmtAttr(Stmt, SQL_ATTR_IMP_PARAM_DESC, &Ipd, SQL_IS_POINTER, NULL));
  CHECK_DESC_RC(Ipd, SQLGetDescField(Ipd, 1, SQL_DESC_PARAMETER_TYPE, &a, SQL_IS_SMALLINT, NULL));

  is_num(a, SQL_PARAM_INPUT);
  return OK;
}


ODBC_TEST(timestruct_param)
{
  SQL_TIMESTAMP_STRUCT ts= { 2020/*year*/, 4, 7, 1/*hour*/, 28, 56, 0/*fractional*/ };
  SQL_TIME_STRUCT tp= {15, 58, 33}, tr;

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_TIME, SQL_TYPE_TIME, 8, 0, &tp, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_TIME, SQL_TYPE_TIME, 8, 0, &tp, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_TIME, SQL_TYPE_TIME, 8, 0, &tp, 0, NULL));
  OK_SIMPLE_STMT(Stmt, "SELECT ? :> TIME, CAST('15:58:33' AS TIME) = ?, {t '15:58:33'} = ?");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_TIME, &tr, sizeof(SQL_TIME_STRUCT), NULL));
  is_num(tr.hour, 15);
  is_num(tr.minute, 58);
  is_num(tr.second, 33);
  is_num(my_fetch_int(Stmt, 2), 1);
  is_num(my_fetch_int(Stmt, 3), 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_RESET_PARAMS));

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 20, 0, &ts, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_TIME, SQL_TYPE_TIME, 8, 0, &tp, 0, NULL));

  OK_SIMPLE_STMT(Stmt, "SELECT 1 FROM DUAL WHERE ('2020-04-07 01:28:56' :> TIMESTAMP)=? AND CAST('15:58:33' AS TIME) = ?");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), 1);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}


/* Test of subsequent direct execution calls with decreasing number of parameters */
ODBC_TEST(consequent_direxec)
{
  SQLINTEGER p1= 1, p2= 2, p3= 3;

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &p1, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &p2, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 3, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &p3, 0, NULL));

  OK_SIMPLE_STMT(Stmt, "SELECT ?, ?, ?");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  is_num(my_fetch_int(Stmt, 1), 1);
  is_num(my_fetch_int(Stmt, 2), 2);
  is_num(my_fetch_int(Stmt, 3), 3);
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_RESET_PARAMS));

  p2= 7;
  p3= 5;
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &p2, 0, NULL));
  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &p3, 0, NULL));
  OK_SIMPLE_STMT(Stmt, "SELECT ?, ?");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));

  is_num(my_fetch_int(Stmt, 1), 7);
  is_num(my_fetch_int(Stmt, 2), 5);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));

  return OK;
}


ODBC_TEST(odbc279)
{
  char buffer[128];
  SQL_TIME_STRUCT ts= { 12/*hour*/, 34, 56 };

  CHECK_STMT_RC(Stmt, SQLSetStmtAttr(Stmt, SQL_ATTR_CURSOR_TYPE,
                                     (SQLPOINTER)SQL_CURSOR_STATIC, 0));

  OK_SIMPLE_STMT(Stmt, "DROP TABLE IF EXISTS t_odbc279");
  OK_SIMPLE_STMT(Stmt, "CREATE TABLE t_odbc279(col1 time)");

  CHECK_STMT_RC(Stmt, SQLBindParameter(Stmt, 1, SQL_PARAM_INPUT, SQL_C_TIME, SQL_TIME, 8, 0, &ts, 0, NULL));
  
  OK_SIMPLE_STMT(Stmt, "INSERT INTO t_odbc279 VALUES(?)");

  OK_SIMPLE_STMT(Stmt, "SELECT col1 FROM t_odbc279");
  CHECK_STMT_RC(Stmt, SQLFetch(Stmt));
  IS_STR(my_fetch_str(Stmt, buffer, 1), "12:34:56", 8);

  if (iOdbc())
  {
    CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
    OK_SIMPLE_STMT(Stmt, "SELECT col1 FROM t_odbc279");
  }
  CHECK_STMT_RC(Stmt, SQLFetchScroll(Stmt, SQL_FETCH_FIRST, 1));
  ts.hour= ts.minute= ts.second= 0;
  CHECK_STMT_RC(Stmt, SQLGetData(Stmt, 1, SQL_C_TIME, &ts, sizeof(SQL_TIME_STRUCT), NULL));

  is_num(ts.hour, 12);
  is_num(ts.minute, 34);
  is_num(ts.second, 56);

  CHECK_STMT_RC(Stmt, SQLFreeStmt(Stmt, SQL_CLOSE));
  OK_SIMPLE_STMT(Stmt, "DROP TABLE t_odbc279");

  return OK;
}


MA_ODBC_TESTS my_tests[]=
{
  {unbuffered_result, "unbuffered_result", NORMAL, ALL_DRIVERS},
  {my_init_table, "my_init_table", NORMAL, ALL_DRIVERS},
  {num_params, "num_params", NORMAL, ALL_DRIVERS},
  {my_param_insert, "my_param_insert", NORMAL, ALL_DRIVERS},
  {my_param_update, "my_param_update", NORMAL, ALL_DRIVERS},
  {my_param_delete, "my_param_delete", NORMAL, ALL_DRIVERS},
  {tmysql_fix, "tmysql_fix", NORMAL, ALL_DRIVERS},
  {t_param_offset, "t_param_offset", NORMAL, ALL_DRIVERS},
  {paramarray_by_row, "paramarray_by_row", NORMAL, ALL_DRIVERS},
  {paramarray_by_column, "paramarray_by_column", NORMAL, ALL_DRIVERS},
  {paramarray_ignore_paramset, "paramarray_ignore_paramset", NORMAL, ALL_DRIVERS},
  {paramarray_select, "paramarray_select", NORMAL, ALL_DRIVERS},
  {t_insert_rewrites, "t_insert_rewrites", NORMAL, ALL_DRIVERS},
  {t_multi_statement_multi_param, "t_multi_statement_multi_param", NORMAL, ALL_DRIVERS},
  {t_bug49029, "t_bug49029", NORMAL, ALL_DRIVERS},
  {t_bug56804, "t_bug56804", NORMAL, ALL_DRIVERS},
  // {t_bug59772, "t_bug59772", NORMAL, ALL_DRIVERS}, TODO: PLAT-6479 enable this test
  {insert_fetched_null, "insert_fetched_null", NORMAL, ALL_DRIVERS},
  {odbc45, "odbc-45-binding2bit", NORMAL, ALL_DRIVERS},
  {odbc151, "odbc-151-buffer_length", NORMAL, ALL_DRIVERS},
  {odbc182, "odbc-182-timestamp2time", NORMAL, ALL_DRIVERS},
  {odbc212, "odbc-212-sqlbindparam_inout_type", NORMAL, ALL_DRIVERS},
  {timestruct_param, "timestruct_param-seconds", NORMAL, ALL_DRIVERS},
  {consequent_direxec, "consequent_direxec", NORMAL, ALL_DRIVERS},
  {odbc279, "odbc-279-timestruct", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
