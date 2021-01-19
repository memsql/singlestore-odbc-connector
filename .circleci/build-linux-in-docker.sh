#!/bin/bash

set -eo pipefail

###################################################################################################################
# test different type of configuration
###################################################################################################################

export PROJ_PATH=`pwd`

## build odbc connector
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL
cmake --build . --config ${BUILD_TYPE}

## Export password
if [ -n "$MEMSQL_PASSWORD" ]
then
  export TEST_PASSWORD=$MEMSQL_PASSWORD
fi

###################################################################################################################
# run test suite
###################################################################################################################
echo "Running tests"

cd test
export ODBCINI="$PWD/odbc.ini"
export ODBCSYSINI=$PWD

ctest -V

