#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int mock_vTaskDelete_calls = 0;

/* ── Test injection API ─────────────────────────────────────────────── */

/* Pointer to wifi_manager's s_reconnect_task — set by
 * mock_set_reconnect_task_handle() for host-test injection. */
static TaskHandle_t *s_injected_task_ptr = NULL;

void mock_set_reconnect_task_handle(TaskHandle_t *task_ptr,
                                    TaskHandle_t  value)
{
    /* Plant a non-NULL sentinel into the manager's static variable so
     * that deinit() exercises the notify-and-poll path in host tests
     * where no real FreeRTOS task is ever scheduled. */
    s_injected_task_ptr = task_ptr;
    if (task_ptr != NULL) {
        *task_ptr = value;
    }
}

void mock_clear_injected_task_handle(void)
{
    /* Nullify the planted handle so the poll loop in deinit() exits on
     * the first iteration (simulating the task having self-deleted). */
    if (s_injected_task_ptr != NULL) {
        *s_injected_task_ptr = NULL;
        s_injected_task_ptr  = NULL;
    }
}

void mock_freertos_task_reset(void)
{
    mock_vTaskDelete_calls  = 0;
    s_injected_task_ptr     = NULL;
}

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
    /* Increment the call counter so tests can verify that deinit
     * does not call vTaskDelete with an external task handle.
     * Accepts any handle value, including NULL. */
    mock_vTaskDelete_calls++;
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
    /* Always returns 0 -- simulates a timeout without notification.
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
