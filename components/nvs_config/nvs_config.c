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
    strncpy(cfg->wifi_ssid,       DEFAULT_WIFI_SSID,       NVS_CONFIG_STR_MAX_LEN - 1);
    cfg->wifi_ssid[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';
    strncpy(cfg->wifi_pass,       DEFAULT_WIFI_PASS,       NVS_CONFIG_STR_MAX_LEN - 1);
    cfg->wifi_pass[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';
    cfg->rtsp_port        = DEFAULT_RTSP_PORT;
    cfg->cam_width        = DEFAULT_CAM_WIDTH;
    cfg->cam_height       = DEFAULT_CAM_HEIGHT;
    cfg->cam_fps          = DEFAULT_CAM_FPS;
    cfg->cam_brightness   = DEFAULT_CAM_BRIGHTNESS;
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
    size_t required_len = 0;
    esp_err_t ret = nvs_get_str(handle, key, NULL, &required_len);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        strncpy(dst, default_val, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_str(%s) size query failed: %s", key, esp_err_to_name(ret));
        strncpy(dst, default_val, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
    char *buf = malloc(required_len);
    if (buf == NULL) {
        ESP_LOGW(TAG, "Failed to allocate %u bytes for string key '%s'",
                 (unsigned)required_len, key);
        strncpy(dst, default_val, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
    ret = nvs_get_str(handle, key, buf, &required_len);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_get_str(%s) read failed: %s", key, esp_err_to_name(ret));
        free(buf);
        strncpy(dst, default_val, dst_size - 1);
        dst[dst_size - 1] = '\0';
        return;
    }
    size_t str_len = required_len - 1;
    if (str_len >= dst_size) {
        ESP_LOGW(TAG, "String key '%s' length %u exceeds buffer size %u, truncating",
                 key, (unsigned)str_len, (unsigned)dst_size);
        str_len = dst_size - 1;
    }
    strncpy(dst, buf, str_len);
    dst[str_len] = '\0';
    free(buf);
}

esp_err_t config_load(app_config_t *cfg)
{
    nvs_handle_t handle = 0;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "NVS namespace '%s' not found -- first boot, using defaults",
                 NVS_NAMESPACE);
        config_apply_defaults(cfg);
        return ESP_OK;
    }
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "nvs_open failed (%s) -- using defaults", esp_err_to_name(ret));
        config_apply_defaults(cfg);
        return ESP_OK;
    }

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

    if (cfg->mdns_name[0] == '\0') {
        ESP_LOGW(TAG, "mdns_name is empty -- applying default '%s'", DEFAULT_MDNS_NAME);
        strncpy(cfg->mdns_name, DEFAULT_MDNS_NAME, NVS_CONFIG_STR_MAX_LEN - 1);
        cfg->mdns_name[NVS_CONFIG_STR_MAX_LEN - 1] = '\0';
    }

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

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t config_save(const app_config_t *cfg)
{
    if (cfg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

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
    if (cfg->mdns_name[0] == '\0') {
        ESP_LOGE(TAG, "config_save: mdns_name must not be empty");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t handle = 0;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open for save failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

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

    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(ret));
    }
    nvs_close(handle);
    return ret;
}

esp_err_t config_reset(app_config_t *cfg)
{
    if (cfg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = nvs_flash_erase();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_erase failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = nvs_flash_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init after erase failed: %s", esp_err_to_name(ret));
        return ret;
    }

    config_apply_defaults(cfg);
    return config_save(cfg);
}

esp_err_t config_init(app_config_t *cfg)
{
    if (cfg == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    config_apply_defaults(cfg);
    return config_load(cfg);
}