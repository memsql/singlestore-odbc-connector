if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    New-Alias -Name cmake -Value "$Env:ProgramFiles\CMake\bin\cmake.exe"
}

mkdir win64
cd win64
cmake .. -G "%CMAKE_PARAM_G%" -DCONC_WITH_MSI=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_SIGNCODE=0 -DWITH_SSL=SCHANNEL -DWITH_OPENSSL=ON
cd ..

cd libmariadb
cmake --build . --config RelWithDebInfo --parallel 2
cd ..
cmake --build . --config RelWithDebInfo --parallel 2

#cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCONC_WITH_MSI=OFF -DWITH_SSL=SCHANNEL .
#cmake --build . --config RelWithDebInfo
#dir
#cd wininstall
#cmake --build . --config RelWithDebInfo
#dir
#msiexec.exe /i wininstall\mariadb-connector-odbc-3.1.10-win64.msi