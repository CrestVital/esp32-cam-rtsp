#pragma once

#include <stdint.h>
#include "esp_err.h"

/* 64 bytes covers the max SSID length (32 octets) plus null terminator,
 * with room to spare for WPA passphrases, mDNS names, and auth strings. */
#define NVS_CONFIG_STR_MAX_LEN  64

/* Selects the active network transport for this device.
 *
 * Stored in NVS as uint8_t under key "net_mode". Values outside
 * [0..2] are invalid and replaced with NETWORK_MODE_WIFI on load. */
typedef enum {
    NETWORK_MODE_WIFI     = 0,   /* WiFi only (default)          */
    NETWORK_MODE_ETHERNET = 1,   /* Ethernet only                */
    NETWORK_MODE_BOTH     = 2,   /* WiFi + Ethernet dual-stack   */
} network_mode_t;

/* Holds all persistent device configuration loaded from NVS.
 *
 * A single global instance of this struct is populated at startup by
 * config_load() and flushed back to NVS by config_save(). */
typedef struct {
    char     wifi_ssid[NVS_CONFIG_STR_MAX_LEN];
    char     wifi_pass[NVS_CONFIG_STR_MAX_LEN];
    uint16_t rtsp_port;
    uint16_t cam_width;
    uint16_t cam_height;
    uint8_t  cam_fps;
    int8_t   cam_brightness;
    char     auth_user[NVS_CONFIG_STR_MAX_LEN];
    char     auth_pass_hash[NVS_CONFIG_STR_MAX_LEN];   /* hash, not plaintext */
    char     mdns_name[NVS_CONFIG_STR_MAX_LEN];
    network_mode_t network_mode;
} app_config_t;

/* Initialise the NVS partition and load defaults if absent. */
esp_err_t config_init(app_config_t *cfg);

/* Read all keys from NVS into *cfg; apply defaults for missing/invalid. */
esp_err_t config_load(app_config_t *cfg);

/* Validate and persist *cfg to NVS; rejects out-of-range fields. */
esp_err_t config_save(const app_config_t *cfg);

/* Wipe the stored config and re-apply compile-time defaults. */
esp_err_t config_reset(app_config_t *cfg);