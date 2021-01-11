#!/bin/bash

set -eo pipefail

# set variables for Connector/ODBC
export TEST_DRIVER="$PWD/libmaodbc.dylib"
export OPENSSL_ROOT_DIR=(/usr/local/Cellar/openssl@1.1/1.1.1*)

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_SSL=OPENSSL -DWITH_IODBC=ON -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} -DOPENSSL_LIBRARIES=${OPENSSL_ROOT_DIR}/lib
cmake --build . --config ${BUILD_TYPE}

###################################################################################################################
# run test suite
###################################################################################################################

# set variables for odbc.ini and odbcinst.ini
export ODBCINI="$PWD/test/odbc.ini"
cat ${ODBCINI}
export ODBCINSTINI="$PWD/test/odbcinst.ini"
cat ${ODBCINSTINI}

echo "Running tests"
cd test
ctest -V
