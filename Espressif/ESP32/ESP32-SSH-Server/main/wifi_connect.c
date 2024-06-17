/* wifi_connect.c
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
#include "wifi_connect.h"
#include "my_config.h"

#ifndef  CONFIG_ESP_WIFI_CHANNEL
    #define CONFIG_ESP_WIFI_CHANNEL 1
#endif

#ifndef  CONFIG_ESP_MAX_STA_CONN
    #define CONFIG_ESP_MAX_STA_CONN 2
#endif

#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_log.h>

/* wolfSSL */
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/version.h>
#include <wolfssl/wolfcrypt/types.h>
#ifndef WOLFSSL_ESPIDF
    #warning "Problem with wolfSSL user_settings."
    #warning "Check components/wolfssl/include"
#endif

#if ESP_IDF_VERSION_MAJOR >= 5
#elif ESP_IDF_VERSION_MAJOR >= 4
    #include "protocol_examples_common.h"
#elif defined(CONFIG_IDF_TARGET_ESP8266)
    /* TODO */
#else
    const static int CONNECTED_BIT = BIT0;
    static EventGroupHandle_t wifi_event_group;
#endif

#if defined(ESP_IDF_VERSION_MAJOR) && defined(ESP_IDF_VERSION_MINOR)
    #if ESP_IDF_VERSION_MAJOR >= 4
        /* likely using examples, see wifi_connect.h */
    #elif defined(CONFIG_IDF_TARGET_ESP8266)
        /* TODO */
    #else
        /* TODO - still supporting pre V4 ? */
        const static int CONNECTED_BIT = BIT0;
        static EventGroupHandle_t wifi_event_group;
    #endif
    #if (ESP_IDF_VERSION_MAJOR == 5)
        #define HAS_WPA3_FEATURES
    #else
        #undef HAS_WPA3_FEATURES
    #endif
#else
    /* TODO Consider pre IDF v5? */
#endif

/* breadcrumb prefix for logging */
static const char *TAG = "wifi_connect";

/* we'll change WiFiEthernetReady in event handler
 *
 * see also wifi_ready()
 */
static volatile bool WiFiEthernetReady = 0;

esp_netif_ip_info_t my_ip;

#if ESP_IDF_VERSION_MAJOR < 4
    #if defined(CONFIG_IDF_TARGET_ESP8266)
        /* TODO */
    #else
    /* event handler for wifi events */
    static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
    {
        switch (event->event_id)
        {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
        #if ESP_IDF_VERSION_MAJOR >= 4
            ESP_LOGI(TAG, "got ip:" IPSTR "\n",
                     IP2STR(&event->event_info.got_ip.ip_info.ip));
        #else
            ESP_LOGI(TAG, "got ip:%s",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        #endif
            /* see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos_idf.html */
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
        }
        return ESP_OK;
    }
    /* not ESP8266 */
    #endif
#else

#ifdef CONFIG_ESP_MAXIMUM_RETRY
    #define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY
#else
    #define CONFIG_ESP_MAXIMUM_RETRY 5
#endif

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

#ifndef ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD
    #define CONFIG_ESP_WIFI_AUTH_WPA2_PSK 1
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event,
 * but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static int s_retry_num = 0;
ip_event_got_ip_t* event;

static void event_handler(void* arg,
                          esp_event_base_t event_base,
                          int32_t event_id,
                          void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        WiFiEthernetReady = 0;
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
        WiFiEthernetReady = 0;
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        event = (ip_event_got_ip_t*) event_data;
        my_ip = event->ip_info;

        wifi_show_ip();
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        WiFiEthernetReady = 1;
    }
}

int wifi_init_sta(void)
{
    int ret = ESP_OK;
#if defined(CONFIG_IDF_TARGET_ESP8266)
    ESP_LOGE(TAG, "Error: wifi_init_sta not implemented for ESP8266");
#else
    /* ESP32, non-ESP8266 WiFi begin */
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
            /* Authmode threshold resets to WPA2 as default if password matches
             * WPA2 standards (pasword len => 8). If you want to connect the
             * device to deprecated WEP/WPA networks, Please set the threshold
             * value WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with
             * length and format matching to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK
             * standards. */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        #ifdef HAS_WPA3_FEATURES
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        #endif
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    #ifdef CONFIG_EXAMPLE_WIFI_SSID
        if (XSTRCMP(CONFIG_EXAMPLE_WIFI_SSID, "myssid") == 0) {
            ESP_LOGW(TAG, "WARNING: CONFIG_EXAMPLE_WIFI_SSID is \"myssid\".");
            ESP_LOGW(TAG, "  Do you have a WiFi AP called \"myssid\", ");
            ESP_LOGW(TAG, "  or did you forget the ESP-IDF configuration?");
        }
    #else
        ESP_LOGW(TAG, "WARNING: CONFIG_EXAMPLE_WIFI_SSID not defined.");
    #endif

    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT)
     * or connection failed for the maximum number of re-tries (WIFI_FAIL_BIT).
     * The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned,
     * hence we can test which event actually happened. */
    #if defined(SHOW_SSID_AND_PASSWORD)
        ESP_LOGW(TAG, "Undefine SHOW_SSID_AND_PASSWORD "
                      "to not show SSID/password");
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                           EXAMPLE_ESP_WIFI_SSID,
                           EXAMPLE_ESP_WIFI_PASS);
        }
        else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                           EXAMPLE_ESP_WIFI_SSID,
                           EXAMPLE_ESP_WIFI_PASS);
        }
        else {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }
    #else
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "Connected to AP");
        }
        else if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(TAG, "Failed to connect to AP");
            ret = -1;
        }
        else {
            ESP_LOGE(TAG, "AP UNEXPECTED EVENT");
            ret = -2;
        }
    #endif /* SHOW_SSID_AND_PASSWORD */
#endif /* ESP32 or ESP8266 implementation */

    return ret;
}

int wifi_show_ip(void)
{
    int ret;
    if (event == NULL) {
        ret = ESP_FAIL;
    }
    else {
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ret = ESP_OK;
    }
    return ret;
}

int wifi_show_listening_ip(int port)
{
    int ret;
    if (event == NULL) {
        ret = ESP_FAIL;
    }
    else {
        ESP_LOGI(TAG, "Listening on port %d address: " IPSTR,
                       port , IP2STR(&my_ip.ip));
        ret = ESP_OK;
    }
    return ret;
}
#endif

/*
 * WiFi wifi_ap_event_handler() Public Domain Sample Code Credit Espressif
 *
 * See https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/softAP/main/softap_example_main.c
 *
 */
#ifndef MACSTR
    #define MACSTR "00:04:a3:12:34:56"
#endif

#if defined(USE_AP)
static void wifi_ap_event_handler(void* arg,
                                  esp_event_base_t event_base,
                                  int32_t event_id,
                                  void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event =
              (wifi_event_ap_staconnected_t*) event_data;

        ESP_LOGI(TAG,
            "station "MACSTR" join, AID=%d",
            MAC2STR(event->mac),
            event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event =
              (wifi_event_ap_stadisconnected_t*) event_data;

        ESP_LOGI(TAG,
            "station "MACSTR" leave, AID=%d",
            MAC2STR(event->mac),
            event->aid);
    }

    /* When acting as AP, we're always ready, as we're not awaiting connection
     * or IP addy. */
    WiFiEthernetReady = 1;
}


/*
 * WiFi wifi_init_softap() Public Domain Sample Code Credit Espressif
 *
 * See https://github.com/espressif/esp-idf/blob/master/examples/wifi/getting_started/softAP/main/softap_example_main.c
 *
 */
void wifi_init_softap(void)
{
    wifi_config_t wifi_config = {
        .ap = {
        .ssid = EXAMPLE_ESP_WIFI_AP_SSID,
        .ssid_len = strlen(EXAMPLE_ESP_WIFI_AP_SSID),
        .channel = EXAMPLE_ESP_WIFI_CHANNEL,
        .password = EXAMPLE_ESP_WIFI_AP_PASS,
        .max_connection = EXAMPLE_MAX_STA_CONN,
        .authmode = WIFI_AUTH_WPA2_PSK
        },
    };
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_ap_event_handler,
        NULL,
        NULL));

    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG,
        "wifi_init_softap finished. SSID:%s password:%s channel:%d",
        EXAMPLE_ESP_WIFI_AP_SSID,
        EXAMPLE_ESP_WIFI_AP_PASS,
        EXAMPLE_ESP_WIFI_CHANNEL);
}
#endif

/*
 * return true when above events determined that WiFi is actually ready.
 */
bool wifi_ready()
{
    ESP_LOGV(TAG, "wifi_ready check");

    return WiFiEthernetReady;
}
