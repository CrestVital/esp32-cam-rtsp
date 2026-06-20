#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_config.h"

static const char *TAG = "nvs_config";

#define NVS_NAMESPACE              "espcamfw"

#define KEY_WIFI_SSID              "wifi_ssid"
#define KEY_WIFI_PASS              "wifi_pass"
#define KEY_RTSP_PORT              "rtsp_port"
#define KEY_CAM_WIDTH              "cam_width"
#define KEY_CAM_HEIGHT             "cam_height"
#define KEY_CAM_FPS                "cam_fps"
#define KEY_CAM_BRIGHTNESS         "cam_bright"
#define KEY_AUTH_USER              "auth_user"
#define KEY_AUTH_PASS_HASH         "auth_pass_hash"
#define KEY_MDNS_NAME              "mdns_name"

#define DEFAULT_WIFI_SSID          ""
#define DEFAULT_WIFI_PASS          ""
#define DEFAULT_RTSP_PORT          554u
#define DEFAULT_CAM_WIDTH          1280u
#define DEFAULT_CAM_HEIGHT         720u
#define DEFAULT_CAM_FPS            15u
#define DEFAULT_CAM_BRIGHTNESS     0
#define DEFAULT_AUTH_USER          ""
#define DEFAULT_AUTH_PASS_HASH     ""
#define DEFAULT_MDNS_NAME          "espcam"

#define RTSP_PORT_MIN        1
#define CAM_WIDTH_MIN        160
#define CAM_WIDTH_MAX        1920
#define CAM_HEIGHT_MIN       120
#define CAM_HEIGHT_MAX       1080
#define CAM_FPS_MIN          1
#define CAM_FPS_MAX          60
#define CAM_BRIGHTNESS_MIN   (-2)
#define CAM_BRIGHTNESS_MAX   2

static void config_apply_defaults(app_config_t *cfg)
{
    /* Apply compile-time defaults to the in-memory config struct.
     *
     * This is a pure in-memory operation -- no NVS access takes
     * place. Called on first boot (when the NVS namespace does not
     * yet exist) and by config_reset() after erasing the flash
     * partition. Every strncpy is followed by an explicit NUL-
     * termination because strncpy does not guarantee a terminating
     * NUL when the source length equals or exceeds the destination
     * buffer size. */

    /* String fields (group 1) -- wifi credentials. The explicit
     * NUL-termination after strncpy guards against unterminated
     * buffers when defaults or stored values exactly fill dst. */
    strncpy(cfg->wifi_ssid,       DEFAULT_WIFI_SSID,       NVS_CONFIG_STR_MAX_LEN - 1);
    cfg->wifi_ssid[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';
    strncpy(cfg->wifi_pass,       DEFAULT_WIFI_PASS,       NVS_CONFIG_STR_MAX_LEN - 1);
    cfg->wifi_pass[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';

    /* Numeric fields -- port, resolution, FPS, brightness. Set
     * directly from compile-time defaults; no range checks needed
     * because the define values themselves are already within the
     * valid range. */
    cfg->rtsp_port        = DEFAULT_RTSP_PORT;
    cfg->cam_width        = DEFAULT_CAM_WIDTH;
    cfg->cam_height       = DEFAULT_CAM_HEIGHT;
    cfg->cam_fps          = DEFAULT_CAM_FPS;
    cfg->cam_brightness   = DEFAULT_CAM_BRIGHTNESS;

    /* String fields (group 2) -- auth credentials and mDNS hostname.
     * Same NUL-termination pattern as wifi fields above. */
    strncpy(cfg->auth_user,       DEFAULT_AUTH_USER,       NVS_CONFIG_STR_MAX_LEN - 1);
    cfg->auth_user[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';
    strncpy(cfg->auth_pass_hash,  DEFAULT_AUTH_PASS_HASH,  NVS_CONFIG_STR_MAX_LEN - 1);
    cfg->auth_pass_hash[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';
    strncpy(cfg->mdns_name,       DEFAULT_MDNS_NAME,       NVS_CONFIG_STR_MAX_LEN - 1);
    cfg->mdns_name[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';
}

static void load_str_field(nvs_handle_t handle, const char *key,
                           char *dst, size_t dst_size, const char *default_val)
{
    /* Read a string from NVS into a fixed-size destination buffer.
     *
     * Uses a two-step read: first query the required buffer size
     * (passing NULL as dst), then dynamically allocate and read the
     * actual value. This avoids fixed-size stack buffers and handles
     * stored strings of unknown length safely. The malloc() uses the
     * internal heap -- acceptable for infrequent config reads.
     *
     * On any failure -- key missing, read error, or allocation
     * failure -- the function falls back to default_val and returns
     * without modifying further state. */

    /* First call: size query with NULL destination buffer.
     * nvs_get_str writes the required buffer length (including the
     * NUL terminator) into required_len without copying any data. */
    size_t required_len = 0;
    esp_err_t ret = nvs_get_str(handle, key, NULL, &required_len);
    /* Key absent -- typical on first boot. Not an error; apply the
     * compile-time default. */
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        strncpy(dst, default_val, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
    /* NVS read error on size query (flash corruption, etc.).
     * Unrecoverable -- fall back to default. */
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_str(%s) size query failed: %s", key, esp_err_to_name(ret));
        strncpy(dst, default_val, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
    /* Allocate a temporary buffer sized exactly to the stored string.
     * Small allocation from internal heap -- acceptable because
     * config_load runs infrequently (once per boot). */
    char *buf = malloc(required_len);
    if (buf == NULL) {
        ESP_LOGW(TAG, "Failed to allocate %u bytes for string key '%s'",
                 (unsigned)required_len, key);
        strncpy(dst, default_val, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
    /* Second call: actual read into the dynamically allocated buffer. */
    ret = nvs_get_str(handle, key, buf, &required_len);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_str(%s) read failed: %s", key, esp_err_to_name(ret));
        /* Free the temporary buffer before falling back to the
         * default to avoid leaking memory on the error path. */
        free(buf);
        strncpy(dst, default_val, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
    /* The stored value may be longer than the current destination
     * buffer size (e.g. after a firmware update reduced the max
     * string length). Truncate to dst_size - 1 and warn. */
    size_t str_len = required_len - 1;
    if (str_len >= dst_size) {
        ESP_LOGW(TAG, "String key '%s' length %u exceeds buffer size %u, truncating",
                 key, (unsigned)str_len, (unsigned)dst_size);
        str_len = dst_size - 1;
    }
    /* Copy the validated (possibly truncated) string into the
     * destination and NUL-terminate explicitly. Free the temporary
     * buffer now that the data has been transferred. */
    strncpy(dst, buf, str_len);
    dst[str_len] = '\0';
    free(buf);
}

esp_err_t config_load(app_config_t *cfg)
{
    /* Load all configuration fields from the NVS namespace
     * "espcamfw" into *cfg.
     *
     * If the namespace does not exist (first boot) or NVS is
     * unreadable, the function silently falls back to compile-time
     * defaults via config_apply_defaults() and returns ESP_OK.
     *
     * Per-field range validation: out-of-range stored values are
     * replaced with compile-time defaults and a warning is logged;
     * no error is returned to the caller. The NVS handle is always
     * closed before returning. */

    /* Open the NVS namespace in read-only mode. No writes are
     * performed during load, so NVS_READONLY avoids unnecessary
     * wear on the flash partition. */
    nvs_handle_t handle = 0;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    /* Namespace absent -- first boot. Apply defaults in RAM and
     * return success; the namespace will be created on the next
     * config_save() call. */
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "NVS namespace '%s' not found -- first boot, using defaults",
                 NVS_NAMESPACE);
        config_apply_defaults(cfg);
        return ESP_OK;
    }
    /* nvs_open failed for a reason other than "not found" (e.g.
     * flash corruption). Silently fall back to defaults so the
     * device remains operational. */
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_open failed (%s) -- using defaults", esp_err_to_name(ret));
        config_apply_defaults(cfg);
        return ESP_OK;
    }

    /* Load string fields from NVS. Each load_str_field call falls
     * back to its compile-time default if the key is absent or the
     * stored value is unreadable. */
    load_str_field(handle, KEY_WIFI_SSID,      cfg->wifi_ssid,
                   NVS_CONFIG_STR_MAX_LEN, DEFAULT_WIFI_SSID);
    load_str_field(handle, KEY_WIFI_PASS,      cfg->wifi_pass,
                   NVS_CONFIG_STR_MAX_LEN, DEFAULT_WIFI_PASS);
    load_str_field(handle, KEY_AUTH_USER,      cfg->auth_user,
                   NVS_CONFIG_STR_MAX_LEN, DEFAULT_AUTH_USER);
    load_str_field(handle, KEY_AUTH_PASS_HASH, cfg->auth_pass_hash,
                   NVS_CONFIG_STR_MAX_LEN, DEFAULT_AUTH_PASS_HASH);
    load_str_field(handle, KEY_MDNS_NAME,      cfg->mdns_name,
                   NVS_CONFIG_STR_MAX_LEN, DEFAULT_MDNS_NAME);

    /* An empty mDNS hostname is invalid -- mDNS requires at least
     * one character in the hostname to advertise a service. If the
     * stored value was empty (e.g. manually set to "" via the web
     * UI), replace it with the compile-time default. */
    if (cfg->mdns_name[0] == '\0') {
        ESP_LOGW(TAG, "mdns_name is empty -- applying default '%s'", DEFAULT_MDNS_NAME);
        strncpy(cfg->mdns_name, DEFAULT_MDNS_NAME, NVS_CONFIG_STR_MAX_LEN - 1);
        cfg->mdns_name[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';
    }

    /* Read RTSP port and validate. Port numbers below RTSP_PORT_MIN
     * are rejected and replaced with the default. No upper bound
     * check is needed -- uint16_t bounds imply [0..65535] and the
     * minimum check already covers the invalid range. */
    uint16_t val_u16 = 0;
    ret = nvs_get_u16(handle, KEY_RTSP_PORT, &val_u16);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "Key '%s' not found -- using default %u", KEY_RTSP_PORT,
                 (unsigned)DEFAULT_RTSP_PORT);
        cfg->rtsp_port = DEFAULT_RTSP_PORT;
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_u16(%s) failed: %s", KEY_RTSP_PORT, esp_err_to_name(ret));
        cfg->rtsp_port = DEFAULT_RTSP_PORT;
    } else if (val_u16 < RTSP_PORT_MIN) {
        ESP_LOGW(TAG, "rtsp_port %u out of range [%d..65535] -- applying default %u",
                 (unsigned)val_u16, RTSP_PORT_MIN, (unsigned)DEFAULT_RTSP_PORT);
        cfg->rtsp_port = DEFAULT_RTSP_PORT;
    } else {
        cfg->rtsp_port = val_u16;
    }

    /* Read camera width and validate against [CAM_WIDTH_MIN..
     * CAM_WIDTH_MAX]. Out-of-range values are replaced with the
     * default to prevent the camera driver from receiving an
     * unsupported resolution. */
    ret = nvs_get_u16(handle, KEY_CAM_WIDTH, &val_u16);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "Key '%s' not found -- using default %u", KEY_CAM_WIDTH,
                 (unsigned)DEFAULT_CAM_WIDTH);
        cfg->cam_width = DEFAULT_CAM_WIDTH;
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_u16(%s) failed: %s", KEY_CAM_WIDTH, esp_err_to_name(ret));
        cfg->cam_width = DEFAULT_CAM_WIDTH;
    } else if (val_u16 < CAM_WIDTH_MIN || val_u16 > CAM_WIDTH_MAX) {
        ESP_LOGW(TAG, "cam_width %u out of range [%d..%d] -- applying default %u",
                 (unsigned)val_u16, CAM_WIDTH_MIN, CAM_WIDTH_MAX, (unsigned)DEFAULT_CAM_WIDTH);
        cfg->cam_width = DEFAULT_CAM_WIDTH;
    } else {
        cfg->cam_width = val_u16;
    }

    /* Read camera height -- same pattern as cam_width; validated against
     * [CAM_HEIGHT_MIN..CAM_HEIGHT_MAX] to prevent unsupported resolutions. */
    ret = nvs_get_u16(handle, KEY_CAM_HEIGHT, &val_u16);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "Key '%s' not found -- using default %u", KEY_CAM_HEIGHT,
                 (unsigned)DEFAULT_CAM_HEIGHT);
        cfg->cam_height = DEFAULT_CAM_HEIGHT;
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_u16(%s) failed: %s", KEY_CAM_HEIGHT, esp_err_to_name(ret));
        cfg->cam_height = DEFAULT_CAM_HEIGHT;
    } else if (val_u16 < CAM_HEIGHT_MIN || val_u16 > CAM_HEIGHT_MAX) {
        ESP_LOGW(TAG, "cam_height %u out of range [%d..%d] -- applying default %u",
                 (unsigned)val_u16, CAM_HEIGHT_MIN, CAM_HEIGHT_MAX, (unsigned)DEFAULT_CAM_HEIGHT);
        cfg->cam_height = DEFAULT_CAM_HEIGHT;
    } else {
        cfg->cam_height = val_u16;
    }

    /* Read camera FPS as uint8_t from NVS (range [1..60]). The u8
     * type in NVS matches the struct member width; out-of-range
     * values are replaced with the default. */
    uint8_t val_u8 = 0;
    ret = nvs_get_u8(handle, KEY_CAM_FPS, &val_u8);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "Key '%s' not found -- using default %u", KEY_CAM_FPS,
                 (unsigned)DEFAULT_CAM_FPS);
        cfg->cam_fps = DEFAULT_CAM_FPS;
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_u8(%s) failed: %s", KEY_CAM_FPS, esp_err_to_name(ret));
        cfg->cam_fps = DEFAULT_CAM_FPS;
    } else if (val_u8 < CAM_FPS_MIN || val_u8 > CAM_FPS_MAX) {
        ESP_LOGW(TAG, "cam_fps %u out of range [%d..%d] -- applying default %u",
                 (unsigned)val_u8, CAM_FPS_MIN, CAM_FPS_MAX, (unsigned)DEFAULT_CAM_FPS);
        cfg->cam_fps = DEFAULT_CAM_FPS;
    } else {
        cfg->cam_fps = val_u8;
    }

    /* Read brightness as int8_t (signed, range [-2..2]). Unlike
     * other numeric fields, brightness is signed -- the camera
     * driver expects a signed integer for exposure adjustment. */
    int8_t val_i8 = 0;
    ret = nvs_get_i8(handle, KEY_CAM_BRIGHTNESS, &val_i8);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "Key '%s' not found -- using default %d", KEY_CAM_BRIGHTNESS,
                 (int)DEFAULT_CAM_BRIGHTNESS);
        cfg->cam_brightness = DEFAULT_CAM_BRIGHTNESS;
    } else if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_i8(%s) failed: %s", KEY_CAM_BRIGHTNESS, esp_err_to_name(ret));
        cfg->cam_brightness = DEFAULT_CAM_BRIGHTNESS;
    } else if (val_i8 < CAM_BRIGHTNESS_MIN || val_i8 > CAM_BRIGHTNESS_MAX) {
        ESP_LOGW(TAG, "cam_brightness %d out of range [%d..%d] -- applying default %d",
                 (int)val_i8, CAM_BRIGHTNESS_MIN, CAM_BRIGHTNESS_MAX, (int)DEFAULT_CAM_BRIGHTNESS);
        cfg->cam_brightness = DEFAULT_CAM_BRIGHTNESS;
    } else {
        cfg->cam_brightness = val_i8;
    }

    /* Release the NVS handle. Read-only namespace, so no commit is
     * needed before closing. */
    nvs_close(handle);
    return ESP_OK;
}

esp_err_t config_save(const app_config_t *cfg)
{
    /* Persist all fields from *cfg into the NVS namespace.
     *
     * All input values are validated against their allowed ranges
     * before opening NVS. This "validate first, open NVS second"
     * strategy avoids a partial write: NVS is transactional only
     * within a single commit cycle, so writing half the fields and
     * then failing would leave the partition inconsistent.
     *
     * On any nvs_set_* failure the handle is closed and the error
     * returned immediately. No rollback is attempted -- the caller
     * must treat a non-ESP_OK return as "NVS may be partially
     * updated; call config_reset() if in doubt."
     * nvs_commit() at the end flushes the pending write buffer to
     * flash. */

    /* NULL pointer here is a caller contract violation -- no config
     * struct to write into. Return immediately without touching NVS. */
    if (cfg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    /* Validate all fields before touching NVS. This prevents a
     * scenario where some fields are written and a later one fails,
     * leaving NVS with an incomplete set of values. Every check is
     * independent of the others -- all must pass before nvs_open. */
    if (cfg->rtsp_port < RTSP_PORT_MIN) {
        ESP_LOGE(TAG, "config_save: rtsp_port %u is invalid (must be >= %d)",
                 (unsigned)cfg->rtsp_port, RTSP_PORT_MIN);
        return ESP_ERR_INVALID_ARG;
    }
    if (cfg->cam_width < CAM_WIDTH_MIN || cfg->cam_width > CAM_WIDTH_MAX) {
        ESP_LOGE(TAG, "config_save: cam_width %u out of range [%d..%d]",
                 (unsigned)cfg->cam_width, CAM_WIDTH_MIN, CAM_WIDTH_MAX);
        return ESP_ERR_INVALID_ARG;
    }
    if (cfg->cam_height < CAM_HEIGHT_MIN || cfg->cam_height > CAM_HEIGHT_MAX) {
        ESP_LOGE(TAG, "config_save: cam_height %u out of range [%d..%d]",
                 (unsigned)cfg->cam_height, CAM_HEIGHT_MIN, CAM_HEIGHT_MAX);
        return ESP_ERR_INVALID_ARG;
    }
    if (cfg->cam_fps < CAM_FPS_MIN || cfg->cam_fps > CAM_FPS_MAX) {
        ESP_LOGE(TAG, "config_save: cam_fps %u out of range [%d..%d]",
                 (unsigned)cfg->cam_fps, CAM_FPS_MIN, CAM_FPS_MAX);
        return ESP_ERR_INVALID_ARG;
    }
    if (cfg->cam_brightness < CAM_BRIGHTNESS_MIN || cfg->cam_brightness > CAM_BRIGHTNESS_MAX) {
        ESP_LOGE(TAG, "config_save: cam_brightness %d out of range [%d..%d]",
                 (int)cfg->cam_brightness, CAM_BRIGHTNESS_MIN, CAM_BRIGHTNESS_MAX);
        return ESP_ERR_INVALID_ARG;
    }
    /* mDNS requires a non-empty hostname. Reject early rather than
     * storing an invalid value that would break mDNS at runtime. */
    if (cfg->mdns_name[0] == '\0') {
        ESP_LOGE(TAG, "config_save: mdns_name must not be empty");
        return ESP_ERR_INVALID_ARG;
    }

    /* Open the namespace for writing. All validation has passed by
     * this point, so any failure here is a genuine I/O error. */
    nvs_handle_t handle = 0;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open for save failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    /* Write string fields. Each error path closes the handle and
     * returns immediately -- continuing after a failure would leave
     * an unpredictable subset of fields persisted. */
    ret = nvs_set_str(handle, KEY_WIFI_SSID, cfg->wifi_ssid);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str(%s) failed: %s", KEY_WIFI_SSID, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_str(handle, KEY_WIFI_PASS, cfg->wifi_pass);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str(%s) failed: %s", KEY_WIFI_PASS, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_str(handle, KEY_AUTH_USER, cfg->auth_user);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str(%s) failed: %s", KEY_AUTH_USER, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_str(handle, KEY_AUTH_PASS_HASH, cfg->auth_pass_hash);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str(%s) failed: %s", KEY_AUTH_PASS_HASH, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_str(handle, KEY_MDNS_NAME, cfg->mdns_name);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str(%s) failed: %s", KEY_MDNS_NAME, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    /* Write numeric fields using their natural NVS types (u16, u8,
     * i8). Same immediate-return-on-error pattern as the string
     * writes above. */
    ret = nvs_set_u16(handle, KEY_RTSP_PORT, cfg->rtsp_port);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_u16(%s) failed: %s", KEY_RTSP_PORT, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_u16(handle, KEY_CAM_WIDTH, cfg->cam_width);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_u16(%s) failed: %s", KEY_CAM_WIDTH, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_u16(handle, KEY_CAM_HEIGHT, cfg->cam_height);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_u16(%s) failed: %s", KEY_CAM_HEIGHT, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_u8(handle, KEY_CAM_FPS, cfg->cam_fps);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_u8(%s) failed: %s", KEY_CAM_FPS, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    ret = nvs_set_i8(handle, KEY_CAM_BRIGHTNESS, cfg->cam_brightness);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_i8(%s) failed: %s", KEY_CAM_BRIGHTNESS, esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    /* Flush the pending write buffer to flash. Even if commit fails,
     * the handle is still closed -- the caller must inspect the
     * return value to know whether persistence succeeded. */
    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(ret));
    }
    nvs_close(handle);
    return ret;
}

esp_err_t config_reset(app_config_t *cfg)
{
    /* Perform a factory reset by erasing the entire NVS flash
     * partition and re-initialising it.
     *
     * This is destructive: it removes ALL NVS namespaces, not just
     * "espcamfw". After erasing and re-initialising, compile-time
     * defaults are written to NVS so that the next config_load()
     * finds valid values. Intended for use by the hardware reset
     * button handler or the web UI. */

    /* NULL cfg means there is nowhere to write the post-reset defaults --
     * abort before touching flash. */
    if (cfg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    /* Erase the entire NVS flash partition. Destructive -- this
     * removes every namespace, including WiFi calibration data and
     * any other component's stored values, not just "espcamfw". */
    esp_err_t ret = nvs_flash_erase();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_erase failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Re-initialise the NVS subsystem on the freshly erased
     * partition. Without this, config_save() would fail because
     * no NVS partition would be mounted. */
    ret = nvs_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init after erase failed: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Two-step: apply fills *cfg with defaults in RAM, then save
     * persists those defaults to flash so that a subsequent
     * config_load() finds valid values. Without the save, the next
     * boot would see an empty namespace and fall back to defaults
     * again -- but config_reset is meant to persist the defaults. */
    config_apply_defaults(cfg);
    return config_save(cfg);
}

esp_err_t config_init(app_config_t *cfg)
{
    /* Entry point called once at startup before any other component
     * reads app_config. Fills *cfg with compile-time defaults as a
     * safe baseline, then overlays any values persisted in NVS.
     *
     * The two-step approach ensures *cfg is always fully initialised
     * even if config_load() encounters a partial or corrupted NVS
     * namespace -- the default values serve as a fallback for any
     * field that cannot be loaded. */

    /* NULL cfg is a caller bug -- config_init must have a valid struct
     * to fill before any other component reads app_config. */
    if (cfg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    /* First, establish a safe baseline: fill every field with its
     * compile-time default. This guarantees no field is ever left
     * uninitialised regardless of what config_load() does. */
    config_apply_defaults(cfg);

    /* Overlay NVS-persisted values on top of the defaults.
     * Fields present in NVS replace the baseline; absent fields
     * keep their default values from the step above. */
    return config_load(cfg);
}
