#pragma once

#include "esp_err.h"

extern esp_err_t mock_nvs_flash_init_ret;
extern esp_err_t mock_nvs_flash_erase_ret;

esp_err_t nvs_flash_init(void);
esp_err_t nvs_flash_erase(void);