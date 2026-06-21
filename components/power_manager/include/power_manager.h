#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Initialise the power manager.
 *
 * Must be called from app_main() after app_event_loop_init().
 * Logs the reset reason, initialises the TWDT with a 30-second timeout,
 * and registers the APP_EVENT_SHUTDOWN handler.
 *
 * @return ESP_OK on success, or an ESP-IDF error code on failure.
 */
esp_err_t power_manager_init(void);

/**
 * @brief Tear down the power manager.
 *
 * Unsubscribes all tasks from the TWDT and disables the watchdog timer.
 * After this call, power_manager_init() may be called again to
 * re-initialise. Safe to call even when not initialised (returns
 * ESP_ERR_INVALID_STATE).
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if not initialised,
 *         or an ESP-IDF error code on failure.
 */
esp_err_t power_manager_deinit(void);

/**
 * @brief Subscribe a named task to the Task Watchdog Timer.
 *
 * The task must call power_manager_wdt_reset() periodically to feed
 * the watchdog. Fails silently (logs error) if the task is already
 * subscribed or if the TWDT has not been initialised.
 *
 * @param task_handle  FreeRTOS task handle, or NULL for the calling task.
 * @param task_name    Human-readable name for log messages.
 * @return ESP_OK on success, or an ESP-IDF error code on failure.
 */
esp_err_t power_manager_wdt_subscribe(TaskHandle_t task_handle,
                                      const char *task_name);

/**
 * @brief Unsubscribe a task from the Task Watchdog Timer.
 *
 * Safe to call even if the task was not subscribed (returns ESP_OK).
 *
 * @param task_handle  FreeRTOS task handle, or NULL for the calling task.
 * @return ESP_OK on success, or an ESP-IDF error code on failure.
 */
esp_err_t power_manager_wdt_unsubscribe(TaskHandle_t task_handle);

/**
 * @brief Feed (reset) the Task Watchdog Timer on behalf of the calling task.
 *
 * Must be called from every subscribed task at least once every 30 seconds.
 * Safe to call from any task context.
 */
void power_manager_wdt_reset(void);

/**
 * @brief Trigger a graceful shutdown and system restart.
 *
 * Posts APP_EVENT_SHUTDOWN to the application event loop. The shutdown
 * handler will unsubscribe WDT tasks, deinit the event loop, and call
 * esp_restart(). This function returns immediately; the restart is
 * asynchronous.
 *
 * @return ESP_OK if the event was posted successfully, error otherwise.
 */
esp_err_t power_manager_shutdown(void);