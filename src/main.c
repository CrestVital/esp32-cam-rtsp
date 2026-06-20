#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "sys_log.h"
#include "app_event.h"

static const char *TAG = "main";

void app_main(void)
{
    /* ESP32-CAM RTSP firmware entry point.
     *
     * Boot sequence: initialise NVS (erasing if the partition layout has
     * changed), print system diagnostics, then enter the idle loop.
     * All hardware initialisation (camera, WiFi, RTSP server) will be
     * added inside the main loop as components are integrated. */

    ESP_LOGI(TAG, "ESP32-CAM RTSP firmware starting");

    /* Initialize NVS — required by WiFi and other components.
     * If the partition has no free pages (first boot or full flash) or
     * a new NVS version is detected, erase and re-init to guarantee a
     * clean working state. */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialise the application event loop.
     * Must run before any component that publishes or subscribes to
     * APP_EVENT_BASE events. Fatal on failure — nothing can function
     * without inter-module messaging. */
    ESP_ERROR_CHECK(app_event_loop_init());

    /* Dump chip info, MAC addresses, free heap/PSRAM, and reset reason.
     * Called once at boot — useful as the first line of a crash log. */
    sys_log_print_system_info();

    /* Main loop placeholder.
     *
     * In the final firmware this loop will run the RTSP server task,
     * WiFi reconnect logic, and OTA update handler. For now it simply
     * prevents app_main() from returning, which would trigger an
     * automatic watchdog reboot. */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
