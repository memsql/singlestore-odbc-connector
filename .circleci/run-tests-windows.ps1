$ErrorActionPreference = "Stop"
# ************************************************************************************
#   Copyright (c) 2021 SingleStore, Inc.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public
#   License along with this library; if not see <http://www.gnu.org/licenses>
#   or write to the Free Software Foundation, Inc.,
#   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
# *************************************************************************************/

if (-not (Get-Command ctest -ErrorAction SilentlyContinue)) {
    New-Alias -Name ctest -Value "$Env:ProgramFiles\CMake\bin\ctest.exe"
}

New-Item -Path "HKCU:\Software\ODBC" -Force
New-Item -Path "HKCU:\Software\ODBC\ODBC.INI" -Force
$regPath = "HKCU:\Software\ODBC\ODBC.INI\$ENV:TEST_DSN"
New-Item -Path $regPath -Force
Set-ItemProperty -Path $regPath -Name "CONN_TIMEOUT" -Value "0"
Set-ItemProperty -Path $regPath -Name "DATABASE" -Value "odbc_test"
Set-ItemProperty -Path $regPath -Name "DESCRIPTION" -Value "SingleStore ODBC test"
Set-ItemProperty -Path $regPath -Name "DRIVER" -Value $ENV:TEST_DRIVER
Set-ItemProperty -Path $regPath -Name "OPTIONS" -Value "0"
Set-ItemProperty -Path $regPath -Name "PORT" -Value $ENV:MEMSQL_PORT
Set-ItemProperty -Path $regPath -Name "PWD" -Value $ENV:MEMSQL_PASSWORD
Set-ItemProperty -Path $regPath -Name "SSLVERIFY" -Value "0"
Set-ItemProperty -Path $regPath -Name "TCPIP" -Value "1"
Set-ItemProperty -Path $regPath -Name "UID" -Value $ENV:MEMSQL_USER
New-Item -Path "HKCU:\Software\ODBC\ODBC.INI\ODBC Data Sources" -Force
Set-ItemProperty -Path "HKCU:\Software\ODBC\ODBC.INI\ODBC Data Sources" -Name $ENV:TEST_DSN -Value $ENV:TEST_DRIVER

$env:TEST_SCHEMA="odbc_test"
$env:TEST_PORT=$ENV:MEMSQL_PORT
$env:TEST_PASSWORD=$ENV:MEMSQL_PASSWORD
$env:TEST_UID=$ENV:MEMSQL_USER
if ($env:DRIVER_TYPE -imatch "unicode")
{
    $env:TEST_SERVER=$ENV:HOST_WINDOWS_UNICODE
    Set-ItemProperty -Path $regPath -Name "SERVER" -Value $ENV:HOST_WINDOWS_UNICODE
    Add-Content -Path $env:windir\System32\drivers\etc\hosts -Value "`n$ENV:HOST_WINDOWS_UNICODE`ttest-memsql-server" -Force
    Add-Content -Path $env:windir\System32\drivers\etc\hosts -Value "`n$ENV:HOST_WINDOWS_UNICODE`ttest-memsql-cluster" -Force
    Add-Content -Path $env:windir\System32\drivers\etc\hosts -Value "`n$ENV:HOST_WINDOWS_UNICODE`tsinglestore.test.com" -Force

} else {
    $env:TEST_SERVER=$ENV:HOST_WINDOWS_ANSI
    Set-ItemProperty -Path $regPath -Name "SERVER" -Value $ENV:HOST_WINDOWS_ANSI
    Add-Content -Path $env:windir\System32\drivers\etc\hosts -Value "`n$ENV:HOST_WINDOWS_ANSI`ttest-memsql-server" -Force
    Add-Content -Path $env:windir\System32\drivers\etc\hosts -Value "`n$ENV:HOST_WINDOWS_ANSI`ttest-memsql-cluster" -Force
    Add-Content -Path $env:windir\System32\drivers\etc\hosts -Value "`n$ENV:HOST_WINDOWS_ANSI`tsinglestore.test.com" -Force

}
Add-Content -Path $env:windir\my.ini -Value "`n[mysqld]`nplugin-load-add=authentication_pam.so`n[client]`nprotocol = TCP`n[odbc]`ndatabase = odbc_test_mycnf" -Force

cd test
ctest -V
cd ..
exit $LASTEXITCODE
