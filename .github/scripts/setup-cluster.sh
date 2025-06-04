#!/usr/bin/env bash
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

set -eu

# this script must be run from the top-level of the repo
cd "$(git rev-parse --show-toplevel)"

DEFAULT_SINGLESTORE_VERSION="8.9"
VERSION="${SINGLESTORE_VERSION:-$DEFAULT_SINGLESTORE_VERSION}"
IMAGE_NAME="ghcr.io/singlestore-labs/singlestoredb-dev:latest"
CONTAINER_NAME="singlestore-integration"

S2_MASTER_PORT="${MEMSQL_PORT:-5506}"
S2_AGG_PORT_1=5507
S2_AGG_PORT_2=5508

EXISTS=$(docker inspect ${CONTAINER_NAME} >/dev/null 2>&1 && echo 1 || echo 0)

if [[ "${EXISTS}" -eq 1 ]]; then
  EXISTING_IMAGE_NAME=$(docker inspect -f '{{.Config.Image}}' ${CONTAINER_NAME})
  if [[ "${IMAGE_NAME}" != "${EXISTING_IMAGE_NAME}" ]]; then
    echo "Existing container ${CONTAINER_NAME} has image ${EXISTING_IMAGE_NAME} when ${IMAGE_NAME} is expected; recreating container."
    docker rm -f ${CONTAINER_NAME}
    EXISTS=0
  fi
fi

if [[ "${EXISTS}" -eq 0 ]]; then
    docker run -d \
        --name ${CONTAINER_NAME} \
        -v ${PWD}/test/ssl:/test-ssl \
        -v ${PWD}/test/jwt:/test-jwt \
        -e SINGLESTORE_LICENSE=${SINGLESTORE_LICENSE} \
        -e ROOT_PASSWORD=${ROOT_PASSWORD} \
        -e SINGLESTORE_VERSION=${VERSION} \
        -p 5506:3306 -p 5507:3307 -p 5508:3308 \
        ${IMAGE_NAME}
fi

docker start ${CONTAINER_NAME}

singlestore-wait-start() {
  echo -n "Waiting for SingleStore to start..."
  while true; do
      if mysql -u root -h 127.0.0.1 -P 5506 -p"${ROOT_PASSWORD}" -e "select 1" >/dev/null 2>/dev/null; then
          break
      fi
      echo -n "."
      sleep 0.2
  done
  echo ". Success!"
}

singlestore-wait-start

if [[ "${EXISTS}" -eq 0 ]]; then
    echo
    echo "Creating aggregator nodes"
    docker exec -it ${CONTAINER_NAME} memsqlctl create-node --yes --password ${ROOT_PASSWORD} --port 3308
    docker exec -it ${CONTAINER_NAME} memsqlctl update-config --yes --all --key minimum_core_count --value 0
    docker exec -it ${CONTAINER_NAME} memsqlctl update-config --yes --all --key minimum_memory_mb --value 0
    docker exec -it ${CONTAINER_NAME} memsqlctl start-node --yes --all
    docker exec -it ${CONTAINER_NAME} memsqlctl add-aggregator --yes --host 127.0.0.1 --password ${ROOT_PASSWORD} --port 3308
fi

echo
echo "Setting up JWT"
docker exec -it ${CONTAINER_NAME} memsqlctl update-config --yes --all --key jwt_auth_config_file --value /test-jwt/jwt_auth_config.json
echo "Setting up SSL"
docker exec -it ${CONTAINER_NAME} memsqlctl update-config --yes --all --key ssl_ca --value /test-ssl/test-ca-cert.pem
docker exec -it ${CONTAINER_NAME} memsqlctl update-config --yes --all --key ssl_cert --value /test-ssl/test-memsql-cert.pem
docker exec -it ${CONTAINER_NAME} memsqlctl update-config --yes --all --key ssl_key --value /test-ssl/test-memsql-key.pem

echo "Restarting cluster"
docker exec -it ${CONTAINER_NAME} memsqlctl restart-node --yes --all
singlestore-wait-start

echo "Setting up root-ssl user"
mysql -u root -h 127.0.0.1 -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" -e 'create user "root-ssl"@"%" require ssl'
mysql -u root -h 127.0.0.1 -P $S2_AGG_PORT_1  -p"${ROOT_PASSWORD}" -e 'create user "root-ssl"@"%" require ssl'

mysql -u root -h 127.0.0.1 -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" -e 'grant all privileges on *.* to "root-ssl"@"%" with grant option'
mysql -u root -h 127.0.0.1 -P $S2_AGG_PORT_1  -p"${ROOT_PASSWORD}" -e 'grant all privileges on *.* to "root-ssl"@"%" with grant option'
echo "Done!"

echo
echo "Ensuring child nodes are connected using container IP"
CONTAINER_IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' ${CONTAINER_NAME})
CURRENT_LEAF_IP=$(mysql -u root -h 127.0.0.1 -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" --batch -N -e 'select host from information_schema.leaves')
if [[ ${CONTAINER_IP} != "${CURRENT_LEAF_IP}" ]]; then
    # remove leaf with current ip
    mysql -u root -h 127.0.0.1 -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" --batch -N -e "remove leaf '${CURRENT_LEAF_IP}':3307"
    # add leaf with correct ip
    mysql -u root -h 127.0.0.1 -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" --batch -N -e "add leaf root:'${ROOT_PASSWORD}'@'${CONTAINER_IP}':3307"
fi
CURRENT_AGG_IP=$(mysql -u root -h 127.0.0.1 -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" --batch -N -e 'select host from information_schema.aggregators where master_aggregator=0')
if [[ ${CONTAINER_IP} != "${CURRENT_AGG_IP}" ]]; then
    # remove aggregator with current ip
    mysql -u root -h 127.0.0.1 -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" --batch -N -e "remove aggregator '${CURRENT_AGG_IP}':3308"
    # add aggregator with correct ip
    mysql -u root -h 127.0.0.1 -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" --batch -N -e "add aggregator root:'${ROOT_PASSWORD}'@'${CONTAINER_IP}':3308"
fi
echo "Done!"

echo "Preparing database and jwt user..."
mysql -h 127.0.0.1 -u root -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" -e "CREATE DATABASE IF NOT EXISTS odbc_test"
mysql -h 127.0.0.1 -u root -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" -e "SET GLOBAL data_conversion_compatibility_level = '6.0'"
mysql -h 127.0.0.1 -u root -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" -e "CREATE USER 'test_jwt_user' IDENTIFIED WITH authentication_jwt"
mysql -h 127.0.0.1 -u root -P $S2_MASTER_PORT -p"${ROOT_PASSWORD}" -e "GRANT ALL PRIVILEGES ON odbc_test.* TO 'test_jwt_user'@'%'"
echo "Done!"
