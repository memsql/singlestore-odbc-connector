$ErrorActionPreference = "Stop"

if (-not (Get-Command ctest -ErrorAction SilentlyContinue)) {
    New-Alias -Name ctest -Value "$Env:ProgramFiles\CMake\bin\ctest.exe"
}

New-Item -Path "HKCU:\Software\ODBC"
New-Item -Path "HKCU:\Software\ODBC\ODBC.INI"
$regPath = "HKCU:\Software\ODBC\ODBC.INI\$ENV:TEST_DSN"
New-Item -Path $regPath
New-ItemProperty -Path $regPath -Name "CONN_TIMEOUT" -Value "0"
New-ItemProperty -Path $regPath -Name "DATABASE" -Value "odbc_test"
New-ItemProperty -Path $regPath -Name "DESCRIPTION" -Value "SingleStore ODBC test"
New-ItemProperty -Path $regPath -Name "DRIVER" -Value $ENV:TEST_DRIVER
New-ItemProperty -Path $regPath -Name "OPTIONS" -Value "0"
New-ItemProperty -Path $regPath -Name "PORT" -Value $ENV:MEMSQL_PORT
New-ItemProperty -Path $regPath -Name "PWD" -Value $ENV:MEMSQL_PASSWORD
New-ItemProperty -Path $regPath -Name "SERVER" -Value $ENV:MEMSQL_HOST
New-ItemProperty -Path $regPath -Name "SSLVERIFY" -Value "0"
New-ItemProperty -Path $regPath -Name "TCPIP" -Value "1"
New-ItemProperty -Path $regPath -Name "UID" -Value $ENV:MEMSQL_USER
New-Item -Path "HKCU:\Software\ODBC\ODBC.INI\ODBC Data Sources"
New-ItemProperty -Path "HKCU:\Software\ODBC\ODBC.INI\ODBC Data Sources" -Name $ENV:TEST_DSN -Value $ENV:TEST_DRIVER

$env:TEST_SCHEMA="odbc_test"
$env:TEST_PORT=$ENV:MEMSQL_PORT
$env:TEST_PASSWORD=$ENV:MEMSQL_PASSWORD
$env:TEST_SERVER=$ENV:MEMSQL_HOST
$env:TEST_UID=$ENV:MEMSQL_USER

cd test
ctest -V
exit $LASTEXITCODE