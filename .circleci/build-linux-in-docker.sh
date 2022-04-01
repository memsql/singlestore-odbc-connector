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

echo "Modifying /etc/hosts and ~/my.cnf to enable connect tests"
echo "$(dig ${MEMSQL_HOST} +short) singlestore.test.com" | tee -a /etc/hosts
echo "$(dig ${MEMSQL_HOST} +short) test-memsql-server" | tee -a /etc/hosts
echo "$(dig ${MEMSQL_HOST} +short) test-memsql-cluster" | tee -a /etc/hosts
echo "[mysqld]
plugin-load-add=authentication_pam.so

[client]
protocol = TCP

[odbc]
database = odbc_test_mycnf
" | tee ~/.my.cnf

export PROJ_PATH=`pwd`

if [ "$WITH_SANITIZER" = "true" ]
then
  SANITIZER_OPTION="ON"
else
  SANITIZER_OPTION="OFF"
fi

## build odbc connector
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL -DWITH_SANITIZER=$SANITIZER_OPTION

cmake --build . --config ${BUILD_TYPE}
