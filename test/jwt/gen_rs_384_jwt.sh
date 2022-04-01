#!/bin/bash
set -eo pipefail
# TODO: make this script configurable

PEM=$( cat ~/RS384.key )

NOW=$( date +%s )
EXP=$((${NOW} + 60*60*24))
EXP=2514359920

EMAIL="test@singlestore.com"
DB_USER="test_jwt_user"
# expire 9 minutes in the future. 10 minutes is the max for github
HEADER_RAW='{"alg":"RS384","typ":"JWT"}'
HEADER=$( echo -n "${HEADER_RAW}" | openssl base64 | tr -d '=' | tr '/+' '_-' | tr -d '\n' )
PAYLOAD_RAW='{"username":"'$DB_USER'","exp":'$EXP',"email":"'$EMAIL'"}'
echo $PAYLOAD_RAW
PAYLOAD=$( echo -n "${PAYLOAD_RAW}" | openssl base64 | tr -d '=' | tr '/+' '_-' | tr -d '\n' )
HEADER_PAYLOAD="${HEADER}"."${PAYLOAD}"
SIGNATURE=$( openssl dgst -sha384 -sign <(echo -n "${PEM}") <(echo -n "${HEADER_PAYLOAD}") | openssl base64 | tr -d '=' | tr '/+' '_-' | tr -d '\n' )
JWT="${HEADER_PAYLOAD}"."${SIGNATURE}"

echo $JWT
