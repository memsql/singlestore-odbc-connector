if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    New-Alias -Name cmake -Value "$Env:ProgramFiles\CMake\bin\cmake.exe"
    New-Alias -Name ctest -Value "$Env:ProgramFiles\CMake\bin\ctest.exe"
}

choco install -y -r --no-progress wixtoolset
refreshenv

cmake -A Win32 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCONC_WITH_MSI=OFF -DWITH_SSL=SCHANNEL .
cmake --build . --config RelWithDebInfo --parallel 2

msiexec.exe /i wininstall\mariadb-connector-odbc-3.1.10-win32.msi

Add-OdbcDsn -Name "maodbc_test" -DriverName "MariaDB ODBC 3.1 Driver" -DsnType "System" -SetPropertyValue @("Server=localhost", "PORT=3306", "Database=test")
New-Item -Path "HKCU:\Software\ODBC"
New-Item -Path "HKCU:\Software\ODBC\ODBC.INI"
$regPath = "HKCU:\Software\ODBC\ODBC.INI\maodbc_test"
New-Item -Path $regPath
New-ItemProperty -Path $regPath -Name "CONN_TIMEOUT" -Value "0"
New-ItemProperty -Path $regPath -Name "DATABASE" -Value "test"
New-ItemProperty -Path $regPath -Name "DESCRIPTION" -Value "MariaDB ODBC test"
New-ItemProperty -Path $regPath -Name "Driver" -Value "MariaDB ODBC 3.1 Driver"
New-ItemProperty -Path $regPath -Name "OPTIONS" -Value "0"
New-ItemProperty -Path $regPath -Name "PORT" -Value "0"
New-ItemProperty -Path $regPath -Name "PWD" -Value ""
New-ItemProperty -Path $regPath -Name "SERVER" -Value "localhost"
New-ItemProperty -Path $regPath -Name "SSLVERIFY" -Value "0"
New-ItemProperty -Path $regPath -Name "TCPIP" -Value "1"
New-ItemProperty -Path $regPath -Name "UID" -Value "root"
New-Item -Path "HKCU:\Software\ODBC\ODBC.INI\ODBC Data Sources"
New-ItemProperty -Path "HKCU:\Software\ODBC\ODBC.INI\ODBC Data Sources" -Name "maodbc_test" -Value "MariaDB ODBC 3.1 Driver"

cd test
ctest -V