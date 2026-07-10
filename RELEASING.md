# Releasing the SingleStore ODBC Driver

Pushing a version tag to GitHub triggers an automated release build.

## Prerequisites

- Changes for the release are merged to `master`
- CI tests on `master` are passing
- You have permission to push tags to the repository

## Version format

Tags must match `vX.Y.Z` or `vX.Y.Z-<suffix>`:

| Tag | Example |
| --- | --- |
| `vX.Y.Z` | `v1.2.2` |
| `vX.Y.Z-<suffix>` | `v1.2.3-beta`, `v1.3.0-rc1` |

The version in `CMakeLists.txt` must match the tag. Use `bump-version.sh` to keep them in sync. The release workflow fails if the tag does not match `SS_ODBC_VERSION_*` in `CMakeLists.txt` (including quality: `ga` for `vX.Y.Z`, or the suffix for `vX.Y.Z-<suffix>`).

## Create a release

### Option 1: Helper script (recommended)

From the repository root on `master`:

```bash
./bump-version.sh 1.2.2
```

The script updates `CMakeLists.txt`, commits the change, creates tag `v1.2.2`, and pushes both to origin. That push starts the release workflow.

For a version with a suffix:

```bash
./bump-version.sh 1.2.3-beta
```

### Option 2: Manual tag

If the version in `CMakeLists.txt` is already correct:

```bash
git checkout master
git pull
git tag v1.2.2
git push origin v1.2.2
```

## What the workflow does

The [Publish installers](.github/workflows/publish.yml) workflow runs on every `v*` tag push and:

1. Builds and tests installers on Ubuntu, CentOS/RHEL, macOS, and Windows
2. Packages platform artifacts
3. Creates a GitHub **Pre-release** with those artifacts attached

The workflow first checks that the tag matches the version in `CMakeLists.txt`. A mismatch fails the job before builds start.

## Finalize the release

1. Open the pre-release on the [Releases](https://github.com/memsql/singlestore-odbc-connector/releases) page
2. Edit the release description as needed
3. Uncheck **Set as a pre-release** and check **Set as the latest release**
4. Save the release

## Artifacts

| Platform | Artifact |
| --- | --- |
| Linux (Ubuntu) | `singlestore-connector-odbc-<version>-ubuntu-amd64.tar.gz` |
| Linux (CentOS/RHEL) | `singlestore-connector-odbc-<version>-centos-amd64.tar.gz` |
| macOS | `singlestore-connector-odbc-<version>-macos-x86_64.pkg` |
| Windows (64-bit) | `singlestore-connector-odbc-<version>-win64.msi` |
| Windows (32-bit) | `singlestore-connector-odbc-<version>-win32.msi` |

Installation instructions: [SingleStore ODBC Driver docs](https://docs.singlestore.com/cloud/developer-resources/connect-with-application-development-tools/connect-with-odbc/the-singlestore-odbc-driver/).
