/* echo_server.c
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

/* Uses portions of code from wolfssh/ide/mplabx/wolfssh.c and
 * wolfssh/examples/echoserver/echoserver.c */

#include "echo_server.h"
#include "freertos_tcp_io.h"

#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/wolfcrypt/coding.h>
#include <wolfssh/ssh.h>
#include <wolfssh/log.h>

/* Use embedded test keys */
#ifndef NO_FILESYSTEM
    #define NO_FILESYSTEM
    #include <wolfssh/certs_test.h>
    #undef NO_FILESYSTEM
#else
    #include <wolfssh/certs_test.h>
#endif

#include "FreeRTOS.h"
#include "task.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Helper functions ported from wolfssh/ide/mplabx/wolfssh.c
 */

static const char echoServerBanner[] = "wolfSSH Echo Server\n";

static const char samplePasswordBuffer[] =
    "jill:upthehill\n"
    "jack:fetchapail\n";

static const char samplePublicKeyEccBuffer[] =
    "ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAA"
    "BBBNkI5JTP6D0lF42tbxX19cE87hztUS6FSDoGvPfiU0CgeNSbI+aFdKIzTP5CQEJSvm25"
    "qUzgDtH7oyaQROUnNvk= hansel\n"
    "ecdsa-sha2-nistp256 AAAAE2VjZHNhLXNoYTItbmlzdHAyNTYAAAAIbmlzdHAyNTYAAA"
    "BBBKAtH8cqaDbtJFjtviLobHBmjCtG56DMkP6A4M2H9zX2/YCg1h9bYS7WHd9UQDwXO1Hh"
    "IZzRYecXh7SG9P4GhRY= gretel\n";

static const char samplePublicKeyRsaBuffer[] =
    "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQC9P3ZFowOsONXHD5MwWiCciXytBRZGho"
    "MNiisWSgUs5HdHcACuHYPi2W6Z1PBFmBWT9odOrGRjoZXJfDDoPi+j8SSfDGsc/hsCmc3G"
    "p2yEhUZUEkDhtOXyqjns1ickC9Gh4u80aSVtwHRnJZh9xPhSq5tLOhId4eP61s+a5pwjTj"
    "nEhBaIPUJO2C/M0pFnnbZxKgJlX7t1Doy7h5eXxviymOIvaCZKU+x5OopfzM/wFkey0EPW"
    "NmzI5y/+pzU5afsdeEWdiQDIQc80H6Pz8fsoFPvYSG+s4/wz0duu7yeeV1Ypoho65Zr+pE"
    "nIf7dO0B8EblgWt+ud+JI8wrAhfE4x hansel\n"
    "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCqDwRVTRVk/wjPhoo66+Mztrc31KsxDZ"
    "+kAV0139PHQ+wsueNpba6jNn5o6mUTEOrxrz0LMsDJOBM7CmG0983kF4gRIihECpQ0rcjO"
    "P6BSfbVTE9mfIK5IsUiZGd8SoE9kSV2pJ2FvZeBQENoAxEFk0zZL9tchPS+OCUGbK4SDjz"
    "uNZl/30Mczs73N3MBzi6J1oPo7sFlqzB6ecBjK2Kpjus4Y1rYFphJnUxtKvB0s+hoaadru"
    "biE57dK6BrH5iZwVLTQKux31uCJLPhiktI3iLbdlGZEctJkTasfVSsUizwVIyRjhVKmbdI"
    "RGwkU38D043AR1h0mUoGCPIKuqcFMf gretel\n";


static INLINE void c32toa(word32 u32, byte* c)
{
    c[0] = (u32 >> 24) & 0xff;
    c[1] = (u32 >> 16) & 0xff;
    c[2] = (u32 >>  8) & 0xff;
    c[3] =  u32 & 0xff;
}


static PwMap* PwMapNew(PwMapList* list, byte type, const byte* username,
                       word32 usernameSz, const byte* p, word32 pSz)
{
    PwMap* map;

    map = (PwMap*)malloc(sizeof(PwMap));
    if (map != NULL) {
        wc_Sha256 sha;
        byte flatSz[4];

        map->type = type;
        if (usernameSz >= sizeof(map->username))
            usernameSz = sizeof(map->username) - 1;
        memcpy(map->username, username, usernameSz + 1);
        map->username[usernameSz] = 0;
        map->usernameSz = usernameSz;

        wc_InitSha256(&sha);
        c32toa(pSz, flatSz);
        wc_Sha256Update(&sha, flatSz, sizeof(flatSz));
        wc_Sha256Update(&sha, p, pSz);
        wc_Sha256Final(&sha, map->p);

        map->next = list->head;
        list->head = map;
    }

    return map;
}


static void PwMapListDelete(PwMapList* list)
{
    if (list != NULL) {
        PwMap* head = list->head;

        while (head != NULL) {
            PwMap* cur = head;
            head = head->next;
            memset(cur, 0, sizeof(PwMap));
            free(cur);
        }
        list->head = NULL;
    }
}


static int LoadPasswordBuffer(byte* buf, word32 bufSz, PwMapList* list)
{
    char* str = (char*)buf;
    char* delimiter;
    char* username;
    char* password;

    if (list == NULL)
        return -1;

    if (buf == NULL || bufSz == 0)
        return 0;

    while (*str != 0) {
        delimiter = strchr(str, ':');
        if (delimiter == NULL)
            break;
        username = str;
        *delimiter = 0;
        password = delimiter + 1;
        str = strchr(password, '\n');
        if (str == NULL)
            break;
        *str = 0;
        str++;
        if (PwMapNew(list, WOLFSSH_USERAUTH_PASSWORD,
                     (byte*)username, (word32)strlen(username),
                     (byte*)password, (word32)strlen(password)) == NULL) {
            return -1;
        }
    }

    return 0;
}


static int LoadPublicKeyBuffer(byte* buf, word32 bufSz, PwMapList* list)
{
    char* str = (char*)buf;
    char* delimiter;
    byte* publicKey64;
    word32 publicKey64Sz;
    byte* username;
    word32 usernameSz;
    byte  publicKey[300];
    word32 publicKeySz;

    if (list == NULL)
        return -1;

    if (buf == NULL || bufSz == 0)
        return 0;

    while (*str != 0) {
        /* Skip the public key type (e.g., ssh-rsa or ecdsa-sha2-nistp256) */
        delimiter = strchr(str, ' ');
        if (delimiter == NULL)
            break;
        str = delimiter + 1;
        delimiter = strchr(str, ' ');
        if (delimiter == NULL)
            break;
        publicKey64 = (byte*)str;
        *delimiter = 0;
        publicKey64Sz = (word32)(delimiter - str);
        str = delimiter + 1;
        delimiter = strchr(str, '\n');
        if (delimiter == NULL)
            break;
        username = (byte*)str;
        *delimiter = 0;
        usernameSz = (word32)(delimiter - str);
        str = delimiter + 1;
        publicKeySz = sizeof(publicKey);

        if (Base64_Decode(publicKey64, publicKey64Sz,
                          publicKey, &publicKeySz) != 0) {
            return -1;
        }

        if (PwMapNew(list, WOLFSSH_USERAUTH_PUBLICKEY,
                     username, usernameSz,
                     publicKey, publicKeySz) == NULL) {
            return -1;
        }
    }

    return 0;
}


/* Load embedded DER key from certs_test.h */
static int load_key(byte isEcc, byte* buf, word32 bufSz)
{
    word32 sz = 0;

    if (isEcc) {
        if (sizeof_ecc_key_der_256_ssh > bufSz) {
            return 0;
        }
        WMEMCPY(buf, ecc_key_der_256_ssh, sizeof_ecc_key_der_256_ssh);
        sz = sizeof_ecc_key_der_256_ssh;
    }
    else {
        if (sizeof_rsa_key_der_2048_ssh > bufSz) {
            return 0;
        }
        WMEMCPY(buf, (byte*)rsa_key_der_2048_ssh, sizeof_rsa_key_der_2048_ssh);
        sz = sizeof_rsa_key_der_2048_ssh;
    }

    return sz;
}


static byte find_char(const byte* str, const byte* buf, word32 bufSz)
{
    const byte* cur;

    while (bufSz) {
        cur = str;
        while (*cur != '\0') {
            if (*cur == *buf)
                return *cur;
            cur++;
        }
        buf++;
        bufSz--;
    }

    return 0;
}


/*
 * Authentication callback
 */
static int wsUserAuth(byte authType,
                      WS_UserAuthData* authData,
                      void* ctx)
{
    PwMapList* list;
    PwMap* map;
    byte authHash[WC_SHA256_DIGEST_SIZE];
    wc_Sha256 sha;
    byte flatSz[4];

    if (ctx == NULL) {
        return WOLFSSH_USERAUTH_FAILURE;
    }

    if (authType != WOLFSSH_USERAUTH_PASSWORD &&
        authType != WOLFSSH_USERAUTH_PUBLICKEY) {
        return WOLFSSH_USERAUTH_FAILURE;
    }

    /* Hash the password or public key with its length */
    wc_InitSha256(&sha);
    if (authType == WOLFSSH_USERAUTH_PASSWORD) {
        c32toa(authData->sf.password.passwordSz, flatSz);
        wc_Sha256Update(&sha, flatSz, sizeof(flatSz));
        wc_Sha256Update(&sha,
                        authData->sf.password.password,
                        authData->sf.password.passwordSz);
    }
    else if (authType == WOLFSSH_USERAUTH_PUBLICKEY) {
        c32toa(authData->sf.publicKey.publicKeySz, flatSz);
        wc_Sha256Update(&sha, flatSz, sizeof(flatSz));
        wc_Sha256Update(&sha,
                        authData->sf.publicKey.publicKey,
                        authData->sf.publicKey.publicKeySz);
    }
    wc_Sha256Final(&sha, authHash);

    list = (PwMapList*)ctx;
    map = list->head;

    while (map != NULL) {
        if (authData->usernameSz == map->usernameSz &&
            memcmp(authData->username, map->username, map->usernameSz) == 0) {

            if (authData->type == map->type) {
                if (memcmp(map->p, authHash, WC_SHA256_DIGEST_SIZE) == 0) {
                    return WOLFSSH_USERAUTH_SUCCESS;
                }
                else {
                    return (authType == WOLFSSH_USERAUTH_PASSWORD ?
                            WOLFSSH_USERAUTH_INVALID_PASSWORD :
                            WOLFSSH_USERAUTH_INVALID_PUBLICKEY);
                }
            }
            else {
                map = map->next;
                continue;
            }
        }
        map = map->next;
    }

    return WOLFSSH_USERAUTH_INVALID_USER;
}


/*
 * Public API
 */

int echoServerInit(WOLFSSH_CTX** ctx, PwMapList* pwMapList)
{
    int useEcc = 0;
    byte buf[SCRATCH_BUFFER_SZ];
    word32 bufSz;
    const char* pubKeyBuf;

    memset(pwMapList, 0, sizeof(PwMapList));

    if (wolfSSH_Init() != WS_SUCCESS) {
        printf("Couldn't initialize wolfSSH.\n");
        return -1;
    }

    *ctx = wolfSSH_CTX_new(WOLFSSH_ENDPOINT_SERVER, NULL);
    if (*ctx == NULL) {
        printf("Couldn't allocate SSH CTX data.\n");
        wolfSSH_Cleanup();
        return -1;
    }

    wolfSSH_SetUserAuth(*ctx, wsUserAuth);
    wolfSSH_CTX_SetBanner(*ctx, echoServerBanner);

    /* Register FreeRTOS-Plus-TCP IO callbacks */
    freertosIO_SetCallbacks(*ctx);

    /* Load server private key */
    bufSz = load_key(useEcc, buf, SCRATCH_BUFFER_SZ);
    if (bufSz == 0) {
        printf("Couldn't load key.\n");
        wolfSSH_CTX_free(*ctx);
        *ctx = NULL;
        wolfSSH_Cleanup();
        return -1;
    }
    if (wolfSSH_CTX_UsePrivateKey_buffer(*ctx, buf, bufSz,
                                         WOLFSSH_FORMAT_ASN1) < 0) {
        printf("Couldn't use key buffer.\n");
        wolfSSH_CTX_free(*ctx);
        *ctx = NULL;
        wolfSSH_Cleanup();
        return -1;
    }

    /* Load passwords */
    bufSz = (word32)strlen(samplePasswordBuffer);
    memcpy(buf, samplePasswordBuffer, bufSz);
    buf[bufSz] = 0;
    if (LoadPasswordBuffer(buf, bufSz, pwMapList) != 0) {
        printf("Couldn't load password buffer.\n");
        PwMapListDelete(pwMapList);
        wolfSSH_CTX_free(*ctx);
        *ctx = NULL;
        wolfSSH_Cleanup();
        return -1;
    }

    /* Load public keys */
    pubKeyBuf = useEcc ? samplePublicKeyEccBuffer : samplePublicKeyRsaBuffer;
    bufSz = (word32)strlen(pubKeyBuf);
    memcpy(buf, pubKeyBuf, bufSz);
    buf[bufSz] = 0;
    if (LoadPublicKeyBuffer(buf, bufSz, pwMapList) != 0) {
        printf("Couldn't load public key buffer.\n");
        PwMapListDelete(pwMapList);
        wolfSSH_CTX_free(*ctx);
        *ctx = NULL;
        wolfSSH_Cleanup();
        return -1;
    }

    printf("wolfSSH Echo Server initialized on port %d\n", ECHO_SERVER_PORT);
    return 0;
}


int echoServerAccept(WOLFSSH_CTX* ctx, WOLFSSH** ssh,
                     PwMapList* pwMapList, void* clientSocket)
{
    int ret;
    int error;

    *ssh = wolfSSH_new(ctx);
    if (*ssh == NULL) {
        printf("Couldn't allocate SSH session.\n");
        return -1;
    }

    wolfSSH_SetUserAuthCtx(*ssh, pwMapList);

    /* Set FreeRTOS-Plus-TCP IO context (socket handle) */
    freertosIO_SetContext(*ssh, clientSocket);

    printf("Performing SSH accept...\n");

    /* Non-blocking accept loop */
    do {
        ret = wolfSSH_accept(*ssh);
        error = wolfSSH_get_error(*ssh);

        if (ret == WS_SUCCESS || ret == WS_SFTP_COMPLETE) {
            break;
        }

        if (error == WS_WANT_READ || error == WS_WANT_WRITE) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        printf("SSH accept error: %d (%s)\n", error,
               wolfSSH_ErrorToName(error));
        wolfSSH_free(*ssh);
        *ssh = NULL;
        return -1;
    } while (1);

    printf("SSH connection accepted.\n");
    return 0;
}


int echoServerLoop(WOLFSSH* ssh)
{
    byte* buf = NULL;
    byte* tmpBuf;
    int bufSz, backlogSz = 0, rxSz, txSz, stop = 0, txSum;

    while (!stop) {
        bufSz = ECHO_BUFFER_SZ + backlogSz;
        tmpBuf = (byte*)realloc(buf, bufSz);
        if (tmpBuf == NULL) {
            stop = 1;
            break;
        }
        buf = tmpBuf;

        rxSz = wolfSSH_stream_read(ssh, buf + backlogSz, ECHO_BUFFER_SZ);

        if (rxSz <= 0) {
            int error = wolfSSH_get_error(ssh);

            if (error == WS_WANT_READ || error == WS_WANT_WRITE) {
                vTaskDelay(pdMS_TO_TICKS(1));
                continue;
            }
            if (error == WS_EOF) {
                printf("Client disconnected (EOF).\n");
                stop = 1;
                break;
            }
            if (error == WS_REKEYING) {
                continue;
            }

            printf("Stream read error: %d (%s)\n", error,
                   wolfSSH_ErrorToName(error));
            stop = 1;
            break;
        }

        backlogSz += rxSz;
        txSum = 0;

        while (backlogSz != txSum && !stop) {
            txSz = wolfSSH_stream_send(ssh, buf + txSum, backlogSz - txSum);

            if (txSz > 0) {
                byte c;
                const byte matches[] = { 0x03, 0x06, 0x00 };

                c = find_char(matches, buf + txSum, txSz);
                switch (c) {
                    case 0x03: /* Ctrl+C */
                        printf("Ctrl+C received, disconnecting.\n");
                        stop = 1;
                        break;
                    case 0x06: /* Ctrl+F */
                        if (wolfSSH_TriggerKeyExchange(ssh) != WS_SUCCESS) {
                            stop = 1;
                        }
                        break;
                    default:
                        break;
                }
                txSum += txSz;
            }
            else if (txSz == WS_REKEYING) {
                /* Continue turning the crank during rekey */
                vTaskDelay(pdMS_TO_TICKS(1));
                continue;
            }
            else {
                int error = wolfSSH_get_error(ssh);
                if (error == WS_WANT_WRITE) {
                    vTaskDelay(pdMS_TO_TICKS(1));
                    continue;
                }
                printf("Stream send error: %d\n", error);
                stop = 1;
                break;
            }
        }

        if (txSum < backlogSz)
            memmove(buf, buf + txSum, backlogSz - txSum);
        backlogSz -= txSum;
    }

    free(buf);
    return 0;
}


void echoServerCleanup(WOLFSSH_CTX* ctx, PwMapList* pwMapList)
{
    PwMapListDelete(pwMapList);
    wolfSSH_CTX_free(ctx);
    wolfSSH_Cleanup();
}
