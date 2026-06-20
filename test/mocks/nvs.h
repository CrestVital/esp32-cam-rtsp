#pragma once

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

typedef uint32_t nvs_handle_t;
typedef int      nvs_open_mode_t;

#define NVS_READONLY   0
#define NVS_READWRITE  1

/* Test-side helpers to pre-populate the in-memory mock NVS store.
 * Call these before invoking the system under test to set up the
 * exact NVS state the test case expects. */
void mock_nvs_reset(void);
void mock_nvs_set_open_ret(esp_err_t ret);
void mock_nvs_set_u8(const char *key, uint8_t value);
void mock_nvs_set_i8(const char *key, int8_t value);
void mock_nvs_set_u16(const char *key, uint16_t value);
void mock_nvs_set_str_val(const char *key, const char *value);
void mock_nvs_set_key_error(const char *key, esp_err_t err);

/* NVS API stubs — mirror the real ESP-IDF NVS function signatures.
 * Implementation lives in mock_nvs_state.c; these allow components to
 * link against a fake NVS layer in host-based unit tests. */
esp_err_t nvs_open(const char *name, nvs_open_mode_t mode, nvs_handle_t *out_handle);
esp_err_t nvs_get_u8(nvs_handle_t handle, const char *key, uint8_t *out);
esp_err_t nvs_set_u8(nvs_handle_t handle, const char *key, uint8_t value);
esp_err_t nvs_get_i8(nvs_handle_t handle, const char *key, int8_t *out);
esp_err_t nvs_set_i8(nvs_handle_t handle, const char *key, int8_t value);
esp_err_t nvs_get_u16(nvs_handle_t handle, const char *key, uint16_t *out);
esp_err_t nvs_set_u16(nvs_handle_t handle, const char *key, uint16_t value);
esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *out, size_t *length);
esp_err_t nvs_set_str(nvs_handle_t handle, const char *key, const char *value);
esp_err_t nvs_commit(nvs_handle_t handle);
void      nvs_close(nvs_handle_t handle);