/* tx_rx_buffer.h
 *
 * Copyright (C) 2014-2022 wolfSSL Inc.
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
#ifndef _TX_RX_BUFFER_H_
#define _TX_RX_BUFFER_H_

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include <string.h>

/* TODO do these really need to be so big? probably not */
/* Sizes for shared transmit and receive buffers, for
 * both external (typically UART) and SSH data streams */
#define EXT_RX_BUF_MAX_SZ 2048
#define EXT_TX_BUF_MAX_SZ 2048

typedef uint8_t byte;

int init_tx_rx_buffer(byte TxPin, byte RxPin);

int Get_ExternalTransmitBuffer(byte **ToData);

int Set_ExternalTransmitBuffer(byte *FromData, int sz);

int Set_ExternalReceiveBuffer(byte *FromData, int sz);

bool ExternalReceiveBuffer_IsChar(char charValue);

#endif /* _TX_RX_BUFFER_H_ */
