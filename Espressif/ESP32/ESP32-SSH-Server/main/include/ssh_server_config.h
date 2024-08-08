/* ssh_server_config.h
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
#ifndef _SSH_SERVER_CONFIG_H_
#define _SSH_SERVER_CONFIG_H_

/* sdkconfig needed for target chipset identification */
#include "sdkconfig.h"

/* Define wolfSSL settings in user_settings.h; NOT HERE! */

/* wolfSSL  */
#include <wolfssl/wolfcrypt/settings.h>

#include <driver/gpio.h>
#include <hal/gpio_types.h>

/**
 ******************************************************************************
 ******************************************************************************
 ** USER SETTINGS BEGIN
 ******************************************************************************
 ******************************************************************************
 **/



/* EdgeRouter-X is 57600, others are typically 115200
 * This is the UART baud rate to use in SSH server, NOT the monitor baud rate!
 **/
#define BAUD_RATE (115200)


/* SSH is usually on port 22, but for our example it lives at port 22222 */
#define SSH_UART_PORT 22222

/* in the case of wired ethernet on the ESN28J60 we need to
 * manually assign an IP address: MY_MAC_ADDRESS see init_ENC28J60() */
#define MY_MAC_ADDRESS  ( (uint8_t[6]) { 0x02, 0x00, 0x00, 0x12, 0x34, 0x56 } )



/* default is wireless unless USE_ENC28J60 is defined */
#undef USE_ENC28J60
/* #define USE_ENC28J60 */

/* WiFi can be either STA or AP
 *  #define WOLFSSH_SERVER_IS_AP
 *  #define WOLFSSH_SERVER_IS_STA
 **/

/* #define WOLFSSH_SERVER_IS_AP */
#define WOLFSSH_SERVER_IS_STA

/* set GPIO pins for UART_NUM_1 */

#undef ULX3S
#undef M5STICKC


/*
 * Example documentation images:
 *   Tx (transmit) is orange wire
 *   Rx (receive)  is yellow wire
 */
#if defined(M5STICKC)
    /* reminder GPIO 34 to 39 are input only */
    #define TXD_PIN (GPIO_NUM_26)
    #define RXD_PIN (GPIO_NUM_36)
#elif defined (ULX3S)
    /* reminder GPIO 34 to 39 are input only */
    #define TXD_PIN (GPIO_NUM_32)
    #define RXD_PIN (GPIO_NUM_33)
#elif defined (SSH_HUZZAH_ESP8266)
    #define EX_UART_NUM UART_NUM_0
#elif defined(CONFIG_IDF_TARGET_ESP8266)
    #define EX_UART_NUM UART_NUM_0
    /* `TXD2` = `GPIO 15` = `D8` (Yellow) */
    /* `RXD2` = `GPIO 13` = `D7` (Orange) */

    #define TXD_PIN (GPIO_Pin_15)
    #define RXD_PIN (GPIO_Pin_13) /* TODO assign valid GPIO */
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
    #ifndef GPIO_NUM_1
        #define GPIO_NUM_1 1
    #endif
    #ifndef GPIO_NUM_3
        #define GPIO_NUM_3 3
    #endif
    #define RXD_PIN (GPIO_NUM_1)
    #define TXD_PIN (GPIO_NUM_3)
#else
    #ifndef GPIO_NUM_17
        #define GPIO_NUM_17 17
    #endif
    #ifndef GPIO_NUM_16
        #define GPIO_NUM_16 16
    #endif
    #define RXD_PIN (GPIO_NUM_16)
    #define TXD_PIN (GPIO_NUM_17)
#endif

/* Optionally disable the entire UART component: */
/* #define DISABLE_SSH_UART */ 

#define SSH_SERVER_BANNER "wolfSSH Example Server\n"

#undef  SO_REUSEPORT

/* set SSH_SERVER_ECHO to a value of 1 to echo UART
 * this is optional and typically not desired as the
 * UART target will usually echo its own characters.
 * Valid values are 0 and 1.
 */
#define SSH_SERVER_ECHO 0

/**
 ******************************************************************************
 ******************************************************************************
 ** USER SETTINGS END
 ******************************************************************************
 ******************************************************************************
 **/

/* UART pins and config */
#include "uart_helper.h"

/* TODO check / optimize these values */
#ifndef EXAMPLE_HIGHWATER_MARK
    #define EXAMPLE_HIGHWATER_MARK 0x3FFF8000 /* 1GB - 32kB */
#endif
#ifndef EXAMPLE_BUFFER_SZ
    #define EXAMPLE_BUFFER_SZ 4096
#endif
#define SCRATCH_BUFFER_SZ 1200

#ifdef  WOLFSSH_SERVER_IS_AP
    #ifdef WOLFSSH_SERVER_IS_STA
        #error "Concurrent WOLFSSH_SERVER_IS_AP and WOLFSSH_SERVER_IS_STA"
        #error "not supported. Pick one. Disable the other."
    #endif
#endif

int ssh_server_config_init(void);

/* sanity checks */

#if defined(USE_ENC28J60) && defined(WOLFSSH_SERVER_IS_AP)
    #error "Server cannot be WiFi AP when using ENC28J60 at this time."
#endif

#if defined(USE_ENC28J60) && defined(WOLFSSH_SERVER_IS_AP)
    #error "Server cannot be WiFi STA when using ENC28J60 at this time."
#endif

#ifdef WOLFSSL_ESP8266
    #error "WOLFSSL_ESP8266 defined for ESP32 project. See user_settings.h"
#endif

#if defined(TXD_PIN) && defined(RXD_PIN)
    #if TXD_PIN == RXD_PIN
        #error "TXD_PIN cannot be the same as RXD_PIN"
    #endif
#endif

#endif /* _SSH_SERVER_CONFIG_H_ */
