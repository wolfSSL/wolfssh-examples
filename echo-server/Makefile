MKDIR ?= mkdir

OBJ = obj

# wolfSSH
WOLFSSH ?= ../wolfssh
SSHDIR = $(WOLFSSH)/src
SSHINC = $(WOLFSSH)
OBJSSH = $(OBJ)/wolfssh

# wolfSSL
WOLFSSL ?= ../wolfssl
CRYPTDIR = $(WOLFSSL)/wolfcrypt/src
CRYPTINC = $(WOLFSSL)
OBJCRYPT = $(OBJ)/wolfssl

# FreeRTOS Kernel
FREERTOS_KERNEL ?= FreeRTOS-Kernel
FREERTOS_PORT = $(FREERTOS_KERNEL)/portable/ThirdParty/GCC/Posix
FREERTOS_HEAP = $(FREERTOS_KERNEL)/portable/MemMang
FREERTOS_UTILS = $(FREERTOS_PORT)/utils
OBJFREERTOS = $(OBJ)/freertos

# FreeRTOS-Plus-TCP
FREERTOS_TCP ?= FreeRTOS-Plus-TCP
FREERTOS_TCP_SRC = $(FREERTOS_TCP)/source
FREERTOS_TCP_INC = $(FREERTOS_TCP_SRC)/include
FREERTOS_TCP_PORT = $(FREERTOS_TCP_SRC)/portable
FREERTOS_TCP_NI = $(FREERTOS_TCP_PORT)/NetworkInterface/linux
FREERTOS_TCP_BUF = $(FREERTOS_TCP_PORT)/BufferManagement
OBJFREERTOS_TCP = $(OBJ)/freertos_tcp

EXTRA_CPPFLAGS ?=
CPPFLAGS := -I. \
  -I$(SSHINC) -I$(CRYPTINC) \
  -I$(FREERTOS_KERNEL)/include -I$(FREERTOS_PORT) \
  -I$(FREERTOS_TCP_INC) -I$(FREERTOS_TCP_PORT)/Compiler/GCC \
  -DWOLFSSL_USER_SETTINGS \
  $(EXTRA_CPPFLAGS)

ifeq ($(BUILD),debug)
    DEBUG ?= -O0 -g -DDEBUG_WOLFSSH
endif
CFLAGS := $(DEBUG) $(CFLAGS)

LDFLAGS ?= -lm -lpthread -lpcap

.PHONY: clean all

all: $(OBJ) echo-server

# Application objects
$(OBJ)/main.o: main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJ)/echo_server.o: echo_server.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJ)/freertos_tcp_io.o: freertos_tcp_io.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

APP_OBJS = $(OBJ)/main.o $(OBJ)/echo_server.o $(OBJ)/freertos_tcp_io.o

# Link
echo-server: $(APP_OBJS) libwolfssh.a libfreertos.a libfreertos_tcp.a
	$(CC) $(CFLAGS) -o $@ $(APP_OBJS) libfreertos_tcp.a libfreertos.a libwolfssh.a $(LDFLAGS)

# --- wolfSSH + wolfSSL static library ---

libwolfssh.a: $(OBJSSH)/internal.o $(OBJSSH)/log.o $(OBJSSH)/ssh.o \
  $(OBJSSH)/io.o $(OBJSSH)/port.o $(OBJSSH)/keygen.o $(OBJSSH)/wolfterm.o \
  $(OBJCRYPT)/aes.o $(OBJCRYPT)/dh.o $(OBJCRYPT)/integer.o $(OBJCRYPT)/tfm.o \
  $(OBJCRYPT)/sha.o $(OBJCRYPT)/sha256.o $(OBJCRYPT)/sha512.o \
  $(OBJCRYPT)/hash.o $(OBJCRYPT)/ecc.o $(OBJCRYPT)/rsa.o $(OBJCRYPT)/memory.o \
  $(OBJCRYPT)/random.o $(OBJCRYPT)/hmac.o $(OBJCRYPT)/wolfmath.o \
  $(OBJCRYPT)/asn.o $(OBJCRYPT)/coding.o $(OBJCRYPT)/signature.o \
  $(OBJCRYPT)/wc_port.o $(OBJCRYPT)/sp_int.o $(OBJCRYPT)/sp_c64.o \
  $(OBJCRYPT)/sp_c32.o
	$(AR) $(ARFLAGS) $@ $^

$(OBJSSH)/%.o: $(SSHDIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJCRYPT)/%.o: $(CRYPTDIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# --- FreeRTOS Kernel static library ---

FREERTOS_OBJS = \
  $(OBJFREERTOS)/tasks.o \
  $(OBJFREERTOS)/queue.o \
  $(OBJFREERTOS)/list.o \
  $(OBJFREERTOS)/timers.o \
  $(OBJFREERTOS)/event_groups.o \
  $(OBJFREERTOS)/stream_buffer.o \
  $(OBJFREERTOS)/port.o \
  $(OBJFREERTOS)/heap_3.o \
  $(OBJFREERTOS)/wait_for_event.o

libfreertos.a: $(FREERTOS_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJFREERTOS)/tasks.o: $(FREERTOS_KERNEL)/tasks.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJFREERTOS)/queue.o: $(FREERTOS_KERNEL)/queue.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJFREERTOS)/list.o: $(FREERTOS_KERNEL)/list.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJFREERTOS)/timers.o: $(FREERTOS_KERNEL)/timers.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJFREERTOS)/event_groups.o: $(FREERTOS_KERNEL)/event_groups.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJFREERTOS)/stream_buffer.o: $(FREERTOS_KERNEL)/stream_buffer.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJFREERTOS)/port.o: $(FREERTOS_PORT)/port.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJFREERTOS)/heap_3.o: $(FREERTOS_HEAP)/heap_3.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
$(OBJFREERTOS)/wait_for_event.o: $(FREERTOS_UTILS)/wait_for_event.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# --- FreeRTOS-Plus-TCP static library ---

FREERTOS_TCP_OBJS = \
  $(OBJFREERTOS_TCP)/FreeRTOS_IP.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_IP_Utils.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_IP_Timers.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_ARP.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_DHCP.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_DNS.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_DNS_Cache.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_DNS_Callback.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_DNS_Networking.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_DNS_Parser.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_ICMP.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_IPv4.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_IPv4_Sockets.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_IPv4_Utils.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_Sockets.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_Stream_Buffer.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_IP.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_IP_IPv4.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_Reception.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_State_Handling.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_State_Handling_IPv4.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_Transmission.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_Transmission_IPv4.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_Utils.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_Utils_IPv4.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_TCP_WIN.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_Tiny_TCP.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_UDP_IP.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_UDP_IPv4.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_Routing.o \
  $(OBJFREERTOS_TCP)/FreeRTOS_BitConfig.o \
  $(OBJFREERTOS_TCP)/BufferAllocation_2.o \
  $(OBJFREERTOS_TCP)/NetworkInterface.o

libfreertos_tcp.a: $(FREERTOS_TCP_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(OBJFREERTOS_TCP)/%.o: $(FREERTOS_TCP_SRC)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJFREERTOS_TCP)/BufferAllocation_2.o: $(FREERTOS_TCP_BUF)/BufferAllocation_2.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJFREERTOS_TCP)/NetworkInterface.o: $(FREERTOS_TCP_NI)/NetworkInterface.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

# --- Object directories ---

$(OBJ):
	@$(MKDIR) -p $(OBJSSH) $(OBJCRYPT) $(OBJFREERTOS) $(OBJFREERTOS_TCP)

# --- Clean ---

clean:
	rm -rf echo-server libwolfssh.a libfreertos.a libfreertos_tcp.a $(OBJ)
