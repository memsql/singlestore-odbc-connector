# PLAT-8159 leaf-kill repro

Explores whether a leaf failure during a transactional
`CREATE TEMPORARY TABLE … AS SELECT` delivers an error to the ODBC client, or
whether the client hangs waiting for a MySQL protocol response that never
arrives (the Helios hang reported in PLAT-8159).

## What it does

1. Starts `ghcr.io/singlestore-labs/singlestoredb-dev` (MA + leaf).
2. Builds a small ODBC client linked against unixODBC + your built `libssodbca.so`.
3. Opens a multi-statement transaction, seeds a table, signals ready.
4. Stops the leaf via `memsqlctl stop-node`.
5. Runs `CREATE TEMPORARY TABLE … AS SELECT` on the ODBC connection.
6. Reports one of:
   - **error delivered** — `SQLExecDirect` returned `SQL_ERROR` (prints SQLSTATE/native/msg)
   - **hang** — still blocked after `--wait-secs` (default 60)
   - **unexpected success** — CTAS succeeded after the leaf was stopped

## Prerequisites

- Docker
- `gcc`, unixODBC (`libodbc`), `mysql` or `mariadb` client
- Built ANSI driver: `build/libssodbca.so`
- `MEMSQL_PASSWORD` (used as container `ROOT_PASSWORD`)
- Optional: `SINGLESTORE_LICENSE`, `SINGLESTORE_VERSION`

## Run

```bash
# From repo root, after building the connector into build/
export MEMSQL_PASSWORD='...'
./test/repro/plat8159_leaf_kill_repro.sh

# Compare with the PLAT-8159 mitigation enabled:
./test/repro/plat8159_leaf_kill_repro.sh --query-timeout 10 --wait-secs 30

# Keep the container for manual inspection:
./test/repro/plat8159_leaf_kill_repro.sh --keep-cluster --reuse-cluster
```

## Files

| File | Role |
|------|------|
| `plat8159_leaf_kill_repro.sh` | Cluster setup, leaf kill, verdict |
| `plat8159_leaf_kill_txn.c` | ODBC txn + CTAS client |

This is a manual investigation tool, not part of `ctest`.
