#!/bin/bash

set -x
set -e

###################################################################################################################
# test different type of configuration
###################################################################################################################

echo 127.0.0.1 singlestore.example.com | sudo tee -a /etc/hosts
export MEMSQL_HOST=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' memsql-integration)
export PROJ_PATH=`pwd`
mkdir tmp
.circleci/gen-ssl.sh singlestore.example.com tmp
export SSLCERT=$PROJ_PATH/tmp

#list ssl certificates
ls -lrt ${SSLCERT}

DEBIAN_FRONTEND=noninteractive sudo apt-get update
DEBIAN_FRONTEND=noninteractive sudo apt-get install --allow-unauthenticated -y --force-yes -m unixodbc-dev odbcinst1debian2 libodbc1 

##build odbc connector
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL
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

