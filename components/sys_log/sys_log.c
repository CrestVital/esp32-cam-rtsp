#include "sys_log.h"

#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"

#ifndef PROJECT_VER
#define PROJECT_VER "unknown"
#endif

/* Canonical module tag strings used across all components.
 *
 * Each tag corresponds to one logical subsystem. Components reference
 * these via extern declarations in sys_log.h to produce consistently
 * tagged log output. */
const char *const LOG_TAG_CAM   = "camera";
const char *const LOG_TAG_RTSP  = "rtsp";
const char *const LOG_TAG_WIFI  = "wifi_mgr";
const char *const LOG_TAG_WEBUI = "webui";
const char *const LOG_TAG_OTA   = "ota";
const char *const LOG_TAG_SYS   = "system";

void sys_log_set_level(const char *tag, esp_log_level_t level)
{
    /* Thin wrapper around the ESP-IDF per-tag log-level API.
     *
     * Pass "*" as the tag to adjust all registered module tags at once.
     * The actual per-tag level map is maintained internally by esp_log. */
    esp_log_level_set(tag, level);
}

static const char *reset_reason_str(esp_reset_reason_t reason)
{
    /* Map every ESP-IDF reset-reason enum value to a human-readable label.
     *
     * The switch is exhaustive for the currently defined enum members.
     * The default branch returns "Unknown" instead of asserting — this lets
     * the firmware survive future IDF releases that may add new enum values
     * without a rebuild. */
    switch (reason) {
        case ESP_RST_POWERON:
            return "Power-on";
        case ESP_RST_EXT:
            return "External reset";
        case ESP_RST_SW:
            return "Software reset";
        case ESP_RST_PANIC:
            return "Panic / exception";
        case ESP_RST_INT_WDT:
            return "Interrupt watchdog";
        case ESP_RST_TASK_WDT:
            return "Task watchdog";
        case ESP_RST_WDT:
            return "Other watchdog";
        case ESP_RST_DEEPSLEEP:
            return "Wake from deep sleep";
        case ESP_RST_BROWNOUT:
            return "Brownout";
        case ESP_RST_SDIO:
            return "SDIO reset";
        case ESP_RST_UNKNOWN:
            return "Unknown reset";
        case ESP_RST_USB:
            return "USB peripheral reset";
        case ESP_RST_JTAG:
            return "JTAG reset";
        case ESP_RST_EFUSE:
            return "eFuse error";
        case ESP_RST_PWR_GLITCH:
            return "Power glitch";
        case ESP_RST_CPU_LOCKUP:
            return "CPU lockup";
        default:
            return "Unknown";
    }
}

static void format_mac(const uint8_t *mac, char *buf, size_t buf_size)
{
    /* Format a 6-byte MAC address as uppercase hex with colon separators.
     *
     * The snprintf format yields "AA:BB:CC:DD:EE:FF". Byte order matches
     * the on-wire (network) order returned by the ESP-IDF MAC functions. */
    snprintf(buf, buf_size, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void sys_log_print_system_info(void)
{
    /* Print a structured system-information block to the log.
     *
     * Reads two distinct MAC addresses: the factory eFuse chip MAC and
     * the Wi-Fi station (STA) MAC, which may differ if the application
     * derives the STA MAC from the base. Both fall back to the string
     * "00:00:00:00:00:00" on read error so the log is still parseable.
     * PSRAM free size is logged separately from internal heap because
     * PSRAM is the critical resource for frame-buffer allocation. */

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    /* Read the factory-programmed chip MAC from eFuse. */
    char chip_mac_str[18];
    uint8_t chip_mac[6];
    esp_err_t ret = esp_efuse_mac_get_default(chip_mac);
    if (ret != ESP_OK) {
        ESP_LOGE(LOG_TAG_SYS, "Failed to read chip MAC: %s", esp_err_to_name(ret));
        snprintf(chip_mac_str, sizeof(chip_mac_str), "00:00:00:00:00:00");
    } else {
        format_mac(chip_mac, chip_mac_str, sizeof(chip_mac_str));
    }

    /* Read the Wi-Fi station MAC — may be derived from the base MAC. */
    char wifi_mac_str[18];
    uint8_t wifi_mac[6];
    ret = esp_read_mac(wifi_mac, ESP_MAC_WIFI_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(LOG_TAG_SYS, "Failed to read WiFi MAC: %s", esp_err_to_name(ret));
        snprintf(wifi_mac_str, sizeof(wifi_mac_str), "00:00:00:00:00:00");
    } else {
        format_mac(wifi_mac, wifi_mac_str, sizeof(wifi_mac_str));
    }

    ESP_LOGI(LOG_TAG_SYS, "========== System Information ==========");
    ESP_LOGI(LOG_TAG_SYS, "Firmware version : %s", PROJECT_VER);
    ESP_LOGI(LOG_TAG_SYS, "IDF version      : %s", esp_get_idf_version());
    ESP_LOGI(LOG_TAG_SYS, "Chip model       : %s rev %d cores %d",
             CONFIG_IDF_TARGET, chip_info.revision, chip_info.cores);
    ESP_LOGI(LOG_TAG_SYS, "Chip ID (MAC)    : %s", chip_mac_str);
    ESP_LOGI(LOG_TAG_SYS, "WiFi MAC (STA)   : %s", wifi_mac_str);
    ESP_LOGI(LOG_TAG_SYS, "Free heap        : %lu bytes",
             (unsigned long)esp_get_free_heap_size());
    ESP_LOGI(LOG_TAG_SYS, "Min free heap    : %lu bytes",
             (unsigned long)esp_get_minimum_free_heap_size());
    ESP_LOGI(LOG_TAG_SYS, "Free PSRAM       : %lu bytes",
             (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(LOG_TAG_SYS, "Reset reason     : %s", reset_reason_str(esp_reset_reason()));
    ESP_LOGI(LOG_TAG_SYS, "========================================");
}