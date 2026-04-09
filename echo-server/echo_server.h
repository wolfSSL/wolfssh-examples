/* echo_server.h
 *
 * Copyright (C) 2026 wolfSSL Inc.
 *
 * wolfSSH is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * wolfSSH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with wolfSSH.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssh/ssh.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ECHO_SERVER_PORT
    #define ECHO_SERVER_PORT    22222
#endif
#ifndef ECHO_BUFFER_SZ
    #define ECHO_BUFFER_SZ      4096
#endif
#ifndef SCRATCH_BUFFER_SZ
    #define SCRATCH_BUFFER_SZ   1200
#endif
#ifndef EXAMPLE_HIGHWATER_MARK
    #define EXAMPLE_HIGHWATER_MARK 0x3FFF8000
#endif

/* Password map entry */
typedef struct PwMap {
    byte type;
    byte username[32];
    word32 usernameSz;
    byte p[WC_SHA256_DIGEST_SIZE];
    struct PwMap* next;
} PwMap;

typedef struct PwMapList {
    PwMap* head;
} PwMapList;

/* Initialize wolfSSH context with keys, auth, and IO callbacks */
int echoServerInit(WOLFSSH_CTX** ctx, PwMapList* pwMapList);

/* Accept an SSH connection on the given FreeRTOS-Plus-TCP socket */
int echoServerAccept(WOLFSSH_CTX* ctx, WOLFSSH** ssh,
                     PwMapList* pwMapList, void* clientSocket);

/* Run the echo loop for one connected session */
int echoServerLoop(WOLFSSH* ssh);

/* Free all resources */
void echoServerCleanup(WOLFSSH_CTX* ctx, PwMapList* pwMapList);

#ifdef __cplusplus
}
#endif

#endif /* ECHO_SERVER_H */
