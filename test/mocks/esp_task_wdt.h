#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    uint32_t timeout_ms;
    uint32_t idle_core_mask;
    bool     trigger_panic;
} esp_task_wdt_config_t;

extern esp_err_t  mock_esp_task_wdt_init_ret;
extern esp_err_t  mock_esp_task_wdt_add_ret;
extern esp_err_t  mock_esp_task_wdt_delete_ret;
extern esp_err_t  mock_esp_task_wdt_deinit_ret;

extern int mock_esp_task_wdt_init_calls;
extern int mock_esp_task_wdt_add_calls;
extern int mock_esp_task_wdt_delete_calls;
extern int mock_esp_task_wdt_reset_calls;
extern int mock_esp_task_wdt_deinit_calls;

extern esp_task_wdt_config_t mock_last_twdt_config;

void mock_esp_task_wdt_reset(void);

esp_err_t esp_task_wdt_init(const esp_task_wdt_config_t *config);
esp_err_t esp_task_wdt_add(TaskHandle_t handle);
esp_err_t esp_task_wdt_delete(TaskHandle_t handle);
esp_err_t esp_task_wdt_reset(void);
esp_err_t esp_task_wdt_deinit(void);