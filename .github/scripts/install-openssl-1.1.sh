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
#
# Source from a Linux CI build script. Builds OpenSSL 1.1.1w when the prefix
# is missing and exports OPENSSL_ROOT_DIR, LD_LIBRARY_PATH, and pkg-config
# paths so CMake links libssl.so.1.1 instead of the distro OpenSSL 3.

_ss_build_openssl_11() {
  local prefix="$1"
  local tmp src
  tmp="$(mktemp -d)"
  src="${tmp}/openssl-1.1.1w"
  if ! curl -fsSL -o "${tmp}/openssl.tar.gz" \
      "https://github.com/openssl/openssl/releases/download/OpenSSL_1_1_1w/openssl-1.1.1w.tar.gz"; then
    curl -fsSL -o "${tmp}/openssl.tar.gz" \
      "https://www.openssl.org/source/openssl-1.1.1w.tar.gz"
  fi
  tar -xzf "${tmp}/openssl.tar.gz" -C "${tmp}"
  (
    cd "${src}"
    # gcc 14 treats implicit declarations as errors; 1.1.1w still hits that.
    export CFLAGS="${CFLAGS:-} -Wno-error=implicit-function-declaration"
    ./config --prefix="${prefix}" --openssldir="${prefix}" --libdir=lib shared -fPIC
    make -j"$(nproc)"
    make install_sw
  )
  rm -rf "${tmp}"
}

_ss_repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
if [ -z "${OPENSSL_ROOT_DIR:-}" ] || [ ! -e "${OPENSSL_ROOT_DIR}/lib/libssl.so.1.1" ]; then
  OPENSSL_ROOT_DIR="${_ss_repo_root}/openssl-1.1"
  if [ ! -e "${OPENSSL_ROOT_DIR}/lib/libssl.so.1.1" ]; then
    echo "Building OpenSSL 1.1.1w into ${OPENSSL_ROOT_DIR}" >&2
    _ss_build_openssl_11 "${OPENSSL_ROOT_DIR}"
  fi
fi
export OPENSSL_ROOT_DIR

_ss_libdir="${OPENSSL_ROOT_DIR}/lib"
if [ ! -e "${_ss_libdir}/libssl.so.1.1" ] && [ -e "${OPENSSL_ROOT_DIR}/lib64/libssl.so.1.1" ]; then
  _ss_libdir="${OPENSSL_ROOT_DIR}/lib64"
fi
export LD_LIBRARY_PATH="${_ss_libdir}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export PKG_CONFIG_LIBDIR="${_ss_libdir}/pkgconfig"
export PKG_CONFIG_PATH="${_ss_libdir}/pkgconfig"

if [ -n "${GITHUB_ENV:-}" ]; then
  {
    echo "OPENSSL_ROOT_DIR=${OPENSSL_ROOT_DIR}"
    echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
    echo "PKG_CONFIG_LIBDIR=${PKG_CONFIG_LIBDIR}"
    echo "PKG_CONFIG_PATH=${PKG_CONFIG_PATH}"
  } >> "${GITHUB_ENV}"
fi

unset _ss_repo_root _ss_libdir
unset -f _ss_build_openssl_11
