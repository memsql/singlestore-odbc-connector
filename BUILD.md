# Building the Connector/ODBC

Following are build instructions for various operating systems.

## Windows

Prior to start building on Windows you need to have following tools installed:
- Microsoft Visual Studio 16 2019 https://visualstudio.microsoft.com/downloads/
- Git https://git-scm.com/download/win
- CMake https://cmake.org/download/
- WIX (Windows Installation eXperience) https://wixtoolset.org/
Reboot after installing the prerequisites.

```
git clone https://github.com/memsql/singlestore-odbc-connector.git
cd singlestore-odbc-connector
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCONC_WITH_MSI=OFF -DWITH_SSL=SCHANNEL .
cmake --build . --config RelWithDebInfo
msiexec.exe /i ${SINGLESTORE_ODBC_ROOT}\wininstall\singestore-connector-odbc-${CURRENT_VERSION}-win64.msi

The driver will be installed in the C:\Program Files\... directory.
To override the installation dir use the following:
msiexec.exe /i ${SINGLESTORE_ODBC_ROOT}\wininstall\singestore-connector-odbc-${CURRENT_VERSION}-win64.msi TARGETDIR="C:\path\to\dir"

To run the tests add the path to the ssodbc.dll to PATH and refresh your cmd and reopen the programs
to load the new env (or simply reboot).
```

**NOTE**: If you use CLion on Windows and want to use the MSVC generator to build the project,
go to File->Settings->CMake and for the build target put the following in CMake options:

```
-G "Visual Studio 16 2019" -DCONC_WITH_UNIT_TESTS=Off -DCONC_WITH_MSI=OFF -DWITH_SSL=SCHANNEL
```

## CentOS

```
sudo yum -y install git cmake make gcc openssl-devel unixODBC unixODBC-devel
git clone https://github.com/memsql/singlestore-odbc-connector.git
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_SSL=OPENSSL
cmake --build . --config RelWithDebInfo
sudo make install
```

## Debian & Ubuntu

```
sudo apt-get update
sudo apt-get install -y git cmake make gcc libssl-dev unixodbc-dev
git clone https://github.com/memsql/singlestore-odbc-connector.git
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_SSL=OPENSSL
cmake --build . --config RelWithDebInfo
sudo make install
```
