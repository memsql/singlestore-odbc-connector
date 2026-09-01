# Releasing the SingleStore ODBC Driver

Pushing a version tag to GitHub triggers an automated release build.

## Prerequisites

- Changes for the release are merged to `master`
- CI tests on `master` are passing
- You have permission to push tags to the repository
- Azure Artifact Signing is configured for this repository (see [Code signing](#code-signing) below)

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

1. Builds and tests installers on Ubuntu, CentOS/RHEL, macOS, and Windows.
2. Signs the Windows driver and plugin DLLs and the MSI installers with Azure Artifact Signing.
3. Packages platform artifacts.
4. Creates a GitHub **Pre-release** with those artifacts attached.

The workflow first checks that the tag matches the version in `CMakeLists.txt`. A mismatch fails the job before builds start.

## Code signing

Windows release artifacts are signed via Azure Artifact Signing (Trusted Signing) using Workload Identity Federation.

An MSI signature does not cover the files the installer puts on disk, so `.github/scripts/build-windows.ps1` signs the driver and plugin DLLs before WiX packages them, and signs the MSI once it is built. The DLL list comes from the WiX sources, so files added to the installer are signed automatically. Signing runs only when `SIGN_WINDOWS_ARTIFACTS` is `true`, which the release workflow sets; the [Run tests](.github/workflows/test.yml) workflow leaves it off because it has no Azure credentials.

### Required GitHub Actions secrets

| Secret | Description |
| --- | --- |
| `AZURE_CLIENT_ID` | Application (client) ID of `github-singlestore-signing` |
| `AZURE_TENANT_ID` | Directory (tenant) ID |
| `AZURE_SUBSCRIPTION_ID` | Azure subscription ID used for signing |

### Required GitHub Actions variables

| Variable | Example value |
| --- | --- |
| `AZURE_SIGNING_ENDPOINT` | `https://eus.codesigning.azure.net` |
| `AZURE_SIGNING_ACCOUNT`  | `SingleStore` |
| `AZURE_SIGNING_PROFILE`  | `ConnectorsReleaseProfile` |

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


### Driver-Server Version Compatibility Matrix

After each release, add a row for the new version rather than copying an older row's engine list. While CI has no pinned engine matrix, take the list from the [EOL policy](https://docs.singlestore.com/db/v9.1/support/singlestore-software-end-of-life-eol-policy/) as of the new tag's date.

| Driver Version | Release date | Supported engine versions |
| -------------- | ------------ | ------------------------- |
| 1.2.2          | 2026-08-20   | 8.9, 9.0, 9.1 RC          |
| 1.2.1          | 2026-02-05   | 8.7, 8.9, 9.0             |
| 1.2.0          | 2025-10-21   | 8.5, 8.7, 8.9, 9.0        |
| 1.1.7          | 2024-11-13   | 7.8, 8.0, 8.1, 8.5, 8.7   |
| 1.1.6          | 2024-09-04   | 7.8, 8.0, 8.1             |
| 1.1.5          | 2024-02-21   | 7.8, 8.0, 8.1             |
| 1.1.4          | 2023-11-09   | 7.8, 8.0, 8.1             |
| 1.1.3          | 2023-06-15   | 7.5, 7.6, 7.8, 8.0        |
| 1.1.2          | 2023-02-10   | 7.5, 7.6, 7.8             |
| 1.1.1          | 2022-11-09   | 7.3, 7.5, 7.6, 7.8        |
| 1.1.0          | 2022-06-14   | 7.3, 7.5, 7.6, 7.8        |
| 1.0.8          | 2022-03-25   | 7.1, 7.3, 7.5, 7.6        |
| 1.0.7          | 2021-11-30   | 7.0, 7.1, 7.3, 7.5        |
| 1.0.6          | 2021-11-17   | 7.0, 7.1, 7.3, 7.5        |
| 1.0.5          | 2021-11-05   | 7.0, 7.1, 7.3, 7.5        |
| 1.0.4          | 2021-08-18   | 7.0, 7.1, 7.3, 7.5        |
| 1.0.3          | 2021-08-10   | 7.0, 7.1, 7.3, 7.5        |
| 1.0.2          | 2021-08-05   | 7.0, 7.1, 7.3, 7.5        |
| 1.0.1          | 2021-07-30   | 7.0, 7.1, 7.3             |
| 1.0.0          | 2021-07-15   | 7.0, 7.1, 7.3             |
| 0.8.2-beta     | 2021-04-09   | 7.0, 7.1, 7.3             |
| 0.8.1-beta     | 2021-03-11   | 7.0, 7.1, 7.3             |
| 0.8.0-beta     | 2021-02-26   | 7.0, 7.1, 7.3             |
