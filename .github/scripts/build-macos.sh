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

# Use the runner / Xcode Apple Clang. Avoid brew install llvm@*: on Intel that
# pulls python from source (~10m+) and Homebrew has dropped Intel bottles.
if ! command -v clang >/dev/null 2>&1; then
  echo "clang not found on PATH; install Xcode CLT or set CC/CXX" >&2
  exit 1
fi
export CC="${CC:-$(command -v clang)}"
export CXX="${CXX:-$(command -v clang++)}"

# FindOpenSSL does not search Homebrew kegs on its own (see build-local.sh).
if [ -z "${OPENSSL_ROOT_DIR:-}" ]; then
  for candidate in "$(brew --prefix openssl@3 2>/dev/null)" /usr/local/opt/openssl@3 /opt/homebrew/opt/openssl@3; do
    if [ -n "$candidate" ] && [ -f "$candidate/include/openssl/opensslv.h" ]; then
      OPENSSL_ROOT_DIR="$candidate"
      break
    fi
  done
fi
if [ -z "${OPENSSL_ROOT_DIR:-}" ]; then
  echo "OpenSSL 3 not found; install openssl@3 (brew) or set OPENSSL_ROOT_DIR" >&2
  exit 1
fi
export OPENSSL_ROOT_DIR

# Test connection details are supplied at test time via env (run-tests-macos.sh).
# Do not require WORKSPACE_ENDPOINT_FILE here — the S2MS cluster is started after
# the build so the expiry window is not burned by brew/compile.
export BUILD_TYPE="${BUILD_TYPE:-RelWithDebInfo}"

# Kill the log spam that comes from Apple SDK headers / ld, not our code:
# nullability (~50k), visionos availability (~3k), typedef redefs, dup libs.
MACOS_C_FLAGS="-Wno-pointer-sign -Wno-nullability-completeness -Wno-availability -Wno-typedef-redefinition -Wno-implicit-function-declaration"
MACOS_LD_FLAGS="-Wl,-no_warn_duplicate_libraries"

cd libmariadb
cmake -S . \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DWITH_SSL=OPENSSL \
  -DOPENSSL_ROOT_DIR="${OPENSSL_ROOT_DIR}" \
  -DCMAKE_C_FLAGS="${MACOS_C_FLAGS}" \
  -DCMAKE_EXE_LINKER_FLAGS="${MACOS_LD_FLAGS}" \
  -DCMAKE_SHARED_LINKER_FLAGS="${MACOS_LD_FLAGS}" \
  -DCMAKE_MODULE_LINKER_FLAGS="${MACOS_LD_FLAGS}"
cmake --build . --config ${BUILD_TYPE}
cd ..

cmake -S . \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DWITH_OPENSSL=ON \
  -DWITH_SSL=OPENSSL \
  -DWITH_IODBC=ON \
  -DIS_ON_S2MS=1 \
  -DOPENSSL_ROOT_DIR="${OPENSSL_ROOT_DIR}" \
  -DCMAKE_C_FLAGS="${MACOS_C_FLAGS}" \
  -DCMAKE_EXE_LINKER_FLAGS="${MACOS_LD_FLAGS}" \
  -DCMAKE_SHARED_LINKER_FLAGS="${MACOS_LD_FLAGS}" \
  -DCMAKE_MODULE_LINKER_FLAGS="${MACOS_LD_FLAGS}"
cmake --build . --config ${BUILD_TYPE}
