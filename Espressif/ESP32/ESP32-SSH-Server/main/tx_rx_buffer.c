/* uart_hlper.c
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
#include "tx_rx_buffer.h"
#include "int_to_string.h"

#include <esp_log.h>
#include <task.h>

#ifdef DISABLE_SSH_UART

#else
    #define SSH_WELCOME_MESSAGE "\r\n"                                      \
                                "Welcome to wolfSSL ESP32 SSH UART Server!" \
                                "\n\r\n\r"
    #define SSH_GPIO_MESSAGE    "You are now connected to UART "
    #define SSH_GPIO_MESSAGE_TX "Tx GPIO "
    #define SSH_GPIO_MESSAGE_RX ", Rx GPIO "
    #define SSH_READY_MESSAGE   ".\r\n\r\n"                                 \
                                "Press [Enter] to start. Ctrl-C to exit."   \
                                "\r\n\r\n"
#endif

static const char *TAG = "tx_rx_buf";


/* TODO define size and check when assigning */
/* Shared external, non ssh buffers. typically the UART */
volatile static char  _ExternalReceiveBuffer[EXT_RX_BUF_MAX_SZ];
volatile static char  _ExternalTransmitBuffer[EXT_TX_BUF_MAX_SZ];
volatile static int _ExternalReceiveBufferSz = 0;
volatile static int _ExternalTransmitBufferSz = 0;


static SemaphoreHandle_t _xExternalReceiveBuffer_Semaphore = NULL;
static SemaphoreHandle_t _xExternalTransmitBuffer_Semaphore = NULL;


/*
 * initialize the external buffer (typically a UART) Receive Semaphore.
 */
int InitReceiveSemaphore(void)
{
    int ret = ESP_OK;
    if (_xExternalReceiveBuffer_Semaphore == NULL) {
        ESP_LOGV(TAG, "Enter InitReceiveSemaphore.");

        /* the case of recursive mutexes is interesting, so alert */
    #ifdef configUSE_RECURSIVE_MUTEXES
        /* see semphr.h */
        ESP_LOGI(TAG,"InitSemaphore UART Rx "
                     "configUSE_RECURSIVE_MUTEXES enabled");
    #endif

        _xExternalReceiveBuffer_Semaphore =  xSemaphoreCreateMutex();
    #ifdef configUSE_RECURSIVE_MUTEXES
        /* see semphr.h */
        ESP_LOGV(TAG,"_xExternalReceiveBuffer_Semaphore complete");
        #ifdef INCLUDE_uxTaskGetStackHighWaterMark
            ESP_LOGV(TAG, "1 rx Stack HWM: %d\n",
            uxTaskGetStackHighWaterMark(NULL));
        #endif
    #endif
    }
    else {
        ESP_LOGV(TAG, "Rx _xExternalTransmitBuffer_Semaphore "
                      "already initialized");
    }
    return ret;
}

/*
 * initialize the external buffer (typically a UART) Transmit Semaphore.
 */
int InitTransmitSemaphore(void)
{
    int ret = ESP_OK;
    if (_xExternalTransmitBuffer_Semaphore == NULL) {

        /* the case of recursive mutexes is interesting, so alert */
    #ifdef configUSE_RECURSIVE_MUTEXES
        /* see semphr.h */
        ESP_LOGI(TAG, "InitSemaphore Tx configUSE_RECURSIVE_MUTEXES enabled");
    #endif
    #ifdef INCLUDE_uxTaskGetStackHighWaterMark
        ESP_LOGV(TAG, "Tx Stack HWM: %d\n", uxTaskGetStackHighWaterMark(NULL));
    #endif

        _xExternalTransmitBuffer_Semaphore =  xSemaphoreCreateMutex();

#ifdef configUSE_RECURSIVE_MUTEXES
        /* see semphr.h */
        ESP_LOGV(TAG, "_xExternalReceiveBuffer_Semaphore complete");
    #endif
    }
    else {
        ESP_LOGV(TAG, "Tx _xExternalTransmitBuffer_Semaphore"
                      "already initialized");
    }
    return ret;
}

/*
 * return true if the Rx buffer is exactly 1 char long and contains charValue
 */
bool __attribute__((optimize("O0")))
ExternalReceiveBuffer_IsChar(char charValue)
{
    bool ret = false; /* assume not a match unless proven otherwise */
    char thisChar; /* typically looking at position 0, e.g. user typing */

    InitReceiveSemaphore();
    if (xSemaphoreTake(_xExternalReceiveBuffer_Semaphore,
                       (TickType_t) 10) == pdTRUE) {

        /* the entire thread-safety wrapper is for this code segment */
        {
            if (_ExternalReceiveBufferSz == 1)
            {
                thisChar =  _ExternalReceiveBuffer[0];
                ret = (thisChar == charValue);
           }
        }
        xSemaphoreGive(_xExternalReceiveBuffer_Semaphore);
    }
    else {
        /* we could not get the semaphore to update the value!
         * TODO how to handle this? */
        ret = false;
    }

    return ret;
}

volatile char* __attribute__((optimize("O0"))) ExternalReceiveBuffer()
{
    return _ExternalReceiveBuffer;
}

volatile char* __attribute__((optimize("O0"))) ExternalTransmitBuffer()
{
    return _ExternalTransmitBuffer;
}

/* RTOS-safe positional value of current receive buffer position.
 * care should be take when using the number as more chars may have arrived!
 */
int ExternalReceiveBufferSz(void)
{
    int ret = 0;

    InitReceiveSemaphore();
    if (xSemaphoreTake(_xExternalReceiveBuffer_Semaphore,
                       (TickType_t) 10) == pdTRUE) {

        /* the entire thread-safety wrapper is for this code statement */
        {
            ret = _ExternalReceiveBufferSz;
        }
        xSemaphoreGive(_xExternalReceiveBuffer_Semaphore);
    }
    else {
        /* we could not get the semaphore to update the value!
         * TODO how to handle this?  */
        ret = 0;
    }

#ifdef SSH_SERVER_PROFILE
    if (ret > MaxSeenRxSize) {
        MaxSeenRxSize = ret;
    }
#endif
    return ret;
}

/* RTOS-safe positional value of current transmit buffer position.
 * care should be take when using the number as more chars may have been sent!
 */
int ExternalTransmitBufferSz(void)
{
    int ret;

    InitTransmitSemaphore();
    if (xSemaphoreTake(_xExternalTransmitBuffer_Semaphore,
                      (TickType_t) 10) == pdTRUE) {

        /* the entire thread-safety wrapper is for this code statement */
        {
            ret = _ExternalTransmitBufferSz;
        }

        xSemaphoreGive(_xExternalTransmitBuffer_Semaphore);
    }
    else {
        /* we could not get the semaphore to update the value!
         * TODO how to handle this? */
        ret = 0;
    }

#ifdef SSH_SERVER_PROFILE
    if (ret > MaxSeenTxSize) {
        MaxSeenTxSize = ret;
    }
#endif

    return ret;
}

/*
 * returns zero if ExternalReceiveBufferSz successfully assigned
 */
int Set_ExternalReceiveBufferSz(int n)
{
    int ret = 0; /* we assume success unless proven otherwise */

    InitReceiveSemaphore();
    if ((n >= 0) && (n < EXT_RX_BUF_MAX_SZ - 1)) {
        /* only assign valid buffer sizes */
        if (xSemaphoreTake(_xExternalReceiveBuffer_Semaphore,
            (TickType_t) 10) == pdTRUE) {

            /* the entire thread-safety wrapper is for this code statement */
            {
                _ExternalReceiveBufferSz = n;

                /* ensure the next char is zero, in case the stuffer of data
                 * does not do it */
                _ExternalReceiveBuffer[n + 1] = 0;
            }

            xSemaphoreGive(_xExternalReceiveBuffer_Semaphore);
        }
        else {
            /* we could not get the semaphore to update the value! */
            ret = 1;
        }
    }
    else {
        /* the new length must be between zero and maximum length! */
        ret = 1;
    }
    return ret;
}

/*
 * returns zero if ExternalTransmitBufferSz successfully assigned
 */
int Set_ExternalTransmitBufferSz(int n)
{
    int ret = 0; /* we assume success unless proven otherwise */

    InitTransmitSemaphore();

    /* we look at ByteExternalTransmitBufferSz + 1
     * since we also append our own zero string termination
     */
    if ( (n < 0) || (n > EXT_TX_BUF_MAX_SZ + 1) ) {
        /* New buffer size length must be between zero and maximum length! */
        ret = 1;
    }
    else {
        /* only assign valid buffer sizes */
        if (xSemaphoreTake(_xExternalTransmitBuffer_Semaphore,
            (TickType_t) 10) == pdTRUE) {

            /* the entire thread-safety wrapper is for this code statement */
            {
                _ExternalTransmitBufferSz = n;

                /* ensure the next char is zero,
                 * in case the stuffer of data does not do it */
                _ExternalTransmitBuffer[n + 1] = 0;
            }

            xSemaphoreGive(_xExternalTransmitBuffer_Semaphore);
        }
        else {
            /* we could not get the semaphore to update the value! */
            ret = 1;
        }
    } /* valid value of n */
    return ret;
}

/* Set the length of the external (typiclly UART) Rx buffer */
int Set_ExternalReceiveBuffer(byte *FromData, int sz)
{
    /* TODO this block has not yet been fully tested */
    int ret = ESP_OK; /* we assume success unless proven otherwise */

    if ( (sz < 0) || (sz > EXT_RX_BUF_MAX_SZ) ) {
        /* we'll only do a copy for valid sizes, otherwise return an error */
        ret = 1;
    }
    else {
        InitReceiveSemaphore();
        if (xSemaphoreTake(_xExternalReceiveBuffer_Semaphore,
            (TickType_t) 10) == pdTRUE) {

            /* The entire thread-safety wrapper is for this code statement.
             * in a multi-threaded environment, a different thread may be
             * reading or writing from the data. We need to ensure it is
             * static at the time of copy.
             */
            {
                memcpy(
                      (byte*)&_ExternalReceiveBuffer[_ExternalReceiveBufferSz],
                      FromData,
                      sz);

                _ExternalReceiveBufferSz = sz;
            } /* thread safe */

            xSemaphoreGive(_xExternalReceiveBuffer_Semaphore);
        }
        else {
            /* we could not get the semaphore to update the value!
             * TODO how to handle this? Will this ever occur?
             * If so, adjust wait time, above. */
            ESP_LOGW(TAG, "Warning: Set_ExternalReceiveBuffer failed to "
                          "take semaphore. Consisder wait time adjustment." );
            ret = 1;
        }
    }

    return ret;
}

/*
 * Thread safe populate ToData with the contents of _ExternalTransmitBuffer
 * returns the size of the data, negative values are errors.
 */
int Get_ExternalTransmitBuffer(byte **ToData)
{
    int ret = 0;
    InitTransmitSemaphore();

    if (xSemaphoreTake(_xExternalTransmitBuffer_Semaphore,
        (TickType_t) 10) == pdTRUE) {

        int thisSize = _ExternalTransmitBufferSz;
        if (thisSize == 0) {
            /* nothing to do */
            ESP_LOGI(TAG,"Get_ExternalTransmitBuffer size is already zero");
        }

        else {
            if (*ToData == NULL) {
                /* we could not allocate memory, so fail */
                ret = -1;
                ESP_LOGI(TAG,"Get_ExternalTransmitBuffer *ToData == NULL");
            }
            else {
                memcpy(*ToData,
                       (byte*)_ExternalTransmitBuffer,
                       thisSize
                      );

                _ExternalTransmitBufferSz = 0;
                ret = thisSize;
            }
        }
        xSemaphoreGive(_xExternalTransmitBuffer_Semaphore);
    }
    else {
        /* We could not get the semaphore to update the value!
         * TODO how to handle this? Wait time adjust? */
        ret = -1;
        ESP_LOGE(TAG,"ERROR: Get_ExternalTransmitBuffer SemaphoreTake "
                     "_xExternalTransmitBuffer_Semaphore failed.");
    }

    return ret;
}


/*
 * Thread safe populate FromData with the contents of _ExternalTransmitBuffer
 * returns the size of the data, negative values are errors.
 */
int Set_ExternalTransmitBuffer(byte *FromData, int sz)
{
    int ret = 0;

    /* here we need to call the thread-safe ExternalTransmitBufferSz() */
    int thisNewSize = sz + ExternalTransmitBufferSz();

    if ( (sz < 0) || (thisNewSize > EXT_TX_BUF_MAX_SZ) ) {
        /* we'll only do a copy for valid sizes, otherwise return an error */
        ret = -1;
    }
    else {
        InitTransmitSemaphore();
        if (xSemaphoreTake(_xExternalTransmitBuffer_Semaphore,
                           (TickType_t) 10) == pdTRUE) {

            /* Trim any trailing zeros from existing data by
             * adjusting our array pointer. */
            int thisStart = _ExternalTransmitBufferSz;
            while (thisStart > 0
                   &&
                   (_ExternalTransmitBuffer[thisStart - 1] == 0x0)) {
                thisStart--;
            }

            /* Actual new size may be smaller than above
             * if we trimmed some zeros. */
            thisNewSize = thisStart + sz;

            /* The entire thread-safety wrapper is for this code statement.
             * In a multi-threaded environment, a different thread may be
             * reading writing from the data. We need to ensure it is static
             * at the time of copy.
             */
            {
                memcpy((byte*)&_ExternalTransmitBuffer[thisStart],
                    FromData,
                    sz);

                _ExternalTransmitBufferSz = thisNewSize;
                ret = thisNewSize;
            }
            xSemaphoreGive(_xExternalTransmitBuffer_Semaphore);
        }
        else {
            /* we could not get the semaphore to update the value!
             * TODO how to handle this? */
            ESP_LOGW(TAG, "xSemaphoreTake failed in "
                          "Set_ExternalTransmitBuffer");
            ret = -1;
        }
    }

    return ret;
}


/*
 * Initialize external buffers and show welcome message.
 * TxPin and RxPin are for display purposes only.
 */
int  init_tx_rx_buffer(byte TxPin, byte RxPin)
{
    int ret = 0;

    char numStr[3] = { ' ', ' ', ' ' }; /* printable 2-digit GPIO numbers */

    /* These inits need to be called only once,
     * but can be repeatedly called as needed. */
    InitReceiveSemaphore();
    InitTransmitSemaphore();

    /*
     *  Init and stuff startup message in buffer.
     */
    Set_ExternalReceiveBufferSz(0);
    Set_ExternalTransmitBufferSz(0);

    /* Typically prints: "Welcome to wolfSSL ESP32 SSH UART Server!" */
    Set_ExternalTransmitBuffer((byte*)SSH_WELCOME_MESSAGE,
                               sizeof(SSH_WELCOME_MESSAGE)
                              );

    /* Typically prints "You are now connected to UART " */
    Set_ExternalTransmitBuffer((byte*)SSH_GPIO_MESSAGE,
                               sizeof(SSH_GPIO_MESSAGE)
                              );

    /* "Tx GPIO " */
    Set_ExternalTransmitBuffer((byte*)SSH_GPIO_MESSAGE_TX,
                               sizeof(SSH_GPIO_MESSAGE_TX)
                              );

    /* The number of the Tx pin, converted to a string.
     *
     * Note despite Clang IntelliSense detecting duplicate code,
     * it is NOT a duplicate. This one compares TxPin,
     * the next one compares RxPin. */
    if (TxPin <= 0x40) {
        int_to_dec((char*)&numStr, TxPin);
        numStr[2] = 0;
        Set_ExternalTransmitBuffer((byte*)&numStr, sizeof(numStr));
    }
    else {
        ESP_LOGE(TAG,"ERROR: bad value for TxPin");
        ret = 1;
    }

    /* ", Rx GPIO " */
    Set_ExternalTransmitBuffer((byte*)SSH_GPIO_MESSAGE_RX,
                               sizeof(SSH_GPIO_MESSAGE_RX)
                              );

    /* the number of the Rx pin, converted to a string */
    if (RxPin <= 0x40)
    {
        int_to_dec((char*)&numStr, RxPin);
        numStr[2] = 0;
        Set_ExternalTransmitBuffer((byte*)&numStr, sizeof(numStr));
    }
    else {
        ESP_LOGE(TAG,"ERROR: bad value for RxPin");
        ret = 1;
    }

    /* typically "Press [Enter] to start. Ctrl-C to exit" */
    Set_ExternalTransmitBuffer((byte*)SSH_READY_MESSAGE,
                               sizeof(SSH_READY_MESSAGE)
                              );
#ifdef INCLUDE_uxTaskGetStackHighWaterMark
    ESP_LOGI(TAG, "Stack HWM: %d\n", uxTaskGetStackHighWaterMark(NULL));
#endif
    return ret;
}
