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
CONNECT_ATTEMPTS = 8
CONNECT_BASE_DELAY_SEC = 5

# Allowlists of required substrings (case-insensitive). Add newly observed
# flaky messages here.
# Example: ManagementError: 500 (... could not acquire lock within duration)
RETRYABLE_CREATE_WORKSPACE_GROUP = (
    "could not acquire lock",
)
# Example: Can't connect to MySQL server on 'svc-....svc.singlestore.com' (timed out)
RETRYABLE_CONNECT = (
    "can't connect to mysql server",
)


def _matches_known_errors(exc, known_errors):
    return any(err in str(exc).lower() for err in known_errors)


def _is_retryable_create_group(exc):
    return _matches_known_errors(exc, RETRYABLE_CREATE_WORKSPACE_GROUP)


def _is_retryable_connect(exc):
    return _matches_known_errors(exc, RETRYABLE_CONNECT)


def _retry(operation_name, operation_fn, is_retryable_fn, attempts, base_delay_sec):
    last_error = None
    for attempt in range(1, attempts + 1):
        try:
            return operation_fn()
        except Exception as exc:
            last_error = exc
            if attempt == attempts or not is_retryable_fn(exc):
                raise
            delay = base_delay_sec * (2 ** (attempt - 1))
            print(
                "Retryable S2MS error during {} "
                "(attempt {}/{}): {}; sleeping {}s".format(
                    operation_name, attempt, attempts, exc, delay
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

    workspace_group = _retry(
        "create_workspace_group",
        create_group,
        _is_retryable_create_group,
        CREATE_GROUP_ATTEMPTS,
        CREATE_GROUP_BASE_DELAY_SEC,
    )
    with open(WORKSPACE_GROUP_ID_FILE, "w") as f:
        f.write(workspace_group.id)

    workspace = workspace_group.create_workspace(
        name=WORKSPACE_NAME, size="S-00", wait_on_active=True, wait_timeout=1200
    )
    with open(WORKSPACE_ENDPOINT_FILE, "w") as f:
        f.write(workspace.endpoint)

    conn = _retry(
        "workspace.connect",
        lambda: workspace.connect(user="admin", port=3306, password=SQL_USER_PASSWORD),
        _is_retryable_connect,
        CONNECT_ATTEMPTS,
        CONNECT_BASE_DELAY_SEC,
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
