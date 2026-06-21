#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_rom_sys.h"
#include <string.h>
#include <stdio.h>

esp_err_t mock_esp_task_wdt_init_ret   = ESP_OK;
esp_err_t mock_esp_task_wdt_add_ret    = ESP_OK;
esp_err_t mock_esp_task_wdt_delete_ret = ESP_OK;
esp_err_t mock_esp_task_wdt_deinit_ret = ESP_OK;

int mock_esp_task_wdt_init_calls   = 0;
int mock_esp_task_wdt_add_calls    = 0;
int mock_esp_task_wdt_delete_calls = 0;
int mock_esp_task_wdt_reset_calls  = 0;
int mock_esp_task_wdt_deinit_calls = 0;

esp_task_wdt_config_t mock_last_twdt_config;

esp_reset_reason_t mock_esp_reset_reason_ret = ESP_RST_POWERON;
int                mock_esp_restart_calls    = 0;

void mock_esp_task_wdt_reset(void)
{
    mock_esp_task_wdt_init_ret   = ESP_OK;
    mock_esp_task_wdt_add_ret    = ESP_OK;
    mock_esp_task_wdt_delete_ret = ESP_OK;
    mock_esp_task_wdt_deinit_ret = ESP_OK;

    mock_esp_task_wdt_init_calls   = 0;
    mock_esp_task_wdt_add_calls    = 0;
    mock_esp_task_wdt_delete_calls = 0;
    mock_esp_task_wdt_reset_calls  = 0;
    mock_esp_task_wdt_deinit_calls = 0;

    memset(&mock_last_twdt_config, 0, sizeof(mock_last_twdt_config));
}

void mock_esp_system_reset(void)
{
    mock_esp_reset_reason_ret = ESP_RST_POWERON;
    mock_esp_restart_calls    = 0;
}

esp_err_t esp_task_wdt_init(const esp_task_wdt_config_t *config)
{
    mock_esp_task_wdt_init_calls++;
    if (config != NULL) {
        memcpy(&mock_last_twdt_config, config, sizeof(*config));
    }
    return mock_esp_task_wdt_init_ret;
}

esp_err_t esp_task_wdt_add(TaskHandle_t handle)
{
    mock_esp_task_wdt_add_calls++;
    (void)handle;
    return mock_esp_task_wdt_add_ret;
}

esp_err_t esp_task_wdt_delete(TaskHandle_t handle)
{
    mock_esp_task_wdt_delete_calls++;
    (void)handle;
    return mock_esp_task_wdt_delete_ret;
}

esp_err_t esp_task_wdt_reset(void)
{
    mock_esp_task_wdt_reset_calls++;
    return ESP_OK;
}

esp_err_t esp_task_wdt_deinit(void)
{
    mock_esp_task_wdt_deinit_calls++;
    return mock_esp_task_wdt_deinit_ret;
}

esp_reset_reason_t esp_reset_reason(void)
{
    return mock_esp_reset_reason_ret;
}

void esp_restart(void)
{
    mock_esp_restart_calls++;
}

void esp_rom_printf(const char *format, ...)
{
    /* Capture the ISR-safe printf output. In tests this goes to stdout
     * for inspection; the real function outputs to the ROM UART. */
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}