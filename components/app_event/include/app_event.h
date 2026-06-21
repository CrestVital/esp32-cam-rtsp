#pragma once

#include "esp_event.h"
#include "esp_err.h"

/** @brief Depth of the application event loop queue.
 *
 * Sized for bursty-but-low-frequency events on a streaming camera node:
 * 16 slots cover simultaneous events from WiFi, camera, and RTSP without
 * blocking publishers under normal load. */
#define APP_EVENT_QUEUE_SIZE       16

/** @brief FreeRTOS priority of the application event loop task.
 *
 * Network-tier priority (range 6-10 per project guidelines). Handlers
 * dispatched from this task may perform network operations without starving
 * the camera pipeline running at priority 11+. */
#define APP_EVENT_TASK_PRIORITY    6

/** @brief Stack size in bytes of the application event loop task.
 *
 * 4096 bytes covers the handler call depth for typical application events.
 * Increase if a registered handler uses deep stack frames. */
#define APP_EVENT_TASK_STACK_SIZE  4096

ESP_EVENT_DECLARE_BASE(APP_EVENT_BASE);

/**
 * @brief Application event identifiers used across all modules.
 *
 * Published by the component owning the event (WiFi manager, camera driver,
 * RTSP server, OTA manager, etc.) and consumed by any module that subscribes.
 */
typedef enum {
    APP_EVENT_WIFI_CONNECTED = 0,
    APP_EVENT_WIFI_DISCONNECTED,
    APP_EVENT_CAMERA_READY,
    APP_EVENT_RTSP_CLIENT_CONNECTED,
    APP_EVENT_RTSP_CLIENT_DISCONNECTED,
    APP_EVENT_OTA_STARTED,
    APP_EVENT_OTA_FINISHED,
    APP_EVENT_CONFIG_UPDATED,
    APP_EVENT_FACTORY_RESET,
    APP_EVENT_SHUTDOWN,
} app_event_id_t;

/**
 * @brief Initialise the application-level event loop.
 *
 * Creates a dedicated FreeRTOS task ("app_event_task") with priority
 * APP_EVENT_TASK_PRIORITY and a queue depth of APP_EVENT_QUEUE_SIZE.
 * Must be called once during boot, before any component that publishes
 * or subscribes to APP_EVENT_BASE events.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if already initialised,
 *         or an ESP-IDF error code if event loop creation fails.
 */
esp_err_t app_event_loop_init(void);

/**
 * @brief Tear down the application event loop.
 *
 * Deletes the event loop task and releases all internal resources. After this
 * call the loop handle is reset to NULL and re-initialisation is possible.
 * Safe to call even if no loop is active (returns ESP_ERR_INVALID_STATE).
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if no loop is active.
 */
esp_err_t app_event_loop_deinit(void);

/**
 * @brief Post an application event to the global event loop.
 *
 * Thread-safe: may be called from any FreeRTOS task. The caller owns the
 * event data buffer; the event loop copies `data_size` bytes internally.
 *
 * @param event_id  The application event to post.
 * @param data      Pointer to event payload (may be NULL if data_size is 0).
 * @param data_size Size of the payload in bytes.
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if the loop has not been
 *         initialised, or an underlying esp_event error code.
 * @note Not ISR-safe. Calls esp_event_post_to() with portMAX_DELAY, which
 *       blocks until the event queue accepts the event. Must not be called
 *       from interrupt handlers — use esp_event_isr_post_to() directly for
 *       ISR posting (requires CONFIG_ESP_EVENT_POST_FROM_ISR in sdkconfig).
 */
esp_err_t app_event_post(app_event_id_t event_id, const void *data,
                         size_t data_size);

/**
 * @brief Register a handler for a specific application event.
 *
 * @param event_id    The event to subscribe to.
 * @param handler     Callback invoked when the event is posted.
 * @param handler_arg Opaque pointer passed as the first argument to the
 *                    handler callback.
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if the loop has not been
 *         initialised, or an underlying esp_event error code.
 */
esp_err_t app_event_handler_register(app_event_id_t event_id,
                                     esp_event_handler_t handler,
                                     void *handler_arg);

/**
 * @brief Unregister a previously registered event handler.
 *
 * @param event_id The event the handler was subscribed to.
 * @param handler  The same callback pointer that was passed to
 *                 app_event_handler_register().
 * @return ESP_OK on success, ESP_ERR_INVALID_STATE if the loop has not been
 *         initialised, or an underlying esp_event error code.
 */
esp_err_t app_event_handler_unregister(app_event_id_t event_id,
                                       esp_event_handler_t handler);