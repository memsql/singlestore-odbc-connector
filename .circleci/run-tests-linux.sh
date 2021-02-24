#!/bin/bash

set -eo pipefail

## Export password
if [ -n "$MEMSQL_PASSWORD" ]
then
  export TEST_PASSWORD=$MEMSQL_PASSWORD
fi

echo "Running tests"

cd test
export ODBCINI="$PWD/odbc.ini"
export ODBCSYSINI=$PWD

ctest -V