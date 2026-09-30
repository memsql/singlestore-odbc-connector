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
closing a connection to a leaf...". The client does not return.

`plat8159_leaf_kill_repro.sh` holds that statement with the ODBC driver.
`plat8159_leaf_kill_mysql.sh` holds it with the `mysql` CLI. Both scripts
use container `singlestore-plat8159` and host port 5606. Run one at a time.

## What it does

1. Starts `ghcr.io/singlestore-labs/singlestoredb-dev` (MA + leaf).
2. The ODBC script builds a small client linked against unixODBC and
   `libssodbca`. The mysql script uses the `mysql` CLI.
3. Turns off `leaf_failure_detection`.
4. Opens a transaction, runs three `UPDATE`s, then `DROP TEMPORARY TABLE`.
5. Runs the create while the leaf is up, waits until it is executing,
   kills the leaf, then refuses leaf port 3307.
6. Exits 0 if the client is still blocked after `--wait-secs` (default 60)
   and `memsql.log` contains the error 1857 message. Exits 1 otherwise.

## Prerequisites

- Docker
- `mysql` or `mariadb` client
- `MEMSQL_PASSWORD` (used as container `ROOT_PASSWORD`)
- Optional: `SINGLESTORE_LICENSE`, `SINGLESTORE_VERSION`
- ODBC script only: `gcc`, unixODBC (`libodbc`), and a built ANSI driver
  (`build/libssodbca.dylib` on macOS, `build/libssodbca.so` elsewhere)

## Run

```bash
export MEMSQL_PASSWORD='...'

# mysql CLI client
./test/repro/plat8159_leaf_kill_mysql.sh

# ODBC client
./test/repro/plat8159_leaf_kill_repro.sh

# Keep the container for manual inspection:
./test/repro/plat8159_leaf_kill_mysql.sh --keep-cluster --reuse-cluster
```

Exit 0 means the client was still blocked after `--wait-secs` (default 60)
and `memsql.log` contains the error 1857 message.

## Files

| File | Role |
|------|------|
| `plat8159_leaf_kill_repro.sh` | ODBC client, cluster, leaf reset, verdict |
| `plat8159_leaf_kill_txn.c` | ODBC transaction client |
| `plat8159_leaf_kill_mysql.sh` | mysql CLI client, same server sequence |

This is a manual investigation tool, not part of `ctest`.
