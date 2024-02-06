/* ssh_server.h
 *
 * Copyright (C) 2014-2024 wolfSSL Inc.
 *
 * This file is part of wolfSSH.
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
#ifndef _SSH_SERVER_H_
#define _SSH_SERVER_H_

#include <freertos/FreeRTOS.h>

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

/* make sure this appears before any other wolfSSL headers */
#ifdef WOLFSSL_USER_SETTINGS
    #include <wolfssl/wolfcrypt/settings.h>
#else
    #include <wolfssl/options.h>
#endif

/* socket includes */
#include "lwip/netdb.h"
#include "lwip/sockets.h"

#ifdef NO_FILESYSTEM
    #include <wolfssh/certs_test.h>
    #ifdef WOLFSSH_SCP
        #include <wolfssh/wolfscp.h>
    #endif
#endif

/* the main SSH Server demo*/
void server_test(void *arg);

/* External buffer functions used between RTOS tasks. (typically the UART)
 * TODO: Implement interrupts rather than polling */
volatile char* __attribute__((optimize("O0"))) ExternalTransmitBuffer(void);
volatile char* __attribute__((optimize("O0"))) ExternalReceiveBuffer(void);

int ExternalTransmitBufferSz(void);
int ExternalReceiveBufferSz(void);

int Set_ExternalTransmitBufferSz(int n);
int Set_ExternalReceiveBufferSz(int n);

#endif /* _SSH_SERVER_H_ */
