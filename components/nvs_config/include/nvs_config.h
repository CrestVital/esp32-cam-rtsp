#pragma once

#include <stdint.h>
#include "esp_err.h"

/** Maximum length for string fields (including null terminator). */
#define NVS_CONFIG_STR_MAX_LEN  64

/**
 * @brief Device configuration — all parameters used across firmware modules.
 *
 * Populated by config_load() and passed between modules by pointer.
 * All string fields are null-terminated and bounded to NVS_CONFIG_STR_MAX_LEN.
 */
typedef struct {
    char     wifi_ssid[NVS_CONFIG_STR_MAX_LEN];   /**< WiFi SSID */
    char     wifi_pass[NVS_CONFIG_STR_MAX_LEN];   /**< WiFi password */
    uint16_t rtsp_port;                            /**< RTSP server port (default: 554) */
    uint16_t cam_width;                            /**< Camera frame width in pixels (default: 1280) */
    uint16_t cam_height;                           /**< Camera frame height in pixels (default: 720) */
    uint8_t  cam_fps;                              /**< Camera frames per second (default: 15) */
    int8_t   cam_brightness;                       /**< Camera brightness adjustment -2..2 (default: 0) */
    char     auth_user[NVS_CONFIG_STR_MAX_LEN];   /**< HTTP Basic Auth username */
    char     auth_pass_hash[NVS_CONFIG_STR_MAX_LEN]; /**< HTTP Basic Auth password hash (SHA-256 hex) */
    char     mdns_name[NVS_CONFIG_STR_MAX_LEN];   /**< mDNS hostname (default: "espcam") */
} app_config_t;

/**
 * @brief Initialise the NVS config module.
 *
 * Must be called after nvs_flash_init() succeeds in app_main().
 * Loads configuration from NVS; applies defaults for missing/invalid keys.
 * On the very first boot (empty NVS) all defaults are applied in memory only;
 * they are not persisted until an explicit config_save() call is made.
 *
 * @param[out] cfg  Pointer to caller-allocated app_config_t to populate.
 * @return          ESP_OK on success (always, after defaults are applied for any
 *                  NVS error).
 *                  ESP_ERR_INVALID_ARG if cfg is NULL.
 */
esp_err_t config_init(app_config_t *cfg);

/**
 * @brief Load configuration from NVS into cfg.
 *
 * For every key that is absent or invalid, the default value is applied.
 * Does not write anything to NVS.
 *
 * @param[out] cfg  Pointer to app_config_t to populate.
 * @return          ESP_OK on success, error code otherwise.
 */
esp_err_t config_load(app_config_t *cfg);

/**
 * @brief Save all fields of cfg to NVS and commit.
 *
 * Validates numeric fields and mdns_name before writing (rejects rtsp_port == 0,
 * cam_width out of [160..1920], cam_height out of [120..1080], cam_fps out of
 * [1..60], cam_brightness out of [-2..2], and empty mdns_name; wifi_ssid,
 * wifi_pass, auth_user, and auth_pass_hash may be empty).
 * Calls nvs_commit() after all fields are written.
 *
 * @param[in]  cfg  Pointer to app_config_t to persist.
 * @return          ESP_OK on success.
 *                  ESP_ERR_INVALID_ARG if cfg is NULL or a field is invalid.
 *                  Propagates nvs_set_* / nvs_commit() error codes on flash failure.
 */
esp_err_t config_save(const app_config_t *cfg);

/**
 * @brief Erase all NVS data and re-initialise with factory defaults.
 *
 * Calls nvs_flash_erase(), then nvs_flash_init(), then populates cfg with
 * defaults and calls config_save() to persist them.
 *
 * @param[out] cfg  Pointer to app_config_t to populate with defaults after reset.
 * @return          ESP_OK on success, error code otherwise.
 */
esp_err_t config_reset(app_config_t *cfg);
