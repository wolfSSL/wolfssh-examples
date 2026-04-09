/* freertos_tcp_io.h
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

#ifndef FREERTOS_TCP_IO_H
#define FREERTOS_TCP_IO_H

#include <wolfssh/ssh.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Register FreeRTOS-Plus-TCP IO callbacks on a wolfSSH context */
void freertosIO_SetCallbacks(WOLFSSH_CTX* ctx);

/* Set the FreeRTOS-Plus-TCP socket as the IO context for a session */
void freertosIO_SetContext(WOLFSSH* ssh, void* socketPtr);

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_TCP_IO_H */
