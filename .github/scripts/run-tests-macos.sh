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

export TEST_SERVER="$(cat WORKSPACE_ENDPOINT_FILE)"
export TEST_UID="${MEMSQL_USER}"
export TEST_PORT="${MEMSQL_PORT}"
export TEST_PASSWORD="${MEMSQL_PASSWORD}"

# odbc.ini is generated at build time (before the S2MS endpoint exists) with
# cmake defaults (SERVER 127.0.0.1, PORT 5506, UID root, empty PASSWORD).
# SQLConnect and DSN connection strings that omit those keys use the file, so
# rewrite them to the workspace values. Runtime env covers tests that put
# PORT/UID/PWD in the connect string themselves.
export ODBCINI="$PWD/test/odbc.ini"
if [ -f "${ODBCINI}" ]; then
  awk '
    BEGIN {
      server = ENVIRON["TEST_SERVER"]
      port = ENVIRON["TEST_PORT"]
      uid = ENVIRON["TEST_UID"]
      password = ENVIRON["TEST_PASSWORD"]
    }
    /^SERVER[[:space:]]*=/   { print "SERVER      = " server; next }
    /^PORT[[:space:]]*=/     { print "PORT        = " port; next }
    /^UID[[:space:]]*=/      { print "UID         = " uid; next }
    /^PASSWORD[[:space:]]*=/ { print "PASSWORD    = " password; next }
    { print }
  ' "${ODBCINI}" > "${ODBCINI}.tmp"
  mv "${ODBCINI}.tmp" "${ODBCINI}"
fi
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
