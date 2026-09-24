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
WORKSPACE_WAIT_TIMEOUT_SEC = 1200
WORKSPACE_WAIT_INTERVAL_SEC = 10


def _exc_msg(exc):
    return str(exc).lower()


def _is_transient_network_error(exc):
    """Connection drops / timeouts talking to the S2MS API or workspace host."""
    if isinstance(exc, (ConnectionError, TimeoutError)):
        return True
    msg = _exc_msg(exc)
    return (
        "timed out" in msg
        or "timeout" in msg
        or "connection reset" in msg
        or "connection aborted" in msg
        or "connection error" in msg
    )


def _is_lock_contention_error(exc):
    return "could not acquire lock" in _exc_msg(exc)


def _is_workspace_name_conflict(exc):
    msg = _exc_msg(exc)
    return (
        "already exists" in msg
        or "name conflict" in msg
        or "duplicate" in msg
        or "conflict" in msg and "workspace" in msg
    )


def _is_retryable_create_group(exc):
    # Group names are unique per attempt (uuid); only lock races and transport flakes.
    return _is_lock_contention_error(exc) or _is_transient_network_error(exc)


def _is_retryable_create_workspace(exc):
    # Fixed workspace name "tests": resume an in-flight workspace on retry rather
    # than treating name conflicts as fatal. Do not retry lock contention here —
    # the group already exists.
    return _is_transient_network_error(exc)


def _is_retryable_connect(exc):
    # DB connect to a freshly Active workspace: network / handshake flakes only.
    return _is_transient_network_error(exc)


def _retry(operation_name, fn, is_retryable):
    last_error = None
    for attempt in range(1, CREATE_GROUP_ATTEMPTS + 1):
        try:
            return fn()
        except Exception as exc:
            last_error = exc
            if attempt == CREATE_GROUP_ATTEMPTS or not is_retryable(exc):
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


def _reusable_workspace(workspace_group):
    # Terminated names can be reused. Anything else still owns the name.
    workspaces = getattr(workspace_group, "workspaces", None) or []
    for workspace in workspaces:
        if workspace.name != WORKSPACE_NAME:
            continue
        if (workspace.state or "").lower() == "terminated":
            continue
        return workspace
    return None


def _wait_for_active_workspace(workspace):
    # Same bound as create_workspace(wait_on_active=True). A hard wait timeout
    # raises RuntimeError (not retryable) so we do not stack 8x full waits.
    # Transient refresh/network errors propagate and may be retried by the caller.
    deadline = time.monotonic() + WORKSPACE_WAIT_TIMEOUT_SEC
    while True:
        state = (workspace.state or "").lower()
        if state == "active":
            return workspace
        if state in ("failed", "terminated"):
            raise RuntimeError(
                "Workspace {} entered state {}".format(workspace.name, workspace.state)
            )
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise RuntimeError(
                "Exceeded waiting time for workspace {} to become Active "
                "(state {})".format(workspace.name, workspace.state)
            )
        time.sleep(min(WORKSPACE_WAIT_INTERVAL_SEC, remaining))
        workspace.refresh()


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
        "create_workspace_group", create_group, _is_retryable_create_group
    )
    with open(WORKSPACE_GROUP_ID_FILE, "w") as f:
        f.write(workspace_group.id)

    def create_workspace():
        # create_workspace is not idempotent. A reset or timeout while
        # wait_on_active is polling leaves the fixed name "tests" in place,
        # and a blind recreate fails with a name conflict. Resume that workspace.
        existing = _reusable_workspace(workspace_group)
        if existing is not None:
            print(
                "Workspace {} already exists (state {}); waiting for Active".format(
                    WORKSPACE_NAME, existing.state
                )
            )
            return _wait_for_active_workspace(existing)
        try:
            return workspace_group.create_workspace(
                name=WORKSPACE_NAME,
                size="S-00",
                wait_on_active=True,
                wait_timeout=WORKSPACE_WAIT_TIMEOUT_SEC,
            )
        except Exception as exc:
            if not _is_workspace_name_conflict(exc):
                raise
            existing = _reusable_workspace(workspace_group)
            if existing is None:
                raise
            print(
                "Workspace {} create conflict (state {}); waiting for Active".format(
                    WORKSPACE_NAME, existing.state
                )
            )
            return _wait_for_active_workspace(existing)

    workspace = _retry(
        "create_workspace", create_workspace, _is_retryable_create_workspace
    )
    with open(WORKSPACE_ENDPOINT_FILE, "w") as f:
        f.write(workspace.endpoint)

    conn = _retry(
        "workspace.connect",
        lambda: workspace.connect(user="admin", port=3306, password=SQL_USER_PASSWORD),
        _is_retryable_connect,
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
