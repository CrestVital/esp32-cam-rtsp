#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

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

    ESP_LOGI(TAG, "NVS initialized");
    ESP_LOGI(TAG, "System ready. Heap free: %lu bytes",
             (unsigned long)esp_get_free_heap_size());

    /* Main loop placeholder */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
