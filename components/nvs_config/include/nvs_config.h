#pragma once

#include <stdint.h>
#include "esp_err.h"

#define NVS_CONFIG_STR_MAX_LEN  64

typedef struct {
    char     wifi_ssid[NVS_CONFIG_STR_MAX_LEN];
    char     wifi_pass[NVS_CONFIG_STR_MAX_LEN];
    uint16_t rtsp_port;
    uint16_t cam_width;
    uint16_t cam_height;
    uint8_t  cam_fps;
    int8_t   cam_brightness;
    char     auth_user[NVS_CONFIG_STR_MAX_LEN];
    char     auth_pass_hash[NVS_CONFIG_STR_MAX_LEN];
    char     mdns_name[NVS_CONFIG_STR_MAX_LEN];
} app_config_t;

esp_err_t config_init(app_config_t *cfg);
esp_err_t config_load(app_config_t *cfg);
esp_err_t config_save(const app_config_t *cfg);
esp_err_t config_reset(app_config_t *cfg);