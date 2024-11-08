This all is not needed with global heap hints introduced in PR
 (https://github.com/wolfSSL/wolfssl/pull/7478). For older versions of wolfSSL
 and wolfSSH though WMALLOC_USER can be used to call a custom heap allocator
 function that uses a set heap hint.

This example will use a custom global heap hint in the SCP client by remapping
 WMALLOC/WFREE calls.

build.sh is a script to help build the example `./build.sh`.

Once built run the exmaple wolfSSH echoserver from another terminal:

`./examples/echoserver/echoserver -j ./keys/hansel-key-rsa.der`

Then run the example scp client produced from `./build.sh`
