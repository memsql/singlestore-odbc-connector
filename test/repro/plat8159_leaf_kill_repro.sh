#!/usr/bin/env bash
# ************************************************************************************
#   Copyright (c) 2026 SingleStore, Inc.
#
#   PLAT-8159 leaf-kill repro
#
#   Sets up a SingleStore cluster from ghcr.io/singlestore-labs/singlestoredb-dev,
#   opens a multi-statement ODBC transaction, stops the leaf node, then runs
#   CREATE TEMPORARY TABLE ... AS SELECT and reports whether the ODBC driver
#   received an error or hung waiting for a protocol response.
#
#   Usage (from repo root):
#     export MEMSQL_PASSWORD=...          # required (ROOT_PASSWORD for the image)
#     export SINGLESTORE_LICENSE=...      # optional; free tier works for small hosts
#     ./test/repro/plat8159_leaf_kill_repro.sh
#
#   Options:
#     --query-timeout N   Set SQL_ATTR_QUERY_TIMEOUT on the victim statement
#     --wait-secs N       Max seconds to wait for ODBC after leaf kill (default 60)
#     --rows N            Seed row count for CTAS source table (default 5000)
#     --keep-cluster      Do not remove the docker container on exit
#     --reuse-cluster     Reuse an already-running container if present
#     --skip-build        Do not rebuild the ODBC helper (use existing binary)
#     --driver PATH       Path to libssodbca.so (default: build/libssodbca.so)
#
#   Exit codes:
#     0  ODBC received SQL_ERROR after leaf kill (error was delivered)
#     1  ODBC succeeded or hung (error was NOT cleanly delivered)
#     2  setup failure
# ************************************************************************************

set -euo pipefail

ROOT="$(git -C "$(dirname "$0")/../.." rev-parse --show-toplevel 2>/dev/null \
  || realpath "$(dirname "$0")/../..")"
cd "$ROOT"

IMAGE_NAME="${IMAGE_NAME:-ghcr.io/singlestore-labs/singlestoredb-dev:latest}"
CONTAINER_NAME="${CONTAINER_NAME:-singlestore-plat8159}"
HOST_PORT="${HOST_PORT:-5606}"
SINGLESTORE_VERSION="${SINGLESTORE_VERSION:-}"
MEMSQL_PASSWORD="${MEMSQL_PASSWORD:-${ROOT_PASSWORD:-}}"
QUERY_TIMEOUT=0
WAIT_SECS=60
ROWS=5000
KEEP_CLUSTER=0
REUSE_CLUSTER=0
SKIP_BUILD=0
DRIVER_SO="${DRIVER_SO:-$ROOT/build/libssodbca.so}"
HELPER_BIN="$ROOT/test/repro/plat8159_leaf_kill_txn"
SYNC_DIR=""
CLIENT_PID=""
ODBC_OUT=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --query-timeout) QUERY_TIMEOUT="$2"; shift 2 ;;
    --wait-secs) WAIT_SECS="$2"; shift 2 ;;
    --rows) ROWS="$2"; shift 2 ;;
    --keep-cluster) KEEP_CLUSTER=1; shift ;;
    --reuse-cluster) REUSE_CLUSTER=1; shift ;;
    --skip-build) SKIP_BUILD=1; shift ;;
    --driver) DRIVER_SO="$2"; shift 2 ;;
    --help|-h)
      sed -n '2,40p' "$0"
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      exit 2
      ;;
  esac
done

die() { echo "ERROR: $*" >&2; exit 2; }
log() { echo "[plat8159] $*"; }

need_cmd() {
  command -v "$1" >/dev/null 2>&1 || die "required command not found: $1"
}

cleanup() {
  local ec=$?
  if [[ -n "${CLIENT_PID}" ]] && kill -0 "$CLIENT_PID" 2>/dev/null; then
    log "Stopping hung ODBC client pid=$CLIENT_PID"
    kill "$CLIENT_PID" 2>/dev/null || true
    wait "$CLIENT_PID" 2>/dev/null || true
  fi
  if [[ -n "${SYNC_DIR}" && -d "${SYNC_DIR}" ]]; then
    rm -rf "${SYNC_DIR}"
  fi
  if [[ "${KEEP_CLUSTER}" -eq 0 ]]; then
    if command -v docker >/dev/null 2>&1 \
         && docker inspect "${CONTAINER_NAME}" >/dev/null 2>&1; then
      log "Removing container ${CONTAINER_NAME}"
      docker rm -f "${CONTAINER_NAME}" >/dev/null 2>&1 || true
    fi
  else
    log "Leaving container ${CONTAINER_NAME} running (--keep-cluster)"
  fi
  exit "$ec"
}
trap cleanup EXIT

need_cmd docker
need_cmd gcc
if ! command -v mysql >/dev/null 2>&1 && ! command -v mariadb >/dev/null 2>&1; then
  die "required command not found: mysql (or mariadb)"
fi

if [[ -z "${MEMSQL_PASSWORD}" ]]; then
  die "Set MEMSQL_PASSWORD (or ROOT_PASSWORD) for the SingleStore container"
fi

MYSQL_CLI="$(command -v mysql || command -v mariadb)"
mysql_ma() {
  "$MYSQL_CLI" -N -B -u root -h 127.0.0.1 -P "${HOST_PORT}" \
    -p"${MEMSQL_PASSWORD}" --protocol=TCP "$@"
}

build_helper() {
  [[ -f "${DRIVER_SO}" ]] || die "ODBC driver not found at ${DRIVER_SO} (build the connector first)"
  if [[ "${SKIP_BUILD}" -eq 1 && -x "${HELPER_BIN}" ]]; then
    log "Reusing existing helper ${HELPER_BIN}"
    return
  fi
  log "Building ${HELPER_BIN}"
  gcc -O2 -Wall -Wextra -o "${HELPER_BIN}" \
    "$ROOT/test/repro/plat8159_leaf_kill_txn.c" \
    -lodbc -ldl -lpthread
}

start_cluster() {
  local exists=0
  if docker inspect "${CONTAINER_NAME}" >/dev/null 2>&1; then
    exists=1
  fi

  if [[ "${exists}" -eq 1 && "${REUSE_CLUSTER}" -eq 1 ]]; then
    if [[ "$(docker inspect -f '{{.State.Running}}' "${CONTAINER_NAME}")" != "true" ]]; then
      log "Starting existing container ${CONTAINER_NAME}"
      docker start "${CONTAINER_NAME}" >/dev/null
    else
      log "Reusing running container ${CONTAINER_NAME}"
    fi
  else
    if [[ "${exists}" -eq 1 ]]; then
      log "Removing previous container ${CONTAINER_NAME}"
      docker rm -f "${CONTAINER_NAME}" >/dev/null
    fi
    log "Starting ${IMAGE_NAME} as ${CONTAINER_NAME} (host port ${HOST_PORT})"
    local -a run_args=(
      docker run -d --name "${CONTAINER_NAME}"
      -e "ROOT_PASSWORD=${MEMSQL_PASSWORD}"
      -p "${HOST_PORT}:3306"
      -p "8080:8080"
      -p "9000:9000"
      --cpus=8 --memory=16g
    )
    if [[ -n "${SINGLESTORE_LICENSE:-}" ]]; then
      run_args+=(-e "SINGLESTORE_LICENSE=${SINGLESTORE_LICENSE}")
    fi
    if [[ -n "${SINGLESTORE_VERSION}" ]]; then
      run_args+=(-e "SINGLESTORE_VERSION=${SINGLESTORE_VERSION}")
    fi
    run_args+=("${IMAGE_NAME}")
    "${run_args[@]}"
  fi

  echo -n "[plat8159] Waiting for master aggregator on 127.0.0.1:${HOST_PORT}"
  local ready=0
  for _ in $(seq 1 300); do
    if mysql_ma -e "SELECT 1" >/dev/null 2>&1; then
      echo " ok"
      ready=1
      break
    fi
    echo -n "."
    sleep 1
  done
  [[ "${ready}" -eq 1 ]] || die "SingleStore did not become ready in time"

  mysql_ma -e "CREATE DATABASE IF NOT EXISTS odbc_test" >/dev/null
  log "Cluster nodes:"
  docker exec "${CONTAINER_NAME}" memsqlctl list-nodes || true
  log "Leaves (information_schema.leaves):"
  mysql_ma -e "SELECT host, port FROM information_schema.leaves" || true
}

leaf_memsql_id() {
  local id=""
  # Quiet memsql-id list filtered to leaf role (memsqlctl >= recent versions).
  id="$(docker exec "${CONTAINER_NAME}" \
        memsqlctl list-nodes --role Leaf -q 2>/dev/null | head -n1 || true)"
  if [[ -z "${id}" ]]; then
    id="$(docker exec "${CONTAINER_NAME}" \
          memsqlctl list-nodes --role leaf -q 2>/dev/null | head -n1 || true)"
  fi
  if [[ -z "${id}" ]]; then
    # Tabular fallback: first column is MemsqlID; find a Leaf row.
    id="$(docker exec "${CONTAINER_NAME}" memsqlctl list-nodes 2>/dev/null \
          | awk 'BEGIN{IGNORECASE=1} $0 ~ /leaf/ && $1 ~ /^[0-9A-Fa-f]/ {print $1; exit}')"
  fi
  [[ -n "${id}" ]] || die "Could not determine leaf memsql-id (is the leaf up?)"
  echo "${id}"
}

kill_leaf() {
  local leaf_id
  leaf_id="$(leaf_memsql_id)"
  log "Stopping leaf node memsql-id=${leaf_id}"
  # Prefer a hard stop so the aggregator sees a disconnect, not a clean drain.
  if ! docker exec "${CONTAINER_NAME}" \
        memsqlctl stop-node --memsql-id "${leaf_id}" --yes --force 2>/dev/null; then
    log "memsqlctl stop-node --force unavailable; trying without --force"
    docker exec "${CONTAINER_NAME}" \
      memsqlctl stop-node --memsql-id "${leaf_id}" --yes \
      || die "Failed to stop leaf node"
  fi
  log "Leaf stop requested. Current node list:"
  docker exec "${CONTAINER_NAME}" memsqlctl list-nodes || true
}

run_scenario() {
  local conn client_rc=0 elapsed=0 start_ts now
  local -a timeout_args=()

  SYNC_DIR="$(mktemp -d /tmp/plat8159-sync.XXXXXX)"
  ODBC_OUT="${SYNC_DIR}/odbc.out"
  rm -f "${SYNC_DIR}/ready" "${SYNC_DIR}/go"

  conn="DRIVER=${DRIVER_SO};SERVER=127.0.0.1;PORT=${HOST_PORT};UID=root;PWD=${MEMSQL_PASSWORD};DATABASE=odbc_test;NO_SSPS=1;"

  if [[ "${QUERY_TIMEOUT}" -gt 0 ]]; then
    timeout_args=(--query-timeout "${QUERY_TIMEOUT}")
  fi

  log "Starting ODBC client (rows=${ROWS}, query_timeout=${QUERY_TIMEOUT})"
  if [[ "${QUERY_TIMEOUT}" -gt 0 ]]; then
    "${HELPER_BIN}" \
      --conn "${conn}" \
      --sync-dir "${SYNC_DIR}" \
      --rows "${ROWS}" \
      --query-timeout "${QUERY_TIMEOUT}" \
      >"${ODBC_OUT}" 2>&1 &
  else
    "${HELPER_BIN}" \
      --conn "${conn}" \
      --sync-dir "${SYNC_DIR}" \
      --rows "${ROWS}" \
      >"${ODBC_OUT}" 2>&1 &
  fi
  CLIENT_PID=$!

  log "Waiting for client ready signal..."
  for _ in $(seq 1 600); do
    if [[ -f "${SYNC_DIR}/ready" ]]; then
      break
    fi
    if ! kill -0 "${CLIENT_PID}" 2>/dev/null; then
      echo
      cat "${ODBC_OUT}" || true
      CLIENT_PID=""
      die "ODBC client exited before signaling ready"
    fi
    sleep 0.2
  done
  [[ -f "${SYNC_DIR}/ready" ]] || die "Timed out waiting for ODBC ready signal"
  log "Client ready (transaction open)."

  kill_leaf

  log "Signaling client to run CTAS"
  echo 1 > "${SYNC_DIR}/go"

  log "Waiting up to ${WAIT_SECS}s for SQLExecDirect to return..."
  start_ts="$(date +%s)"
  while kill -0 "${CLIENT_PID}" 2>/dev/null; do
    now="$(date +%s)"
    elapsed=$((now - start_ts))
    if [[ "${elapsed}" -ge "${WAIT_SECS}" ]]; then
      log "TIMEOUT: ODBC client still blocked after ${WAIT_SECS}s — error was NOT delivered (hang)"
      echo "----- ODBC client output so far -----"
      cat "${ODBC_OUT}" || true
      echo "------------------------------------"
      kill "${CLIENT_PID}" 2>/dev/null || true
      wait "${CLIENT_PID}" 2>/dev/null || true
      CLIENT_PID=""
      echo
      log "VERDICT: hang — cluster may have logged a failure, but no protocol error reached the driver within ${WAIT_SECS}s"
      return 1
    fi
    sleep 0.2
  done

  set +e
  wait "${CLIENT_PID}"
  client_rc=$?
  set -e
  CLIENT_PID=""
  now="$(date +%s)"
  elapsed=$((now - start_ts))

  echo "----- ODBC client output -----"
  cat "${ODBC_OUT}" || true
  echo "------------------------------"
  echo
  log "ODBC client exited rc=${client_rc} after ${elapsed}s"

  log "SHOW LEAVES (best effort after kill):"
  mysql_ma -e "SHOW LEAVES" 2>/dev/null || true

  case "${client_rc}" in
    0)
      log "VERDICT: error WAS delivered to the ODBC driver (SQL_ERROR)"
      return 0
      ;;
    1)
      log "VERDICT: CTAS succeeded after leaf kill — error was NOT observed by ODBC"
      return 1
      ;;
    *)
      log "VERDICT: ODBC client failed during setup or aborted (rc=${client_rc})"
      return 1
      ;;
  esac
}

build_helper
start_cluster
run_scenario
