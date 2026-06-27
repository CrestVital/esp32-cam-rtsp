#pragma once

#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Initialise the WiFi subsystem.
 *
 * Performs esp_netif_init(), creates the default station netif, calls
 * esp_wifi_init() and esp_wifi_set_mode(WIFI_MODE_STA). On ESP32 (non-S3)
 * targets disables the Bluetooth controller to avoid RF coexistence issues.
 * Must be called once before any other wifi_manager function.
 *
 * @return ESP_OK on success, ESP_ERR_NOT_SUPPORTED if BOARD_HAS_WIFI == 0,
 *         ESP_ERR_INVALID_STATE if already initialised.
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Deinitialise the WiFi subsystem.
 *
 * Stops the reconnect task if running, calls esp_wifi_stop() and
 * esp_wifi_deinit(), then releases the netif. After this call the manager
 * returns to the uninitialised state and init() may be called again.
 *
 * @return ESP_OK on success, ESP_ERR_NOT_SUPPORTED if BOARD_HAS_WIFI == 0.
 */
esp_err_t wifi_manager_deinit(void);

/**
 * @brief Connect to the given access point and enable auto-reconnect.
 *
 * Stores SSID/password in the static connect context, calls
 * esp_wifi_start(). Connection is initiated by the WIFI_EVENT_STA_START
 * event handler, which calls esp_wifi_connect(). Enables the exponential
 * backoff reconnect loop on disconnection.
 *
 * @param ssid     Null-terminated SSID string. Must not be NULL or empty.
 * @param password Null-terminated WPA passphrase. May be NULL (open network).
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if ssid is NULL or empty,
 *         ESP_ERR_INVALID_STATE if not initialised,
 *         ESP_ERR_NOT_SUPPORTED if BOARD_HAS_WIFI == 0.
 */
esp_err_t wifi_manager_connect(const char *ssid, const char *password);

/**
 * @brief Disconnect from the current access point and suppress reconnect.
 *
 * Calls esp_wifi_disconnect() and sets s_reconnect_enabled to false so
 * the exponential-backoff task does not attempt to rejoin the network.
 *
 * @return ESP_OK on success, ESP_ERR_NOT_SUPPORTED if BOARD_HAS_WIFI == 0.
 */
esp_err_t wifi_manager_disconnect(void);

/**
 * @brief Query whether the station has obtained an IP address.
 *
 * Returns true only after IP_EVENT_STA_GOT_IP has fired; false in all
 * other states (disconnected, connecting, or never initialised).
 *
 * @return true if connected and IP obtained, false otherwise.
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Persist WiFi credentials to NVS.
 *
 * Stores ssid and password under namespace "wifi_cfg" with keys
 * "wifi_ssid" and "wifi_pass". NVS handle is committed and closed on
 * every path. Existing values are overwritten.
 *
 * @param ssid     Null-terminated SSID. Must not be NULL or empty.
 * @param password Null-terminated passphrase.
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG for NULL/empty ssid,
 *         ESP_ERR_NOT_SUPPORTED if BOARD_HAS_WIFI == 0,
 *         or an NVS/ESP-IDF error code.
 */
esp_err_t wifi_manager_save_credentials(const char *ssid, const char *password);

/**
 * @brief Load WiFi credentials from NVS.
 *
 * Reads keys "wifi_ssid" and "wifi_pass" from namespace "wifi_cfg".
 * Each output buffer receives the stored string or remains empty if the
 * key is absent. NVS handle is closed on every path.
 *
 * @param ssid     Output buffer for SSID (null-terminated).
 * @param ssid_len Size of the ssid buffer in bytes.
 * @param password Output buffer for password (null-terminated).
 * @param pass_len Size of the password buffer in bytes.
 * @return ESP_OK on success (regardless of whether the keys exist),
 *         ESP_ERR_NOT_SUPPORTED if BOARD_HAS_WIFI == 0,
 *         or an NVS/ESP-IDF error code on read failure.
 */
esp_err_t wifi_manager_load_credentials(char *ssid, size_t ssid_len,
                                         char *password, size_t pass_len);

#if defined(UNIT_TEST)
#include "freertos/FreeRTOS.h"

/**
 * @brief Test-only: return address of s_reconnect_task for injection.
 * Not compiled in production builds.
 */
TaskHandle_t *wifi_manager_get_reconnect_task_ptr(void);

/**
 * @brief Test-only: return s_reconnect_generation for verification.
 * Not compiled in production builds.
 *
 * @return Current value of s_reconnect_generation.
 */
uint32_t wifi_manager_get_reconnect_generation(void);

/**
 * @brief Test-only: invoke the orphan-guard logic directly.
 *
 * Calls reconnect_should_clear_handle(captured_gen) and returns the result.
 * Allows host tests to verify the guard branch without running the
 * FreeRTOS task. Not compiled in production builds.
 *
 * @param captured_gen Generation value captured at hypothetical task creation.
 * @return true if the task should clear s_reconnect_task, false if orphaned.
 */
bool wifi_manager_reconnect_should_clear_handle_test(uint32_t captured_gen);
#endif /* UNIT_TEST */
