if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    New-Alias -Name cmake -Value "$Env:ProgramFiles\CMake\bin\cmake.exe"
}

#mkdir win64
#cd win64
#cmake .. -G "Visual Studio 16 2019 Win64" -DCONC_WITH_MSI=OFF -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_SIGNCODE=0 -DWITH_SSL=SCHANNEL -DWITH_OPENSSL=ON
#cd ..
#
#cd libmariadb
#cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo
#cmake --build . --config RelWithDebInfo --parallel 2
#cd ..

choco install -y -r --no-progress wixtoolset

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCONC_WITH_MSI=OFF -DWITH_SSL=SCHANNEL .
cmake --build . --config RelWithDebInfo --parallel 2

cd test
ctest -V
#cmake --build . --config RelWithDebInfo
#dir
#cd wininstall
#cmake --build . --config RelWithDebInfo
#dir
#msiexec.exe /i wininstall\mariadb-connector-odbc-3.1.10-win64.msi