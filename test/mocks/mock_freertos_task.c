#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

BaseType_t xTaskCreate(void (*fn)(void *), const char *name, uint32_t stack,
                       void *params, UBaseType_t prio, TaskHandle_t *handle)
{
    (void)fn;
    (void)name;
    (void)stack;
    (void)params;
    (void)prio;

    /* Return a non-NULL sentinel handle on success so the caller can
     * distinguish a created task from a creation failure. */
    if (handle != NULL) {
        *handle = (TaskHandle_t)0xCAFE0001;
    }
    return pdPASS;
}

void vTaskDelete(TaskHandle_t handle)
{
    /* No-op: the mock does not manage real task stacks or TCBs.
     * Silently accept any handle value, including NULL. */
    (void)handle;
}

void vTaskDelay(TickType_t ticks)
{
    /* No-op: the mock does not have a simulated tick counter.
     * Tests must not depend on real timing behaviour. */
    (void)ticks;
}

uint32_t ulTaskNotifyTake(BaseType_t clear_on_exit, TickType_t timeout)
{
    /* Always returns 0 — simulates a timeout without notification.
     * Tests that need the reconnect task to proceed should inject
     * behaviour at a higher level (e.g. by manipulating static state
     * directly before calling the public API). */
    (void)clear_on_exit;
    (void)timeout;
    return 0;
}

BaseType_t xTaskNotifyGive(TaskHandle_t handle)
{
    /* No-op: the mock does not forward notifications to tasks.
     * Accepts any handle value. */
    (void)handle;
    return pdPASS;
}
