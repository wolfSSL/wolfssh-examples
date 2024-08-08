/* main.c
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
 *
 * Adapted from Public Domain Espressif ENC28J60 Example
 *
 * https://github.com/espressif/esp-idf/blob/047903c612e2c7212693c0861966bf7c83430ebf/examples/ethernet/enc28j60/main/enc28j60_example_main.c#L1
 *
 *
 * WARNING: although this code makes use of the UART #2 (as UART_NUM_0)
 *
 * DO NOT LEAVE ANYTHING CONNECTED TO TXD2 (GPIO 15) and RXD2 (GPIO 13)
 *
 * In particular, GPIO 15 must not be high during programming.
 *
 */

#include "sdkconfig.h"
/* include ssh_server_config.h first  */
#include "my_config.h"
#include "ssh_server_config.h"
#include "time_helper.h"
#include "main.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* wolfSSL */
#ifndef WOLFSSL_USER_SETTINGS
    #error "WOLFSSL_USER_SETTINGS should have been defined in project cmake"
#endif
/* Important: make sure settings.h appears before any other wolfSSL headers */
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/port/Espressif/esp32-crypt.h>
#include <wolfssl/wolfcrypt/logging.h>
#ifndef WOLFSSL_ESPIDF
    #error "Problem with wolfSSL user_settings."
    #error "Check [project]/components/wolfssl/include"
#endif
#ifdef WOLFSSL_STALE_EXAMPLE
    #warning "This project is configured using local, stale wolfSSL code. See Makefile."
#endif

/* See ssh_server_config.h for optional physical ethernet: USE_ENC28J60 */
#ifdef USE_ENC28J60
    #include <enc28j60_helper.h>
#endif

#ifdef USE_ENC28J60
    /* no WiFi when using external ethernet */
#else
    #include "wifi_connect.h"
#endif

#include "ssh_server.h"

/* logging
 *
 * see
 *   https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html
 *   https://github.com/wolfSSL/wolfssl/blob/master/wolfssl/wolfcrypt/logging.h
 */
#ifdef LOG_LOCAL_LEVEL
    #undef LOG_LOCAL_LEVEL
#endif
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <esp_log.h>

#define THIS_MAX_MAIN_STACK_SIZE 6000

static const char *TAG = "SSH Server main";

/* 10 seconds, used for heartbeat message in thread */
static TickType_t DelayTicks = (60000 / portTICK_PERIOD_MS);


void server_session(void* args)
{
    while (1) {
        server_test(args);
        vTaskDelay(DelayTicks ? DelayTicks : 1); /* Minimum delay = 1 tick */

#ifdef DEBUG_WDT
        /* if we get panic faults, perhaps the watchdog needs attention? */
        esp_task_wdt_reset();
#endif
    }
}

/*
 * there may be any one of multiple ethernet interfaces
 * do we have one or not?
 **/
bool NoEthernet()
{
    bool ret = true;
#ifdef USE_ENC28J60
    /* the ENC28J60 is only available if one has been installed  */
    if (EthernetReady_ENC28J60()) {
        ret = false;
    }
#endif

#ifndef USE_ENC28J60
    /* WiFi is pretty much always available on the ESP32 */
    if (wifi_ready()) {
        ret = false;
    }
#endif

    return ret;
}

#if defined(WOLFSSH_SERVER_IS_AP) || defined(WOLFSSH_SERVER_IS_STA)
void init_nvsflash()
{
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, "Setting up nvs flash for WiFi.");

    ret = nvs_flash_init();

#if defined(CONFIG_IDF_TARGET_ESP8266)
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
#else
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES
          ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND
       ) {

        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
#endif

    ESP_ERROR_CHECK(ret);
}
#endif

/*
 * main initialization for UART, optional ethernet, time, etc.
 */
int init(void)
{
    int ret = ESP_OK;
    TickType_t EthernetWaitDelayTicks = (1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Begin main init.");

    #ifdef DEBUG_WOLFSSH
    {
        ESP_LOGI(TAG, "wolfSSH debugging on.");
        wolfSSH_Debugging_ON();
    }
    #endif

    #ifdef DEBUG_WOLFSSL
    {
        ESP_LOGI(TAG, "wolfSSL debugging on.");
        wolfSSL_Debugging_ON();
        ESP_LOGI(TAG, "Debug ON");
    }
    /* TODO ShowCiphers(); */
    #endif

    /* Set time for cert validation.
     * Some lwIP APIs, including SNTP functions, are not thread safe. */
    ret = set_time(); /* need to setup NTP before WiFi */


#ifdef DISABLE_SSH_UART
    setvbuf(stdout, NULL, _IONBF, 0);
#else
    /* Our "External" device will be the UART, connected to the SSH server */
    init_UART();
#endif

    /*
     * here we have one of three options:
     *
     * Wired Ethernet: USE_ENC28J60
     *
     * WiFi Access Point: WOLFSSH_SERVER_IS_AP
     *
     * WiFi Station: WOLFSSH_SERVER_IS_STA
     **/
    #if defined(USE_ENC28J60)
    {
        /* wired ethernet */
        ESP_LOGI(TAG, "Found USE_ENC28J60 config.");
        init_ENC28J60(MY_MAC_ADDRESS);
    }

    #elif defined( WOLFSSH_SERVER_IS_AP)
    {
        /* acting as an access point */
        init_nvsflash();

        ESP_LOGI(TAG, "Begin setup WiFi Soft AP.");
        wifi_init_softap();
        ESP_LOGI(TAG, "End setup WiFi Soft AP.");
    }

    #elif defined(WOLFSSH_SERVER_IS_STA)
    {
        /* acting as a WiFi Station (client) */
        init_nvsflash();

        ESP_LOGI(TAG, "Begin setup WiFi STA.");
        wifi_init_sta();
        ESP_LOGI(TAG, "End setup WiFi STA.");
    }
    #else
    {
        /* we should never get here */
        while (1)
        {
            ESP_LOGE(TAG,
                "ERROR: No network is defined... choose USE_ENC28J60, \
                            WOLFSSH_SERVER_IS_AP, or WOLFSSH_SERVER_IS_STA ");
            vTaskDelay(EthernetWaitDelayTicks ? EthernetWaitDelayTicks : 1);
        }
    }
    #endif

    while (NoEthernet()) {
        ESP_LOGI(TAG,"Waiting for ethernet...");
        vTaskDelay(EthernetWaitDelayTicks ? EthernetWaitDelayTicks : 1);
    }

    ESP_LOGI(TAG,"inet_pton"); /* TODO */

    /* Once we are connected to the network, start & wait for NTP time */
    ret = set_time_wait_for_ntp();

    if (ret < -1) {
        /* a value of -1 means there was no NTP server, so no need to wait */
        ESP_LOGI(TAG, "Waiting 10 more seconds for NTP to complete." );
        vTaskDelay(10000 / portTICK_PERIOD_MS); /* brute-force solution */
        esp_show_current_datetime();
    }

    ret = wolfSSH_Init();

    return ret;
}

/**
 * @brief Checks the netif description if it contains specified prefix.
 * All netifs created withing common connect component are prefixed with
 * the module TAG, so it returns true if the specified netif is owned
 * by this module

TODO

static bool is_our_netif(const char *prefix, esp_netif_t *netif) {
    return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix) - 1) == 0;
}

*/

void app_main(void)
{
    /* main stack size: 4048 */
    int stack_start = 0;

    ESP_LOGI(TAG, "------------------ wolfSSH UART Example ----------------");
    ESP_LOGI(TAG, "--------------------------------------------------------");
    ESP_LOGI(TAG, "--------------------------------------------------------");
    ESP_LOGI(TAG, "---------------------- BEGIN MAIN ----------------------");
    ESP_LOGI(TAG, "--------------------------------------------------------");
    ESP_LOGI(TAG, "--------------------------------------------------------");
#ifdef CONFIG_ESP_MAIN_TASK_STACK_SIZE
    ESP_LOGI(TAG, "ESP_TASK_MAIN_STACK: %d", CONFIG_ESP_MAIN_TASK_STACK_SIZE);
    if (CONFIG_ESP_MAIN_TASK_STACK_SIZE > THIS_MAX_MAIN_STACK_SIZE) {
        ESP_LOGW(TAG, "Warning: excessively large main task size!");
    }
#endif
#ifdef ESP_TASK_MAIN_STACK
    ESP_LOGI(TAG, "ESP_TASK_MAIN_STACK: %d", ESP_TASK_MAIN_STACK);
#endif
#ifdef ESP_TASK_MAIN_STACK
    ESP_LOGI(TAG, "ESP_TASK_MAIN_STACK: %d", ESP_TASK_MAIN_STACK);
#endif
#ifdef TASK_EXTRA_STACK_SIZE
    ESP_LOGI(TAG, "TASK_EXTRA_STACK_SIZE: %d", TASK_EXTRA_STACK_SIZE);
#endif
#ifdef INCLUDE_uxTaskGetStackHighWaterMark
    ESP_LOGI(TAG, "CONFIG_ESP_MAIN_TASK_STACK_SIZE = %d bytes (%d words)",
                   CONFIG_ESP_MAIN_TASK_STACK_SIZE,
                   (int)(CONFIG_ESP_MAIN_TASK_STACK_SIZE / sizeof(void*)));

    /* Returns the high water mark of the stack associated with xTask. That is,
     * the minimum free stack space there has been (in bytes not words, unlike
     * vanilla FreeRTOS) since the task started. The smaller the returned
     * number the closer the task has come to overflowing its stack.
     * see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_idf.html
     */
    stack_start = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG, "Stack Start HWM: %d bytes", stack_start);
#endif
    ESP_LOGI(TAG, "UART_RX_TASK_STACK_SIZE:   %d bytes",
                   UART_RX_TASK_STACK_SIZE);
    ESP_LOGI(TAG, "UART_TX_TASK_STACK_SIZE:   %d bytes",
                   UART_TX_TASK_STACK_SIZE);
    ESP_LOGI(TAG, "SERVER_SESSION_STACK_SIZE: %d bytes",
                   SERVER_SESSION_STACK_SIZE);
#ifdef ESP_ENABLE_WOLFSSH
    ESP_LOGI(TAG, "SSH DEFAULT_WINDOW_SZ:     %d bytes",
                   DEFAULT_WINDOW_SZ);
#else
    #error "ESP_ENABLE_WOLFSSH ust be enabled for this project"
#endif
#if defined(HAVE_VERSION_EXTENDED_INFO)
    esp_ShowExtendedSystemInfo();
#endif

    init();
    /* Note that by the time we get here, the scheduler is already running!
     * See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications
     * Unlike Vanilla FreeRTOS, users must not call vTaskStartScheduler();
     *
     * All of the tasks are at the same, highest idle priority, so they will
     * all get equal attentiom when priority was set to
     *   configMAX_PRIORITIES - [1,2,3]
     * there was an odd WDT timeout warning.
     */
#ifndef DISABLE_SSH_UART
    xTaskCreate(uart_rx_task, "uart_rx_task",
                UART_RX_TASK_STACK_SIZE, NULL,
                tskIDLE_PRIORITY, NULL);

    xTaskCreate(uart_tx_task, "uart_tx_task",
                UART_TX_TASK_STACK_SIZE, NULL,
                tskIDLE_PRIORITY, NULL);
#endif

    xTaskCreate(server_session, "server_session",
                SERVER_SESSION_STACK_SIZE, NULL,
                tskIDLE_PRIORITY, NULL);

#ifndef NO_EXAMPLE_HEARTBEAT
    for (;;) {
        /* we're not actually doing anything here, other than a heartbeat message */
        ESP_LOGI(TAG,"wolfSSH Server main loop heartbeat!");

        taskYIELD();
        vTaskDelay(DelayTicks ? DelayTicks : 1); /* Minimum delay = 1 tick */
    }
#endif

    /* TODO this is unreachable with RTOS threads, do we ever want to shut down? */
    /* wolfSSH_Cleanup(); */
} /* app_main */
