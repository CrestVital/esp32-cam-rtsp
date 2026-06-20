#pragma once

#include "esp_err.h"

/* Injectable return values for nvs_flash_init/nvs_flash_erase.
 * Set e.g. mock_nvs_flash_init_ret = ESP_FAIL before calling the
 * function under test to simulate flash-init failure scenarios. */
extern esp_err_t mock_nvs_flash_init_ret;
extern esp_err_t mock_nvs_flash_erase_ret;

esp_err_t nvs_flash_init(void);
esp_err_t nvs_flash_erase(void);