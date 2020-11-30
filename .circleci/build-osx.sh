#!/bin/bash

set -eo pipefail

# set variables for Connector/ODBC
export TEST_DRIVER="$PWD/libmaodbc.dylib"

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL -DWITH_IODBC=ON -DOPENSSL_ROOT_DIR=/usr/local/Cellar/openssl@1.1/1.1.1h -DOPENSSL_LIBRARIES=/usr/local/Cellar/openssl@1.1/1.1.1h/lib
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
