#!/bin/bash

# Clone and build wolfSSL
if [ ! -d wolfssl ]; then
    git clone --depth=1 git@github.com:wolfssl/wolfssl
fi
cd wolfssl && ./autogen.sh && ./configure --enable-ssh --enable-staticmemory CPPFLAGS="-DWOLFSSL_NO_MALLOC -DWOLFSSL_DEBUG_MEMORY -DWOLFMEM_BUCKETS=64,128,256,1024,2048,3456,8192,16000,34000" --disable-examples --disable-crypttests --prefix=$PWD/../wolfssl-install/ && make install
cd ..

# Clone and build wolfSSH
if [ ! -d wolfssh ]; then
    git clone --depth=1 git@github.com:wolfssl/wolfssh
fi
cd wolfssh
patch -p1 < ../scp-client-static-memory.patch
./autogen.sh && ./configure CPPFLAGS="-DWMALLOC_USER -DDEFAULT_WINDOW_SZ=4096" --with-wolfssl=$PWD/../wolfssl-install/ --prefix=$PWD/../wolfssl-install/ --disable-examples --enable-scp --enable-debug && make src/libwolfssh.la
cd ..

# Build modified example
gcc -DDEBUG_WOLFSSH -DWOLFSSH_SCP -DWMALLOC_USER -I$PWD/wolfssl-install/include -I$PWD/wolfssh/ wolfssh/examples/scpclient/scpclient.c wolfssh/examples/client/common.c -L$PWD/wolfssl-install/lib -L$PWD/wolfssh/src/.libs -lwolfssl -lwolfssh -o static-memory-scp-client


echo ""
echo "Done"
echo "Run the example application with :"
echo "head -c 1M </dev/urandom >/tmp/test.txt"
echo ""
echo "LD_LIBRARY_PATH=$PWD/wolfssl-install/lib:$PWD/wolfssh/src/.libs ./static-memory-scp-client valgrind ./static-memory-scp-client -H 127.0.0.1 -p 22222 -u jill -L /tmp/test.txt:/tmp/test-copy.txt -P upthehill"

exit 0

