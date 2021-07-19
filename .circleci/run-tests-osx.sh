#!/bin/bash
# ************************************************************************************
#   Copyright (c) 2021 SingleStore, Inc.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public
#   License along with this library; if not see <http://www.gnu.org/licenses>
#   or write to the Free Software Foundation, Inc.,
#   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
# *************************************************************************************/

set -eo pipefail

# set variables for odbc.ini and odbcinst.ini
export ODBCINI="$PWD/test/odbc.ini"
cat ${ODBCINI}
export ODBCINSTINI="$PWD/test/odbcinst.ini"
cat ${ODBCINSTINI}

echo "Modifying /etc/hosts and ~/my.cnf to enable connect tests"
echo "${TEST_SERVER} test-memsql-server" | sudo tee -a /etc/hosts
echo "${TEST_SERVER} test-memsql-cluster" | sudo tee -a /etc/hosts
echo "${TEST_SERVER} singlestore.test.com" | sudo tee -a /etc/hosts
echo "[mysqld]
plugin-load-add=authentication_pam.so

[client]
protocol = TCP

[odbc]
database = odbc_test_mycnf
" | sudo tee -a ~/.my.cnf

echo "Running tests"
cd test
ctest -V
