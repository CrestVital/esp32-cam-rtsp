#pragma once

#include <stdint.h>

typedef void *TaskHandle_t;

typedef unsigned int  UBaseType_t;
typedef uint32_t      TickType_t;
typedef int           BaseType_t;

#define pdTRUE   1
#define pdFALSE  0
#define pdPASS   pdTRUE

#define pdMS_TO_TICKS(ms)  ((TickType_t)(ms))

BaseType_t xTaskCreate(void (*fn)(void *), const char *name, uint32_t stack,
                       void *params, UBaseType_t prio, TaskHandle_t *handle);
void       vTaskDelete(TaskHandle_t handle);
void       vTaskDelay(TickType_t ticks);

/* Notification API — needed by the reconnect task */
static inline TaskHandle_t xTaskGetCurrentTaskHandle(void) { return (TaskHandle_t)0xBEEF0002; }
uint32_t ulTaskNotifyTake(BaseType_t clear_on_exit, TickType_t timeout);
BaseType_t xTaskNotifyGive(TaskHandle_t handle);

/* Test injection API — host tests only */
void mock_set_reconnect_task_handle(TaskHandle_t *task_ptr, TaskHandle_t value);
void mock_clear_injected_task_handle(void);
