MKDIR ?= mkdir

OBJ = obj

WOLFSSH ?= wolfssh
SSHDIR = $(WOLFSSH)/src
SSHINC = $(WOLFSSH)
OBJSSH = $(OBJ)/$(WOLFSSH)

WOLFSSL ?= wolfssl
CRYPTDIR = $(WOLFSSL)/wolfcrypt/src
CRYPTINC = $(WOLFSSL)
OBJCRYPT = $(OBJ)/$(WOLFSSL)

CPPFLAGS ?= -I. -I$(SSHINC) -I$(CRYPTINC) -DWOLFSSL_USER_SETTINGS
ifeq ($(BUILD),debug)
    DEBUG ?= -O0 -g -DDEBUG_WOLFSSH
endif
CFLAGS := $(DEBUG) $(CFLAGS)

LDFLAGS ?= -lm -pthread

.PHONY: clean all

all: $(OBJ) libwolfssh.a testsuite keys/server-key-rsa.der

testsuite: $(OBJ)/testsuite.o $(OBJ)/echoserver.o $(OBJ)/client.o libwolfssh.a
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

libwolfssh.a: $(OBJSSH)/agent.o $(OBJSSH)/keygen.o $(OBJSSH)/port.o \
  $(OBJSSH)/wolfsftp.o $(OBJSSH)/internal.o $(OBJSSH)/log.o $(OBJSSH)/ssh.o \
  $(OBJSSH)/wolfterm.o $(OBJSSH)/io.o $(OBJSSH)/wolfscp.o \
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

$(OBJSSH)/agent.o: $(SSHDIR)/agent.c
$(OBJSSH)/keygen.o: $(SSHDIR)/keygen.c
$(OBJSSH)/port.o: $(SSHDIR)/port.c
$(OBJSSH)/wolfsftp.o: $(SSHDIR)/wolfsftp.c
$(OBJSSH)/internal.o: $(SSHDIR)/internal.c
$(OBJSSH)/log.o: $(SSHDIR)/log.c
$(OBJSSH)/ssh.o: $(SSHDIR)/ssh.c
$(OBJSSH)/wolfterm.o: $(SSHDIR)/wolfterm.c
$(OBJSSH)/io.o: $(SSHDIR)/io.c
$(OBJSSH)/wolfscp.o: $(SSHDIR)/wolfscp.c

$(OBJCRYPT)/%.o: $(CRYPTDIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJCRYPT)/aes.o: $(CRYPTDIR)/aes.c
$(OBJCRYPT)/dh.o: $(CRYPTDIR)/dh.c
$(OBJCRYPT)/tfm.o: $(CRYPTDIR)/tfm.c
$(OBJCRYPT)/integer.o: $(CRYPTDIR)/integer.c
$(OBJCRYPT)/sp_int.o: $(CRYPTDIR)/sp_int.c
$(OBJCRYPT)/sp_c64.o: $(CRYPTDIR)/sp_c32.c
$(OBJCRYPT)/sp_c64.o: $(CRYPTDIR)/sp_c64.c
$(OBJCRYPT)/sha.o: $(CRYPTDIR)/sha.c
$(OBJCRYPT)/sha256.o: $(CRYPTDIR)/sha256.c
$(OBJCRYPT)/sha512.o: $(CRYPTDIR)/sha512.c
$(OBJCRYPT)/hash.o: $(CRYPTDIR)/hash.c
$(OBJCRYPT)/hmac.o: $(CRYPTDIR)/hmac.c
$(OBJCRYPT)/rsa.o: $(CRYPTDIR)/rsa.c
$(OBJCRYPT)/ecc.o: $(CRYPTDIR)/ecc.c
$(OBJCRYPT)/memory.o: $(CRYPTDIR)/memory.c
$(OBJCRYPT)/random.o: $(CRYPTDIR)/random.c
$(OBJCRYPT)/wolfmath.o: $(CRYPTDIR)/wolfmath.c
$(OBJCRYPT)/asn.o: $(CRYPTDIR)/asn.c
$(OBJCRYPT)/coding.o: $(CRYPTDIR)/coding.c
$(OBJCRYPT)/signature.o: $(CRYPTDIR)/signature.c
$(OBJCRYPT)/wc_port.o: $(CRYPTDIR)/wc_port.c

$(OBJ)/testsuite.o: $(WOLFSSH)/tests/testsuite.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJ)/echoserver.o: $(WOLFSSH)/examples/echoserver/echoserver.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(OBJ)/client.o: $(WOLFSSH)/examples/client/client.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

keys/server-key-rsa.der:
	@$(MKDIR) -p keys
	@cp $(WOLFSSH)/keys/server-key-rsa.der keys
	@cp $(WOLFSSH)/keys/server-key-rsa.pem keys

$(OBJ):
	@$(MKDIR) -p $(OBJSSH) $(OBJCRYPT)

clean:
	rm -rf libwolfssh.a testsuite $(OBJ)
