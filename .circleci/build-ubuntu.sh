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

echo 127.0.0.1 singlestore.test.com | sudo tee -a /etc/hosts
export PROJ_PATH=`pwd`
mkdir tmp
.circleci/gen-ssl.sh singlestore.test.com tmp
export SSLCERT=$PROJ_PATH/tmp

# list ssl certificates
ls -lrt ${SSLCERT}

DEBIAN_FRONTEND=noninteractive sudo apt-get update
DEBIAN_FRONTEND=noninteractive sudo apt-get install --allow-unauthenticated -y --force-yes -m unixodbc-dev odbcinst1debian2 libodbc1 

## build odbc connector
if [ "$WITH_SANITIZER" = "true" ]
then
  cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_SANITIZER=ON -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL
else
  cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL
fi
cmake --build . --config ${BUILD_TYPE}