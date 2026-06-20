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

static mock_entry_t s_entries[MOCK_MAX_KEYS];
static int          s_entry_count = 0;
static esp_err_t    s_open_ret    = ESP_OK;

void mock_nvs_reset(void)
{
    memset(s_entries, 0, sizeof(s_entries));
    s_entry_count            = 0;
    s_open_ret               = ESP_OK;
    mock_nvs_flash_init_ret  = ESP_OK;
    mock_nvs_flash_erase_ret = ESP_OK;
}

void mock_nvs_set_open_ret(esp_err_t ret) { s_open_ret = ret; }

static mock_entry_t *find_or_create(const char *key)
{
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
    for (int i = 0; i < s_entry_count; i++) {
        if (strcmp(s_entries[i].key, key) == 0) return &s_entries[i];
    }
    return NULL;
}

void mock_nvs_set_u8(const char *key, uint8_t value)
{
    mock_entry_t *e = find_or_create(key);
    if (e) { e->type = VAL_U8; e->u8 = value; }
}

void mock_nvs_set_i8(const char *key, int8_t value)
{
    mock_entry_t *e = find_or_create(key);
    if (e) { e->type = VAL_I8; e->i8 = value; }
}

void mock_nvs_set_u16(const char *key, uint16_t value)
{
    mock_entry_t *e = find_or_create(key);
    if (e) { e->type = VAL_U16; e->u16 = value; }
}

void mock_nvs_set_str_val(const char *key, const char *value)
{
    mock_entry_t *e = find_or_create(key);
    if (e) {
        e->type = VAL_STR;
        strncpy(e->str, value, MOCK_STR_LEN - 1);
        e->str[MOCK_STR_LEN - 1] = '\0';
    }
}

void mock_nvs_set_key_error(const char *key, esp_err_t err)
{
    mock_entry_t *e = find_or_create(key);
    if (e) { e->type = VAL_ERR; e->err = err; }
}

esp_err_t nvs_open(const char *name, nvs_open_mode_t mode, nvs_handle_t *out_handle)
{
    (void)name; (void)mode;
    if (s_open_ret != ESP_OK) return s_open_ret;
    *out_handle = 1;
    return ESP_OK;
}

esp_err_t nvs_get_u8(nvs_handle_t handle, const char *key, uint8_t *out)
{
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
    (void)handle; mock_nvs_set_u8(key, value); return ESP_OK;
}

esp_err_t nvs_get_i8(nvs_handle_t handle, const char *key, int8_t *out)
{
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
    (void)handle; mock_nvs_set_i8(key, value); return ESP_OK;
}

esp_err_t nvs_get_u16(nvs_handle_t handle, const char *key, uint16_t *out)
{
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
    (void)handle; mock_nvs_set_u16(key, value); return ESP_OK;
}

esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *out, size_t *length)
{
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
    (void)handle; mock_nvs_set_str_val(key, value); return ESP_OK;
}

esp_err_t nvs_commit(nvs_handle_t handle) { (void)handle; return ESP_OK; }
void      nvs_close(nvs_handle_t handle)  { (void)handle; }