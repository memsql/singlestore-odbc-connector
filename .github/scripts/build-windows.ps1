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

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    New-Alias -Name cmake -Value "$Env:ProgramFiles\CMake\bin\cmake.exe"
}

# Due to complications related to invoking executables within the powershell, this is the handy wrapper around the
# exe calls which exits if a non-zero code is returned (equivalent of 'set -e' in bash).
function Invoke-Executable {
    param (
        [scriptblock]$ScriptBlock,
        [string]$ErrorAction = $ErrorActionPreference
    )
    & @ScriptBlock
    if (($lastexitcode -ne 0) -and $ErrorAction -eq "Stop") {
        exit $lastexitcode
    }
}

# The MSI embeds a copy of every file it installs, so the payload has to be signed before
# light.exe packs it. The file list is read from the WiX sources to keep it in sync with
# the installer contents.
function Get-MsiPayloadDlls {
    $defines = @{}
    foreach ($match in (Select-String -Path "wininstall\binaries_dir.xml" -Pattern '<\?define\s+(\w+)="([^"]+)"' -AllMatches).Matches) {
        $defines[$match.Groups[1].Value] = $match.Groups[2].Value
    }

    $dlls = @()
    foreach ($match in (Select-String -Path "wininstall\ss_odbc.xml" -Pattern 'Source="\$\(var\.(\w+)\)/([^"]+\.dll)"' -AllMatches).Matches) {
        $dlls += "$($defines[$match.Groups[1].Value])/$($match.Groups[2].Value)"
    }

    $dlls = $dlls | Sort-Object -Unique
    if ($dlls.Count -eq 0) {
        throw "Found no DLLs to sign in wininstall\ss_odbc.xml"
    }
    return $dlls
}

$SignArtifacts = $ENV:SIGN_WINDOWS_ARTIFACTS -eq 'true'

Invoke-Executable -ScriptBlock { choco install -y -r --no-progress wixtoolset } -ErrorAction Stop
refreshenv
# TODO: PLAT-6167 find actual $env:WIX dynamically
if (-not (Test-Path env:WIX)) { $env:WIX = "C:\Program Files (x86)\WiX Toolset v3.11\" }

if ($ENV:ARCH_BITS -eq 32)
{
    $CmakeArch = "Win32"
} else {
    $CmakeArch = "x64"
}

Invoke-Executable -ScriptBlock { cmake -A $CmakeArch -DCMAKE_BUILD_TYPE=$ENV:BUILD_TYPE -DCONC_WITH_UNIT_TESTS=Off -DCONC_WITH_MSI=OFF -DWITH_SSL=SCHANNEL -DCMAKE_INSTALL_PREFIX="C:/Program Files/SingleStore/SingleStore ODBC Driver 64-bit" -DIS_ON_S2MS=1 . } -ErrorAction Stop

if ($SignArtifacts)
{
    # ODBC_WIX depends on every driver and plugin that goes into the MSI, so building it
    # first produces the payload while the installer itself is not assembled yet
    Invoke-Executable -ScriptBlock { cmake --build . --config $ENV:BUILD_TYPE --parallel 2 --target ODBC_WIX } -ErrorAction Stop
    $payload = Get-MsiPayloadDlls
    & .github\scripts\sign-windows.ps1 -Files $payload
}

Invoke-Executable -ScriptBlock { cmake --build . --config $ENV:BUILD_TYPE --parallel 2 } -ErrorAction Stop

$msifile = Get-ChildItem "wininstall\singlestore-connector-odbc*.msi" | Select-Object -First 1

if ($SignArtifacts)
{
    # A relink in the build above would silently strip the signatures added earlier
    $unsigned = $payload | Where-Object { (Get-AuthenticodeSignature $_).Status -eq 'NotSigned' }
    if ($unsigned)
    {
        throw "MSI payload lost its signature during the build: $($unsigned -join ', ')"
    }
    & .github\scripts\sign-windows.ps1 -Files $msifile.FullName
}

Invoke-Executable -ScriptBlock { msiexec.exe /i $msifile INSTALLDIR="C:\singlestore-odbc" /qn } -ErrorAction Stop

$oldpath = (Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH).path
$newpath = "$oldpath;C:\singlestore-odbc\SingleStore\SingleStore ODBC Driver 64-bit"
Set-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH -Value $newPath
(Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH).Path
