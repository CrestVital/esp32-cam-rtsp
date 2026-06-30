#include "sdkconfig.h"

#if CONFIG_WIFI_MANAGER_ENABLED

#include "wifi_manager.h"

#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "app_event.h"
#if CONFIG_IDF_TARGET_ESP32 && !CONFIG_IDF_TARGET_ESP32S3 && CONFIG_BT_CONTROLLER_ENABLED
#include "esp_bt.h"
#endif

static const char *TAG = "wifi_manager";

#define WIFI_RECONNECT_TASK_NAME      "wifi_reconnect"
#define WIFI_RECONNECT_TASK_PRIORITY  6
#define WIFI_RECONNECT_TASK_STACK     4096
#define WIFI_RECONNECT_MAX_ATTEMPTS   10
#define WIFI_RECONNECT_CAP_MS         30000

#define WIFI_NVS_NAMESPACE            "wifi_cfg"
#define WIFI_NVS_KEY_SSID             "wifi_ssid"
#define WIFI_NVS_KEY_PASS             "wifi_pass"

#define WIFI_RECONNECT_DELAY_BASE_MS  1000

#define WIFI_SSID_BUF_LEN  (33U)   /* 32 bytes SSID + 1 NUL */
#define WIFI_PASS_BUF_LEN  (65U)   /* 64 bytes WPA2 password + 1 NUL */

static bool s_initialized        = false;
static bool s_connected          = false;
static bool s_reconnect_enabled  = false;
static TaskHandle_t s_reconnect_task = NULL;
static SemaphoreHandle_t s_reconnect_mutex = NULL;

static char  s_target_ssid[WIFI_SSID_BUF_LEN];
static char  s_target_pass[WIFI_PASS_BUF_LEN];
static int   s_attempt_count = 0;

/* Generation counter incremented on every wifi_manager_init(). Used by
 * reconnect_task() to detect whether it has been orphaned by a deinit
 * timeout followed by a new init cycle, so it can skip the
 * s_reconnect_task = NULL write that would overwrite the new task's
 * handle and cause a kernel panic on ESP32-S3.
 *
 * uint32_t wraparound (~4e9 init cycles) is safe: the == comparison
 * remains correct modulo 2^32, and no realistic device lifetime
 * approaches the wraparound point. */
static uint32_t s_reconnect_generation = 0;

/* Active reconnect task counter. Incremented under s_reconnect_mutex at
 * the start of reconnect_task() and decremented in the cleanup section.
 * A value greater than 1 triggers an ESP_LOGE tripwire, signalling that
 * two or more reconnect tasks are running concurrently — a protocol
 * violation that must never occur in a correct implementation. */
static int s_active_reconnect_tasks = 0;

static bool reconnect_should_clear_handle(uint32_t captured_generation);

#if defined(UNIT_TEST)
TaskHandle_t *wifi_manager_get_reconnect_task_ptr(void)
{
    /* Return the address of s_reconnect_task for host-test injection.
     * Never call this from production code. */
    return &s_reconnect_task;
}

uint32_t wifi_manager_get_reconnect_generation(void)
{
    /* Return s_reconnect_generation for host-test verification only.
     * Never call this from production code. */
    return s_reconnect_generation;
}

bool wifi_manager_reconnect_should_clear_handle_test(uint32_t captured_gen)
{
    /* Thin wrapper around reconnect_should_clear_handle() for direct
     * invocation from host tests without running reconnect_task().
     * Never call this from production code. */
    return reconnect_should_clear_handle(captured_gen);
}

int wifi_manager_get_active_reconnect_tasks(void)
{
    /* Return s_active_reconnect_tasks for tripwire verification in host
     * tests. Never call from production code. */
    return s_active_reconnect_tasks;
}

void wifi_manager_set_active_reconnect_tasks_for_test(int value)
{
    /* Direct injection for tripwire tests — bypasses the mutex
     * intentionally since host tests run single-threaded. Never call from
     * production code. */
    s_active_reconnect_tasks = value;
}
#endif /* UNIT_TEST */

static void wifi_event_handler(void *arg, esp_event_base_t base,
                                int32_t event_id, void *event_data);
static void reconnect_task(void *param);

static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    /* Central handler for all WiFi- and IP-level events on the default
     * event loop. Dispatches by event base and ID as defined in the
     * wifi_manager event table. Transitions the static state variables and
     * posts application-level events so other components can react. */

    (void)arg;
    (void)event_data;

    if (base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            /* Station mode started — trigger a connect attempt.
             * This is the first concrete action after esp_wifi_start(). */
            ESP_LOGI(TAG, "STA started; connecting to \"%s\"", s_target_ssid);
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
            /* Link-layer association succeeded. IP assignment follows
             * asynchronously (IP_EVENT_STA_GOT_IP). */
            ESP_LOGI(TAG, "STA connected to AP (awaiting IP)");
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            /* Notify subscribers and, if reconnect is enabled, schedule
             * the backoff reconnect task. */
            s_connected = false;
            ESP_LOGW(TAG, "STA disconnected");
            app_event_post(APP_EVENT_WIFI_DISCONNECTED, NULL, 0);

            TaskHandle_t task_to_notify = NULL;

            if (s_reconnect_mutex != NULL &&
                xSemaphoreTake(s_reconnect_mutex, portMAX_DELAY) == pdTRUE) {
                if (s_reconnect_enabled) {
                    /* Under mutex: check whether a reconnect task already
                     * exists. If it does, copy the handle so it can be
                     * notified after releasing the mutex. If it does not,
                     * create a fresh task for the current disconnect
                     * episode. */
                    if (s_reconnect_task != NULL) {
                        /* Copy before release — the task may clear
                         * s_reconnect_task immediately after waking, so
                         * the local copy keeps the notify target valid.
                         * The mutex is released before xTaskNotifyGive to
                         * avoid holding it when the notified task resumes
                         * on a higher effective priority. */
                        task_to_notify = s_reconnect_task;
                    } else {
                        /* Fresh disconnect episode — reset attempt counter
                         * before creating the reconnect task so the
                         * exponential backoff starts from delay_base. */
                        s_attempt_count = 0;
                        BaseType_t created = xTaskCreate(
                            reconnect_task, WIFI_RECONNECT_TASK_NAME,
                            WIFI_RECONNECT_TASK_STACK,
                            (void *)(uintptr_t)s_reconnect_generation,
                            WIFI_RECONNECT_TASK_PRIORITY,
                            &s_reconnect_task);
                        if (created != pdPASS) {
                            ESP_LOGE(TAG,
                                "Failed to create reconnect task");
                        }
                    }
                }
                xSemaphoreGive(s_reconnect_mutex);
            }

            /* Notify outside the mutex — prevents holding the mutex when
             * the woken task resumes on a higher effective priority. */
            if (task_to_notify != NULL) {
                xTaskNotifyGive(task_to_notify);
            }
        }
    } else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        /* Full connectivity — reset the attempt counter, mark connected,
         * and notify the rest of the system. */
        s_connected      = true;
        s_attempt_count  = 0;
        ESP_LOGI(TAG, "Got IP address — WiFi connected");
        app_event_post(APP_EVENT_WIFI_CONNECTED, NULL, 0);
    }
}

/* Determine whether a reconnect task with the given captured generation
 * should clear s_reconnect_task. Returns true if the generation matches
 * the current counter (task is not orphaned), false if the generation has
 * advanced (task is orphaned and must exit silently).
 *
 * Extracted from the cleanup section of reconnect_task() so that the
 * guard logic can be called directly from host tests, where FreeRTOS
 * tasks cannot run under the native scheduler. Must be called under
 * s_reconnect_mutex. */
static bool reconnect_should_clear_handle(uint32_t captured_generation)
{
    return (s_reconnect_generation == captured_generation);
}

static void reconnect_task(void *param)
{
    /* Exponential-backoff reconnect loop.
     *
     * Waits for either a notification from the WiFi event handler (on
     * disconnect) or a timeout. When triggered, computes the current
     * backoff delay, waits for it, then calls esp_wifi_connect().
     * Terminates when s_reconnect_enabled is cleared or when the attempt
     * counter reaches WIFI_RECONNECT_MAX_ATTEMPTS. */

    /* Retrieve the generation captured at task creation time (passed via
     * param under s_reconnect_mutex in wifi_event_handler). Using param
     * rather than reading s_reconnect_generation directly eliminates a
     * scheduler-dependent window: if deinit-timeout + new init() occurred
     * between xTaskCreate and the first instruction of this function, the
     * direct read would capture the new (wrong) generation. */
    uint32_t my_generation = (uint32_t)(uintptr_t)param;

    /* Increment active task counter under mutex; fire tripwire if > 1.
     * The counter must never exceed 1 — if it does, two reconnect tasks
     * are running concurrently, which is a protocol violation. */
    if (s_reconnect_mutex != NULL &&
        xSemaphoreTake(s_reconnect_mutex, portMAX_DELAY) == pdTRUE) {
        s_active_reconnect_tasks++;
        int n = s_active_reconnect_tasks;
        xSemaphoreGive(s_reconnect_mutex);
        if (n > 1) {
            ESP_LOGE(TAG,
                "tripwire: %d reconnect tasks active simultaneously", n);
        }
    }

    while (s_reconnect_enabled && s_attempt_count < WIFI_RECONNECT_MAX_ATTEMPTS) {
        /* Compute backoff: 1s, 2s, 4s, 8s, 16s, capped at 30s.
         * The delay is applied even on the first iteration so that the
         * first reconnection is not instantaneous — gives the AP time to
         * recover from a transient glitch. */
        uint32_t delay_ms = WIFI_RECONNECT_DELAY_BASE_MS << s_attempt_count;
        if (delay_ms > WIFI_RECONNECT_CAP_MS) {
            delay_ms = WIFI_RECONNECT_CAP_MS;
        }

        /* Wait for notification or timeout. A notification (from
         * wifi_event_handler on disconnect) restarts the wait — this
         * prevents stacking multiple connect calls if disconnects arrive
         * faster than the backoff period. */
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(delay_ms));

        /* Re-check flags since they may have changed during the wait. */
        if (!s_reconnect_enabled) {
            break;
        }

        /* Acquire mutex to safely check the generation counter.
         * If this task's generation is stale, it has been orphaned by a
         * deinit timeout followed by a new init cycle — exit immediately
         * without calling esp_wifi_connect() or touching s_attempt_count. */
        if (s_reconnect_mutex != NULL &&
            xSemaphoreTake(s_reconnect_mutex, portMAX_DELAY) == pdTRUE) {
            bool stale = !reconnect_should_clear_handle(my_generation);
            xSemaphoreGive(s_reconnect_mutex);
            if (stale) {
                ESP_LOGW(TAG,
                    "orphaned reconnect task (gen %lu), exiting before"
                    " reconnect",
                    (unsigned long)my_generation);
                break;
            }
        }

        ESP_LOGI(TAG, "Reconnect attempt %d/%d (backoff %lu ms)",
                 s_attempt_count + 1, WIFI_RECONNECT_MAX_ATTEMPTS,
                 (unsigned long)delay_ms);

        esp_wifi_connect();
        s_attempt_count++;
    }

    if (s_attempt_count >= WIFI_RECONNECT_MAX_ATTEMPTS) {
        ESP_LOGE(TAG, "Reconnect failed after %d attempts — giving up",
                 WIFI_RECONNECT_MAX_ATTEMPTS);
    }

    /* Clear the handle under mutex so deinit's poll loop sees NULL
     * and knows this task has finished. vTaskDelete(NULL) then removes
     * this task from the scheduler. Without the mutex, a concurrent
     * deinit could call vTaskDelete on this handle after it has already
     * been freed, causing a FreeRTOS kernel panic on ESP32-S3. */
    if (s_reconnect_mutex != NULL &&
        xSemaphoreTake(s_reconnect_mutex, portMAX_DELAY) == pdTRUE) {
        /* Decrement active task counter. */
        s_active_reconnect_tasks--;
        /* Use the helper so the guard logic is testable independently of
         * the FreeRTOS scheduler. */
        if (reconnect_should_clear_handle(my_generation)) {
            s_reconnect_task = NULL;
        } else {
            /* Log both the captured and the current generation to aid
             * diagnosis of init-cycle races. */
            ESP_LOGW(TAG,
                "Orphaned reconnect task (gen %lu, current %lu) exiting"
                " silently",
                (unsigned long)my_generation,
                (unsigned long)s_reconnect_generation);
        }
        /* TODO #ESPCAMFW-47: join-based teardown (R2 — stack/TCB leak on
         * timeout path). If deinit becomes a regular runtime path, resolve
         * ESPCAMFW-47 first. */
        xSemaphoreGive(s_reconnect_mutex);
    }
    vTaskDelete(NULL);
}

esp_err_t wifi_manager_init(void)
{
    /* Initialise the WiFi subsystem: netif, WiFi init, station mode,
     * and register event handlers on the default event loop.
     *
     * Must be called once, before any other wifi_manager function. A
     * second call is rejected to prevent duplicate initialisation of
     * the underlying WiFi stack. On ESP32 (non-S3) targets the BT
     * controller is disabled to prevent RF coexistence issues — failure
     * to disable BT is non-fatal and logs a warning. */

    if (s_initialized) {
        ESP_LOGE(TAG, "WiFi manager already initialised");
        return ESP_ERR_INVALID_STATE;
    }

    /* Reset reconnect state in case a previous deinit timed out while
     * the reconnect task was still running — ensures init always starts
     * from a clean slate regardless of how the previous cycle ended. */
    s_reconnect_task        = NULL;
    s_attempt_count         = 0;
    s_reconnect_enabled     = false;
    s_connected             = false;
    s_active_reconnect_tasks = 0;
    /* Advance the generation counter so any task orphaned by a previous
     * deinit timeout will detect the mismatch and exit without
     * overwriting the new task's handle. */
    s_reconnect_generation++;

    /* Initialise LwIP netif layer and create the default station netif.
     * Must happen before esp_wifi_init(). */
    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Create the default station interface. Ignored return value is safe:
     * esp_netif_create_default_wifi_sta() returns NULL only when the
     * netif list is full, which implies a memory allocation failure
     * already caught by the configuration check in esp_netif_init(). */
    (void)esp_netif_create_default_wifi_sta();

    /* Initialise the WiFi driver with default configuration. */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Set station mode — this firmware never operates as an AP. */
    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(ret));
        goto fail_deinit_wifi;
    }

    /* On ESP32 (non-S3) targets, disable the Bluetooth controller to
     * free the shared RF path for WiFi. The guard uses the ESP-IDF
     * target macros so the code compiles cleanly on all chips. A BT
     * disable failure is non-fatal: BT may have already been disabled
     * by menuconfig or a prior component. */
#if CONFIG_IDF_TARGET_ESP32 && !CONFIG_IDF_TARGET_ESP32S3 && CONFIG_BT_CONTROLLER_ENABLED
    esp_err_t bt_ret = esp_bt_controller_disable();
    if (bt_ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_bt_controller_disable failed: %s -- continuing",
                 esp_err_to_name(bt_ret));
    }
#endif

    /* Create the reconnect mutex before registering event handlers so
     * that no incoming WiFi event can arrive with s_reconnect_mutex == NULL.
     * This mutex guards s_reconnect_task across all three contexts: event
     * handler, deinit, and the reconnect task itself. On a dual-core Xtensa
     * LX7, concurrent reads and writes without this mutex can produce torn
     * reads, stale values, or double-task-handle kernel panics. */
    s_reconnect_mutex = xSemaphoreCreateMutex();
    if (s_reconnect_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create reconnect mutex");
        goto fail_deinit_wifi;
    }

    /* Register WiFi-level event handler on the default event loop.
     * All WIFI_EVENT_* events are delivered here. */
    ret = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                     &wifi_event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WiFi event handler: %s",
                 esp_err_to_name(ret));
        vSemaphoreDelete(s_reconnect_mutex);
        s_reconnect_mutex = NULL;
        goto fail_deinit_wifi;
    }

    /* Register IP-level event handler for GOT_IP only. */
    ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                     &wifi_event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register IP event handler: %s",
                 esp_err_to_name(ret));
        /* Unregister the WiFi handler to leave a clean state. */
        esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                     &wifi_event_handler);
        vSemaphoreDelete(s_reconnect_mutex);
        s_reconnect_mutex = NULL;
        goto fail_deinit_wifi;
    }

    s_initialized = true;
    ESP_LOGI(TAG, "WiFi manager initialised");
    return ESP_OK;

    /* Cleanup path: esp_wifi_deinit() must be called whenever
     * esp_wifi_init() succeeded but a later step failed, so the WiFi
     * driver returns to its uninitialised state. */
fail_deinit_wifi:
    esp_err_t d_ret = esp_wifi_deinit();
    if (d_ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_deinit on cleanup failed: %s",
                 esp_err_to_name(d_ret));
    }
    return ret;
}

esp_err_t wifi_manager_deinit(void)
{
    /* Deinitialise the WiFi subsystem.
     *
     * Sets s_reconnect_enabled = false to signal the reconnect task to stop,
     * then wakes the task (xTaskNotifyGive) so it exits on its next
     * s_reconnect_enabled check. Polls s_reconnect_task under the mutex
     * in a bounded loop (500 ms) waiting for the task to NULL the handle
     * before calling vTaskDelete(NULL). After the task exits, deletes the
     * mutex, unregisters event handlers, and tears down the WiFi driver.
     * Safe to call even if not initialised. */

    /* TODO #ESPCAMFW-47: if deinit becomes a regular runtime path (not
     * just one-shot shutdown), implement join-based teardown from
     * ESPCAMFW-47 before calling this function repeatedly. */
    /* Signal the reconnect task to exit and suppress new creations.
     * s_reconnect_enabled is a bool — single-shot write that does not
     * need mutex protection, but must be set before waking the task. */
    s_reconnect_enabled = false;

    if (s_reconnect_mutex != NULL) {
        /* Wake the task so it sees s_reconnect_enabled == false on its
         * next loop iteration and exits promptly, without waiting for
         * the full exponential backoff timeout. */
        if (xSemaphoreTake(s_reconnect_mutex, portMAX_DELAY) == pdTRUE) {
            TaskHandle_t task_to_wake = s_reconnect_task;
            xSemaphoreGive(s_reconnect_mutex);
            if (task_to_wake != NULL) {
                xTaskNotifyGive(task_to_wake);
            }
        }

        /* Poll for the task to NULL s_reconnect_task (cooperative exit).
         * The mutex is acquired and released on each iteration so the
         * task can acquire it itself during its cleanup section.
         * vTaskDelay is called outside the critical section — the task
         * can run unimpeded between the release and the next take. */
        const int max_polls = 50;
        for (int i = 0; i < max_polls; i++) {
            bool task_done = false;
            if (xSemaphoreTake(s_reconnect_mutex, pdMS_TO_TICKS(10))
                == pdTRUE) {
                task_done = (s_reconnect_task == NULL);
                xSemaphoreGive(s_reconnect_mutex);
            }
            if (task_done) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            if (i == max_polls - 1) {
                ESP_LOGW(TAG,
                    "Reconnect task did not exit within 500 ms");
            }
        }

        /* Force-clear the handle even on the timeout path: if the task
         * did not exit within 500 ms, s_reconnect_mutex is about to be
         * deleted, so the task's cleanup section will skip the NULL
         * assignment. Without this, a subsequent init + disconnect would
         * call xTaskNotifyGive on a freed task handle. */
        s_reconnect_task = NULL;

        /* Tear down the mutex — the manager is being deinitialised and
         * all future accesses to s_reconnect_task are illegal. */
        vSemaphoreDelete(s_reconnect_mutex);
        s_reconnect_mutex = NULL;
    }

    /* Unregister event handlers — these are no-ops if not registered,
     * so it is safe to call unconditionally. */
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                 &wifi_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                 &wifi_event_handler);

    /* Stop and deinit the WiFi driver; failures are logged but not
     * propagated — these are best-effort teardown calls. */
    esp_err_t stop_ret = esp_wifi_stop();
    if (stop_ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_stop failed: %s", esp_err_to_name(stop_ret));
    }

    esp_err_t deinit_ret = esp_wifi_deinit();
    if (deinit_ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_deinit failed: %s",
                 esp_err_to_name(deinit_ret));
    }

    s_initialized = false;
    s_connected   = false;
    ESP_LOGI(TAG, "WiFi manager deinitialised");
    return ESP_OK;
}

esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    /* Connect to an access point and enable automatic reconnection.
     *
     * Copies SSID and password into static buffers so they survive past
     * the call. Calls esp_wifi_start() which fires WIFI_EVENT_STA_START;
     * the event handler then calls esp_wifi_connect() — a direct call is
     * omitted here to avoid a redundant second invocation. Enables the
     * reconnect task which is woken by the WIFI_EVENT_STA_DISCONNECTED
     * handler. */

    if (ssid == NULL || ssid[0] == '\0') {
        ESP_LOGE(TAG, "SSID must not be NULL or empty");
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_initialized) {
        ESP_LOGE(TAG, "WiFi manager not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    /* Copy credentials to static context for use by the event handler
     * and reconnect task. Max lengths match the ESP-IDF wifi_config_t
     * union sizes: 32 bytes for SSID, 64 bytes for password. */
    strncpy(s_target_ssid, ssid, sizeof(s_target_ssid) - 1);
    s_target_ssid[sizeof(s_target_ssid) - 1] = '\0';

    /* Treat NULL password as empty string — supports open networks. */
    const char *pw = (password != NULL) ? password : "";
    strncpy(s_target_pass, pw, sizeof(s_target_pass) - 1);
    s_target_pass[sizeof(s_target_pass) - 1] = '\0';

    /* Configure station with the provided credentials. */
    wifi_config_t wifi_cfg = {0};
    memcpy(wifi_cfg.sta.ssid, s_target_ssid, sizeof(wifi_cfg.sta.ssid));
    wifi_cfg.sta.ssid[sizeof(wifi_cfg.sta.ssid) - 1] = '\0';
    memcpy(wifi_cfg.sta.password, s_target_pass,
           sizeof(wifi_cfg.sta.password));
    wifi_cfg.sta.password[sizeof(wifi_cfg.sta.password) - 1] = '\0';

    esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Start the WiFi driver if not already started — esp_wifi_start() is
     * idempotent after the first successful call. */
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Enable reconnect and reset the attempt counter so the task starts
     * from attempt 1 on the next disconnect. */
    s_reconnect_enabled = true;
    s_attempt_count     = 0;

    /* Connection is initiated by the WIFI_EVENT_STA_START handler.
     * esp_wifi_start() above fires STA_START, which calls
     * esp_wifi_connect() from the event handler — no need for a
     * direct call here. */

    ESP_LOGI(TAG, "Connecting to \"%s\"", s_target_ssid);
    return ESP_OK;
}

esp_err_t wifi_manager_disconnect(void)
{
    /* Disconnect and suppress the auto-reconnect task.
     *
     * Clearing s_reconnect_enabled causes the reconnect task to exit on its
     * next iteration. esp_wifi_disconnect() triggers the disconnect event
     * handler, which will see s_reconnect_enabled == false and skip task
     * creation. */

    s_reconnect_enabled = false;

    esp_err_t ret = esp_wifi_disconnect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_disconnect failed: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "WiFi disconnected (reconnect suppressed)");
    return ret;
}

bool wifi_manager_is_connected(void)
{
    /* Return the static connected flag. This flag transitions to true only
     * inside the IP_EVENT_STA_GOT_IP handler, so it accurately reflects
     * full connectivity (including IP assignment). */
    return s_connected;
}

esp_err_t wifi_manager_save_credentials(const char *ssid, const char *password)
{
    /* Persist SSID and password to NVS namespace "wifi_cfg".
     *
     * Opens the NVS handle in read-write mode, writes both strings, and
     * commits. The handle is always closed before returning. An empty or
     * NULL SSID is rejected before any NVS operations. */

    if (ssid == NULL || ssid[0] == '\0') {
        ESP_LOGE(TAG, "SSID must not be NULL or empty");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Write SSID — pass the raw buffer; nvs_set_str handles null
     * termination internally. */
    ret = nvs_set_str(handle, WIFI_NVS_KEY_SSID, ssid);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str(ssid) failed: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    /* Write password — treat NULL as empty string (open network). */
    const char *pw = (password != NULL) ? password : "";
    ret = nvs_set_str(handle, WIFI_NVS_KEY_PASS, pw);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str(pass) failed: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    /* Commit ensures the writes are flushed to the underlying storage
     * media (flash). A commit failure leaves the handle open — close it
     * manually. */
    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(ret));
    }

    nvs_close(handle);
    return ret;
}

esp_err_t wifi_manager_load_credentials(char *ssid, size_t ssid_len,
                                         char *password, size_t pass_len)
{
    /* Load SSID and password from NVS namespace "wifi_cfg".
     *
     * Both reads are best-effort: if a key is absent, the output buffer
     * is zeroed and the function continues. The NVS handle is always
     * closed before returning. The output buffers are zeroed on entry so
     * callers can safely check ssid[0] for an empty credential set. */

    /* Zero output buffers so absent keys produce empty strings. */
    if (ssid != NULL) {
        memset(ssid, 0, ssid_len);
    }
    if (password != NULL) {
        memset(password, 0, pass_len);
    }

    nvs_handle_t handle;
    esp_err_t ret = nvs_open(WIFI_NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open(RO) failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Read SSID — if not found, leave the buffer empty (already zeroed). */
    if (ssid != NULL) {
        size_t len = ssid_len;
        ret = nvs_get_str(handle, WIFI_NVS_KEY_SSID, ssid, &len);
        if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGE(TAG, "nvs_get_str(ssid) failed: %s",
                     esp_err_to_name(ret));
            nvs_close(handle);
            return ret;
        }
        /* If key not found, ensure the buffer stays empty. */
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            ssid[0] = '\0';
        }
    }

    /* Read password with the same best-effort pattern. */
    if (password != NULL) {
        size_t len = pass_len;
        ret = nvs_get_str(handle, WIFI_NVS_KEY_PASS, password, &len);
        if (ret != ESP_OK && ret != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGE(TAG, "nvs_get_str(pass) failed: %s",
                     esp_err_to_name(ret));
            nvs_close(handle);
            return ret;
        }
        if (ret == ESP_ERR_NVS_NOT_FOUND) {
            password[0] = '\0';
        }
    }

    nvs_close(handle);
    return ESP_OK;
}

#else /* !CONFIG_WIFI_MANAGER_ENABLED */

#include "wifi_manager.h"

esp_err_t wifi_manager_init(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t wifi_manager_deinit(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    (void)ssid;
    (void)password;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t wifi_manager_disconnect(void)
{
    return ESP_ERR_NOT_SUPPORTED;
}

bool wifi_manager_is_connected(void)
{
    return false;
}

esp_err_t wifi_manager_save_credentials(const char *ssid, const char *password)
{
    (void)ssid;
    (void)password;
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t wifi_manager_load_credentials(char *ssid, size_t ssid_len,
                                         char *password, size_t pass_len)
{
    (void)ssid;
    (void)ssid_len;
    (void)password;
    (void)pass_len;
    return ESP_ERR_NOT_SUPPORTED;
}

#endif /* CONFIG_WIFI_MANAGER_ENABLED */
