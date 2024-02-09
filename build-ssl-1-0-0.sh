wget https://ftp.openssl.org/source/old/1.0.0/openssl-1.0.0.tar.gz
tar -zxf openssl-1.0.0.tar.gz
cd openssl-1.0.0
./config shared
make
make install_sw
rm -rf openssl-1.0.0*
cd ..
