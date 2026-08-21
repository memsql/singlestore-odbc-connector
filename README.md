# SingleStore ODBC Driver
![logo](singlestore_logo_horizontal_color_on-white_rgb.png)

This is a GA release of the SingleStore ODBC Driver.
 
SingleStore ODBC Driver is released under version 2.1 of the GNU Lesser Public License.
License information can be found in the LICENSE file.

## Installing

- **Windows:** download a versioned MSI from GitHub Releases (for example `https://github.com/memsql/singlestore-odbc-connector/releases/download/v1.2.2/singlestore-connector-odbc-1.2.2-win64.msi`) and install it interactively, or silently for scripted rollout. See [wininstall/README.md](wininstall/README.md) for download URLs, `msiexec` options, install directories, and features.
- **Linux:** see [linuxinstall/README.md](linuxinstall/README.md) for UnixODBC setup.

## Building

See [BUILD.md](BUILD.md) for build instructions.

## Releasing

Pushing a `v*` version tag triggers an automated GitHub Pre-release with installers for Linux, macOS, and Windows. See [RELEASING.md](RELEASING.md) for the full process.
