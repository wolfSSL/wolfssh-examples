# wolfssh-examples

These wolfSSH examples depend on:

* [wolfssl](https://github.com/wolfSSL/wolfssl)
* [wolfssh](https://github.com/wolfSSL/wolfssh)

For the standard Getting Started examples, see:

* [wolfssl examples](https://github.com/wolfSSL/wolfssl/tree/master/examples)
* [wolfssh examples](https://github.com/wolfSSL/wolfssh/tree/master/examples)

## Espressif

See the [wolfSSL Espressif Home Page](https://www.wolfssl.com/espressif/).
The following examples are available for Espressif devices:

### ESP32
* [ESP32-SSH-Server](./Espressif/ESP32/ESP32-SSH-Server/README.md) SSH-to-UART.

### ESP8266

* [ESP32-SSH-Server](./Espressif/ESP8266/ESP8266-SSH-Server/README.md)
SSH-to-UART.

## make-testsuite

This example isn't manufacturer specific, but it has only been tested on
Ubuntu and macOS. This takes a wolfSSL directory and a wolfSSH directory and
builds it into a single static library (libwolfssh.a) and the testsuite
test tool. It uses a Makefile and has a preconfigured user_settings.h file.

# Configuration recommendations

## wolfSSH Task Priority

When setting up your thread that runs wolfSSL, it must have the same
or lower priority than the networking stack thread. For example, in FreeRTOS,
you may set the network stack thread's priority to 6 and the wolfSSH thread
to 8, or DEFAULTTASKPRIORITY. (In FreeRTOS, the lower the priority value
the higher the priority.)

# Support

For any issues related to wolfSSL or wolfSSH, please open an
[issue](https://github.com/wolfssl/wolfssl/issues) on GitHub,
visit the [wolfSSL support forum](https://www.wolfssl.com/forums/),
send an email to [support@wolfssl.com](mailto:support@wolfssl.com),
or [contact us](https://www.wolfssl.com/contact/).
