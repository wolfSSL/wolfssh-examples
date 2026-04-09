/* main.c
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

/* FreeRTOS entry point for the wolfSSH echo server.
 * Runs on the FreeRTOS POSIX/Linux simulator with FreeRTOS-Plus-TCP. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "echo_server.h"

/* Network configuration for the FreeRTOS-Plus-TCP stack.
 * The stack runs on one end of a veth pair with its own IP address.
 * Adjust for your network environment. */
static const uint8_t ucIPAddress[4]        = { 10, 0, 0, 2 };
static const uint8_t ucNetMask[4]          = { 255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[4]   = { 10, 0, 0, 1 };
static const uint8_t ucDNSServerAddress[4] = { 8, 8, 8, 8 };
static const uint8_t ucMACAddress[6]       = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 };


static void sshEchoServerTask(void* pvParameters)
{
    Socket_t xListenSocket, xClientSocket;
    struct freertos_sockaddr xBindAddr, xClientAddr;
    socklen_t xClientAddrLen;
    TickType_t xRecvTimeout = pdMS_TO_TICKS(1000);
    WOLFSSH_CTX* ctx = NULL;
    WOLFSSH* ssh = NULL;
    PwMapList pwMapList;

    (void)pvParameters;

    /* Wait for the network stack to be ready */
    printf("Waiting for network...\n");
    while (FreeRTOS_IsNetworkUp() == pdFALSE) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    {
        uint32_t ulIPAddress, ulNetMask, ulGateway, ulDNS;
        char cBuf[16];
        FreeRTOS_GetAddressConfiguration(&ulIPAddress, &ulNetMask,
                                          &ulGateway, &ulDNS);
        FreeRTOS_inet_ntoa(ulIPAddress, cBuf);
        printf("Network up. IP Address: %s\n", cBuf);
        printf("Connect with: ssh -p %d jill@%s  (password: upthehill)\n",
               ECHO_SERVER_PORT, cBuf);
    }

    /* Initialize wolfSSH */
    if (echoServerInit(&ctx, &pwMapList) != 0) {
        printf("Echo server init failed.\n");
        vTaskDelete(NULL);
        return;
    }

    /* Create TCP listening socket */
    xListenSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM,
                                     FREERTOS_IPPROTO_TCP);
    if (xListenSocket == FREERTOS_INVALID_SOCKET) {
        printf("Failed to create listen socket.\n");
        echoServerCleanup(ctx, &pwMapList);
        vTaskDelete(NULL);
        return;
    }

    /* Set receive timeout so accept doesn't block forever */
    FreeRTOS_setsockopt(xListenSocket, 0, FREERTOS_SO_RCVTIMEO,
                         &xRecvTimeout, sizeof(xRecvTimeout));

    memset(&xBindAddr, 0, sizeof(xBindAddr));
    xBindAddr.sin_port = FreeRTOS_htons(ECHO_SERVER_PORT);

    xBindAddr.sin_family = FREERTOS_AF_INET;

    if (FreeRTOS_bind(xListenSocket, &xBindAddr, sizeof(xBindAddr)) != 0) {
        printf("Failed to bind to port %d.\n", ECHO_SERVER_PORT);
        FreeRTOS_closesocket(xListenSocket);
        echoServerCleanup(ctx, &pwMapList);
        vTaskDelete(NULL);
        return;
    }

    if (FreeRTOS_listen(xListenSocket, 1) != 0) {
        printf("FreeRTOS_listen failed.\n");
        FreeRTOS_closesocket(xListenSocket);
        echoServerCleanup(ctx, &pwMapList);
        vTaskDelete(NULL);
        return;
    }
    printf("Listening on port %d...\n", ECHO_SERVER_PORT);

    /* Accept loop */
    for (;;) {
        xClientAddrLen = sizeof(xClientAddr);
        xClientSocket = FreeRTOS_accept(xListenSocket, &xClientAddr,
                                         &xClientAddrLen);

        if (xClientSocket != NULL &&
            xClientSocket != FREERTOS_INVALID_SOCKET) {

            TickType_t xTimeout = pdMS_TO_TICKS(300000); /* 5 minutes */

            /* Set timeouts on the client socket */
            FreeRTOS_setsockopt(xClientSocket, 0, FREERTOS_SO_RCVTIMEO,
                                 &xTimeout, sizeof(xTimeout));
            FreeRTOS_setsockopt(xClientSocket, 0, FREERTOS_SO_SNDTIMEO,
                                 &xTimeout, sizeof(xTimeout));

            printf("Client connected.\n");

            if (echoServerAccept(ctx, &ssh, &pwMapList,
                                  &xClientSocket) == 0) {
                echoServerLoop(ssh);
            }

            if (ssh != NULL) {
                wolfSSH_free(ssh);
                ssh = NULL;
            }

            FreeRTOS_closesocket(xClientSocket);
            printf("Client disconnected. Waiting for next connection...\n");
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


/*
 * FreeRTOS and FreeRTOS-Plus-TCP required hooks/callbacks
 */

void vApplicationIdleHook(void)
{
    usleep(15000);
}

void vApplicationIPNetworkEventHook_Multi(eIPCallbackEvent_t eNetworkEvent,
    struct xNetworkEndPoint* pxEndPoint)
{
    (void)pxEndPoint;
    if (eNetworkEvent == eNetworkUp) {
        printf("Network interface is up.\n");
    }
}

/* Called by FreeRTOS-Plus-TCP when a ping reply is received */
void vApplicationPingReplyHook(ePingReplyStatus_t eStatus,
                                uint16_t usIdentifier)
{
    (void)eStatus;
    (void)usIdentifier;
}

/* Provide a random number for TCP sequence numbers */
uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress,
    uint16_t usSourcePort, uint32_t ulDestinationAddress,
    uint16_t usDestinationPort)
{
    (void)ulSourceAddress;
    (void)usSourcePort;
    (void)ulDestinationAddress;
    (void)usDestinationPort;

    /* Use wolfCrypt RNG for production; simple rand() for this demo */
    return (uint32_t)rand();
}

BaseType_t xApplicationGetRandomNumber(uint32_t* pulNumber)
{
    *pulNumber = (uint32_t)rand();
    return pdTRUE;
}

/* heap_3.c uses malloc/free, so these aren't meaningful, but the TCP
 * stack references them for diagnostic prints */
size_t xPortGetMinimumEverFreeHeapSize(void)
{
    return 0;
}

size_t xPortGetFreeHeapSize(void)
{
    return 0;
}


int main(void)
{
    /* Disable stdout buffering so output shows immediately */
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("Starting wolfSSH Echo Server with FreeRTOS + FreeRTOS-Plus-TCP\n");

    /* Initialize the FreeRTOS-Plus-TCP stack */
    FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress,
                     ucDNSServerAddress, ucMACAddress);

    /* Create the SSH echo server task.
     * Priority must be lower than the IP task (configMAX_PRIORITIES - 2). */
    xTaskCreate(sshEchoServerTask, "SSHEcho",
                configMINIMAL_STACK_SIZE * 4, NULL,
                tskIDLE_PRIORITY + 1, NULL);

    /* Start the FreeRTOS scheduler - does not return */
    vTaskStartScheduler();

    return 0;
}
