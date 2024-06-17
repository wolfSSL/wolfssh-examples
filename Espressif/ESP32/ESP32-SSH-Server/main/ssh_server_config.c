/* ssh_server_config.c
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
#include "ssh_server_config.h"

const char* TAG = "ssh config";

int ssh_server_config_init(void)
{
    int ret = ESP_OK;
#ifdef DEBUG_WOLFSSH
    ESP_LOGI("init", "ssh_server_config_init");
#endif

    if ((SSH_SERVER_ECHO < 0) || (SSH_SERVER_ECHO > 1)) {
        ESP_LOGE(TAG, "Not a valid value for DISABLE_SSH_UART: %d",
                       SSH_SERVER_ECHO);
    }

    /* TODO make public the esp_util show_macro() */
/*  show_macro("NO_ESPIDF_DEFAULT",         STR_IFNDEF(NO_ESPIDF_DEFAULT)); */
    return ret;
}


