if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    New-Alias -Name cmake -Value "$Env:ProgramFiles\CMake\bin\cmake.exe"
    New-Alias -Name ctest -Value "$Env:ProgramFiles\CMake\bin\ctest.exe"
}

cd test
ctest -V