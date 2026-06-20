#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"

esp_err_t mock_nvs_flash_init_ret  = ESP_OK;
esp_err_t mock_nvs_flash_erase_ret = ESP_OK;

esp_err_t nvs_flash_init(void)  { return mock_nvs_flash_init_ret; }
esp_err_t nvs_flash_erase(void) { return mock_nvs_flash_erase_ret; }

#define MOCK_MAX_KEYS 32
#define MOCK_STR_LEN  128

/* Type tag used to dispatch mock read operations to the correct field.
 * Each entry in the mock store carries exactly one type; read helpers
 * check the tag before returning the stored value. */
typedef enum { VAL_NONE, VAL_U8, VAL_I8, VAL_U16, VAL_STR, VAL_ERR } val_type_t;

typedef struct {
    char       key[16];
    val_type_t type;
    uint8_t    u8;
    int8_t     i8;
    uint16_t   u16;
    char       str[MOCK_STR_LEN];
    esp_err_t  err;
} mock_entry_t;

/* Injectable test state — not thread-safe (tests are serial).
 *
 * s_entries[] is the in-memory NVS storage. s_open_ret controls the
 * return value of nvs_open() to simulate NVS unavailability. The
 * flash-init injection globals are also cleared by mock_nvs_reset(). */
static mock_entry_t s_entries[MOCK_MAX_KEYS];
static int          s_entry_count = 0;
static esp_err_t    s_open_ret    = ESP_OK;

void mock_nvs_reset(void)
{
    /* Full memset ensures no state leaks between consecutive tests.
     * Also resets the flash-init injection variables so that each test
     * starts with a predictable (ESP_OK) flash-init default. */
    memset(s_entries, 0, sizeof(s_entries));
    s_entry_count            = 0;
    s_open_ret               = ESP_OK;
    mock_nvs_flash_init_ret  = ESP_OK;
    mock_nvs_flash_erase_ret = ESP_OK;
}

void mock_nvs_set_open_ret(esp_err_t ret)
{
    /* Inject the return value that nvs_open() will return on the next
     * call. Use ESP_FAIL or ESP_ERR_NVS_NOT_FOUND to simulate a
     * missing or corrupt NVS partition. */
    s_open_ret = ret;
}

static mock_entry_t *find_or_create(const char *key)
{
    /* Linear scan for an entry matching key; O(n) is acceptable because
     * the test fixture never exceeds ~32 keys. If the key is absent, a
     * new entry is appended. Returns NULL when the table is already full
     * — callers must handle NULL safely. */
    for (int i = 0; i < s_entry_count; i++) {
        if (strcmp(s_entries[i].key, key) == 0) return &s_entries[i];
    }
    if (s_entry_count >= MOCK_MAX_KEYS) return NULL;
    mock_entry_t *e = &s_entries[s_entry_count++];
    strncpy(e->key, key, sizeof(e->key) - 1);
    e->key[sizeof(e->key) - 1] = '\0';
    return e;
}

static const mock_entry_t *find(const char *key)
{
    /* Read-only counterpart of find_or_create. Returns NULL when the key
     * is absent — the caller (nvs_get_*) converts that to NVS_NOT_FOUND. */
    for (int i = 0; i < s_entry_count; i++) {
        if (strcmp(s_entries[i].key, key) == 0) return &s_entries[i];
    }
    return NULL;
}

void mock_nvs_set_u8(const char *key, uint8_t value)
{
    /* Pre-populate the mock store with a u8 value under the given key.
     * Call before invoking the SUT so that nvs_get_u8() finds it. */
    mock_entry_t *e = find_or_create(key);
    if (e) { e->type = VAL_U8; e->u8 = value; }
}

void mock_nvs_set_i8(const char *key, int8_t value)
{
    /* Pre-populate the mock store with an i8 value under the given key.
     * Signed; used for cam_brightness which ranges from -2 to +2. */
    mock_entry_t *e = find_or_create(key);
    if (e) { e->type = VAL_I8; e->i8 = value; }
}

void mock_nvs_set_u16(const char *key, uint16_t value)
{
    /* Pre-populate the mock store with a u16 value under the given key.
     * Used for numeric config fields such as rtsp_port and cam_width. */
    mock_entry_t *e = find_or_create(key);
    if (e) { e->type = VAL_U16; e->u16 = value; }
}

void mock_nvs_set_str_val(const char *key, const char *value)
{
    /* Pre-populate the mock store with a string value. strncpy caps the
     * copy at MOCK_STR_LEN-1 bytes; the final byte is explicitly zeroed
     * to guarantee NUL-termination even when value is at the limit. */
    mock_entry_t *e = find_or_create(key);
    if (e) {
        e->type = VAL_STR;
        strncpy(e->str, value, MOCK_STR_LEN - 1);
        e->str[MOCK_STR_LEN - 1] = '\0';
    }
}

void mock_nvs_set_key_error(const char *key, esp_err_t err)
{
    /* Inject an error to be returned whenever key is read. Allows tests
     * to simulate per-key NVS failures without affecting other keys. */
    mock_entry_t *e = find_or_create(key);
    if (e) { e->type = VAL_ERR; e->err = err; }
}

esp_err_t nvs_open(const char *name, nvs_open_mode_t mode, nvs_handle_t *out_handle)
{
    /* If a test has injected a non-OK return via mock_nvs_set_open_ret(),
     * return that error immediately — simulates an unavailable NVS
     * partition. The dummy handle value 1 is returned otherwise; real
     * handles are not needed since all mock operations are global. */
    (void)name; (void)mode;
    if (s_open_ret != ESP_OK) return s_open_ret;
    *out_handle = 1;
    return ESP_OK;
}

esp_err_t nvs_get_u8(nvs_handle_t handle, const char *key, uint8_t *out)
{
    /* Type-check dispatch rules:
     *  - key absent       →  ESP_ERR_NVS_NOT_FOUND
     *  - key has VAL_ERR  →  injected error
     *  - wrong type tag   →  ESP_ERR_NVS_NOT_FOUND (mimics real NVS)
     *  - type matches     →  write *out, return ESP_OK */
    (void)handle;
    const mock_entry_t *e = find(key);
    if (!e)                 return ESP_ERR_NVS_NOT_FOUND;
    if (e->type == VAL_ERR) return e->err;
    if (e->type != VAL_U8)  return ESP_ERR_NVS_NOT_FOUND;
    *out = e->u8;
    return ESP_OK;
}

esp_err_t nvs_set_u8(nvs_handle_t handle, const char *key, uint8_t value)
{
    /* Delegates to the shared mock helper. Always returns ESP_OK —
     * this mock never simulates NVS write errors. */
    (void)handle; mock_nvs_set_u8(key, value); return ESP_OK;
}

esp_err_t nvs_get_i8(nvs_handle_t handle, const char *key, int8_t *out)
{
    /* Same type-check dispatch as nvs_get_u8 — absent / wrong-type /
     * injected-error → NVS_NOT_FOUND; matching VAL_I8 → ESP_OK. */
    (void)handle;
    const mock_entry_t *e = find(key);
    if (!e)                 return ESP_ERR_NVS_NOT_FOUND;
    if (e->type == VAL_ERR) return e->err;
    if (e->type != VAL_I8)  return ESP_ERR_NVS_NOT_FOUND;
    *out = e->i8;
    return ESP_OK;
}

esp_err_t nvs_set_i8(nvs_handle_t handle, const char *key, int8_t value)
{
    /* Delegates to mock helper; always returns ESP_OK. */
    (void)handle; mock_nvs_set_i8(key, value); return ESP_OK;
}

esp_err_t nvs_get_u16(nvs_handle_t handle, const char *key, uint16_t *out)
{
    /* Same type-check dispatch pattern; returns NVS_NOT_FOUND for absent
     * keys, injected errors, or type mismatches (non-VAL_U16). */
    (void)handle;
    const mock_entry_t *e = find(key);
    if (!e)                  return ESP_ERR_NVS_NOT_FOUND;
    if (e->type == VAL_ERR)  return e->err;
    if (e->type != VAL_U16)  return ESP_ERR_NVS_NOT_FOUND;
    *out = e->u16;
    return ESP_OK;
}

esp_err_t nvs_set_u16(nvs_handle_t handle, const char *key, uint16_t value)
{
    /* Delegates to mock helper; always returns ESP_OK. */
    (void)handle; mock_nvs_set_u16(key, value); return ESP_OK;
}

esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *out, size_t *length)
{
    /* Special size-probe case: when out == NULL, only the required
     * buffer length is written back — mirrors the real NVS API. If
     * *length < required_len, ESP_ERR_NVS_INVALID_LENGTH is returned,
     * signalling the caller to retry with a larger buffer. */
    (void)handle;
    const mock_entry_t *e = find(key);
    if (!e)                 return ESP_ERR_NVS_NOT_FOUND;
    if (e->type == VAL_ERR) return e->err;
    if (e->type != VAL_STR) return ESP_ERR_NVS_NOT_FOUND;
    size_t len = strlen(e->str) + 1;
    if (out == NULL) {
        *length = len;
        return ESP_OK;
    }
    if (*length < len) return ESP_ERR_NVS_INVALID_LENGTH;
    memcpy(out, e->str, len);
    *length = len;
    return ESP_OK;
}

esp_err_t nvs_set_str(nvs_handle_t handle, const char *key, const char *value)
{
    /* Delegates to mock helper; always returns ESP_OK. */
    (void)handle; mock_nvs_set_str_val(key, value); return ESP_OK;
}

esp_err_t nvs_commit(nvs_handle_t handle)
{
    /* No-op — NVS persistence is simulated entirely in the in-memory
     * s_entries[] table. The real nvs_commit() flushes to flash, but
     * the mock has no backing store to flush to. */
    (void)handle; return ESP_OK;
}

void nvs_close(nvs_handle_t handle)
{
    /* No-op — the mock does not track open handles or require cleanup.
     * The real nvs_close() releases a handle slot; the mock uses a
     * fixed dummy handle and needs no teardown. */
    (void)handle;
}
