#include "sdkconfig.h"
#include "status_indicator.h"
#include "esp_log.h"
#include "esp_err.h"
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#if CONFIG_STATUS_INDICATOR_HAS_DISPLAY
#  if __has_include("display_manager.h")
#    include "display_manager.h"
#  else
typedef struct {
    const char *state_name; /* Human-readable state label */
    int         state_id;   /* Numeric state for programmatic use */
} display_status_t;

static inline esp_err_t display_manager_update_status(const display_status_t *s)
{
    /* TODO #ESPCAMFW-26: replace stub with real display_manager call.
     * When display_manager is implemented, remove this stub and add
     * display_manager.h to the component's REQUIRES in CMakeLists.txt. */
    (void)s;
    return ESP_OK;
}
#  endif
#endif

/* Provide safe defaults if sdkconfig has not yet been generated.
 * espressif32@7.x does not always apply board_build.sdkconfig_defaults
 * before the first compilation pass, leaving these macros undefined.
 * Defaulting to 0 / -1 selects the log-only path, which requires no
 * hardware headers and compiles on every target. */
#ifndef CONFIG_STATUS_INDICATOR_HAS_DISPLAY
#  define CONFIG_STATUS_INDICATOR_HAS_DISPLAY 0
#endif
#ifndef CONFIG_STATUS_INDICATOR_LED_PIN
#  define CONFIG_STATUS_INDICATOR_LED_PIN -1
#endif
#ifndef CONFIG_STATUS_INDICATOR_LED_ACTIVE_LOW
#  define CONFIG_STATUS_INDICATOR_LED_ACTIVE_LOW 0
#endif

/* LEDC and GPIO headers are only needed on LED path (LED_PIN != -1).
 * Including them on log-only boards (e.g. Olimex, LED_PIN == -1) or
 * before sdkconfig is generated would cause a missing-header error. */
#if !CONFIG_STATUS_INDICATOR_HAS_DISPLAY && defined(ESP_PLATFORM) \
    && CONFIG_STATUS_INDICATOR_LED_PIN != -1
#  include "driver/ledc.h"
#  include "driver/gpio.h"
#endif

static const char *TAG = "status_indicator";

/* -----------------------------------------------------------------------
 * LED blink pattern table (LED path, ESP_PLATFORM only)
 * -----------------------------------------------------------------------
 *
 * Each entry defines the on/off timing and repeat count for a state.
 * repeat=0 means the pattern loops continuously with no gap. pause_ms
 * is only used when repeat>0 to insert a gap between blink cycles.
 *
 * INDICATOR_STATE_ERROR is a special case: both on_ms and off_ms are
 * zero, signalling the task to enter steady-on hardware mode. */

#if !CONFIG_STATUS_INDICATOR_HAS_DISPLAY && CONFIG_STATUS_INDICATOR_LED_PIN != -1 \
    && defined(ESP_PLATFORM)

typedef struct {
    uint32_t on_ms;
    uint32_t off_ms;
    uint32_t repeat;
    uint32_t pause_ms;
} led_blink_pattern_t;

static const led_blink_pattern_t s_blink_patterns[INDICATOR_STATE_COUNT] = {
    {  500,  500, 0,    0 },   /* INDICATOR_STATE_BOOT            */
    {  100,  100, 0,    0 },   /* INDICATOR_STATE_WIFI_CONNECTING */
    {  100,  100, 2, 2000 },   /* INDICATOR_STATE_WIFI_CONNECTED  */
    {  100,  100, 3, 1000 },   /* INDICATOR_STATE_WIFI_ERROR      */
    { 1000, 1000, 0,    0 },   /* INDICATOR_STATE_STREAMING       */
    {  100,  500, 0,    0 },   /* INDICATOR_STATE_OTA             */
    {    0,    0, 0,    0 },   /* INDICATOR_STATE_ERROR           */
};

#endif /* LED path + ESP_PLATFORM */

/* -----------------------------------------------------------------------
 * FreeRTOS task constants and global state
 * ----------------------------------------------------------------------- */

#define INDICATOR_TASK_STACK    2048
#define INDICATOR_TASK_PRIORITY 2

#if !CONFIG_STATUS_INDICATOR_HAS_DISPLAY && CONFIG_STATUS_INDICATOR_LED_PIN != -1 \
    && defined(ESP_PLATFORM)
static TaskHandle_t              s_task_handle   = NULL;
static volatile TaskHandle_t     s_deinit_notify_handle = NULL;

/* LEDC active-low tracking: when CONFIG_STATUS_INDICATOR_LED_ACTIVE_LOW=y,
 * the "on" level is duty=0 and the "off" level is duty=max. */
static int                       s_active_low    = 0;
static volatile int              s_task_running  = 0;
#endif
static volatile indicator_state_t s_current_state = INDICATOR_STATE_BOOT;
static SemaphoreHandle_t         s_state_mutex   = NULL;

/* -----------------------------------------------------------------------
 * Display path — status strings for log-only fallback
 * ----------------------------------------------------------------------- */

static const char *s_state_names[INDICATOR_STATE_COUNT] = {
    "BOOT",
    "WIFI_CONNECTING",
    "WIFI_CONNECTED",
    "WIFI_ERROR",
    "STREAMING",
    "OTA",
    "ERROR",
};

/* =======================================================================
 * LED path — LEDC helper functions (ESP_PLATFORM only)
 * ======================================================================= */

#if !CONFIG_STATUS_INDICATOR_HAS_DISPLAY && CONFIG_STATUS_INDICATOR_LED_PIN != -1 \
    && defined(ESP_PLATFORM)

/**
 * @brief Set the LED hardware to ON (lowest duty for active-low, max for
 *        active-high).
 */
static void led_set_on(void)
{
    uint32_t duty = s_active_low ? 0 : ((1 << LEDC_TIMER_13_BIT) - 1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

/**
 * @brief Set the LED hardware to OFF (highest duty for active-low, zero
 *        for active-high).
 */
static void led_set_off(void)
{
    uint32_t duty = s_active_low ? ((1 << LEDC_TIMER_13_BIT) - 1) : 0;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

/**
 * @brief Initialise the LEDC timer and channel for the LED pin.
 *
 * Selects LEDC_LOW_SPEED_MODE for ESP32-S3 and LEDC_HIGH_SPEED_MODE
 * for ESP32 classic. Uses 13-bit resolution and 5000 Hz PWM frequency.
 *
 * @return ESP_OK on success, or an ESP-IDF error code.
 */
static esp_err_t ledc_init(void)
{
    /* Configure the LEDC timer with 13-bit resolution at 5 kHz.
     * The resolution is chosen from ticket spec; frequency is low
     * enough to avoid unnecessary switching losses while still being
     * fast enough for human-visible patterns. */
    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = 5000,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    esp_err_t ret = ledc_timer_config(&timer_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC timer config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Configure the LEDC channel to drive the configured GPIO pin.
     * The channel is initialised with zero duty to keep the LED off
     * until the first state is set. */
    ledc_channel_config_t ch_cfg = {
        .gpio_num   = CONFIG_STATUS_INDICATOR_LED_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0,
    };
    ret = ledc_channel_config(&ch_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LEDC channel config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Reset the GPIO to a known output state.
     * ESP-IDF LEDC driver in ESP-IDF 6.0.0 does not call
     * gpio_reset_pin() automatically, so we explicitly configure
     * the pin as a floating output before the LEDC peripheral
     * takes over. */
    gpio_reset_pin(CONFIG_STATUS_INDICATOR_LED_PIN);
    gpio_set_direction(CONFIG_STATUS_INDICATOR_LED_PIN, GPIO_MODE_OUTPUT);

    /* Start with LED off. For active-low boards this means setting
     * maximum duty so that the pin is driven high (LED off). */
    led_set_off();

    return ESP_OK;
}

/**
 * @brief Deinitialise the LEDC timer and channel.
 *
 * Stops the PWM output and sets the pin back to floating input to
 * avoid leaving the LED in an unknown state after deinit.
 *
 * @return ESP_OK on success.
 */
static esp_err_t ledc_deinit(void)
{
    /* Stop the LEDC timer before releasing the channel.
     * This ensures no spurious signal edges occur during teardown. */
    esp_err_t ret = ledc_timer_pause(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "LEDC timer pause failed: %s", esp_err_to_name(ret));
    }

    /* Set the GPIO to a known high-impedance state so the LED is not
     * left partially lit after deinitialisation. */
    gpio_set_direction(CONFIG_STATUS_INDICATOR_LED_PIN, GPIO_MODE_DISABLE);

    return ESP_OK;
}

/**
 * @brief LED blink task — runs the LED blink patterns in a loop.
 *
 * Reads the current state under mutex protection, looks up the
 * corresponding blink pattern, and drives the LED through one cycle
 * of on/off toggles. The ERROR state is handled as a special case —
 * the LED is held steadily on and the state is polled every 100 ms.
 *
 * @param arg  Unused (FreeRTOS task signature).
 */
static void indicator_task(__attribute__((unused)) void *arg)
{
    /* Main blink loop.
     *
     * Each iteration reads the current state under the mutex and
     * executes the corresponding blink pattern. For patterns with
     * repeat>0, the cycle is executed 'repeat' times followed by
     * a pause. For patterns with repeat=0, the pattern loops
     * continuously until the state changes. */

    while (s_task_running) {
        /* Copy the current state under mutex protection so the
         * task sees a consistent value even if set_state() is
         * called from another thread mid-cycle. */
        if (xSemaphoreTake(s_state_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
            continue;
        }
        indicator_state_t state = s_current_state;
        xSemaphoreGive(s_state_mutex);

        if (state >= INDICATOR_STATE_COUNT) {
            /* Should not happen, but guard against corrupt state. */
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        const led_blink_pattern_t *p = &s_blink_patterns[state];

        /* ERROR state: steady-on hardware mode.
         * Turn the LED on and poll state every 100 ms until the
         * state changes from ERROR. */
        if (p->on_ms == 0 && p->off_ms == 0) {
            led_set_on();
            while (s_task_running) {
                if (xSemaphoreTake(s_state_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                    state = s_current_state;
                    xSemaphoreGive(s_state_mutex);
                    if (state != INDICATOR_STATE_ERROR) {
                        break;
                    }
                }
                /* vTaskDelay inside the inner loop gives us a 100 ms
                 * polling interval without busy-waiting. */
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            continue;
        }

        /* Normal blink pattern with continuous loop (repeat=0).
         * Toggle on/off repeatedly until the state changes. */
        if (p->repeat == 0) {
            led_set_on();
            vTaskDelay(pdMS_TO_TICKS(p->on_ms));
            if (!s_task_running) break;

            led_set_off();
            vTaskDelay(pdMS_TO_TICKS(p->off_ms));
            continue;
        }

        /* Blink pattern with a finite repeat count followed by a
         * pause. Execute 'repeat' on/off cycles, then pause. */
        for (uint32_t i = 0; i < p->repeat; i++) {
            led_set_on();
            vTaskDelay(pdMS_TO_TICKS(p->on_ms));
            if (!s_task_running) break;

            led_set_off();
            vTaskDelay(pdMS_TO_TICKS(p->off_ms));
            if (!s_task_running) break;
        }

        if (s_task_running) {
            vTaskDelay(pdMS_TO_TICKS(p->pause_ms));
        }
    }

    /* Task exiting cleanly — turn off LED and notify deinit(). */
    led_set_off();
    if (s_deinit_notify_handle != NULL) {
        xTaskNotifyGive(s_deinit_notify_handle);
    }
    s_task_handle = NULL;
    vTaskDelete(NULL);
}

#endif /* LED path + ESP_PLATFORM */

/* =======================================================================
 * Shared init / deinit / set_state
 * ======================================================================= */

esp_err_t status_indicator_init(void)
{
    /* Initialise the status indicator subsystem.
     *
     * Which backend is selected depends entirely on Kconfig choices.
     * - Display path: no background task; set_state() calls through
     *   to display_manager (or its stub) immediately.
     * - LED path: configure LEDC PWM, create the mutex, and spawn
     *   the indicator_task (ESP_PLATFORM only; host tests fall
     *   through to log-only).
     * - Log-only path: only creates the mutex; state changes are
     *   logged at ESP_LOGI level.
     *
     * Re-init is detected by checking the mutex pointer — if non-null
     * we return ESP_ERR_INVALID_STATE. */

    if (s_state_mutex != NULL) {
        ESP_LOGE(TAG, "Init called but already initialised");
        return ESP_ERR_INVALID_STATE;
    }

    /* Create mutex first — shared by all backends. It must exist
     * before any call to set_state() can succeed. */
    s_state_mutex = xSemaphoreCreateMutex();
    if (s_state_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create state mutex");
        return ESP_ERR_NO_MEM;
    }

    s_current_state = INDICATOR_STATE_BOOT;

#if CONFIG_STATUS_INDICATOR_HAS_DISPLAY
    /* Display path: no additional initialisation needed.
     * The display_manager is expected to be initialised separately
     * by the application. If it doesn't exist, the stub is used. */
    ESP_LOGI(TAG, "Status indicator initialised (display path)");

#elif CONFIG_STATUS_INDICATOR_LED_PIN != -1 && defined(ESP_PLATFORM)
    /* LED path: configure hardware and start the blink task. */
#if CONFIG_STATUS_INDICATOR_LED_ACTIVE_LOW
    s_active_low = 1;
#else
    s_active_low = 0;
#endif

    esp_err_t ret = ledc_init();
    if (ret != ESP_OK) {
        /* Clean up the mutex on LEDC init failure so the caller
         * can retry init() later. */
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
        return ret;
    }

    s_task_running = 1;
    BaseType_t task_created = xTaskCreate(
        indicator_task,
        "indicator",
        INDICATOR_TASK_STACK,
        NULL,
        INDICATOR_TASK_PRIORITY,
        &s_task_handle
    );
    if (task_created != pdPASS) {
        ESP_LOGE(TAG, "Failed to create indicator task");
        s_task_running = 0;
        ledc_deinit();
        vSemaphoreDelete(s_state_mutex);
        s_state_mutex = NULL;
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Status indicator initialised (LED path, pin=%d, active_low=%d)",
             CONFIG_STATUS_INDICATOR_LED_PIN, s_active_low);

#else
    /* Log-only path: no hardware, no task. State changes are
     * printed to the serial log via ESP_LOGI.
     *
     * This path is also used for host-side unit tests where
     * ESP_PLATFORM is not defined, regardless of Kconfig values. */
    ESP_LOGI(TAG, "Status indicator initialised (log-only path)");
#endif

    return ESP_OK;
}

esp_err_t status_indicator_deinit(void)
{
    /* Deinitialise the status indicator subsystem.
     *
     * Stops the indicator task if running, tears down LEDC (LED
     * path), and destroys the mutex. Restores state to allow a
     * subsequent re-init. */

    if (s_state_mutex == NULL) {
        ESP_LOGE(TAG, "Deinit called but not initialised");
        return ESP_ERR_INVALID_STATE;
    }

#if !CONFIG_STATUS_INDICATOR_HAS_DISPLAY && CONFIG_STATUS_INDICATOR_LED_PIN != -1 \
    && defined(ESP_PLATFORM)
    /* Stop the blink task and wait for it to exit.
     * We must not delete the mutex until the task has stopped
     * using it. */
    if (s_task_handle != NULL) {
        /* Signal the task to stop and wait for it to acknowledge
         * before deleting the mutex. This prevents deleting a mutex
         * that the task may still hold during a long vTaskDelay. */
        s_deinit_notify_handle = xTaskGetCurrentTaskHandle();
        s_task_running = 0;
        /* Wait up to 3000 ms — covers the longest blink delay
         * (WIFI_CONNECTED pause = 2000 ms) plus margin. */
        uint32_t notified = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3000));
        if (notified == 0) {
            /* Timeout — task did not exit cleanly; force-kill as fallback. */
            ESP_LOGW(TAG, "Indicator task did not exit cleanly; force-deleting");
            vTaskDelete(s_task_handle);
        }
        s_task_handle = NULL;
        s_deinit_notify_handle = NULL;
    }

    ledc_deinit();
#endif

    vSemaphoreDelete(s_state_mutex);
    s_state_mutex  = NULL;
    s_current_state = INDICATOR_STATE_BOOT;

    ESP_LOGI(TAG, "Status indicator deinitialised");
    return ESP_OK;
}

esp_err_t status_indicator_set_state(indicator_state_t state)
{
    /* Set the device status state.
     *
     * Thread-safe: protects s_current_state with the mutex.
     * No-op if the state is unchanged to avoid redundant log
     * output and hardware updates.
     *
     * The actual hardware/logging behaviour depends on the
     * compile-time backend selection. */

    if (s_state_mutex == NULL) {
        ESP_LOGE(TAG, "set_state called but not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    if (state >= INDICATOR_STATE_COUNT) {
        ESP_LOGE(TAG, "set_state called with invalid state %d", (int)state);
        return ESP_ERR_INVALID_ARG;
    }

    /* Take the mutex before reading current state to avoid a
     * TOCTOU race with the indicator task. */
    if (xSemaphoreTake(s_state_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take state mutex");
        return ESP_FAIL;
    }

    /* No-op if state has not actually changed. */
    if (s_current_state == state) {
        xSemaphoreGive(s_state_mutex);
        return ESP_OK;
    }

    s_current_state = state;
    xSemaphoreGive(s_state_mutex);

#if CONFIG_STATUS_INDICATOR_HAS_DISPLAY
    {
        /* Build a display status update from the state enum.
         * This is a simple text mapping; the display manager is
         * expected to render the string on-screen. */
        display_status_t ds = {
            .state_name = s_state_names[state],
            .state_id   = (int)state,
        };
        ESP_LOGI(TAG, "Display status updated: %s", ds.state_name);
        display_manager_update_status(&ds);
    }
#elif CONFIG_STATUS_INDICATOR_LED_PIN != -1 && defined(ESP_PLATFORM)
    ESP_LOGI(TAG, "LED status updated: %s", s_state_names[state]);
#else
    /* Log-only path: simply log the state change.
     * No hardware to drive, no task to notify. */
    ESP_LOGI(TAG, "Status: %s", s_state_names[state]);
#endif

    return ESP_OK;
}
