#!/bin/bash

set -x
set -e

###################################################################################################################
# test different type of configuration
###################################################################################################################
mysql=( mysql --protocol=tcp -uroot -h127.0.0.1 --port=3306 )


#list ssl certificates
ls -lrt ${SSLCERT}


DEBIAN_FRONTEND=noninteractive sudo apt-get update
DEBIAN_FRONTEND=noninteractive sudo apt-get install --allow-unauthenticated -y --force-yes -m unixodbc-dev odbcinst1debian2 libodbc1 

##build odbc connector
#export TEST_DRIVER=singlestore_test
#export TEST_DSN=singlestore_test
#export TEST_SERVER=singlestore.example.com
#export TEST_SOCKET=
#export TEST_SCHEMA=odbc_test
#export TEST_UID=bob
#export TEST_PASSWORD=

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
# In Travis we are interested in tests with latest C/C version, while for release we must use only latest release tag
#git submodule update --remote
cmake --build . --config RelWithDebInfo 

###################################################################################################################
# run test suite
###################################################################################################################
echo "Running tests"

cd test
export ODBCINI="$PWD/odbc.ini"
export ODBCSYSINI=$PWD


ctest -V

