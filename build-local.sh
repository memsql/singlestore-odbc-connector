#!/bin/bash
set -euo pipefail

# This branch links OpenSSL 1.1. Distro OpenSSL on current Linux images is 3.x,
# which CMake rejects. Build 1.1.1w when a usable prefix is not already present.
. .github/scripts/install-openssl-1.1.sh

mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_SSL=OPENSSL -DOPENSSL_ROOT_DIR="${OPENSSL_ROOT_DIR}" -DOPENSSL_INCLUDE_DIR="${OPENSSL_ROOT_DIR}/include" -DOPENSSL_SSL_LIBRARY="${OPENSSL_ROOT_DIR}/lib/libssl.so" -DOPENSSL_CRYPTO_LIBRARY="${OPENSSL_ROOT_DIR}/lib/libcrypto.so"
cmake --build . --config RelWithDebInfo
cd ..
