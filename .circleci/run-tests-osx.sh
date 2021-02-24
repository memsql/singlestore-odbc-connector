#!/bin/bash

set -eo pipefail

# set variables for odbc.ini and odbcinst.ini
export ODBCINI="$PWD/test/odbc.ini"
cat ${ODBCINI}
export ODBCINSTINI="$PWD/test/odbcinst.ini"
cat ${ODBCINSTINI}

echo "Running tests"
cd test
ctest -V
