# PLAT-8159 hang repro

Replays the batch that never returned to the client. After an open
transaction, a few `UPDATE`s, and a `DROP TEMPORARY TABLE` that returns, the
script runs:

```sql
CREATE TEMPORARY TABLE tmp_latest AS
SELECT * FROM (
  SELECT *,
         row_number() OVER (PARTITION BY source_key
                            ORDER BY event_seq DESC) AS rn,
         count(*) OVER (PARTITION BY source_key) AS grp_count
  FROM src_staging
  WHERE event_seq BETWEEN 1000 AND 1005
    AND SLEEP(30) = 0
) WHERE rn = 1
```

`SLEEP` in the select holds the insert on the leaf. The script then kills
the leaf and refuses its port. `leaf_failure_detection` is off, so the
restarted leaf is not failed over. `memsql.log` records error 1857
(`ER_ROLLED_BACK_TRANSACTION`): "An error in this transaction required
closing a connection to a leaf...". `SQLExecDirect` does not return. The
same hang happens with the `mysql` CLI.

## What it does

1. Starts `ghcr.io/singlestore-labs/singlestoredb-dev` (MA + leaf).
2. Builds a small ODBC client linked against unixODBC and `libssodbca`.
3. Turns off `leaf_failure_detection`.
4. Opens a transaction, runs three `UPDATE`s, then `DROP TEMPORARY TABLE`.
5. Runs the create while the leaf is up, waits until it is executing,
   kills the leaf, then refuses leaf port 3307.
6. Exits 0 if the client is still blocked after `--wait-secs` (default 60)
   and `memsql.log` contains the error 1857 message. Exits 1 otherwise.

## Prerequisites

- Docker
- `gcc`, unixODBC (`libodbc`)
- `mysql` or `mariadb` client
- Built ANSI driver: `build/libssodbca.dylib` on macOS, `build/libssodbca.so` elsewhere
- `MEMSQL_PASSWORD` (used as container `ROOT_PASSWORD`)
- Optional: `SINGLESTORE_LICENSE`, `SINGLESTORE_VERSION`

## Run

```bash
export MEMSQL_PASSWORD='...'
./test/repro/plat8159_leaf_kill_repro.sh

# Keep the container for manual inspection:
./test/repro/plat8159_leaf_kill_repro.sh --keep-cluster --reuse-cluster
```

## Files

| File | Role |
|------|------|
| `plat8159_leaf_kill_repro.sh` | Cluster, leaf reset, verdict |
| `plat8159_leaf_kill_txn.c` | ODBC transaction client |

This is a manual investigation tool, not part of `ctest`.
