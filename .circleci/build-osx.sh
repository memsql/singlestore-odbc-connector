#!/bin/bash

set -eo pipefail

# set variables for Connector/ODBC
export OPENSSL_ROOT_DIR=(/usr/local/Cellar/openssl@1.1/1.1.1*)

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL -DWITH_IODBC=ON -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} -DOPENSSL_LIBRARIES=${OPENSSL_ROOT_DIR}/lib
cmake --build . --config ${BUILD_TYPE}