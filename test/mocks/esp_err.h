#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t esp_err_t;

#define ESP_OK                  0
#define ESP_FAIL               -1
#define ESP_ERR_NO_MEM         0x101
#define ESP_ERR_INVALID_ARG    0x102
#define ESP_ERR_INVALID_STATE  0x103
#define ESP_ERR_NOT_FOUND      0x105
#define ESP_ERR_NVS_NOT_FOUND      0x1102
#define ESP_ERR_NVS_INVALID_NAME   0x1103
#define ESP_ERR_NVS_INVALID_LENGTH 0x1104

/* Host-side stub covering only the error codes used in this project's
 * unit tests. Not exhaustive — real ESP-IDF defines hundreds of codes.
 * Unknown values fall through to "UNKNOWN_ERROR" to keep tests running. */
static inline const char *esp_err_to_name(esp_err_t code)
{
    switch (code) {
        case ESP_OK:                return "ESP_OK";
        case ESP_FAIL:              return "ESP_FAIL";
        case ESP_ERR_NO_MEM:        return "ESP_ERR_NO_MEM";
        case ESP_ERR_INVALID_ARG:   return "ESP_ERR_INVALID_ARG";
        case ESP_ERR_INVALID_STATE: return "ESP_ERR_INVALID_STATE";
        case ESP_ERR_NOT_FOUND:     return "ESP_ERR_NOT_FOUND";
        case ESP_ERR_NVS_NOT_FOUND:   return "ESP_ERR_NVS_NOT_FOUND";
        case ESP_ERR_NVS_INVALID_LENGTH: return "ESP_ERR_NVS_INVALID_LENGTH";
        default:                    return "UNKNOWN_ERROR";
    }
}

/* Aborts the process on non-OK esp_err_t values.
 *
 * Uses fprintf(stderr, ...) instead of ESP_LOGE because this mock runs
 * on the host build system where the ESP-IDF logging layer is absent.
 * The file:line annotation helps locate failing checks in test output. */
#define ESP_ERROR_CHECK(x) do { \
    esp_err_t _err = (x); \
    if (_err != ESP_OK) { \
        fprintf(stderr, "ESP_ERROR_CHECK failed: %s at %s:%d\n", \
                esp_err_to_name(_err), __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)