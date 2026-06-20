#pragma once

#include <stdio.h>

/* Host-side logging stubs.
 *
 * Debug and verbose levels are suppressed to keep test output readable
 * and deterministic. Errors and warnings go to stderr so they are
 * visible even when stdout is redirected or captured by the test runner.*/
#define ESP_LOGE(tag, fmt, ...) fprintf(stderr, "[E][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) fprintf(stderr, "[W][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGI(tag, fmt, ...) fprintf(stderr, "[I][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, fmt, ...) /* suppress debug */
#define ESP_LOGV(tag, fmt, ...) /* suppress verbose */