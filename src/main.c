#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "sys_log.h"
#include "nvs_config.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-CAM RTSP firmware starting");

    /* Initialize NVS — required by WiFi and other components */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Load configuration from NVS */
    static app_config_t g_config;
    ESP_ERROR_CHECK(config_init(&g_config));
    ESP_LOGI(TAG, "Config loaded: RTSP port=%u, resolution=%ux%u, FPS=%u, mDNS=%s",
             g_config.rtsp_port, g_config.cam_width, g_config.cam_height,
             g_config.cam_fps, g_config.mdns_name);

    sys_log_print_system_info();

    /* Main loop placeholder */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
