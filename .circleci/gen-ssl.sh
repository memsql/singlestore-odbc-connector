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

set -e

log () {
  echo "$@" 1>&2
}

print_error () {
  echo "$@" 1>&2
  exit 1
}

print_usage () {
  print_error "Usage: gen-ssl-cert-key <fqdn> <output-dir>"
}

gen_cert_subject () {
  local fqdn="$1"
  [[ "${fqdn}" != "" ]] || print_error "FQDN cannot be blank"
  echo "/C=/ST=/O=/localityName=/CN=${fqdn}/organizationalUnitName=/emailAddress=/"
}

main () {
  local fqdn="$1"
  local sslDir="$2"
  [[ "${fqdn}" != "" ]] || print_usage
  [[ -d "${sslDir}" ]] || print_error "Directory does not exist: ${sslDir}"

  local caCertFile="${sslDir}/ca.crt"
  local caKeyFile="${sslDir}/ca.key"
  local certFile="${sslDir}/server.crt"
  local keyFile="${sslDir}/server.key"
  local csrFile=$(mktemp)
  local clientCertFile="${sslDir}/client.crt"
  local clientKeyFile="${sslDir}/client.key"
  local clientKeystoreFile="${sslDir}/client-keystore.jks"
  local fullClientKeystoreFile="${sslDir}/fullclient-keystore.jks"
  local tmpKeystoreFile=$(mktemp)
  local pcks12FullKeystoreFile="${sslDir}/fullclient-keystore.p12"
  local clientReqFile=$(mktemp)

  log "Generating CA key"
  openssl genrsa -out "${caKeyFile}" 2048

  log "Generating CA certificate"
  openssl req \
    -sha256 \
    -new \
    -x509 \
    -nodes \
    -days 3650 \
    -subj "$(gen_cert_subject ca.example.com)" \
    -key "${caKeyFile}" \
    -out "${caCertFile}"

  log "Generating private key"
  openssl genrsa -out "${keyFile}" 2048

  log "Generating certificate signing request"
  openssl req \
    -new \
    -batch \
    -sha256 \
    -subj "$(gen_cert_subject "$fqdn")" \
    -set_serial 01 \
    -key "${keyFile}" \
    -out "${csrFile}" \
    -nodes

  log "Generating X509 certificate"
  openssl x509 \
    -req \
    -sha256 \
    -set_serial 01 \
    -CA "${caCertFile}" \
    -CAkey "${caKeyFile}" \
    -days 3650 \
    -in "${csrFile}" \
    -out "${certFile}"

  log "Generating client certificate"
  openssl req \
    -batch \
    -newkey rsa:2048 \
    -days 3600 \
    -subj "$(gen_cert_subject "$fqdn")" \
    -nodes \
    -keyout "${clientKeyFile}" \
    -out "${clientReqFile}"

  openssl x509 \
    -req \
    -in "${clientReqFile}" \
    -days 3600 \
    -CA "${caCertFile}" \
    -CAkey "${caKeyFile}" \
    -set_serial 01 \
    -out "${clientCertFile}"

  # Now generate a keystore with the client cert & key
  log "Generating client keystore"
  openssl pkcs12 \
    -export \
    -in "${clientCertFile}" \
    -inkey "${clientKeyFile}" \
    -out "${tmpKeystoreFile}" \
    -name "mysqlAlias" \
    -passout pass:kspass


  # Now generate a full keystore with the client cert & key + trust certificates
  log "Generating full client keystore"
  openssl pkcs12 \
    -export \
    -in "${clientCertFile}" \
    -inkey "${clientKeyFile}" \
    -out "${pcks12FullKeystoreFile}" \
    -name "mysqlAlias" \
    -passout pass:kspass



  # Clean up CSR file:
  rm "$csrFile"
  rm "$clientReqFile"
  rm "$tmpKeystoreFile"

  log "Generated key file and certificate in: ${sslDir}"
  ls -l "${sslDir}"
}

main "$@"

