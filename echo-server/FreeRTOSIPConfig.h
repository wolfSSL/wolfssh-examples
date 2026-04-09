/* FreeRTOSIPConfig.h
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

#ifndef FREERTOS_IP_CONFIG_H
#define FREERTOS_IP_CONFIG_H

/* Use the backward-compatible single-interface FreeRTOS_IPInit() API */
#define ipconfigIPv4_BACKWARD_COMPATIBLE        1
#define ipconfigUSE_IPv6                        0

/* TCP/IP task */
#define ipconfigIP_TASK_PRIORITY                ( configMAX_PRIORITIES - 2 )
#define ipconfigIP_TASK_STACK_SIZE_WORDS         ( configMINIMAL_STACK_SIZE * 4 )

/* Protocol support */
#define ipconfigUSE_TCP                         1
#define ipconfigUSE_DNS                         0
#define ipconfigUSE_DHCP                        0

/* Network tuning */
#define ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS  60
#define ipconfigNETWORK_MTU                     1500
#define ipconfigTCP_MSS                         ( ipconfigNETWORK_MTU - 40 )
#define ipconfigTCP_RX_BUFFER_LENGTH            ( 16 * 1024 )
#define ipconfigTCP_TX_BUFFER_LENGTH            ( 16 * 1024 )
#define ipconfigTCP_WIN_SEG_COUNT               64

/* Event processing */
#define ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES  1
#define ipconfigETHERNET_MINIMUM_PACKET_BYTES        0
#define ipconfigBUFFER_PADDING                       0
#define ipconfigPACKET_FILLER_SIZE                   2
#define ipconfigEVENT_QUEUE_LENGTH                   ( ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS + 5 )

/* ARP */
#define ipconfigUSE_ARP_REMOVE_ENTRY            1
#define ipconfigUSE_ARP_REVERSED_LOOKUP         1
#define ipconfigARP_CACHE_ENTRIES               6
#define ipconfigMAX_ARP_RETRANSMISSIONS         5
#define ipconfigMAX_ARP_AGE                     150

/* TCP parameters */
#define ipconfigTCP_HANG_PROTECTION             1
#define ipconfigTCP_HANG_PROTECTION_TIME        30
#define ipconfigTCP_KEEP_ALIVE                  1
#define ipconfigTCP_KEEP_ALIVE_INTERVAL         20

/* Misc */
#define ipconfigZERO_COPY_TX_DRIVER             0
#define ipconfigZERO_COPY_RX_DRIVER             0
#define ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM  0
#define ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM  0
#define ipconfigSUPPORT_OUTGOING_PINGS          1
#define ipconfigREPLY_TO_INCOMING_PINGS         1
#define ipconfigSUPPORT_SELECT_FUNCTION         0
#define ipconfigFILTER_OUT_NON_ETHERNET_II_FRAMES  1
#define ipconfigBYTE_ORDER                      pdFREERTOS_LITTLE_ENDIAN
#define ipconfigHAS_DEBUG_PRINTF                0
#define ipconfigHAS_PRINTF                      1

/* Linux pcap network interface.
 * Override at compile time with -DipconfigNETWORK_INTERFACE_TO_USE=N */
#ifndef ipconfigNETWORK_INTERFACE_TO_USE
    #define ipconfigNETWORK_INTERFACE_TO_USE    2
#endif

/* Linux NetworkInterface.c requires these defines */
#define configNETWORK_INTERFACE_TO_USE          ipconfigNETWORK_INTERFACE_TO_USE
#define configMAC_ISR_SIMULATOR_PRIORITY        ( configMAX_PRIORITIES - 1 )
#define configWINDOWS_MAC_INTERRUPT_SIMULATOR_DELAY  ( 2 / portTICK_PERIOD_MS )
#define configNET_MASK0                         255
#define configNET_MASK1                         255
#define configNET_MASK2                         255
#define configNET_MASK3                         0

#endif /* FREERTOS_IP_CONFIG_H */
