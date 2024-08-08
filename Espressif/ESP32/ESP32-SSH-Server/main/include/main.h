/* main.h
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
#ifndef _MAIN_H_
#define _MAIN_H_

/* Espressif libraries */
#include "sdkconfig.h"
#include <nvs_flash.h>
#include <esp_log.h>

/* wolfSSL  */
#include <wolfssl/wolfcrypt/settings.h>
/* Reminder: settings.h includes wolfssl/user_settings.h */
#ifndef WOLFSSL_ESPIDF
    #error "Problem with wolfSSL user_settings."
    #error "Check components/wolfssl/include"
#endif
#include <wolfssl/version.h>

/* wolfSSH  */
#include <wolfssh/ssh.h>
#include <wolfssh/log.h>

/* Project settings */

/* See also user.settings.g, in particular DEFAULT_WINDOW_SZ
 * that can drammatically affect required memory. */
#define UART_RX_TASK_STACK_SIZE   ( 4 * 1024)
#define UART_TX_TASK_STACK_SIZE   ( 4 * 1024)

#ifdef WOLFSSH_TEST_THREADING
    /* 4KB Observed to be too small; exact minimum not determined. */
    #define SERVER_SESSION_STACK_SIZE (5 * 1024)

    /* SSH Server will use pthreads task */
    #ifdef CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT
        #if (CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT < 20096)
            #error "CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT too small"
        #endif
    #else
        #error "CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT needs to be defined " \
               "when WOLFSSH_TEST_THREADING is enabled "
    #endif

    #if defined(SINGLE_THREADED)
        #error "Cannot enable both WOLFSSH_TEST_THREADING and SINGLE_THREADED"
    #endif
#else
    /* 20K is known to work for demo w/ ECC; TODO determine more exact minimum.
     * 15K observed to fail with default settings
     *
     * Currently faling with SINGLE_THREADED,
     *    even on S3 with SERVER_SESSION_STACK_SIZE = 153KB */
    #define SERVER_SESSION_STACK_SIZE (23 * 1024)

    /* SSH Server will use NOT pthreads task without WOLFSSH_TEST_THREADING */
    #ifdef CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT
        #if (CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT > 4096)
            #error "CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT too large"
        #endif

        /* The smallest confirmed stack size is 23KB */
        #if (SERVER_SESSION_STACK_SIZE < 23 * 1024)
            #error "SERVER_SESSION_STACK_SIZE too small"
        #endif
    #else
        #error "CONFIG_PTHREAD_TASK_STACK_SIZE_DEFAULT needs to be defined "
    #endif
#endif

#endif /* _MAIN_H_ */
