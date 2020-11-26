if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    New-Alias -Name ctest -Value "$Env:ProgramFiles\CMake\bin\ctest.exe"
}

cd test
ctest -V -u $ENV:MEMSQL_USER -p $ENV:MEMSQL_PASSWORD -P $ENV:MEMSQL_PORT -S $ENV:MEMSQL_HOST -s "test"