# wolfSSH Echo Server on FreeRTOS

A minimal SSH echo server using wolfSSH, running on the FreeRTOS POSIX/Linux
simulator with FreeRTOS-Plus-TCP networking via libpcap.

## Dependencies

* [wolfSSL](https://github.com/wolfSSL/wolfssl)
* [wolfSSH](https://github.com/wolfSSL/wolfssh)
* [FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) (POSIX port)
* [FreeRTOS-Plus-TCP](https://github.com/FreeRTOS/FreeRTOS-Plus-TCP) (Linux network interface)
* libpcap development headers

### Installing dependencies

Fedora:

```
sudo dnf install libpcap-devel
```

Debian/Ubuntu:

```
sudo apt-get install libpcap-dev
```

Fetch the library sources (as submodules or cloned into the project directory):

```
git submodule update --init
```

## Building

```
make
```

Debug build (enables wolfSSH protocol logging):

```
make BUILD=debug
```

## Network Setup

FreeRTOS-Plus-TCP uses libpcap to send and receive raw Ethernet frames,
operating as a separate IP host. On Linux, a **veth pair** provides an isolated
virtual link between the host and the FreeRTOS stack.

### Create the veth pair

```
sudo ip link add veth0 type veth peer name veth1
sudo ip link set veth0 up
sudo ip link set veth1 up
sudo ip addr add 10.0.0.1/24 dev veth0
```

This creates two linked virtual interfaces:

* **veth0** -- the host side, with IP `10.0.0.1`
* **veth1** -- the FreeRTOS side, accessed via libpcap

### Disable checksum offload

The Linux kernel uses TCP checksum offload by default, which leaves partial
checksums in outgoing packets. Since there is no real NIC on a veth pair,
FreeRTOS-Plus-TCP sees invalid checksums and drops the packets. Disable TX
offload on the host side:

```
sudo ethtool -K veth0 tx off
```

### Verify the pcap interface number

FreeRTOS-Plus-TCP opens a pcap device by index number. After creating the veth
pair, check which number `veth1` gets:

```
tcpdump --list-interfaces
```

The output will look something like:

```
1.wlp170s0 [Up, Running, Wireless, Associated]
2.veth1 [Up, Running, Connected]
3.veth0 [Up, Running, Connected]
...
```

The default in `FreeRTOSIPConfig.h` is interface 2. If `veth1` has a different
number on your system, either edit the define or override at compile time:

```
make EXTRA_CPPFLAGS=-DipconfigNETWORK_INTERFACE_TO_USE=3
```

### Teardown

To remove the veth pair when done:

```
sudo ip link del veth0
```

## Running

The server needs raw socket access for libpcap:

```
sudo ./echo-server
```

Once the server prints `Listening on port 22222...`, connect from the host:

```
ssh -p 22222 jill@10.0.0.2
```

Password: `upthehill`

Other test credentials: `jack` / `fetchapail`

**Security warning:** The above usernames/passwords and the SSH host keys
bundled with this demo are for testing only. Replace all credentials and keys
before any non-demo use.

### Control keys

* **Ctrl+C** -- Disconnect
* **Ctrl+F** -- Trigger SSH key re-exchange

### Quick setup reference

All the commands in one block for copy-paste:

```
# One-time network setup
sudo ip link add veth0 type veth peer name veth1
sudo ip link set veth0 up
sudo ip link set veth1 up
sudo ip addr add 10.0.0.1/24 dev veth0
sudo ethtool -K veth0 tx off

# Build and run
make
sudo ./echo-server
```

## Network Configuration

The FreeRTOS-Plus-TCP stack uses a static IP configured in `main.c`:

| Setting    | Default       |
|------------|---------------|
| IP address | `10.0.0.2`    |
| Netmask    | `255.255.255.0` |
| Gateway    | `10.0.0.1`    |
| MAC        | `02:00:00:00:00:01` |

Edit these in `main.c` if your network setup differs.

`FreeRTOSIPConfig.h` contains the pcap interface index and TCP/IP stack tuning
parameters.

## Architecture

* **`main.c`** -- FreeRTOS entry point: initializes FreeRTOS-Plus-TCP, creates
  the SSH echo server task, starts the scheduler. Also contains required
  FreeRTOS hook functions.
* **`echo_server.c`** -- Platform-agnostic core: wolfSSH initialization,
  authentication (password + public key), echo read/send loop. Ported from
  `wolfssh/ide/mplabx/wolfssh.c`.
* **`freertos_tcp_io.c`** -- wolfSSH IO callbacks bridging
  `FreeRTOS_recv()`/`FreeRTOS_send()` to wolfSSH's IO layer.
* **`user_settings.h`** -- wolfSSL/wolfSSH build configuration.
* **`FreeRTOSConfig.h`** -- FreeRTOS kernel configuration.
* **`FreeRTOSIPConfig.h`** -- FreeRTOS-Plus-TCP stack configuration.

### Task priorities

The SSH echo server task must run at a **lower priority** than the
FreeRTOS-Plus-TCP IP task. The defaults are:

| Task             | Priority                    | Default |
|------------------|-----------------------------|---------|
| MAC ISR simulator| `configMAX_PRIORITIES - 1`  | 4       |
| IP task          | `configMAX_PRIORITIES - 2`  | 3       |
| SSH echo server  | `tskIDLE_PRIORITY + 1`      | 1       |

## Embedded Integration (PIC32MZ / Harmony)

To use on PIC32MZ with Microchip Harmony:

1. Add `echo_server.c`, `echo_server.h`, `freertos_tcp_io.c`,
   `freertos_tcp_io.h` to your MPLABX project
2. In `user_settings.h`, uncomment the Harmony-specific defines
3. Replace `main.c` with your Harmony `app.c`, calling `echoServerInit()`,
   `echoServerAccept()`, and `echoServerLoop()` from your task or state machine
4. Adjust `FreeRTOSConfig.h` and `FreeRTOSIPConfig.h` for your hardware

## user_settings.h

Key defines controlling the wolfSSL/wolfSSH build:

* `WOLFSSH_USER_IO` -- Disables default BSD socket IO; custom callbacks in
  `freertos_tcp_io.c` are used instead
* `FREERTOS` / `WOLFSSL_FREERTOS` -- Enables FreeRTOS support
* `WOLFSSH_NO_AGENT` -- Strips unused SSH agent support
