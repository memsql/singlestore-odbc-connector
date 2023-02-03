#!/bin/bash
# This is a template script to run and debug tests locally on Linux
export TEST_PASSWORD=''  # local DB password
export TEST_PORT=3376  # local DB port

./build-local.sh

cd build/test
cd test
``
export ODBCINI="$PWD/odbc.ini"
export ODBCSYSINI=$PWD

ctest -V # optionally append -R <test to run> to run specific tests
# gdb ./<test to run> can be used to debug a specific test, e.g. gdb ./odbc_connect
