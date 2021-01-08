if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    New-Alias -Name cmake -Value "$Env:ProgramFiles\CMake\bin\cmake.exe"
}

choco install -y -r --no-progress wixtoolset
refreshenv

cmake -DCMAKE_BUILD_TYPE=$ENV:BUILD_TYPE -DCONC_WITH_UNIT_TESTS=Off -DCONC_WITH_MSI=OFF -DWITH_SSL=SCHANNEL .
cmake --build . --config $ENV:BUILD_TYPE --parallel 2

New-Item -Path "HKCU:\Software\ODBC"
New-Item -Path "HKCU:\Software\ODBC\ODBC.INI"
$regPath = "HKCU:\Software\ODBC\ODBC.INI\maodbc_test"
New-Item -Path $regPath
New-ItemProperty -Path $regPath -Name "CONN_TIMEOUT" -Value "0"
New-ItemProperty -Path $regPath -Name "DATABASE" -Value "odbc_test"
New-ItemProperty -Path $regPath -Name "DESCRIPTION" -Value "MariaDB ODBC test"
New-ItemProperty -Path $regPath -Name "DRIVER" -Value "MariaDB ODBC 3.1 Driver"
New-ItemProperty -Path $regPath -Name "OPTIONS" -Value "0"
New-ItemProperty -Path $regPath -Name "PORT" -Value $ENV:MEMSQL_PORT
New-ItemProperty -Path $regPath -Name "PWD" -Value $ENV:MEMSQL_PASSWORD
New-ItemProperty -Path $regPath -Name "SERVER" -Value $ENV:MEMSQL_HOST
New-ItemProperty -Path $regPath -Name "SSLVERIFY" -Value "0"
New-ItemProperty -Path $regPath -Name "TCPIP" -Value "1"
New-ItemProperty -Path $regPath -Name "UID" -Value $ENV:MEMSQL_USER
New-Item -Path "HKCU:\Software\ODBC\ODBC.INI\ODBC Data Sources"
New-ItemProperty -Path "HKCU:\Software\ODBC\ODBC.INI\ODBC Data Sources" -Name "maodbc_test" -Value "MariaDB ODBC 3.1 Driver"

refreshenv

$msifile = Get-ChildItem C:\Users\circleci\project\wininstall\mariadb-connector-odbc*.msi | Select-Object -First 1
msiexec.exe /i $msifile INSTALLDIR="C:\mariadb-odbc" /qn

$oldpath = (Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH).path
$newpath = "$oldpath;C:\maria-odbc\MariaDB\MariaDB ODBC Driver 64-bit"
Set-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH -Value $newPath
(Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH).Path
