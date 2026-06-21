#include "power_manager.h"
#include "app_event.h"
#include "sys_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "esp_rom_sys.h"

static const char *TAG = "power_mgr";

static bool s_initialized = false;

/* String in DRAM so it is accessible when the flash cache is disabled
 * (e.g., during OTA flash erase/write). The handler itself is in IRAM
 * for the same reason. A bare string literal would remain in .rodata
 * (flash) and would cause a cache exception if accessed with cache off. */
static const char DRAM_ATTR s_twdt_timeout_msg[] =
    "TWDT timeout -- system will reset\n";

void IRAM_ATTR esp_task_wdt_isr_user_handler(void)
{
    /* Override the default TWDT timeout behaviour. Runs in ISR context
     * — only esp_rom_printf() is safe (no ESP_LOG*, no malloc, no
     * blocking calls). trigger_panic=true ensures the system resets
     * after this handler returns; do not call esp_restart() here. */
    esp_rom_printf(s_twdt_timeout_msg);
}

static void shutdown_handler(void *arg, esp_event_base_t base,
                             int32_t event_id, void *event_data)
{
    /* Graceful shutdown sequence triggered by APP_EVENT_SHUTDOWN.
     *
     * Step 1: deinit TWDT (unsubscribes all tasks, disables timer).
     * Step 2: tear down the event loop.
     * Step 3: restart the system.
     *
     * The handler runs in the app_event_task context. Blocking is
     * acceptable because the entire system is shutting down. */

    (void)arg;
    (void)base;
    (void)event_id;
    (void)event_data;

    ESP_LOGI(TAG, "Shutdown sequence started");

    /* Disable the TWDT before tearing down the event loop. The return
     * value is intentionally ignored: if deinit fails here, the system
     * is already committed to restarting and there is no safe recovery
     * path — proceeding to esp_restart() is the correct action. */
    (void)esp_task_wdt_deinit();
    s_initialized = false;

    /* Tear down the event loop before restarting. A failed deinit is
     * logged but does not block the restart — the system is going down
     * regardless. */
    esp_err_t ret = app_event_loop_deinit();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Event loop deinit returned: %s",
                 esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "Restarting system...");
    esp_restart();
}

esp_err_t power_manager_init(void)
{
    /* Initialise the power manager: log the boot reset reason, configure
     * the Task Watchdog Timer with a 30-second timeout, and register the
     * APP_EVENT_SHUTDOWN handler for graceful restart.
     *
     * Must be called once after app_event_loop_init(). A second call
     * returns ESP_ERR_INVALID_STATE. Not re-entrant. */

    if (s_initialized) {
        ESP_LOGW(TAG, "power_manager_init: already initialised");
        return ESP_ERR_INVALID_STATE;
    }

    /* Map the reset reason to a human-readable string and choose the
     * appropriate log level. WDT and panic resets are warnings —
     * they indicate a preceding failure that must be investigated. */
    esp_reset_reason_t reason = esp_reset_reason();
    const char *reason_str = NULL;
    bool is_warning = false;

    switch (reason) {
    case ESP_RST_POWERON:
        reason_str = "Power-on";
        break;
    case ESP_RST_UNKNOWN:
        reason_str = "Unknown";
        break;
    case ESP_RST_EXT:
        reason_str = "External pin reset";
        break;
    case ESP_RST_SDIO:
        reason_str = "SDIO reset";
        break;
    case ESP_RST_SW:
        reason_str = "Software reset";
        break;
    case ESP_RST_PANIC:
        reason_str = "Exception/panic";
        is_warning = true;
        break;
    case ESP_RST_INT_WDT:
        reason_str = "Interrupt watchdog";
        is_warning = true;
        break;
    case ESP_RST_TASK_WDT:
        reason_str = "Task watchdog (TWDT)";
        is_warning = true;
        break;
    case ESP_RST_WDT:
        reason_str = "Other watchdog";
        is_warning = true;
        break;
    case ESP_RST_DEEPSLEEP:
        reason_str = "Deep sleep wake";
        break;
    case ESP_RST_BROWNOUT:
        reason_str = "Brownout";
        is_warning = true;
        break;
    default:
        reason_str = "Unknown";
        break;
    }

    if (is_warning) {
        ESP_LOGW(TAG, "Reset reason: %s (%d)", reason_str, (int)reason);
    } else {
        ESP_LOGI(TAG, "Reset reason: %s (%d)", reason_str, (int)reason);
    }

    /* Configure the TWDT: 30-second timeout, monitor idle tasks on both
     * CPU cores, and trigger a panic-on-timeout which automatically
     * resets the system via the panic handler. */
    static const esp_task_wdt_config_t s_twdt_config = {
        .timeout_ms     = 30000,
        .idle_core_mask = (1 << 0) | (1 << 1),
        .trigger_panic  = true,
    };

    esp_err_t ret = esp_task_wdt_init(&s_twdt_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "TWDT init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "TWDT initialised (timeout=%lu ms, both cores)",
             (unsigned long)s_twdt_config.timeout_ms);

    /* Register the APP_EVENT_SHUTDOWN handler. The handler runs in the
     * app_event_task context and orchestrates the shutdown sequence.
     * If registration fails, deinit the TWDT to leave the system in a
     * clean state for a potential retry. */
    ret = app_event_handler_register(APP_EVENT_SHUTDOWN,
                                     shutdown_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register shutdown handler: %s",
                 esp_err_to_name(ret));
        esp_task_wdt_deinit();
        return ret;
    }

    s_initialized = true;
    return ESP_OK;
}

esp_err_t power_manager_deinit(void)
{
    /* Tear down the power manager: unsubscribe all tasks from the TWDT
     * and disable the watchdog timer. After this call, re-initialisation
     * via power_manager_init() is possible. */

    if (!s_initialized) {
        ESP_LOGW(TAG, "power_manager_deinit: not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_task_wdt_deinit();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "TWDT deinit failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_initialized = false;
    return ESP_OK;
}

esp_err_t power_manager_wdt_subscribe(TaskHandle_t task_handle,
                                      const char *task_name)
{
    /* Subscribe a task to the TWDT. The task must call
     * power_manager_wdt_reset() at least once every 30 seconds.
     * Fails gracefully if the TWDT has not been initialised or if
     * the subscription itself fails (task already subscribed, etc.). */

    if (!s_initialized) {
        ESP_LOGW(TAG, "WDT subscribe('%s') skipped: TWDT not initialised",
                 task_name ? task_name : "NULL");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_task_wdt_add(task_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WDT subscribe('%s') failed: %s",
                 task_name ? task_name : "NULL", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Subscribed '%s' to TWDT",
             task_name ? task_name : "calling_task");
    return ESP_OK;
}

esp_err_t power_manager_wdt_unsubscribe(TaskHandle_t task_handle)
{
    /* Unsubscribe a task from the TWDT. Safe to call even if the task
     * was not previously subscribed — esp_task_wdt_delete() on an
     * unsubscribed task is a no-op and returns ESP_OK. */

    if (!s_initialized) {
        ESP_LOGW(TAG, "WDT unsubscribe skipped: TWDT not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = esp_task_wdt_delete(task_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "WDT unsubscribe failed: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

void power_manager_wdt_reset(void)
{
    /* Feed the TWDT on behalf of the calling task.
     *
     * Must be called periodically (well within 30 seconds) from every
     * subscribed task. Safe to call unconditionally — if the TWDT has
     * not been initialised or the task is not subscribed, the underlying
     * esp_task_wdt_reset() is effectively a no-op. */

    esp_task_wdt_reset();
}

esp_err_t power_manager_shutdown(void)
{
    /* Trigger a graceful shutdown by posting APP_EVENT_SHUTDOWN to the
     * application event loop. Returns immediately; the actual shutdown
     * sequence runs asynchronously in the app_event_task context.
     *
     * If the event loop is unavailable, the error propagates to the
     * caller for handling at that level. */

    return app_event_post(APP_EVENT_SHUTDOWN, NULL, 0);
}