# MAKEFILE EXAMPLE #

This example uses the sources from wolfSSL v5.3.0 and wolfSSH v1.4.10.
Fetch the additional sources with the command:

```
    git submodule update --init
```

Running **make** should produce **libwolfssl.a** and the **testsuite**
application. It will also copy some keys from the the wolfSSH directory into
the **keys** directory. This Makefile does not do any dependency tracking
beyond the source file for an object.

The file **user_settings.h** controls the build settings. Directions for
changing the math library used are in the file.

This has been tested on both an M1 Mac mini with macOS and on an AMD based
Ubuntu computer. Both are 64-bit.
