#pragma once

/* Host-side stub for esp_netif.h.
 * Re-declares esp_netif_init and esp_netif_create_default_wifi_sta
 * so wifi_manager.c can include both <esp_netif.h> and <esp_wifi.h>
 * without conflict. Actual implementations live in mock_esp_wifi.c. */

#include "esp_err.h"

esp_err_t esp_netif_init(void);
void     *esp_netif_create_default_wifi_sta(void);
