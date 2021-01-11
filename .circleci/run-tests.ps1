if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    New-Alias -Name ctest -Value "$Env:ProgramFiles\CMake\bin\ctest.exe"
}

$env:TEST_SCHEMA=""
$env:TEST_PORT=$ENV:MEMSQL_PORT
$env:TEST_PASSWORD=$ENV:MEMSQL_PASSWORD
$env:TEST_SERVER=$ENV:MEMSQL_HOST
$env:TEST_UID=$ENV:MEMSQL_USER

cd test
ctest -V
exit $LASTEXITCODE