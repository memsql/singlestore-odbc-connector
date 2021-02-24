$ErrorActionPreference = "Stop"

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

Invoke-Executable -ScriptBlock { choco install -y -r --no-progress wixtoolset } -ErrorAction Stop
refreshenv

Invoke-Executable -ScriptBlock { cmake -DCMAKE_BUILD_TYPE=$ENV:BUILD_TYPE -DCONC_WITH_UNIT_TESTS=Off -DCONC_WITH_MSI=OFF -DWITH_SSL=SCHANNEL . } -ErrorAction Stop
Invoke-Executable -ScriptBlock { cmake --build . --config $ENV:BUILD_TYPE --parallel 2 } -ErrorAction Stop

$msifile = Get-ChildItem C:\Users\circleci\project\wininstall\singlestore-connector-odbc*.msi | Select-Object -First 1
Invoke-Executable -ScriptBlock { msiexec.exe /i $msifile INSTALLDIR="C:\singlestore-odbc" /qn } -ErrorAction Stop

$oldpath = (Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH).path
$newpath = "$oldpath;C:\singlestore-odbc\SingleStore\SingleStore ODBC Driver 64-bit"
Set-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH -Value $newPath
(Get-ItemProperty -Path 'Registry::HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\Session Manager\Environment' -Name PATH).Path
