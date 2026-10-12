# SingleStore ODBC Driver
![logo](singlestore_logo_horizontal_color_on-white_rgb.png)

This is a GA release of the SingleStore ODBC Driver.
 
SingleStore ODBC Driver is released under version 2.1 of the GNU Lesser Public License.
License information can be found in the LICENSE file.

## Installing

This branch publishes Linux builds linked against OpenSSL 1.1. See [linuxinstall/README.md](linuxinstall/README.md) for UnixODBC setup and the OpenSSL 1.1 runtime libraries.

## Building

See [BUILD.md](BUILD.md) for build instructions. Linux builds must use OpenSSL 1.1.

## Releasing

Pushing a `v*` version tag on `release/1.1` triggers an automated GitHub Pre-release with Linux installers. See [RELEASING.md](RELEASING.md) for the full process.
