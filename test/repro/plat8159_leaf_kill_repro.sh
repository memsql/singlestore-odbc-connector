#!/usr/bin/env bash
# ************************************************************************************
#   Copyright (c) 2026 SingleStore, Inc.
#
#   PLAT-8159 hang repro
#
#   Replays the batch that never returned to the client: an open transaction,
#   a few UPDATEs, DROP TEMPORARY TABLE (this returns), then
#   CREATE TEMPORARY TABLE tmp_latest AS SELECT with window functions.
#
#   The create runs while the leaf is up. SLEEP in the INSERT ... SELECT
#   holds that statement on the leaf. The script then kills leaf memsqld and
#   refuses the leaf port. leaf_failure_detection is off, so the restarted
#   leaf is not failed over and the create is not sent back to the client.
#   memsql.log records ER_ROLLED_BACK_TRANSACTION (1857) as:
#   "An error in this transaction required closing a connection to a leaf..."
#   Production tracelogs print that message, not the symbol. The ODBC client
#   stays blocked. The same hang happens with the mysql CLI.
#
#   Confirmed still blocked after 5 minutes on singlestoredb-dev 9.0.13.
#
#   Usage (from repo root):
#     export MEMSQL_PASSWORD=...
#     ./test/repro/plat8159_leaf_kill_repro.sh
#
#   Options:
#     --wait-secs N       How long the create must stay blocked (default 60)
#     --keep-cluster      Do not remove the docker container on exit
#     --reuse-cluster     Reuse an already-running container if present
#     --skip-build        Do not rebuild the ODBC helper
#     --driver PATH       ANSI driver (default: build/libssodbca.dylib on
#                         macOS, build/libssodbca.so elsewhere)
#
#   Exit codes:
#     0  hang reproduced and memsql.log contains error 1857
#        (ER_ROLLED_BACK_TRANSACTION); client still blocked
#     1  the create returned, or the client hung without error 1857
#     2  setup failure
# ************************************************************************************

set -euo pipefail

ROOT="$(git -C "$(dirname "$0")/../.." rev-parse --show-toplevel 2>/dev/null \
  || realpath "$(dirname "$0")/../..")"
cd "$ROOT"

IMAGE_NAME="${IMAGE_NAME:-ghcr.io/singlestore-labs/singlestoredb-dev:latest}"
CONTAINER_NAME="${CONTAINER_NAME:-singlestore-plat8159}"
FW_NAME="${FW_NAME:-${CONTAINER_NAME}-fw}"
LEAF_PORT="${LEAF_PORT:-3307}"
HOST_PORT="${HOST_PORT:-5606}"
SINGLESTORE_VERSION="${SINGLESTORE_VERSION:-}"
MEMSQL_PASSWORD="${MEMSQL_PASSWORD:-password}"
WAIT_SECS=60
KEEP_CLUSTER=0
REUSE_CLUSTER=0
SKIP_BUILD=0
case "$(uname -s)" in
  Darwin) DRIVER_LIB="libssodbca.dylib" ;;
  *) DRIVER_LIB="libssodbca.so" ;;
esac
DRIVER_SO="${DRIVER_SO:-$ROOT/build/${DRIVER_LIB}}"
HELPER_BIN="$ROOT/test/repro/plat8159_leaf_kill_txn"
SYNC_DIR=""
CLIENT_PID=""
ODBC_OUT=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --wait-secs) WAIT_SECS="$2"; shift 2 ;;
    --keep-cluster) KEEP_CLUSTER=1; shift ;;
    --reuse-cluster) REUSE_CLUSTER=1; shift ;;
    --skip-build) SKIP_BUILD=1; shift ;;
    --driver) DRIVER_SO="$2"; shift 2 ;;
    --help|-h)
      sed -n '2,35p' "$0"
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

# kill -0 is true for a zombie. An exited client must count as finished.
pid_alive() {
  local pid="$1" state=""
  [[ -n "${pid}" ]] || return 1
  if ! kill -0 "${pid}" 2>/dev/null; then
    return 1
  fi
  state="$(ps -o state= -p "${pid}" 2>/dev/null | tr -d '[:space:]' || true)"
  if [[ -z "${state}" || "${state}" == Z* ]]; then
    return 1
  fi
  return 0
}

cleanup() {
  local ec=$?
  if pid_alive "${CLIENT_PID}"; then
    log "Stopping hung ODBC client pid=${CLIENT_PID}"
    kill "${CLIENT_PID}" 2>/dev/null || true
    wait "${CLIENT_PID}" 2>/dev/null || true
  fi
  if [[ -n "${SYNC_DIR}" && -d "${SYNC_DIR}" ]]; then
    rm -rf "${SYNC_DIR}"
  fi
  if [[ "${KEEP_CLUSTER}" -eq 0 ]]; then
    if command -v docker >/dev/null 2>&1 \
         && docker inspect "${FW_NAME}" >/dev/null 2>&1; then
      docker rm -f "${FW_NAME}" >/dev/null 2>&1 || true
    fi
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
      docker rm -f "${FW_NAME}" >/dev/null 2>&1 || true
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
}

FW_IMAGE="${FW_IMAGE:-plat8159-iptables}"

ensure_firewall_image() {
  if docker image inspect "${FW_IMAGE}" >/dev/null 2>&1; then
    return
  fi
  log "Building ${FW_IMAGE} (alpine + iptables)"
  docker build -t "${FW_IMAGE}" - <<'EOF'
FROM alpine:latest
RUN apk add --no-cache iptables
EOF
}

start_firewall() {
  ensure_firewall_image
  if docker inspect "${FW_NAME}" >/dev/null 2>&1; then
    docker rm -f "${FW_NAME}" >/dev/null
  fi
  log "Starting iptables sidecar ${FW_NAME}"
  docker run -d --name "${FW_NAME}" \
    --network "container:${CONTAINER_NAME}" \
    --cap-add NET_ADMIN \
    "${FW_IMAGE}" sleep 3600 >/dev/null
}

refuse_leaf_connects() {
  log "Refusing TCP connections to leaf port ${LEAF_PORT} (tcp-reset)"
  docker exec "${FW_NAME}" \
    iptables -I INPUT -p tcp --dport "${LEAF_PORT}" -j REJECT --reject-with tcp-reset
}

# memsqld_safe restarts the leaf. The port reject stays in place, so the
# aggregator's create retry does not finish. Killing the process is what
# makes the leaf raise ER_ROLLED_BACK_TRANSACTION (1857).
kill_leaf_processes() {
  log "Killing leaf memsqld so the open transaction records error 1857"
  docker exec "${CONTAINER_NAME}" sh -c \
    'ps -eo pid,args | grep "/data/leaf/memsql.cnf" | grep -v memsqld_safe | grep -v grep | awk "{print \$1}" | xargs -r kill -9'
}

# Production tracelogs print the numeric code. 1857 is ER_ROLLED_BACK_TRANSACTION
# (error_code_numbers.txt). The leaf message is "Transaction rolled back mid-query".
master_log_1857() {
  docker exec "${CONTAINER_NAME}" \
    grep -E "error 1857|Transaction rolled back|required closing a connection to a leaf" \
    /logs/master/tracelogs/memsql.log || true
}

# The probe builds the pattern with CONCAT so it does not match itself.
processlist_ctas() {
  local out="${SYNC_DIR}/processlist.out" pid=""
  : >"${out}"
  "$MYSQL_CLI" -N -B -u root -h 127.0.0.1 -P "${HOST_PORT}" \
    -p"${MEMSQL_PASSWORD}" --protocol=TCP --connect-timeout=2 -e \
    "SELECT CONCAT(STATE, ' | ', LEFT(INFO, 160)) FROM information_schema.processlist WHERE INFO LIKE CONCAT('%tmp_lat', 'est%') AND INFO NOT LIKE 'DROP %' AND INFO NOT LIKE 'drop %' LIMIT 1" \
    >"${out}" 2>/dev/null &
  pid=$!
  local _
  for _ in $(seq 1 10); do
    if ! kill -0 "${pid}" 2>/dev/null; then
      wait "${pid}" 2>/dev/null || true
      cat "${out}" 2>/dev/null || true
      return 0
    fi
    sleep 0.2
  done
  kill "${pid}" 2>/dev/null || true
  wait "${pid}" 2>/dev/null || true
}

dump_client_output() {
  if [[ -n "${ODBC_OUT}" && -f "${ODBC_OUT}" ]]; then
    echo "----- ODBC client output -----"
    cat "${ODBC_OUT}" || true
    echo "------------------------------"
  fi
}

run_scenario() {
  local conn ctas_rc=0 elapsed=0 start_ts now info=""

  SYNC_DIR="$(mktemp -d /tmp/plat8159-sync.XXXXXX)"
  ODBC_OUT="${SYNC_DIR}/odbc.out"
  rm -f "${SYNC_DIR}/ready" "${SYNC_DIR}/go"

  conn="DRIVER=${DRIVER_SO};SERVER=127.0.0.1;PORT=${HOST_PORT};UID=root;PWD=${MEMSQL_PASSWORD};DATABASE=odbc_test;NO_SSPS=1;"
  log "Starting ODBC client"
  "${HELPER_BIN}" --conn "${conn}" --sync-dir "${SYNC_DIR}" >"${ODBC_OUT}" 2>&1 &
  CLIENT_PID=$!

  log "Waiting for ODBC ready signal..."
  for _ in $(seq 1 600); do
    if [[ -f "${SYNC_DIR}/ready" ]]; then
      break
    fi
    if ! pid_alive "${CLIENT_PID}"; then
      dump_client_output
      die "ODBC client exited before signaling ready (drop did not return)"
    fi
    sleep 0.2
  done
  [[ -f "${SYNC_DIR}/ready" ]] || die "Timed out waiting for ODBC ready signal"
  log "DROP TEMPORARY TABLE returned. Starting the create while the leaf is up."
  echo 1 > "${SYNC_DIR}/go"

  log "Waiting for CREATE TEMPORARY TABLE to reach the aggregator..."
  ctas_rc=2
  for _ in $(seq 1 200); do
    info="$(processlist_ctas)"
    if [[ -n "${info}" ]]; then
      log "Statement is on the server: ${info}"
      ctas_rc=0
      break
    fi
    if ! pid_alive "${CLIENT_PID}"; then
      ctas_rc=1
      break
    fi
    sleep 0.05
  done

  if [[ "${ctas_rc}" -eq 1 ]]; then
    wait "${CLIENT_PID}" 2>/dev/null || true
    CLIENT_PID=""
    dump_client_output
    log "VERDICT: create returned — hang was NOT reproduced"
    return 1
  fi
  if [[ "${ctas_rc}" -eq 2 ]]; then
    dump_client_output
    die "Create never appeared in processlist and the client is still blocked"
  fi

  # Compilation does not touch the leaf. Wait until the INSERT ... SELECT
  # is executing so SLEEP is on the leaf, then kill it. That is the path
  # that raises ER_ROLLED_BACK_TRANSACTION. Refusing the port afterward
  # keeps the restarted leaf from letting the create finish.
  log "Waiting until the statement is past compilation..."
  local executing=0
  for _ in $(seq 1 120); do
    info="$(processlist_ctas)"
    if [[ -n "${info}" && "${info}" != Compil* ]]; then
      log "Statement is executing: ${info}"
      executing=1
      break
    fi
    if ! pid_alive "${CLIENT_PID}"; then
      break
    fi
    sleep 0.5
  done
  if [[ "${executing}" -ne 1 ]]; then
    dump_client_output
    die "Create stayed in compilation; leaf was not killed"
  fi
  kill_leaf_processes
  refuse_leaf_connects

  log "Waiting ${WAIT_SECS}s to see whether the client stays blocked..."
  start_ts="$(date +%s)"
  while pid_alive "${CLIENT_PID}"; do
    now="$(date +%s)"
    elapsed=$((now - start_ts))
    if [[ "${elapsed}" -ge "${WAIT_SECS}" ]]; then
      info="$(processlist_ctas)"
      dump_client_output
      log "processlist: ${info:-<statement no longer listed>}"
      kill "${CLIENT_PID}" 2>/dev/null || true
      wait "${CLIENT_PID}" 2>/dev/null || true
      CLIENT_PID=""
      if [[ -z "${info}" ]]; then
        log "VERDICT: client still blocked after ${WAIT_SECS}s, but the create is no longer in processlist"
        return 1
      fi
      local rolled
      rolled="$(master_log_1857)"
      if [[ -z "${rolled}" ]]; then
        log "VERDICT: client stayed blocked, but memsql.log has no error 1857 (ER_ROLLED_BACK_TRANSACTION)"
        return 1
      fi
      echo "----- memsql.log ER_ROLLED_BACK_TRANSACTION (error 1857) -----"
      echo "${rolled}"
      echo "--------------------------------------------------------------"
      log "VERDICT: hang reproduced — client still blocked, memsql.log contains ER_ROLLED_BACK_TRANSACTION (1857)"
      return 0
    fi
    if [[ $((elapsed % 15)) -eq 0 && "${elapsed}" -gt 0 ]]; then
      info="$(processlist_ctas)"
      log "still blocked at ${elapsed}s: ${info:-<statement not in processlist>}"
    fi
    sleep 1
  done

  wait "${CLIENT_PID}" 2>/dev/null || true
  CLIENT_PID=""
  dump_client_output
  log "VERDICT: create returned — hang was NOT reproduced"
  return 1
}

build_helper
start_cluster
start_firewall
log "Disabling leaf failure detection so a reset is not rewritten as a failover"
mysql_ma -e "SET GLOBAL leaf_failure_detection = OFF" >/dev/null
rc=0
run_scenario || rc=$?
exit "${rc}"
