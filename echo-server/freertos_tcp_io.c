/* freertos_tcp_io.c
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

/* wolfSSH IO callbacks for FreeRTOS-Plus-TCP.
 * Follows the pattern from wolfip/src/port/wolfssh_io.c */

#include "freertos_tcp_io.h"

#include <wolfssh/ssh.h>
#include <wolfssh/error.h>

#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"


/* Receive callback: FreeRTOS_recv() -> wolfSSH IO return codes */
static int freertosRecv(WOLFSSH* ssh, void* buf, word32 sz, void* ctx)
{
    Socket_t xSocket;
    BaseType_t ret;

    (void)ssh;

    if (ctx == NULL)
        return WS_CBIO_ERR_GENERAL;

    xSocket = *(Socket_t*)ctx;

    ret = FreeRTOS_recv(xSocket, buf, (size_t)sz, 0);

    if (ret > 0)
        return (int)ret;
    if (ret == 0)
        return WS_CBIO_ERR_CONN_CLOSE;
    if (ret == -pdFREERTOS_ERRNO_EWOULDBLOCK ||
        ret == -pdFREERTOS_ERRNO_EAGAIN)
        return WS_CBIO_ERR_WANT_READ;
    if (ret == -pdFREERTOS_ERRNO_ENOTCONN)
        return WS_CBIO_ERR_CONN_CLOSE;

    return WS_CBIO_ERR_GENERAL;
}


/* Send callback: FreeRTOS_send() -> wolfSSH IO return codes */
static int freertosSend(WOLFSSH* ssh, void* buf, word32 sz, void* ctx)
{
    Socket_t xSocket;
    BaseType_t ret;

    (void)ssh;

    if (ctx == NULL)
        return WS_CBIO_ERR_GENERAL;

    xSocket = *(Socket_t*)ctx;

    ret = FreeRTOS_send(xSocket, buf, (size_t)sz, 0);

    if (ret > 0)
        return (int)ret;
    if (ret == 0)
        return WS_CBIO_ERR_CONN_CLOSE;
    if (ret == -pdFREERTOS_ERRNO_EWOULDBLOCK ||
        ret == -pdFREERTOS_ERRNO_EAGAIN)
        return WS_CBIO_ERR_WANT_WRITE;
    if (ret == -pdFREERTOS_ERRNO_ENOTCONN)
        return WS_CBIO_ERR_CONN_CLOSE;

    return WS_CBIO_ERR_GENERAL;
}


void freertosIO_SetCallbacks(WOLFSSH_CTX* ctx)
{
    if (ctx != NULL) {
        wolfSSH_SetIORecv(ctx, freertosRecv);
        wolfSSH_SetIOSend(ctx, freertosSend);
    }
}


void freertosIO_SetContext(WOLFSSH* ssh, void* socketPtr)
{
    if (ssh != NULL) {
        wolfSSH_SetIOReadCtx(ssh, socketPtr);
        wolfSSH_SetIOWriteCtx(ssh, socketPtr);
    }
}
