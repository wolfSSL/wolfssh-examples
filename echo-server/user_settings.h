/* user_settings.h
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

#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

/* Math library: single-precision */
#define WOLFSSL_SP_MATH_ALL
#define WOLFSSL_HAVE_SP_RSA
#define WOLFSSL_HAVE_SP_DH
#define WOLFSSL_HAVE_SP_ECC

#if defined(__x86_64__) || defined(__aarch64__) || defined(_M_X64)
    #define SP_WORD_SIZE 64
    #define HAVE___UINT128_T
#else
    #define SP_WORD_SIZE 32
#endif

/* wolfSSL core */
#define NO_MD5
#define NO_DSA
#define WOLFCRYPT_ONLY
#define NO_PKCS8
#define NO_PKCS12
#define HAVE_ECC
#define TFM_TIMING_RESISTANT
#define ECC_TIMING_RESISTANT
#define WC_RSA_BLINDING
#define HAVE_AESGCM
#define WOLFSSL_SHA384
#define WOLFSSL_SHA512

/* FreeRTOS */
#define FREERTOS
#define WOLFSSL_FREERTOS

/* wolfSSH */
#define WOLFSSL_WOLFSSH
#define WOLFSSH_USER_IO
#define DEFAULT_WINDOW_SZ     16384
#define HAVE_WC_ECC_SET_RNG
#define NO_MAIN_DRIVER
#define WOLFSSH_NO_AGENT

/* Uncomment for Microchip Harmony on PIC32MZ:
 * #define MICROCHIP_MPLAB_HARMONY
 * #define NO_FILESYSTEM
 * #define SINGLE_THREADED
 * #define WOLFSSH_NO_EXIT
 */

#endif /* USER_SETTINGS_H */
