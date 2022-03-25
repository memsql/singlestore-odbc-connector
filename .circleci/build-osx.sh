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

# set variables for Connector/ODBC
export OPENSSL_ROOT_DIR=(/usr/local/Cellar/openssl@1.1/1.1.1*)

cd libmariadb
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_SSL=OPENSSL -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} && cmake --build . --config DEBUG
cd ..

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DWITH_OPENSSL=ON -DWITH_SSL=OPENSSL -DWITH_IODBC=ON -DOPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR} -DOPENSSL_LIBRARIES=${OPENSSL_ROOT_DIR}/lib -Wno-pointer-sign
cmake --build . --config ${BUILD_TYPE}
