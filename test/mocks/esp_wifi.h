#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "esp_event.h"

/* Host-side stubs for ESP WiFi types and API.
 * Only the symbols used by wifi_manager.c are stubbed here. */

typedef enum {
    WIFI_MODE_NULL = 0,
    WIFI_MODE_STA,
    WIFI_MODE_AP,
    WIFI_MODE_APSTA,
} wifi_mode_t;

typedef enum {
    WIFI_IF_STA = 0,
    WIFI_IF_AP,
} wifi_interface_t;

#define WIFI_EVENT                       "WIFI_EVENT"
#define IP_EVENT                         "IP_EVENT"
#define WIFI_EVENT_STA_START             0
#define WIFI_EVENT_STA_CONNECTED         1
#define WIFI_EVENT_STA_DISCONNECTED      2
#define IP_EVENT_STA_GOT_IP              3

typedef struct {
    uint8_t ssid[32];
    uint8_t password[64];
} wifi_sta_config_t;

typedef union {
    wifi_sta_config_t sta;
} wifi_config_t;

typedef void *wifi_init_config_t;

#define WIFI_INIT_CONFIG_DEFAULT()  NULL

/* ── Injectable return values ──────────────────────────────────────── */

extern esp_err_t mock_esp_netif_init_ret;
extern esp_err_t mock_esp_wifi_init_ret;
extern esp_err_t mock_esp_wifi_set_mode_ret;
extern esp_err_t mock_esp_wifi_set_config_ret;
extern esp_err_t mock_esp_wifi_start_ret;
extern esp_err_t mock_esp_wifi_connect_ret;
extern esp_err_t mock_esp_wifi_disconnect_ret;
extern esp_err_t mock_esp_wifi_stop_ret;
extern esp_err_t mock_esp_wifi_deinit_ret;
extern esp_err_t mock_esp_bt_controller_disable_ret;

/* ── Call counters ─────────────────────────────────────────────────── */

extern int mock_esp_netif_init_calls;
extern int mock_esp_wifi_init_calls;
extern int mock_esp_wifi_set_mode_calls;
extern int mock_esp_wifi_set_config_calls;
extern int mock_esp_wifi_start_calls;
extern int mock_esp_wifi_connect_calls;
extern int mock_esp_wifi_disconnect_calls;
extern int mock_esp_wifi_stop_calls;
extern int mock_esp_wifi_deinit_calls;
extern int mock_esp_bt_controller_disable_calls;

/* ── Captured arguments ────────────────────────────────────────────── */

extern wifi_mode_t          mock_last_wifi_mode;
extern wifi_interface_t     mock_last_wifi_iface;
extern wifi_config_t        mock_last_wifi_config;
extern wifi_init_config_t   mock_last_wifi_init_cfg;

/* ── ESP event handler registration capture ────────────────────────── */

extern esp_err_t mock_esp_event_handler_register_ret;
extern esp_err_t mock_esp_event_handler_unregister_ret;
extern int mock_esp_event_handler_register_calls;
extern int mock_esp_event_handler_unregister_calls;
extern esp_event_base_t mock_last_handler_base;
extern int32_t          mock_last_handler_event_id;

/* Reset all injected values and counters to defaults */
void mock_esp_wifi_reset(void);

/* ── Stubbed WiFi API ──────────────────────────────────────────────── */

esp_err_t esp_netif_init(void);
void     *esp_netif_create_default_wifi_sta(void);
esp_err_t esp_wifi_init(const wifi_init_config_t *config);
esp_err_t esp_wifi_set_mode(wifi_mode_t mode);
esp_err_t esp_wifi_set_config(wifi_interface_t iface, wifi_config_t *conf);
esp_err_t esp_wifi_start(void);
esp_err_t esp_wifi_connect(void);
esp_err_t esp_wifi_disconnect(void);
esp_err_t esp_wifi_stop(void);
esp_err_t esp_wifi_deinit(void);
