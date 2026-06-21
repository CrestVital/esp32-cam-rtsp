#pragma once

#include <stdio.h>

/* Host-side logging stubs with per-level call counters.
 *
 * Counters allow tests to verify that specific log paths (ESP_LOGW vs
 * ESP_LOGI) were exercised without parsing stderr output.
 * Each macro wraps the fprintf call and the counter increment in a
 * do-while(0) block so the macro remains a single statement.
 *
 * Debug and verbose levels are suppressed to keep test output readable
 * and deterministic. Errors and warnings go to stderr so they are
 * visible even when stdout is redirected or captured by the test runner.*/

typedef enum {
    ESP_LOG_NONE    = 0,
    ESP_LOG_ERROR   = 1,
    ESP_LOG_WARN    = 2,
    ESP_LOG_INFO    = 3,
    ESP_LOG_DEBUG   = 4,
    ESP_LOG_VERBOSE = 5,
} esp_log_level_t;

extern int mock_esp_log_error_count;
extern int mock_esp_log_warning_count;
extern int mock_esp_log_info_count;

void mock_esp_log_reset(void);

#define ESP_LOGE(tag, fmt, ...) do { \
    fprintf(stderr, "[E][%s] " fmt "\n", tag, ##__VA_ARGS__); \
    mock_esp_log_error_count++; \
} while(0)
#define ESP_LOGW(tag, fmt, ...) do { \
    fprintf(stderr, "[W][%s] " fmt "\n", tag, ##__VA_ARGS__); \
    mock_esp_log_warning_count++; \
} while(0)
#define ESP_LOGI(tag, fmt, ...) do { \
    fprintf(stderr, "[I][%s] " fmt "\n", tag, ##__VA_ARGS__); \
    mock_esp_log_info_count++; \
} while(0)
#define ESP_LOGD(tag, fmt, ...) /* suppress debug */
#define ESP_LOGV(tag, fmt, ...) /* suppress verbose */