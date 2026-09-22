set -eu pipefail

# FindOpenSSL searches neither the Homebrew keg nor a custom prefix on its own
if [ -z "${OPENSSL_ROOT_DIR:-}" ]; then
	for candidate in /usr/local/opt/openssl@3 /opt/homebrew/opt/openssl@3 /opt/openssl-3.0; do
		if [ -f "$candidate/include/openssl/opensslv.h" ]; then
			OPENSSL_ROOT_DIR="$candidate"
			break
		fi
	done
fi

mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCONC_WITH_UNIT_TESTS=Off -DCMAKE_INSTALL_PREFIX=/usr/local -DWITH_SSL=OPENSSL -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ${OPENSSL_ROOT_DIR:+-DOPENSSL_ROOT_DIR="$OPENSSL_ROOT_DIR"}
cmake --build . --config RelWithDebInfo
cd ..

# clangd only looks for the compilation database next to the sources
ln -sf build/compile_commands.json compile_commands.json
