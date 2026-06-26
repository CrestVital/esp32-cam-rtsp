#include <string.h>
#include <stdbool.h>
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"

esp_err_t mock_nvs_flash_init_ret  = ESP_OK;
esp_err_t mock_nvs_flash_erase_ret = ESP_OK;

esp_err_t nvs_flash_init(void)  { return mock_nvs_flash_init_ret; }
esp_err_t nvs_flash_erase(void) { return mock_nvs_flash_erase_ret; }

#define MOCK_MAX_KEYS 32
#define MOCK_STR_LEN  128
#define MOCK_NS_LEN    16
#define MOCK_KEY_LEN   16

/* Type tag used to dispatch mock read operations to the correct field.
 * Each entry in the mock store carries exactly one type; read helpers
 * check the tag before returning the stored value. */
typedef enum { VAL_NONE, VAL_U8, VAL_I8, VAL_U16, VAL_STR, VAL_ERR } val_type_t;

typedef struct {
    char       ns[MOCK_NS_LEN];    /* namespace; "*" = wildcard (legacy helpers) */
    char       key[MOCK_KEY_LEN];
    val_type_t type;
    uint8_t    u8;
    int8_t     i8;
    uint16_t   u16;
    char       str[MOCK_STR_LEN];
    esp_err_t  err;
} mock_entry_t;

#define MOCK_MAX_HANDLES 8

typedef struct {
    nvs_handle_t handle;
    char         ns[MOCK_NS_LEN];
    bool         in_use;
} mock_handle_entry_t;

/* Injectable test state — not thread-safe (tests are serial).
 *
 * s_entries[] is the in-memory NVS storage. s_open_ret controls the
 * return value of nvs_open() to simulate NVS unavailability. The
 * flash-init injection globals are also cleared by mock_nvs_reset().
 *
 * s_handles[] maps opaque nvs_handle_t values to namespace strings
 * so that nvs_get_* / nvs_set_* can route to the correct namespace.
 * s_last_write_ns records the namespace that received the last write
 * performed by the SUT through a real NVS handle (not by the legacy
 * wildcard helpers). */
static mock_entry_t       s_entries[MOCK_MAX_KEYS];
static int                s_entry_count   = 0;
static esp_err_t          s_open_ret      = ESP_OK;
static mock_handle_entry_t s_handles[MOCK_MAX_HANDLES];
static nvs_handle_t       s_next_handle   = 1;
static char               s_last_write_ns[MOCK_NS_LEN] = "";

void mock_nvs_reset(void)
{
    /* Full memset ensures no state leaks between consecutive tests.
     * Also resets the flash-init injection variables and handle table
     * so that each test starts with a predictable (ESP_OK) default. */
    memset(s_entries,  0, sizeof(s_entries));
    memset(s_handles,  0, sizeof(s_handles));
    s_entry_count            = 0;
    s_open_ret               = ESP_OK;
    mock_nvs_flash_init_ret  = ESP_OK;
    mock_nvs_flash_erase_ret = ESP_OK;
    s_next_handle            = 1;
    s_last_write_ns[0]       = '\0';
}

void mock_nvs_set_open_ret(esp_err_t ret)
{
    /* Inject the return value that nvs_open() will return on the next
     * call. Use ESP_FAIL or ESP_ERR_NVS_NOT_FOUND to simulate a
     * missing or corrupt NVS partition. */
    s_open_ret = ret;
}

static const char *resolve_ns(nvs_handle_t handle)
{
    /* Look up the namespace string associated with a handle in the
     * handle table. Returns NULL when the handle is not found, which
     * callers treat as an invalid-handle error. */
    for (int i = 0; i < MOCK_MAX_HANDLES; i++) {
        if (s_handles[i].in_use && s_handles[i].handle == handle) {
            return s_handles[i].ns;
        }
    }
    return NULL;
}

static const mock_entry_t *find_ns(const char *ns, const char *key)
{
    /* Lookup order:
     *  1. Exact match (ns, key)
     *  2. Wildcard match ("*", key)
     * Returns NULL if neither is found. */
    for (int i = 0; i < s_entry_count; i++) {
        if (strcmp(s_entries[i].ns, ns) == 0 &&
            strcmp(s_entries[i].key, key) == 0) {
            return &s_entries[i];
        }
    }
    for (int i = 0; i < s_entry_count; i++) {
        if (strcmp(s_entries[i].ns, "*") == 0 &&
            strcmp(s_entries[i].key, key) == 0) {
            return &s_entries[i];
        }
    }
    return NULL;
}

static mock_entry_t *find_or_create_ns(const char *ns, const char *key)
{
    /* Linear scan for an entry matching (ns, key); O(n) is acceptable
     * because the test fixture never exceeds ~32 keys. If the pair is
     * absent, a new entry is appended. Returns NULL when the table is
     * already full — callers must handle NULL safely. */
    for (int i = 0; i < s_entry_count; i++) {
        if (strcmp(s_entries[i].ns, ns) == 0 &&
            strcmp(s_entries[i].key, key) == 0) {
            return &s_entries[i];
        }
    }
    if (s_entry_count >= MOCK_MAX_KEYS) return NULL;
    mock_entry_t *e = &s_entries[s_entry_count++];
    strncpy(e->ns, ns, MOCK_NS_LEN - 1);
    e->ns[MOCK_NS_LEN - 1] = '\0';
    strncpy(e->key, key, MOCK_KEY_LEN - 1);
    e->key[MOCK_KEY_LEN - 1] = '\0';
    return e;
}

/* Legacy namespace-unaware helpers (wildcard "*" namespace) */

void mock_nvs_set_u8(const char *key, uint8_t value)
{
    /* Pre-populate the mock store with a u8 value under the given key
     * in the wildcard "*" namespace. Call before invoking the SUT so
     * that nvs_get_u8() finds it via wildcard fallback. */
    mock_entry_t *e = find_or_create_ns("*", key);
    if (e) { e->type = VAL_U8; e->u8 = value; }
}

void mock_nvs_set_i8(const char *key, int8_t value)
{
    /* Pre-populate the mock store with an i8 value under the given key
     * in the wildcard "*" namespace. Signed; used for cam_brightness
     * which ranges from -2 to +2. */
    mock_entry_t *e = find_or_create_ns("*", key);
    if (e) { e->type = VAL_I8; e->i8 = value; }
}

void mock_nvs_set_u16(const char *key, uint16_t value)
{
    /* Pre-populate the mock store with a u16 value under the given key
     * in the wildcard "*" namespace. Used for numeric config fields
     * such as rtsp_port and cam_width. */
    mock_entry_t *e = find_or_create_ns("*", key);
    if (e) { e->type = VAL_U16; e->u16 = value; }
}

void mock_nvs_set_str_val(const char *key, const char *value)
{
    /* Pre-populate the mock store with a string value in the wildcard
     * "*" namespace. strncpy caps the copy at MOCK_STR_LEN-1 bytes;
     * the final byte is explicitly zeroed to guarantee NUL-termination
     * even when value is at the limit. */
    mock_entry_t *e = find_or_create_ns("*", key);
    if (e) {
        e->type = VAL_STR;
        strncpy(e->str, value, MOCK_STR_LEN - 1);
        e->str[MOCK_STR_LEN - 1] = '\0';
    }
}

void mock_nvs_set_key_error(const char *key, esp_err_t err)
{
    /* Inject an error to be returned whenever key is read in the "*"
     * wildcard namespace. Allows tests to simulate per-key NVS failures
     * without affecting other keys. */
    mock_entry_t *e = find_or_create_ns("*", key);
    if (e) { e->type = VAL_ERR; e->err = err; }
}

/* Namespace-aware helpers */

const char *mock_nvs_get_last_write_ns(void)
{
    /* Return the namespace that received the last write performed by the
     * SUT through an NVS handle (not by legacy wildcard helpers).
     * Returns "" when no write has occurred since the last reset. */
    return s_last_write_ns;
}

void mock_nvs_set_str_val_ns(const char *ns, const char *key,
                              const char *value)
{
    /* Pre-populate the mock store with a string value under the exact
     * (ns, key) pair. Unlike mock_nvs_set_str_val(), which uses the
     * wildcard namespace, this helper writes to the caller-specified
     * namespace so that only readers of that namespace can find it. */
    mock_entry_t *e = find_or_create_ns(ns, key);
    if (e) {
        e->type = VAL_STR;
        strncpy(e->str, value, MOCK_STR_LEN - 1);
        e->str[MOCK_STR_LEN - 1] = '\0';
    }
}

/* NVS API stubs */

esp_err_t nvs_open(const char *name, nvs_open_mode_t mode,
                   nvs_handle_t *out_handle)
{
    /* If a test has injected a non-OK return via mock_nvs_set_open_ret(),
     * return that error immediately — simulates an unavailable NVS
     * partition. Otherwise allocate a handle slot from s_handles[],
     * store the namespace string, and return a unique opaque handle
     * value. Returns ESP_ERR_NO_MEM when all slots are full. */
    (void)mode;
    if (s_open_ret != ESP_OK) return s_open_ret;
    for (int i = 0; i < MOCK_MAX_HANDLES; i++) {
        if (!s_handles[i].in_use) {
            s_handles[i].in_use = true;
            s_handles[i].handle = s_next_handle;
            strncpy(s_handles[i].ns, name, MOCK_NS_LEN - 1);
            s_handles[i].ns[MOCK_NS_LEN - 1] = '\0';
            *out_handle = s_next_handle++;
            return ESP_OK;
        }
    }
    return ESP_ERR_NO_MEM;
}

void nvs_close(nvs_handle_t handle)
{
    /* Mark the corresponding handle slot as free so it can be reused by
     * a later nvs_open() call. If the handle is not found (e.g. already
     * closed), this is a no-op. */
    for (int i = 0; i < MOCK_MAX_HANDLES; i++) {
        if (s_handles[i].in_use && s_handles[i].handle == handle) {
            s_handles[i].in_use = false;
            return;
        }
    }
}

esp_err_t nvs_get_u8(nvs_handle_t handle, const char *key, uint8_t *out)
{
    /* Resolve namespace from handle, then dispatch:
     *  - handle not found    →  ESP_ERR_NVS_NOT_FOUND
     *  - key absent          →  ESP_ERR_NVS_NOT_FOUND
     *  - key has VAL_ERR     →  injected error
     *  - wrong type tag      →  ESP_ERR_NVS_NOT_FOUND (mimics real NVS)
     *  - type matches        →  write *out, return ESP_OK */
    const char *ns = resolve_ns(handle);
    if (!ns) return ESP_ERR_NVS_NOT_FOUND;
    const mock_entry_t *e = find_ns(ns, key);
    if (!e)                 return ESP_ERR_NVS_NOT_FOUND;
    if (e->type == VAL_ERR) return e->err;
    if (e->type != VAL_U8)  return ESP_ERR_NVS_NOT_FOUND;
    *out = e->u8;
    return ESP_OK;
}

esp_err_t nvs_set_u8(nvs_handle_t handle, const char *key, uint8_t value)
{
    /* Stores a u8 value under the namespace resolved from the handle.
     * Records s_last_write_ns only after a successful entry allocation
     * to prevent a stale write-ns on table overflow.
     * Returns ESP_ERR_NO_MEM when the entry table is full. */
    const char *ns = resolve_ns(handle);
    if (!ns) return ESP_FAIL;
    mock_entry_t *e = find_or_create_ns(ns, key);
    if (!e) return ESP_ERR_NO_MEM;
    e->type = VAL_U8;
    e->u8   = value;
    strncpy(s_last_write_ns, ns, MOCK_NS_LEN - 1);
    s_last_write_ns[MOCK_NS_LEN - 1] = '\0';
    return ESP_OK;
}

esp_err_t nvs_get_i8(nvs_handle_t handle, const char *key, int8_t *out)
{
    /* Same namespace-aware type-check dispatch as nvs_get_u8. */
    const char *ns = resolve_ns(handle);
    if (!ns) return ESP_ERR_NVS_NOT_FOUND;
    const mock_entry_t *e = find_ns(ns, key);
    if (!e)                 return ESP_ERR_NVS_NOT_FOUND;
    if (e->type == VAL_ERR) return e->err;
    if (e->type != VAL_I8)  return ESP_ERR_NVS_NOT_FOUND;
    *out = e->i8;
    return ESP_OK;
}

esp_err_t nvs_set_i8(nvs_handle_t handle, const char *key, int8_t value)
{
    /* Stores an i8 value under the resolved namespace. Records
     * s_last_write_ns only after a successful entry allocation.
     * Returns ESP_ERR_NO_MEM when the entry table is full. */
    const char *ns = resolve_ns(handle);
    if (!ns) return ESP_FAIL;
    mock_entry_t *e = find_or_create_ns(ns, key);
    if (!e) return ESP_ERR_NO_MEM;
    e->type = VAL_I8;
    e->i8   = value;
    strncpy(s_last_write_ns, ns, MOCK_NS_LEN - 1);
    s_last_write_ns[MOCK_NS_LEN - 1] = '\0';
    return ESP_OK;
}

esp_err_t nvs_get_u16(nvs_handle_t handle, const char *key, uint16_t *out)
{
    /* Same namespace-aware type-check dispatch pattern. */
    const char *ns = resolve_ns(handle);
    if (!ns) return ESP_ERR_NVS_NOT_FOUND;
    const mock_entry_t *e = find_ns(ns, key);
    if (!e)                  return ESP_ERR_NVS_NOT_FOUND;
    if (e->type == VAL_ERR)  return e->err;
    if (e->type != VAL_U16)  return ESP_ERR_NVS_NOT_FOUND;
    *out = e->u16;
    return ESP_OK;
}

esp_err_t nvs_set_u16(nvs_handle_t handle, const char *key, uint16_t value)
{
    /* Stores a u16 value under the resolved namespace. Records
     * s_last_write_ns only after a successful entry allocation.
     * Returns ESP_ERR_NO_MEM when the entry table is full. */
    const char *ns = resolve_ns(handle);
    if (!ns) return ESP_FAIL;
    mock_entry_t *e = find_or_create_ns(ns, key);
    if (!e) return ESP_ERR_NO_MEM;
    e->type = VAL_U16;
    e->u16  = value;
    strncpy(s_last_write_ns, ns, MOCK_NS_LEN - 1);
    s_last_write_ns[MOCK_NS_LEN - 1] = '\0';
    return ESP_OK;
}

esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *out,
                       size_t *length)
{
    /* Special size-probe case: when out == NULL, only the required
     * buffer length is written back — mirrors the real NVS API. If
     * *length < required_len, ESP_ERR_NVS_INVALID_LENGTH is returned,
     * signalling the caller to retry with a larger buffer. */
    const char *ns = resolve_ns(handle);
    if (!ns) return ESP_ERR_NVS_NOT_FOUND;
    const mock_entry_t *e = find_ns(ns, key);
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
    /* Stores a string value under the resolved namespace. Records
     * s_last_write_ns only after a successful entry allocation so that
     * ESP_ERR_NO_MEM is never paired with a stale write-ns.
     * Returns ESP_ERR_NO_MEM when the entry table is full. */
    const char *ns = resolve_ns(handle);
    if (!ns) return ESP_FAIL;
    mock_entry_t *e = find_or_create_ns(ns, key);
    if (!e) return ESP_ERR_NO_MEM;
    e->type = VAL_STR;
    strncpy(e->str, value, MOCK_STR_LEN - 1);
    e->str[MOCK_STR_LEN - 1] = '\0';
    strncpy(s_last_write_ns, ns, MOCK_NS_LEN - 1);
    s_last_write_ns[MOCK_NS_LEN - 1] = '\0';
    return ESP_OK;
}

esp_err_t nvs_commit(nvs_handle_t handle)
{
    /* No-op — NVS persistence is simulated entirely in the in-memory
     * s_entries[] table. The real nvs_commit() flushes to flash, but
     * the mock has no backing store to flush to. */
    (void)handle; return ESP_OK;
}
