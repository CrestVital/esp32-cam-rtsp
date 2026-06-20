#include "app_event.h"
#include "esp_log.h"

static const char *TAG = "app_event";

ESP_EVENT_DEFINE_BASE(APP_EVENT_BASE);

static esp_event_loop_handle_t s_app_loop = NULL;

esp_err_t app_event_loop_init(void)
{
    /* Create a dedicated application event loop with its own FreeRTOS task.
     *
     * The loop must not already exist: a second call indicates a logic error
     * in the boot sequence. The task runs at network-tier priority
     * (APP_EVENT_TASK_PRIORITY) so handlers can perform network operations
     * (DNS, HTTP) without starving the camera pipeline. Queue depth
     * (APP_EVENT_QUEUE_SIZE) is sized for a streaming camera node where
     * events are bursty but low-frequency.
     * loop_args is static const — the configuration is immutable and shared
     * across any hypothetical re-init after deinit. */

    if (s_app_loop != NULL) {
        ESP_LOGE(TAG, "app_event_loop_init: loop is already initialised");
        return ESP_ERR_INVALID_STATE;
    }

    static const esp_event_loop_args_t loop_args = {
        .queue_size       = APP_EVENT_QUEUE_SIZE,
        .task_name        = "app_event_task",
        .task_priority    = APP_EVENT_TASK_PRIORITY,
        .task_stack_size  = APP_EVENT_TASK_STACK_SIZE,
        .task_core_id     = tskNO_AFFINITY,
    };

    esp_err_t ret = esp_event_loop_create(&loop_args, &s_app_loop);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create app event loop: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Application event loop created");
    return ESP_OK;
}

esp_err_t app_event_loop_deinit(void)
{
    /* Tear down the event loop, releasing the task and internal resources.
     *
     * After deletion the handle is reset to NULL so the component can be
     * re-initialised (e.g. during an OTA-controlled restart sequence).
     * Deinit without a prior init is a programming error and is flagged. */

    if (s_app_loop == NULL) {
        ESP_LOGW(TAG, "app_event_loop_deinit: no loop to deinit");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_event_loop_delete(s_app_loop);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to delete app event loop: %s",
                 esp_err_to_name(ret));
        return ret;
    }

    /* Only NULL the handle when deletion actually succeeded, preventing
     * a dangling pointer if a caller retries after a transient failure. */
    s_app_loop = NULL;
    return ESP_OK;
}

esp_err_t app_event_post(app_event_id_t event_id, const void *data,
                         size_t data_size)
{
    /* Enqueue an event into the internal event queue for asynchronous
     * delivery to registered handlers. The call is thread-safe and blocks
     * indefinitely (portMAX_DELAY) only if the queue is full — which
     * should never happen under normal load with queue_size=16. */

    if (s_app_loop == NULL) {
        ESP_LOGE(TAG, "app_event_post: event loop not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_event_post_to(s_app_loop, APP_EVENT_BASE,
                                      (int32_t)event_id, data, data_size,
                                      portMAX_DELAY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to post event %d: %s", (int)event_id,
                 esp_err_to_name(ret));
    } else {
        ESP_LOGD(TAG, "Event %d posted", (int)event_id);
    }

    return ret;
}

esp_err_t app_event_handler_register(app_event_id_t event_id,
                                     esp_event_handler_t handler,
                                     void *handler_arg)
{
    /* Subscribe a callback to the specified event. The handler runs in the
     * context of the app_event_task and should not block for long periods.
     * Multiple handlers may be registered for the same event; they are
     * dispatched in registration order. */

    if (s_app_loop == NULL) {
        ESP_LOGE(TAG, "app_event_handler_register: event loop not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_event_handler_register_with(
        s_app_loop, APP_EVENT_BASE, (int32_t)event_id, handler, handler_arg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register handler for event %d: %s",
                 (int)event_id, esp_err_to_name(ret));
    }

    return ret;
}

esp_err_t app_event_handler_unregister(app_event_id_t event_id,
                                       esp_event_handler_t handler)
{
    /* Remove a previously registered handler. The handler pointer must
     * match exactly what was passed to app_event_handler_register().
     * Unregistering an unknown handler is a no-op at the esp_event layer
     * and returns ESP_OK. */

    if (s_app_loop == NULL) {
        ESP_LOGE(TAG,
                 "app_event_handler_unregister: event loop not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_event_handler_unregister_with(
        s_app_loop, APP_EVENT_BASE, (int32_t)event_id, handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to unregister handler for event %d: %s",
                 (int)event_id, esp_err_to_name(ret));
    }

    return ret;
}
