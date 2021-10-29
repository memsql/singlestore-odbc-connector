mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DCONC_WITH_UNIT_TESTS=Off -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_SSL=OPENSSL
cmake --build . --config DEBUG
