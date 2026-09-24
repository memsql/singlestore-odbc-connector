import os
import sys
import time
import uuid

import singlestoredb as s2


SQL_USER_PASSWORD = os.getenv("MEMSQL_PASSWORD")  # project UI env-var reference
S2MS_API_KEY = os.getenv("S2MS_API_KEY")  # project UI env-var reference

WORKSPACE_GROUP_BASE_NAME = "ODBC-connector-ci-tests"
WORKSPACE_NAME = "tests"
WORKSPACE_ENDPOINT_FILE = "WORKSPACE_ENDPOINT_FILE"
WORKSPACE_GROUP_ID_FILE = "WORKSPACE_GROUP_ID_FILE"

# Parallel CI jobs often race on S2MS workspace-group creation and hit
# transient "could not acquire lock" 500s or connect timeouts.
CREATE_GROUP_ATTEMPTS = 8
CREATE_GROUP_BASE_DELAY_SEC = 5


def _is_retryable_s2ms_error(exc):
    msg = str(exc).lower()
    return (
        "could not acquire lock" in msg
        or "timed out" in msg
        or "timeout" in msg
        or "connection reset" in msg
        or "connection aborted" in msg
        or "connection error" in msg
        or "protocolerror" in msg
        or "500" in msg
    )


def _retry(operation_name, fn):
    last_error = None
    for attempt in range(1, CREATE_GROUP_ATTEMPTS + 1):
        try:
            return fn()
        except Exception as exc:
            last_error = exc
            if attempt == CREATE_GROUP_ATTEMPTS or not _is_retryable_s2ms_error(exc):
                raise
            delay = CREATE_GROUP_BASE_DELAY_SEC * (2 ** (attempt - 1))
            print(
                "Retryable S2MS error during {} "
                "(attempt {}/{}): {}; sleeping {}s".format(
                    operation_name, attempt, CREATE_GROUP_ATTEMPTS, exc, delay
                )
            )
            time.sleep(delay)
    raise last_error


def cmd_start(workspace_manager, db_name=None):
    region = None
    for reg in workspace_manager.regions:
        if "US" in reg.name:
            region = reg
            break
    if region is None:
        raise RuntimeError("No US S2MS region available")

    def create_group():
        w_group_name = WORKSPACE_GROUP_BASE_NAME + "-" + uuid.uuid4().hex
        workspace_group = workspace_manager.create_workspace_group(
            name=w_group_name,
            region=region.id,
            firewall_ranges=["0.0.0.0/0"],
            admin_password=SQL_USER_PASSWORD,
            expires_at="1h30m",
        )
        print("Created workspace group {}".format(w_group_name))
        return workspace_group

    workspace_group = _retry("create_workspace_group", create_group)
    with open(WORKSPACE_GROUP_ID_FILE, "w") as f:
        f.write(workspace_group.id)

    workspace = _retry(
        "create_workspace",
        lambda: workspace_group.create_workspace(
            name=WORKSPACE_NAME, size="S-00", wait_on_active=True, wait_timeout=1200
        ),
    )
    with open(WORKSPACE_ENDPOINT_FILE, "w") as f:
        f.write(workspace.endpoint)

    conn = _retry(
        "workspace.connect",
        lambda: workspace.connect(user="admin", port=3306, password=SQL_USER_PASSWORD),
    )

    cursor = conn.cursor()
    cursor.execute("SET GLOBAL data_conversion_compatibility_level = '6.0'")
    cursor.close()

    if db_name is not None:
        cursor = conn.cursor()
        cursor.execute("CREATE DATABASE " + db_name)
        cursor.close()

    conn.close()


def cmd_terminate(workspace_manager):
    if not os.path.exists(WORKSPACE_GROUP_ID_FILE):
        print("No workspace group id file; nothing to terminate")
        return

    with open(WORKSPACE_GROUP_ID_FILE, "r") as f:
        workspace_group_id = f.read().strip()
    if not workspace_group_id:
        print("Empty workspace group id file; nothing to terminate")
        return

    workspace_group = workspace_manager.get_workspace_group(workspace_group_id)

    for workspace in workspace_group.workspaces:
        workspace.terminate(wait_on_terminated=True)
    workspace_group.terminate()


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Not enough arguments to start/terminate cluster!")
        exit(1)
    command = sys.argv[1]
    db_name = None
    if len(sys.argv) > 2:
        db_name = sys.argv[2]

    workspace_manager = s2.manage_workspaces(access_token=S2MS_API_KEY)

    if command == "start":
        cmd_start(workspace_manager, db_name)

    elif command == "terminate":
        cmd_terminate(workspace_manager)
