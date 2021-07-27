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

typedef struct function_support {
  SQLUSMALLINT function;
  SQLUSMALLINT is_supported;
} FUNCTION_SUPPORT;

FUNCTION_SUPPORT function_supports[] = {
  {SQL_API_SQLALLOCHANDLE, SQL_TRUE},
  {SQL_API_SQLBINDCOL, SQL_TRUE},
  {SQL_API_SQLCANCEL, SQL_TRUE},
  {SQL_API_SQLCLOSECURSOR, SQL_TRUE},
  {SQL_API_SQLCOLATTRIBUTE, SQL_TRUE},
  {SQL_API_SQLCONNECT, SQL_TRUE},
  {SQL_API_SQLCOPYDESC, SQL_TRUE},
  {SQL_API_SQLDATASOURCES, SQL_TRUE},
  {SQL_API_SQLDESCRIBECOL, SQL_TRUE},
  {SQL_API_SQLDISCONNECT, SQL_TRUE},
  {SQL_API_SQLDRIVERS, SQL_TRUE},
  {SQL_API_SQLENDTRAN, SQL_TRUE},
  {SQL_API_SQLEXECDIRECT, SQL_TRUE},
  {SQL_API_SQLEXECUTE, SQL_TRUE},
  {SQL_API_SQLFETCH, SQL_TRUE},
  {SQL_API_SQLFETCHSCROLL, SQL_TRUE},
  {SQL_API_SQLFREEHANDLE, SQL_TRUE},
  {SQL_API_SQLFREESTMT, SQL_TRUE},
  {SQL_API_SQLGETCONNECTATTR, SQL_TRUE},
  {SQL_API_SQLGETCURSORNAME, SQL_TRUE},
  {SQL_API_SQLGETDATA, SQL_TRUE},
  {SQL_API_SQLGETDESCFIELD, SQL_TRUE},
  {SQL_API_SQLGETDESCREC, SQL_TRUE},
  {SQL_API_SQLGETDIAGFIELD, SQL_TRUE},
  {SQL_API_SQLGETDIAGREC, SQL_TRUE},
  {SQL_API_SQLGETENVATTR, SQL_TRUE},
  {SQL_API_SQLGETFUNCTIONS, SQL_TRUE},
  {SQL_API_SQLGETINFO, SQL_TRUE},
  {SQL_API_SQLGETSTMTATTR, SQL_TRUE},
  {SQL_API_SQLGETTYPEINFO, SQL_TRUE},
  {SQL_API_SQLNUMRESULTCOLS, SQL_TRUE},
  {SQL_API_SQLPARAMDATA, SQL_TRUE},
  {SQL_API_SQLPREPARE, SQL_TRUE},
  {SQL_API_SQLPUTDATA, SQL_TRUE},
  {SQL_API_SQLROWCOUNT, SQL_TRUE},
  {SQL_API_SQLSETCONNECTATTR, SQL_TRUE},
  {SQL_API_SQLSETCURSORNAME, SQL_TRUE},
  {SQL_API_SQLSETDESCFIELD, SQL_TRUE},
  {SQL_API_SQLSETDESCREC, SQL_FALSE}, // TODO PLAT-5516
  {SQL_API_SQLSETENVATTR, SQL_TRUE},
  {SQL_API_SQLSETSTMTATTR, SQL_TRUE},

  {SQL_API_SQLCOLUMNS, SQL_TRUE},
  {SQL_API_SQLSPECIALCOLUMNS, SQL_TRUE},
  {SQL_API_SQLSTATISTICS, SQL_TRUE},
  {SQL_API_SQLTABLES, SQL_TRUE},

  {SQL_API_SQLBINDPARAMETER, SQL_TRUE},
  {SQL_API_SQLBROWSECONNECT, SQL_FALSE}, // TODO PLAT-5515
  {SQL_API_SQLBULKOPERATIONS, SQL_TRUE},
  {SQL_API_SQLCOLUMNPRIVILEGES, SQL_TRUE},
  {SQL_API_SQLDESCRIBEPARAM, SQL_TRUE},
  {SQL_API_SQLDRIVERCONNECT, SQL_TRUE},
  {SQL_API_SQLFOREIGNKEYS, SQL_FALSE},
  {SQL_API_SQLMORERESULTS	,SQL_TRUE},
  {SQL_API_SQLNATIVESQL, SQL_TRUE},
  {SQL_API_SQLNUMPARAMS, SQL_TRUE},
  {SQL_API_SQLPRIMARYKEYS, SQL_TRUE},
  {SQL_API_SQLPROCEDURECOLUMNS, SQL_TRUE},
  {SQL_API_SQLPROCEDURES, SQL_TRUE},
  {SQL_API_SQLSETPOS, SQL_TRUE},
  {SQL_API_SQLTABLEPRIVILEGES, SQL_TRUE},

  // {SQL_API_SQLCANCELHANDLE, SQL_FALSE}, TODO PLAT-5514
};

ODBC_TEST(sql_api_odbc3_all_functions)
{
  SQLUSMALLINT fExists[SQL_API_ODBC3_ALL_FUNCTIONS_SIZE];
  int i;

  CHECK_DBC_RC(Connection, SQLGetFunctions(Connection, SQL_API_ODBC3_ALL_FUNCTIONS, fExists));

  for (i = 0; i < sizeof(function_supports)/sizeof(FUNCTION_SUPPORT); i++)
  {
    IS(SQL_FUNC_EXISTS(fExists, function_supports[i].function) == function_supports[i].is_supported)
  }
  return OK;
}

#define SQL_API_ALL_FUNCTIONS_SIZE 100
ODBC_TEST(sql_api_all_functions)
{
  SQLUSMALLINT fExists[SQL_API_ALL_FUNCTIONS_SIZE];
  int i;

  CHECK_DBC_RC(Connection, SQLGetFunctions(Connection, SQL_API_ALL_FUNCTIONS, fExists));

  for (i = 0; i < sizeof(function_supports)/sizeof(FUNCTION_SUPPORT); i++)
  {
    if (function_supports[i].function < SQL_API_ALL_FUNCTIONS_SIZE)
    {
#ifdef WIN32
      // Windows DM calls SQLGetFunctions with SQL_API_ODBC3_ALL_FUNCTIONS at the beginning of the execution
      // Then it handles all cals to SQLGetFunctions by its own
      // For some reason, it treats SQL_API_SQLBULKOPERATIONS as unsupported
      if (function_supports[i].function == SQL_API_SQLBULKOPERATIONS)
      {
        continue;
      }
#endif
      IS(fExists[function_supports[i].function] == function_supports[i].is_supported);
    }
  }
  return OK;
}

ODBC_TEST(single_odbc_function)
{
  SQLUSMALLINT exists;
  int i;

  for (i = 0; i < sizeof(function_supports)/sizeof(FUNCTION_SUPPORT); i++)
  {
    if (iOdbc() && function_supports[i].is_supported == SQL_FALSE) {
        CHECK_DBC_ERR(Connection, SQLGetFunctions(Connection, function_supports[i].function, &exists), "HY095", 0, "Function type out of range");
    } else {
      CHECK_DBC_RC(Connection, SQLGetFunctions(Connection, function_supports[i].function, &exists));
      IS(exists == function_supports[i].is_supported)
    }
  }
  return OK;
}

MA_ODBC_TESTS my_tests[]=
{
  {sql_api_odbc3_all_functions, "sql_api_odbc3_all_functions", NORMAL, ALL_DRIVERS},
  {sql_api_all_functions, "sql_api_all_functions", NORMAL, ALL_DRIVERS},
  {single_odbc_function, "single_odbc_function", NORMAL, ALL_DRIVERS},
  {NULL, NULL, NORMAL, ALL_DRIVERS}
};

int main(int argc, char **argv)
{
  int tests= sizeof(my_tests)/sizeof(MA_ODBC_TESTS) - 1;
  get_options(argc, argv);
  plan(tests);
  return run_tests(my_tests);
}
