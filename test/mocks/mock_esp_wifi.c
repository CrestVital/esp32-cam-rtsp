#include "esp_wifi.h"
#include <string.h>

/* ── Injectable return values ──────────────────────────────────────── */

esp_err_t mock_esp_netif_init_ret       = ESP_OK;
esp_err_t mock_esp_wifi_init_ret        = ESP_OK;
esp_err_t mock_esp_wifi_set_mode_ret    = ESP_OK;
esp_err_t mock_esp_wifi_set_config_ret  = ESP_OK;
esp_err_t mock_esp_wifi_start_ret       = ESP_OK;
esp_err_t mock_esp_wifi_connect_ret     = ESP_OK;
esp_err_t mock_esp_wifi_disconnect_ret  = ESP_OK;
esp_err_t mock_esp_wifi_stop_ret        = ESP_OK;
esp_err_t mock_esp_wifi_deinit_ret      = ESP_OK;

esp_err_t mock_esp_bt_controller_disable_ret = ESP_OK;
int       mock_esp_bt_controller_disable_calls = 0;

/* ── Call counters ─────────────────────────────────────────────────── */

int mock_esp_netif_init_calls       = 0;
int mock_esp_wifi_init_calls        = 0;
int mock_esp_wifi_set_mode_calls    = 0;
int mock_esp_wifi_set_config_calls  = 0;
int mock_esp_wifi_start_calls       = 0;
int mock_esp_wifi_connect_calls     = 0;
int mock_esp_wifi_disconnect_calls  = 0;
int mock_esp_wifi_stop_calls        = 0;
int mock_esp_wifi_deinit_calls      = 0;

/* ── Captured arguments ────────────────────────────────────────────── */

wifi_mode_t        mock_last_wifi_mode      = WIFI_MODE_NULL;
wifi_interface_t   mock_last_wifi_iface     = WIFI_IF_STA;
wifi_config_t      mock_last_wifi_config;
wifi_init_config_t mock_last_wifi_init_cfg  = NULL;

/* ── ESP event handler registration capture ────────────────────────── */

esp_err_t mock_esp_event_handler_register_ret    = ESP_OK;
esp_err_t mock_esp_event_handler_unregister_ret  = ESP_OK;
int mock_esp_event_handler_register_calls        = 0;
int mock_esp_event_handler_unregister_calls      = 0;
esp_event_base_t mock_last_handler_base          = NULL;
int32_t          mock_last_handler_event_id      = 0;

void mock_esp_wifi_reset(void)
{
    /* Zero all injected return values, counters, and captured
     * arguments so each test starts with a clean mock state. */

    mock_esp_netif_init_ret      = ESP_OK;
    mock_esp_wifi_init_ret       = ESP_OK;
    mock_esp_wifi_set_mode_ret   = ESP_OK;
    mock_esp_wifi_set_config_ret = ESP_OK;
    mock_esp_wifi_start_ret      = ESP_OK;
    mock_esp_wifi_connect_ret    = ESP_OK;
    mock_esp_wifi_disconnect_ret = ESP_OK;
    mock_esp_wifi_stop_ret       = ESP_OK;
    mock_esp_wifi_deinit_ret     = ESP_OK;

    mock_esp_netif_init_calls      = 0;
    mock_esp_wifi_init_calls       = 0;
    mock_esp_wifi_set_mode_calls   = 0;
    mock_esp_wifi_set_config_calls = 0;
    mock_esp_wifi_start_calls      = 0;
    mock_esp_wifi_connect_calls    = 0;
    mock_esp_wifi_disconnect_calls = 0;
    mock_esp_wifi_stop_calls       = 0;
    mock_esp_wifi_deinit_calls     = 0;

    mock_esp_bt_controller_disable_ret  = ESP_OK;
    mock_esp_bt_controller_disable_calls = 0;

    mock_last_wifi_mode    = WIFI_MODE_NULL;
    mock_last_wifi_iface   = WIFI_IF_STA;
    memset(&mock_last_wifi_config, 0, sizeof(mock_last_wifi_config));
    mock_last_wifi_init_cfg = NULL;

    mock_esp_event_handler_register_ret    = ESP_OK;
    mock_esp_event_handler_unregister_ret  = ESP_OK;
    mock_esp_event_handler_register_calls   = 0;
    mock_esp_event_handler_unregister_calls = 0;
    mock_last_handler_base      = NULL;
    mock_last_handler_event_id  = 0;
}

/* ── Stubbed WiFi API implementations ──────────────────────────────── */

esp_err_t esp_netif_init(void)
{
    mock_esp_netif_init_calls++;
    return mock_esp_netif_init_ret;
}

void *esp_netif_create_default_wifi_sta(void)
{
    /* Return a non-NULL sentinel pointer to satisfy any NULL checks
     * in the calling code. The mock does not track netif handles. */
    return (void *)0xCAFEBABE;
}

esp_err_t esp_wifi_init(const wifi_init_config_t *config)
{
    mock_esp_wifi_init_calls++;
    if (config != NULL) {
        mock_last_wifi_init_cfg = *config;
    }
    return mock_esp_wifi_init_ret;
}

esp_err_t esp_wifi_set_mode(wifi_mode_t mode)
{
    mock_esp_wifi_set_mode_calls++;
    mock_last_wifi_mode = mode;
    return mock_esp_wifi_set_mode_ret;
}

esp_err_t esp_wifi_set_config(wifi_interface_t iface, wifi_config_t *conf)
{
    mock_esp_wifi_set_config_calls++;
    mock_last_wifi_iface = iface;
    if (conf != NULL) {
        memcpy(&mock_last_wifi_config, conf, sizeof(mock_last_wifi_config));
    }
    return mock_esp_wifi_set_config_ret;
}

esp_err_t esp_wifi_start(void)
{
    mock_esp_wifi_start_calls++;
    return mock_esp_wifi_start_ret;
}

esp_err_t esp_wifi_connect(void)
{
    mock_esp_wifi_connect_calls++;
    return mock_esp_wifi_connect_ret;
}

esp_err_t esp_wifi_disconnect(void)
{
    mock_esp_wifi_disconnect_calls++;
    return mock_esp_wifi_disconnect_ret;
}

esp_err_t esp_wifi_stop(void)
{
    mock_esp_wifi_stop_calls++;
    return mock_esp_wifi_stop_ret;
}

esp_err_t esp_wifi_deinit(void)
{
    mock_esp_wifi_deinit_calls++;
    return mock_esp_wifi_deinit_ret;
}

esp_err_t esp_bt_controller_disable(void)
{
    mock_esp_bt_controller_disable_calls++;
    return mock_esp_bt_controller_disable_ret;
}
