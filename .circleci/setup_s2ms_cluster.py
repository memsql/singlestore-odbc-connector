import os
import sys
import singlestoredb as s2
import uuid


SQL_USER_PASSWORD = os.getenv("MEMSQL_PASSWORD")  # project UI env-var reference
S2MS_API_KEY = os.getenv("S2MS_API_KEY")  # project UI env-var reference

WORKSPACE_GROUP_BASE_NAME = "ODBC-connector-ci-tests"
WORKSPACE_NAME = "tests"
AWS_EU_CENTRAL_REGION = "7e7ffd27-20f7-44b6-87e6-e72828a81ac7"
WORKSPACE_ENDPOINT_FILE = "WORKSPACE_ENDPOINT_FILE"
WORKSPACE_GROUP_ID_FILE = "WORKSPACE_GROUP_ID_FILE"


def cmd_start(workspace_manager):
    w_group_name = WORKSPACE_GROUP_BASE_NAME + "-" + uuid.uuid4().hex
    workspace_group = workspace_manager.create_workspace_group(
        name=w_group_name,
        region=AWS_EU_CENTRAL_REGION,
        firewall_ranges=["0.0.0.0/0"],
        admin_password=SQL_USER_PASSWORD,
        expires_at="1h30m"
    )
    with open(WORKSPACE_GROUP_ID_FILE, "w") as f:
        f.write(workspace_group.id)
    print("Created workspace group {}".format(w_group_name))

    workspace = workspace_group.create_workspace(name=WORKSPACE_NAME, size="S-00", wait_on_active=True, wait_timeout=1200)
    with open(WORKSPACE_ENDPOINT_FILE, "w") as f:
        f.write(workspace.endpoint)

    conn = workspace.connect(user='admin', port=3306, password=SQL_USER_PASSWORD)
    cursor = conn.cursor()
    cursor.execute("SET GLOBAL data_conversion_compatibility_level = '6.0'")
    cursor.close()

    if db_name is not None:
        cursor = conn.cursor()
        cursor.execute("CREATE DATABASE " + db_name)
        cursor.close()
    
    conn.close()


def cmd_terminate(workspace_manager):
    with open(WORKSPACE_GROUP_ID_FILE, "r") as f:
        workspace_group_id = f.read()
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
        cmd_start(workspace_manager)

    elif command == "terminate":
        cmd_terminate(workspace_manager)
