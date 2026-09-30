#!/usr/bin/env bash
# ************************************************************************************
#   Copyright (c) 2026 SingleStore, Inc.
#
#   PLAT-8159 hang repro, mysql CLI client
#
#   Same server sequence as plat8159_leaf_kill_repro.sh, with the mysql (or
#   mariadb) client holding the open transaction instead of the ODBC helper.
#
#   The client seeds src_staging, starts a transaction, runs three UPDATEs,
#   DROP TEMPORARY TABLE (this returns), then CREATE TEMPORARY TABLE
#   tmp_latest AS SELECT with window functions and SLEEP(30). SLEEP holds
#   the insert on the leaf. Once processlist shows the create executing,
#   the script kills leaf memsqld and refuses leaf port 3307.
#   leaf_failure_detection is off, so the restarted leaf is not failed over.
#   memsql.log records ER_ROLLED_BACK_TRANSACTION (1857) as:
#   "An error in this transaction required closing a connection to a leaf..."
#   Production tracelogs print that message, not the symbol. The mysql
#   client stays blocked and does not print the error.
#
#   Uses the same container name and host port as the ODBC script. Do not
#   run both at once.
#
#   Usage (from repo root):
#     export MEMSQL_PASSWORD=...
#     ./test/repro/plat8159_leaf_kill_mysql.sh
#
#   Options:
#     --wait-secs N       How long the create must stay blocked (default 60)
#     --keep-cluster      Do not remove the docker container on exit
#     --reuse-cluster     Reuse an already-running container if present
#
#   COMPILE_WAIT_SECS (default 180) is how long to wait for the create to
#   leave "Compiling Query" and reach "executing" before the leaf is killed.
#
#   Exit codes:
#     0  hang reproduced and memsql.log contains error 1857
#        (ER_ROLLED_BACK_TRANSACTION); mysql client still blocked
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
COMPILE_WAIT_SECS="${COMPILE_WAIT_SECS:-180}"
KEEP_CLUSTER=0
REUSE_CLUSTER=0
SYNC_DIR=""
CLIENT_PID=""
CLIENT_OUT=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --wait-secs) WAIT_SECS="$2"; shift 2 ;;
    --keep-cluster) KEEP_CLUSTER=1; shift ;;
    --reuse-cluster) REUSE_CLUSTER=1; shift ;;
    --help|-h)
      sed -n '2,41p' "$0"
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      exit 2
      ;;
  esac
done

die() { echo "ERROR: $*" >&2; exit 2; }
log() { echo "[plat8159-mysql] $*"; }

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
    log "Stopping hung mysql client pid=${CLIENT_PID}"
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
if ! command -v mysql >/dev/null 2>&1 && ! command -v mariadb >/dev/null 2>&1; then
  die "required command not found: mysql (or mariadb)"
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

  echo -n "[plat8159-mysql] Waiting for master aggregator on 127.0.0.1:${HOST_PORT}"
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
# Give the probe several seconds. During compilation the aggregator can be
# slow to answer, and killing those probes early hides the transition to
# "executing".
processlist_ctas() {
  local out="${SYNC_DIR}/processlist.out" pid="" waited=0
  : >"${out}"
  "$MYSQL_CLI" -N -B -u root -h 127.0.0.1 -P "${HOST_PORT}" \
    -p"${MEMSQL_PASSWORD}" --protocol=TCP --connect-timeout=5 -e \
    "SELECT CONCAT(STATE, ' | ', LEFT(INFO, 160)) FROM information_schema.processlist WHERE INFO LIKE CONCAT('%tmp_lat', 'est%') AND INFO NOT LIKE 'DROP %' AND INFO NOT LIKE 'drop %' LIMIT 1" \
    >"${out}" 2>/dev/null &
  pid=$!
  # 40 * 0.2s = 8s. waited counts ticks, not seconds.
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
  log "Seeding src_staging and batch_log"
  mysql_ma --database=odbc_test <<'SQL'
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
  local ctas_rc=0 elapsed=0 start_ts now info="" sql_file=""

  SYNC_DIR="$(mktemp -d /tmp/plat8159-mysql.XXXXXX)"
  CLIENT_OUT="${SYNC_DIR}/mysql.out"
  sql_file="${SYNC_DIR}/txn.sql"

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

  log "Starting mysql client on 127.0.0.1:${HOST_PORT}"
  "$MYSQL_CLI" -u root -h 127.0.0.1 -P "${HOST_PORT}" \
    -p"${MEMSQL_PASSWORD}" --protocol=TCP --database=odbc_test \
    <"${sql_file}" >"${CLIENT_OUT}" 2>&1 &
  CLIENT_PID=$!

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
  # keeps the restarted leaf from letting the create finish. Any state
  # other than "executing" (including a slow processlist probe) keeps waiting.
  log "Waiting until the statement is executing (up to ${COMPILE_WAIT_SECS}s)..."
  local executing=0 compile_start compile_now compile_elapsed last_log=0 state_lc=""
  compile_start="$(date +%s)"
  while true; do
    info="$(processlist_ctas)"
    compile_now="$(date +%s)"
    compile_elapsed=$((compile_now - compile_start))
    state_lc="$(printf '%s' "${info}" | tr '[:upper:]' '[:lower:]')"
    if [[ "${state_lc}" == executing* ]]; then
      log "Statement is executing: ${info}"
      executing=1
      break
    fi
    if ! pid_alive "${CLIENT_PID}"; then
      log "mysql client exited while the create was still compiling"
      break
    fi
    if [[ $((compile_elapsed - last_log)) -ge 15 ]]; then
      log "still not executing at ${compile_elapsed}s: ${info:-<no processlist row>}"
      last_log="${compile_elapsed}"
    fi
    if [[ "${compile_elapsed}" -ge "${COMPILE_WAIT_SECS}" ]]; then
      break
    fi
    sleep 2
  done
  if [[ "${executing}" -ne 1 ]]; then
    dump_client_output
    dump_processlist
    die "Create never reached executing (last state: ${info:-<no processlist row>}); leaf was not killed"
  fi
  kill_leaf_processes
  refuse_leaf_connects

  log "Waiting ${WAIT_SECS}s to see whether the mysql client stays blocked..."
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
      log "VERDICT: hang reproduced — mysql client still blocked, memsql.log contains ER_ROLLED_BACK_TRANSACTION (1857)"
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

start_cluster
start_firewall
log "Disabling leaf failure detection so a reset is not rewritten as a failover"
mysql_ma -e "SET GLOBAL leaf_failure_detection = OFF" >/dev/null
seed_schema
rc=0
run_scenario || rc=$?
exit "${rc}"
