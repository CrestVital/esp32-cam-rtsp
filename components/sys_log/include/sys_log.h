#pragma once

#include "esp_log.h"

/* Canonical log-tag constants.
 *
 * Defined in sys_log.c — do not redefine these strings in other
 * translation units. Include this header and use the extern symbols. */
extern const char *const LOG_TAG_CAM;
extern const char *const LOG_TAG_RTSP;
extern const char *const LOG_TAG_WIFI;
extern const char *const LOG_TAG_WEBUI;
extern const char *const LOG_TAG_OTA;
extern const char *const LOG_TAG_SYS;

/* Convenience macros for the camera component. */
#define LOG_CAM_E(fmt, ...) ESP_LOGE(LOG_TAG_CAM, fmt, ##__VA_ARGS__)
#define LOG_CAM_W(fmt, ...) ESP_LOGW(LOG_TAG_CAM, fmt, ##__VA_ARGS__)
#define LOG_CAM_I(fmt, ...) ESP_LOGI(LOG_TAG_CAM, fmt, ##__VA_ARGS__)
#define LOG_CAM_D(fmt, ...) ESP_LOGD(LOG_TAG_CAM, fmt, ##__VA_ARGS__)
#define LOG_CAM_V(fmt, ...) ESP_LOGV(LOG_TAG_CAM, fmt, ##__VA_ARGS__)

/* Convenience macros for the RTSP server component. */
#define LOG_RTSP_E(fmt, ...) ESP_LOGE(LOG_TAG_RTSP, fmt, ##__VA_ARGS__)
#define LOG_RTSP_W(fmt, ...) ESP_LOGW(LOG_TAG_RTSP, fmt, ##__VA_ARGS__)
#define LOG_RTSP_I(fmt, ...) ESP_LOGI(LOG_TAG_RTSP, fmt, ##__VA_ARGS__)
#define LOG_RTSP_D(fmt, ...) ESP_LOGD(LOG_TAG_RTSP, fmt, ##__VA_ARGS__)
#define LOG_RTSP_V(fmt, ...) ESP_LOGV(LOG_TAG_RTSP, fmt, ##__VA_ARGS__)

/* Convenience macros for the WiFi manager component. */
#define LOG_WIFI_E(fmt, ...) ESP_LOGE(LOG_TAG_WIFI, fmt, ##__VA_ARGS__)
#define LOG_WIFI_W(fmt, ...) ESP_LOGW(LOG_TAG_WIFI, fmt, ##__VA_ARGS__)
#define LOG_WIFI_I(fmt, ...) ESP_LOGI(LOG_TAG_WIFI, fmt, ##__VA_ARGS__)
#define LOG_WIFI_D(fmt, ...) ESP_LOGD(LOG_TAG_WIFI, fmt, ##__VA_ARGS__)
#define LOG_WIFI_V(fmt, ...) ESP_LOGV(LOG_TAG_WIFI, fmt, ##__VA_ARGS__)

/* Convenience macros for the web UI component. */
#define LOG_WEBUI_E(fmt, ...) ESP_LOGE(LOG_TAG_WEBUI, fmt, ##__VA_ARGS__)
#define LOG_WEBUI_W(fmt, ...) ESP_LOGW(LOG_TAG_WEBUI, fmt, ##__VA_ARGS__)
#define LOG_WEBUI_I(fmt, ...) ESP_LOGI(LOG_TAG_WEBUI, fmt, ##__VA_ARGS__)
#define LOG_WEBUI_D(fmt, ...) ESP_LOGD(LOG_TAG_WEBUI, fmt, ##__VA_ARGS__)
#define LOG_WEBUI_V(fmt, ...) ESP_LOGV(LOG_TAG_WEBUI, fmt, ##__VA_ARGS__)

/* Convenience macros for the OTA update component. */
#define LOG_OTA_E(fmt, ...) ESP_LOGE(LOG_TAG_OTA, fmt, ##__VA_ARGS__)
#define LOG_OTA_W(fmt, ...) ESP_LOGW(LOG_TAG_OTA, fmt, ##__VA_ARGS__)
#define LOG_OTA_I(fmt, ...) ESP_LOGI(LOG_TAG_OTA, fmt, ##__VA_ARGS__)
#define LOG_OTA_D(fmt, ...) ESP_LOGD(LOG_TAG_OTA, fmt, ##__VA_ARGS__)
#define LOG_OTA_V(fmt, ...) ESP_LOGV(LOG_TAG_OTA, fmt, ##__VA_ARGS__)

/* Convenience macros for the system / diagnostics component. */
#define LOG_SYS_E(fmt, ...) ESP_LOGE(LOG_TAG_SYS, fmt, ##__VA_ARGS__)
#define LOG_SYS_W(fmt, ...) ESP_LOGW(LOG_TAG_SYS, fmt, ##__VA_ARGS__)
#define LOG_SYS_I(fmt, ...) ESP_LOGI(LOG_TAG_SYS, fmt, ##__VA_ARGS__)
#define LOG_SYS_D(fmt, ...) ESP_LOGD(LOG_TAG_SYS, fmt, ##__VA_ARGS__)
#define LOG_SYS_V(fmt, ...) ESP_LOGV(LOG_TAG_SYS, fmt, ##__VA_ARGS__)

/**
 * @brief Set the log level for a named module tag at runtime.
 *
 * @param tag    Module tag string, e.g. "camera". Pass "*" to set all tags.
 * @param level  Target log level (ESP_LOG_NONE ... ESP_LOG_VERBOSE).
 */
void sys_log_set_level(const char *tag, esp_log_level_t level);

/**
 * @brief Print structured system information to the log.
 *
 * Call once from app_main() after nvs_flash_init(). Logs under LOG_TAG_SYS.
 */
void sys_log_print_system_info(void);