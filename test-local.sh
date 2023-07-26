#!/bin/bash
# This is a template script to run and debug tests locally on Linux
export TEST_PASSWORD='1'  # local DB password
export TEST_PORT=5506  # local DB port

./build-local.sh

cd build/test

export ODBCINI="$PWD/odbc.ini"
export ODBCSYSINI=$PWD

ctest -R odbc_unicode
# gdb ./<test to run> can be used to debug a specific test, e.g. gdb ./odbc_connect
