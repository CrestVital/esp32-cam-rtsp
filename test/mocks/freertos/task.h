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
uint32_t ulTaskNotifyTake(BaseType_t clear_on_exit, TickType_t timeout);
BaseType_t xTaskNotifyGive(TaskHandle_t handle);
