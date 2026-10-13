#!/usr/bin/env bash
# Copyright (c) 2026 SingleStore, Inc.
#
# PLAT-8159: CREATE TEMPORARY TABLE ... AS SELECT logs
# ER_ROLLED_BACK_TRANSACTION (1857) and never returns it to the client.
#
# The mysql client opens a transaction, runs three UPDATEs, and
# DROP TEMPORARY TABLE (that statement returns). It then runs:
#
#   CREATE TEMPORARY TABLE tmp_latest AS
#   SELECT * FROM (
#     SELECT *,
#            row_number() OVER (PARTITION BY source_key
#                               ORDER BY event_seq DESC) AS rn,
#            count(*) OVER (PARTITION BY source_key) AS grp_count
#     FROM src_staging
#     WHERE event_seq BETWEEN 1000 AND 1005
#       AND SLEEP(30) = 0
#   ) WHERE rn = 1
#
# SLEEP holds the insert on the leaf. Once processlist shows the statement
# executing, this script kills leaf memsqld and rejects new TCP connections
# to the leaf port. leaf_failure_detection is off, so the restarted leaf is
# not failed over.
#
# memsql.log then contains:
#   "An error in this transaction required closing a connection to a leaf.
#    The open transaction has rolled back. ..."
# Production tracelogs print that message, not the symbol
# ER_ROLLED_BACK_TRANSACTION. The mysql client prints nothing and stays
# blocked in the CREATE.
#
# Requires Docker and the mysql or mariadb client. Copy this file anywhere
# and run it; it does not use the rest of the repository.
#
#   export MEMSQL_PASSWORD='...'          # container root password
#   ./plat8159_leaf_kill_mysql.sh
#   ./plat8159_leaf_kill_mysql.sh --wait-secs 15 --keep-cluster
#
# Optional environment:
#   SINGLESTORE_LICENSE   SINGLESTORE_VERSION
#   IMAGE_NAME            default ghcr.io/singlestore-labs/singlestoredb-dev:latest
#   COMPILE_WAIT_SECS     seconds to wait for state "executing" (default 180)
#
# Options:
#   --wait-secs N     How long the CREATE must stay blocked (default 60)
#   --keep-cluster    Leave the Docker container running on exit
#   --reuse-cluster   Reuse a container that is already running
#
# Exit codes:
#   0  client still blocked and memsql.log contains error 1857
#   1  the CREATE returned, or the client hung without error 1857
#   2  setup failure

set -euo pipefail

IMAGE_NAME="${IMAGE_NAME:-ghcr.io/singlestore-labs/singlestoredb-dev:latest}"
CONTAINER_NAME="${CONTAINER_NAME:-singlestore-plat8159}"
FW_NAME="${FW_NAME:-${CONTAINER_NAME}-fw}"
FW_IMAGE="${FW_IMAGE:-plat8159-iptables}"
LEAF_PORT="${LEAF_PORT:-3307}"
HOST_PORT="${HOST_PORT:-5606}"
DB_NAME="${DB_NAME:-plat8159}"
MEMSQL_PASSWORD="${MEMSQL_PASSWORD:-password}"
WAIT_SECS=60
COMPILE_WAIT_SECS="${COMPILE_WAIT_SECS:-180}"
KEEP_CLUSTER=0
REUSE_CLUSTER=0
WORK_DIR=""
CLIENT_PID=""
CLIENT_OUT=""

usage() {
  sed -n '2,53p' "$0"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --wait-secs) WAIT_SECS="$2"; shift 2 ;;
    --keep-cluster) KEEP_CLUSTER=1; shift ;;
    --reuse-cluster) REUSE_CLUSTER=1; shift ;;
    --help|-h) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
  esac
done

die() { echo "ERROR: $*" >&2; exit 2; }
log() { echo "[plat8159] $*"; }

# kill -0 is true for a zombie. An exited client must count as finished.
pid_alive() {
  local pid="$1" state=""
  [[ -n "${pid}" ]] || return 1
  kill -0 "${pid}" 2>/dev/null || return 1
  state="$(ps -o state= -p "${pid}" 2>/dev/null | tr -d '[:space:]' || true)"
  [[ -n "${state}" && "${state}" != Z* ]]
}

cleanup() {
  local ec=$?
  if pid_alive "${CLIENT_PID}"; then
    log "Stopping hung mysql client pid=${CLIENT_PID}"
    kill "${CLIENT_PID}" 2>/dev/null || true
    wait "${CLIENT_PID}" 2>/dev/null || true
  fi
  [[ -n "${WORK_DIR}" && -d "${WORK_DIR}" ]] && rm -rf "${WORK_DIR}"
  if [[ "${KEEP_CLUSTER}" -eq 0 ]]; then
    if command -v docker >/dev/null 2>&1; then
      docker rm -f "${FW_NAME}" >/dev/null 2>&1 || true
      if docker inspect "${CONTAINER_NAME}" >/dev/null 2>&1; then
        log "Removing container ${CONTAINER_NAME}"
        docker rm -f "${CONTAINER_NAME}" >/dev/null 2>&1 || true
      fi
    fi
  else
    log "Leaving container ${CONTAINER_NAME} running (--keep-cluster)"
  fi
  exit "$ec"
}
trap cleanup EXIT

command -v docker >/dev/null 2>&1 || die "docker is required"
if ! command -v mysql >/dev/null 2>&1 && ! command -v mariadb >/dev/null 2>&1; then
  die "mysql or mariadb client is required"
fi
MYSQL_CLI="$(command -v mysql || command -v mariadb)"

mysql_ma() {
  "$MYSQL_CLI" -N -B -u root -h 127.0.0.1 -P "${HOST_PORT}" \
    -p"${MEMSQL_PASSWORD}" --protocol=TCP "$@"
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
      --cpus=8 --memory=16g
    )
    if [[ -n "${SINGLESTORE_LICENSE:-}" ]]; then
      run_args+=(-e "SINGLESTORE_LICENSE=${SINGLESTORE_LICENSE}")
    fi
    if [[ -n "${SINGLESTORE_VERSION:-}" ]]; then
      run_args+=(-e "SINGLESTORE_VERSION=${SINGLESTORE_VERSION}")
    fi
    run_args+=("${IMAGE_NAME}")
    "${run_args[@]}"
  fi

  echo -n "[plat8159] Waiting for the master aggregator on 127.0.0.1:${HOST_PORT}"
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
  mysql_ma -e "CREATE DATABASE IF NOT EXISTS ${DB_NAME}" >/dev/null
}

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
  docker rm -f "${FW_NAME}" >/dev/null 2>&1 || true
  log "Starting iptables sidecar ${FW_NAME}"
  docker run -d --name "${FW_NAME}" \
    --network "container:${CONTAINER_NAME}" \
    --cap-add NET_ADMIN \
    "${FW_IMAGE}" sleep 3600 >/dev/null
}

# memsqld_safe restarts the leaf. Rejecting the port keeps that restart from
# finishing the CREATE. Killing the process is what records error 1857.
kill_leaf() {
  log "Killing leaf memsqld"
  docker exec "${CONTAINER_NAME}" sh -c \
    'ps -eo pid,args | grep "/data/leaf/memsql.cnf" | grep -v memsqld_safe | grep -v grep | awk "{print \$1}" | xargs -r kill -9'
}

refuse_leaf_port() {
  log "Rejecting TCP to leaf port ${LEAF_PORT}"
  docker exec "${FW_NAME}" \
    iptables -I INPUT -p tcp --dport "${LEAF_PORT}" -j REJECT --reject-with tcp-reset
}

# 1857 is ER_ROLLED_BACK_TRANSACTION. The log prints the message, not the symbol.
master_log_1857() {
  docker exec "${CONTAINER_NAME}" \
    grep -E "error 1857|Transaction rolled back|required closing a connection to a leaf" \
    /logs/master/tracelogs/memsql.log || true
}

# CONCAT keeps this probe from matching its own query text. The probe is killed
# if it runs longer than 8 seconds: during compilation the aggregator can be
# slow, and after the leaf is killed a processlist query can hang.
processlist_create() {
  local out="${WORK_DIR}/processlist.out" pid="" waited=0
  : >"${out}"
  "$MYSQL_CLI" -N -B -u root -h 127.0.0.1 -P "${HOST_PORT}" \
    -p"${MEMSQL_PASSWORD}" --protocol=TCP --connect-timeout=5 -e \
    "SELECT CONCAT(STATE, ' | ', LEFT(INFO, 160)) FROM information_schema.processlist WHERE INFO LIKE CONCAT('%tmp_lat', 'est%') AND INFO NOT LIKE 'DROP %' AND INFO NOT LIKE 'drop %' LIMIT 1" \
    >"${out}" 2>/dev/null &
  pid=$!
  while [[ "${waited}" -lt 40 ]]; do
    if ! kill -0 "${pid}" 2>/dev/null; then
      wait "${pid}" 2>/dev/null || true
      cat "${out}" 2>/dev/null || true
      return 0
    fi
    sleep 0.2
    waited=$((waited + 1))
  done
  kill "${pid}" 2>/dev/null || true
  wait "${pid}" 2>/dev/null || true
  cat "${out}" 2>/dev/null || true
}

dump_processlist() {
  log "Full processlist:"
  mysql_ma --connect-timeout=5 -e \
    "SELECT ID, DB, COMMAND, STATE, LEFT(INFO, 120) AS INFO FROM information_schema.processlist" \
    || true
}

dump_client_output() {
  if [[ -n "${CLIENT_OUT}" && -f "${CLIENT_OUT}" ]]; then
    echo "----- mysql client output -----"
    cat "${CLIENT_OUT}" || true
    echo "-------------------------------"
  fi
}

seed_schema() {
  log "Creating ${DB_NAME}.src_staging and ${DB_NAME}.batch_log"
  mysql_ma --database="${DB_NAME}" <<'SQL'
DROP TABLE IF EXISTS src_staging;
DROP TABLE IF EXISTS batch_log;
CREATE TABLE src_staging (
  event_seq BIGINT NOT NULL,
  source_key VARCHAR(64) NOT NULL,
  action_code VARCHAR(8) NOT NULL,
  v VARCHAR(64) NOT NULL
);
CREATE TABLE batch_log (
  id INT NOT NULL PRIMARY KEY,
  seq_no BIGINT NOT NULL
);
INSERT INTO src_staging VALUES
  (1000, 'src-a', 'I', 'row-0'),
  (1001, 'src-a', 'U', 'row-1'),
  (1002, 'src-2', 'I', 'row-2'),
  (1003, 'src-3', 'I', 'row-3'),
  (1004, 'src-4', 'I', 'row-4'),
  (1005, 'src-5', 'I', 'row-5');
INSERT INTO batch_log VALUES (1, 0), (2, 0), (3, 0);
SQL
}

run_scenario() {
  local seen=0 elapsed=0 start_ts now info="" sql_file=""

  WORK_DIR="$(mktemp -d /tmp/plat8159.XXXXXX)"
  CLIENT_OUT="${WORK_DIR}/mysql.out"
  sql_file="${WORK_DIR}/txn.sql"

  cat >"${sql_file}" <<'SQL'
SET autocommit = 0;
START TRANSACTION;
UPDATE batch_log SET seq_no = 1000 WHERE id = 1;
UPDATE batch_log SET seq_no = 1001 WHERE id = 2;
UPDATE batch_log SET seq_no = 1002 WHERE id = 3;
SELECT @@trancount;
DROP TEMPORARY TABLE IF EXISTS tmp_latest;
SELECT @@trancount;
CREATE TEMPORARY TABLE tmp_latest AS
SELECT * FROM (
  SELECT *,
         row_number() OVER (PARTITION BY source_key
                            ORDER BY event_seq DESC) AS rn,
         count(*) OVER (PARTITION BY source_key) AS grp_count
  FROM src_staging
  WHERE event_seq BETWEEN 1000 AND 1005
    AND SLEEP(30) = 0
) WHERE rn = 1;
SQL

  log "Starting the mysql client on 127.0.0.1:${HOST_PORT}"
  "$MYSQL_CLI" -u root -h 127.0.0.1 -P "${HOST_PORT}" \
    -p"${MEMSQL_PASSWORD}" --protocol=TCP --database="${DB_NAME}" \
    <"${sql_file}" >"${CLIENT_OUT}" 2>&1 &
  CLIENT_PID=$!

  # Compilation does not touch the leaf. Wait until the INSERT ... SELECT is
  # executing, so SLEEP is on the leaf, before killing it. Killing earlier
  # only retries error 1735 and does not record error 1857.
  log "Waiting up to ${COMPILE_WAIT_SECS}s for the CREATE to reach state executing"
  start_ts="$(date +%s)"
  local last_log=0 state_lc=""
  while true; do
    info="$(processlist_create)"
    now="$(date +%s)"
    elapsed=$((now - start_ts))
    state_lc="$(printf '%s' "${info}" | tr '[:upper:]' '[:lower:]')"
    if [[ -n "${info}" && "${seen}" -eq 0 ]]; then
      log "Statement is on the server: ${info}"
      seen=1
    fi
    if [[ "${state_lc}" == executing* ]]; then
      log "Statement is executing: ${info}"
      break
    fi
    if ! pid_alive "${CLIENT_PID}"; then
      dump_client_output
      log "VERDICT: the CREATE returned before the leaf was killed"
      return 1
    fi
    if [[ "${seen}" -eq 0 && "${elapsed}" -ge 30 ]]; then
      dump_client_output
      die "CREATE never appeared in processlist"
    fi
    if [[ $((elapsed - last_log)) -ge 15 && "${elapsed}" -gt 0 ]]; then
      log "still not executing at ${elapsed}s: ${info:-<no processlist row>}"
      last_log="${elapsed}"
    fi
    if [[ "${elapsed}" -ge "${COMPILE_WAIT_SECS}" ]]; then
      dump_client_output
      dump_processlist
      die "CREATE never reached executing (last state: ${info:-<no processlist row>}); leaf was not killed"
    fi
    sleep 2
  done

  kill_leaf
  refuse_leaf_port

  log "Waiting ${WAIT_SECS}s to see whether the mysql client stays blocked"
  start_ts="$(date +%s)"
  while pid_alive "${CLIENT_PID}"; do
    now="$(date +%s)"
    elapsed=$((now - start_ts))
    if [[ "${elapsed}" -ge "${WAIT_SECS}" ]]; then
      info="$(processlist_create)"
      dump_client_output
      log "processlist: ${info:-<statement no longer listed>}"
      kill "${CLIENT_PID}" 2>/dev/null || true
      wait "${CLIENT_PID}" 2>/dev/null || true
      CLIENT_PID=""
      if [[ -z "${info}" ]]; then
        log "VERDICT: client still blocked after ${WAIT_SECS}s, but the CREATE is no longer in processlist"
        return 1
      fi
      local rolled
      rolled="$(master_log_1857)"
      if [[ -z "${rolled}" ]]; then
        log "VERDICT: client stayed blocked, but memsql.log has no error 1857"
        return 1
      fi
      echo "----- memsql.log error 1857 (ER_ROLLED_BACK_TRANSACTION) -----"
      echo "${rolled}"
      echo "--------------------------------------------------------------"
      log "VERDICT: hang reproduced. The client is still blocked and memsql.log contains error 1857."
      return 0
    fi
    if [[ $((elapsed % 15)) -eq 0 && "${elapsed}" -gt 0 ]]; then
      info="$(processlist_create)"
      log "still blocked at ${elapsed}s: ${info:-<statement not in processlist>}"
    fi
    sleep 1
  done

  wait "${CLIENT_PID}" 2>/dev/null || true
  CLIENT_PID=""
  dump_client_output
  log "VERDICT: the CREATE returned. The hang was not reproduced."
  return 1
}

start_cluster
start_firewall
log "Disabling leaf failure detection so a reset is not rewritten as a failover"
mysql_ma -e "SET GLOBAL leaf_failure_detection = OFF" >/dev/null
seed_schema
rc=0
run_scenario || rc=$?
exit "${rc}"
