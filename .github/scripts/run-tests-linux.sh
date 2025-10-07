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

## Export password and port
if [ -n "$MEMSQL_PASSWORD" ]
then
  export TEST_PASSWORD=$MEMSQL_PASSWORD
fi

if [ -n "$MEMSQL_PORT" ]
then
  export TEST_PORT=$MEMSQL_PORT
fi

export MEMSQL_JWT="eyJhbGciOiJSUzM4NCIsInR5cCI6IkpXVCJ9."\
"eyJ1c2VybmFtZSI6InRlc3Rfand0X3VzZXIiLCJleHAiOjI1MTQzNTk5MjAsImVtYWlsIjoidGVzdEBzaW5nbGVzdG9yZS5jb20ifQ."\
"kzcqllfR9HIjJLG8ZxS8Ck_N1PUjqdPTdXOv_jRWJCwzdBP8x0kuOtvtx-XScJq2PN3x41I8BkC74T3eUt1dUo_PqhDGcgePNDKgdrqvyGxLZxNiyydt"\
"_zO75lj8frfxf1rDa57VCPOaqGDITJEHgcgU9gK42RhfR9tL5vdw8DfdIOWz17CSZFmiFwO_rhGRtlaR1rFl_LbBKXpSKyEuSBSd5ED1Ywx4KjU4Nh1u"\
"DivNvZ1PQFQNQ8_up_dCu4-kzcENuQcEKJWkzYuXyMJjtnTJVXRT5RMMFewzaKQ7WzOjdl_WtpyKY7Cjoms9dyxstJW6M0-m8c8ADlUn_e3mvyE7KqgS"\
"Xe8tYBMs8Ojlme7B2D5FFMZe80wwAcNv3T-vA0Ah8G9b1t0PvbHJ5NRScFyItkJb0akPmDsSh4p-CXM2rErJGI-i-PW3VFswiIwfkavSzn-ElOoZPuc-"\
"lp-ffRjnBJSNjTYF5wUwCQdaUcaklfEJJYhNHrRBQ4dTu9Jq2CbZztex6zfIpd2PRiPauaEcE0Di4nyJZjSSxQj0ao-us3523eC2XDXvIH1E7Y2I235h"\
"udKDUWVrYZJtxZU2Ci3ZuEdYteXJ3VoUBG7m_Ydsky2GUz7sNZWhdsaYD_Ghy66XsO-2cW-kX7GjvX28HOBWWzzPpzT_25W54cxsc-c"

echo "Running tests from $PWD"

cd build/test
export ODBCINI="$PWD/odbc.ini"
export ODBCSYSINI=$PWD

ctest -V
