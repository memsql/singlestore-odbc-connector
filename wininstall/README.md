# Installing the Windows MSI

The Windows installer is a per-machine MSI. It registers the ANSI and Unicode ODBC drivers with the Windows Driver Manager. Run the installer from an elevated Command Prompt or PowerShell session (local Administrator or equivalent).

Package names follow `singlestore-connector-odbc-<version>-win64.msi` and `singlestore-connector-odbc-<version>-win32.msi`. Use the 64-bit MSI for 64-bit applications and the 32-bit MSI for 32-bit applications. Install both if you need both architectures on the same machine.

## Download a specific version

Release assets are published on GitHub. The tag is `v<version>`; the file name
uses `<version>` without the `v` prefix.

```
https://github.com/memsql/singlestore-odbc-connector/releases/download/v<version>/singlestore-connector-odbc-<version>-win64.msi
https://github.com/memsql/singlestore-odbc-connector/releases/download/v<version>/singlestore-connector-odbc-<version>-win32.msi
```

Example for `1.2.2` (64-bit):

```
https://github.com/memsql/singlestore-odbc-connector/releases/download/v1.2.2/singlestore-connector-odbc-1.2.2-win64.msi
```

Browse available versions at
https://github.com/memsql/singlestore-odbc-connector/releases.
For a pre-release tag such as `v1.2.3-beta`, use the same pattern with
`1.2.3-beta` in both the tag and the file name.

PowerShell (replace the version and architecture as needed):

```powershell
$Version = "1.2.2"   # or "1.2.3-beta"
$Arch    = "win64"   # or "win32"
$Msi     = "singlestore-connector-odbc-$Version-$Arch.msi"
$Url     = "https://github.com/memsql/singlestore-odbc-connector/releases/download/v$Version/$Msi"
$OutFile = Join-Path $env:TEMP $Msi
Invoke-WebRequest -Uri $Url -OutFile $OutFile -UseBasicParsing
```

Command Prompt (`curl.exe` ships with Windows 10 and later):

```bat
set VERSION=1.2.2
set ARCH=win64
set MSI=singlestore-connector-odbc-%VERSION%-%ARCH%.msi
curl.exe -L -o "%TEMP%\%MSI%" "https://github.com/memsql/singlestore-odbc-connector/releases/download/v%VERSION%/%MSI%"
```

Pin `$Version` / `%VERSION%` in deployment scripts so every machine gets the
same build. Do not use a floating "latest" URL unless you accept unplanned
upgrades.

Default install directories:

| Package | Directory |
| --- | --- |
| 64-bit | `C:\Program Files\SingleStore\SingleStore ODBC Driver 64-bit` |
| 32-bit | `C:\Program Files (x86)\SingleStore\SingleStore ODBC Driver` |

Registered driver names:

- `SingleStore ODBC ANSI Driver`
- `SingleStore ODBC Unicode Driver`

## Silent install

`/qn` runs with no UI. `/norestart` suppresses an automatic reboot if one is
required. Logging is optional but recommended for unattended deployments.

After downloading (see above), install with `msiexec`.

Command Prompt:

```bat
msiexec.exe /i "%TEMP%\%MSI%" /qn /norestart /l*v "%TEMP%\ssodbc-install.log"
```

PowerShell:

```powershell
$Log = Join-Path $env:TEMP "ssodbc-install.log"
$p = Start-Process -FilePath "msiexec.exe" -ArgumentList @(
    "/i", $OutFile,
    "/qn",
    "/norestart",
    "/l*v", $Log
) -Wait -PassThru
if ($p.ExitCode -notin 0, 3010) {
    throw "MSI install failed with exit code $($p.ExitCode). See $Log"
}
```

Exit code `0` is success. `3010` is success with a reboot required. Treat any
other non-zero code as failure.

`/qb` shows a basic progress bar and is useful while testing automation. Prefer
`/qn` for production scripts, SCCM, Intune, and GPO startup scripts.

## Public properties

Pass properties after the MSI path as `NAME=value`. Quote values that contain
spaces.

| Property | Default | Purpose |
| --- | --- | --- |
| `INSTALLFOLDER` | See default directories above | Directory that contains the driver DLLs |
| `INSTALLLEVEL` | `1` | Features whose Level is less than or equal to this value are installed. `1` is Typical (driver only). `1000` is Complete (driver plus debug symbols). |
| `ADDLOCAL` | Typical features | Comma-separated feature IDs to install. Overrides the default feature set. |
| `REMOVE` | *(none)* | Comma-separated feature IDs to omit or uninstall. |
| `ALLUSERS` | `1` (per-machine) | Do not change. The package is authored with `InstallScope="perMachine"`. |

Example: custom directory, typical (driver-only) feature set:

```bat
msiexec.exe /i singlestore-connector-odbc-VERSION-win64.msi /qn /norestart INSTALLFOLDER="C:\SingleStore\ODBC"
```

`INSTALLFOLDER` is the final driver directory, not a parent folder. Files such as
`ssodbcw.dll` are placed directly in that path. Authentication plugin DLLs go in
a `plugin` subdirectory of `INSTALLFOLDER`.

Do not use `TARGETDIR` or `INSTALLDIR` to choose the install path. Those names
are not the directory ID used by this MSI.

## Features

The MSI uses WiX `WixUI_Mondo` setup types. In the UI, Typical installs the
driver; Complete also installs PDB debug files.

| Feature ID | Level | Included in Typical (`INSTALLLEVEL=1`) | Contents |
| --- | --- | --- | --- |
| `ProductFeature` | 1 | Yes | Driver DLLs (`ssodbca.dll`, `ssodbcw.dll`), setup library (`ssodbcs.dll`), import libraries, authentication plugins, ODBC Driver Manager registration |
| `DebugFeature` | 2 | No | PDB files for the driver and setup libraries |
| `PluginsFeature` | 2 | No | PDB files for authentication plugins |

Authentication plugin DLLs are part of `ProductFeature` and are always installed
with the driver. `PluginsFeature` only adds plugin debug symbols.

Complete silent install (driver plus all PDBs):

```bat
msiexec.exe /i singlestore-connector-odbc-VERSION-win64.msi /qn /norestart INSTALLLEVEL=1000
```

Equivalent using feature names:

```bat
msiexec.exe /i singlestore-connector-odbc-VERSION-win64.msi /qn /norestart ADDLOCAL=ProductFeature,DebugFeature,PluginsFeature
```

Driver only, no debug symbols (same as the silent default):

```bat
msiexec.exe /i singlestore-connector-odbc-VERSION-win64.msi /qn /norestart ADDLOCAL=ProductFeature
```

## Repair, upgrade, and uninstall

Repair the current install:

```bat
msiexec.exe /f singlestore-connector-odbc-VERSION-win64.msi /qn /norestart
```

A newer MSI of the same architecture upgrades in place (`MajorUpgrade`). Install
the new package with the same silent options; you do not need to uninstall first.

Uninstall by product MSI:

```bat
msiexec.exe /x singlestore-connector-odbc-VERSION-win64.msi /qn /norestart
```

Uninstall by ProductCode (useful when the original MSI file is not on the
machine). Look up the code under
`HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall` (and
`HKLM\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall` for
32-bit on 64-bit Windows), then:

```bat
msiexec.exe /x {PRODUCT-CODE-GUID} /qn /norestart
```

## Scripted rollout notes

- The process must be elevated. Per-user (`ALLUSERS=2`) install is not supported.
- Pin the MSI version in your deployment package so every machine gets the same build.
- Capture `/l*v` logs centrally on first-wave installs.
- After install, confirm the drivers in ODBC Data Source Administrator (the 64-bit
  or 32-bit `odbcad32.exe` that matches the MSI) or with:

  ```bat
  reg query "HKLM\SOFTWARE\ODBC\ODBCINST.INI\SingleStore ODBC Unicode Driver"
  ```

- Adding the install directory to `PATH` is not required for ODBC applications.
  Applications load the driver through the Driver Manager using the registered
  driver name.
- Group Policy, Microsoft Intune, SCCM/ConfigMgr, and similar tools can deploy
  the MSI as a standard Windows Installer package using the same properties
  shown above.
