#!/bin/bash

set -eo pipefail

export PROJ_PATH=`pwd`

## build odbc connector
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL
cmake --build . --config ${BUILD_TYPE}