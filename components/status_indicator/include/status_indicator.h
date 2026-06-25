#pragma once

#include "esp_err.h"

/**
 * @brief Device status states for the status indicator.
 *
 * Maps each firmware lifecycle state to a visual or log representation.
 * Output depends on board capabilities configured via Kconfig.
 */
typedef enum {
    INDICATOR_STATE_BOOT            = 0,  /**< Device initialising */
    INDICATOR_STATE_WIFI_CONNECTING,      /**< WiFi association in progress */
    INDICATOR_STATE_WIFI_CONNECTED,       /**< WiFi connected, IP obtained */
    INDICATOR_STATE_WIFI_ERROR,           /**< WiFi connection failed */
    INDICATOR_STATE_STREAMING,            /**< RTSP stream active */
    INDICATOR_STATE_OTA,                  /**< OTA update in progress */
    INDICATOR_STATE_ERROR,                /**< Critical firmware error */
    INDICATOR_STATE_COUNT,                /**< Sentinel */
} indicator_state_t;

/**
 * @brief Initialise the status indicator.
 *
 * Selects backend (display, LED, log-only) from Kconfig. For LED boards,
 * configures LEDC PWM and spawns indicator_task (priority 2, stack 2048
 * bytes). Returns ESP_ERR_INVALID_STATE on re-init.
 *
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
esp_err_t status_indicator_init(void);

/**
 * @brief Deinitialise the status indicator.
 *
 * Stops indicator_task if running, deinitialises LEDC if used, resets
 * state. Returns ESP_ERR_INVALID_STATE if called before init.
 *
 * @return ESP_OK on success.
 */
esp_err_t status_indicator_deinit(void);

/**
 * @brief Set the device status state.
 *
 * Thread-safe. Forwards to active backend. No-op if state unchanged.
 *
 * @param state  The new indicator state.
 * @return ESP_OK, ESP_ERR_INVALID_STATE if not initialised,
 *         ESP_ERR_INVALID_ARG if state >= INDICATOR_STATE_COUNT.
 */
esp_err_t status_indicator_set_state(indicator_state_t state);
