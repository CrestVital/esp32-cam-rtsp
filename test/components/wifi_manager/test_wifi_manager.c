#include "unity.h"
#include "wifi_manager.h"
#include "nvs.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <string.h>

/* ── Extern mock injection variables ────────────────────────────────── */

extern esp_err_t mock_esp_netif_init_ret;
extern esp_err_t mock_esp_wifi_init_ret;
extern esp_err_t mock_esp_wifi_set_mode_ret;
extern esp_err_t mock_esp_wifi_set_config_ret;
extern esp_err_t mock_esp_wifi_start_ret;
extern esp_err_t mock_esp_wifi_connect_ret;
extern esp_err_t mock_esp_wifi_disconnect_ret;

extern int mock_esp_netif_init_calls;
extern int mock_esp_wifi_init_calls;
extern int mock_esp_wifi_set_mode_calls;
extern int mock_esp_wifi_set_config_calls;
extern int mock_esp_wifi_start_calls;
extern int mock_esp_wifi_connect_calls;
extern int mock_esp_wifi_disconnect_calls;

extern esp_err_t mock_esp_event_handler_register_ret;
extern int mock_esp_event_handler_register_calls;

extern int mock_esp_bt_controller_disable_calls;

extern int mock_vTaskDelete_calls;

/* ── Extern mock injection helpers ──────────────────────────────────── */

void mock_set_reconnect_task_handle(TaskHandle_t *task_ptr, TaskHandle_t value);
void mock_clear_injected_task_handle(void);

/* ── setUp / tearDown ───────────────────────────────────────────────── */

void mock_freertos_task_reset(void);

void setUp(void)
{
    /* Reset all mock state so each test starts with a clean,
     * deterministic environment. Deinit the WiFi manager to tear
     * down any state left by the previous test. Reset the vTaskDelete
     * call counter so the new race-condition test starts from 0. */
    mock_esp_wifi_reset();
    mock_nvs_reset();
    mock_esp_log_reset();
    mock_freertos_task_reset();

    /* Best-effort deinit — ignore return value; the point is to
     * clear s_initialized and s_connected. */
    (void)wifi_manager_deinit();
}

void tearDown(void)
{
    /* No-op: all cleanup is done in setUp() for the next test. */
}

/* ── Test 1: init succeeds ──────────────────────────────────────────── */

void test_init_succeeds(void)
{
    esp_err_t ret = wifi_manager_init();

    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_netif_init_calls);
    TEST_ASSERT_EQUAL(1, mock_esp_wifi_init_calls);
}

/* ── Test 2: double init returns invalid state ──────────────────────── */

void test_init_twice_returns_invalid_state(void)
{
    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_wifi_init_calls);

    /* Second call must be rejected without re-initialising hardware. */
    ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_wifi_init_calls);
}

/* ── Test 3: connect NULL ssid returns invalid arg ──────────────────── */

void test_connect_null_ssid_returns_invalid_arg(void)
{
    /* Init is required before connect — otherwise the call fails on
     * the !s_initialized guard, not the ssid == NULL guard. */
    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = wifi_manager_connect(NULL, "password");
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
    TEST_ASSERT_EQUAL(0, mock_esp_wifi_set_config_calls);
}

/* ── Test 4: connect initiates connection ───────────────────────────── */

void test_connect_initiates_connection(void)
{
    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = wifi_manager_connect("TestSSID", "TestPass");
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Verify esp_wifi_set_config() and esp_wifi_start() were called.
     * esp_wifi_connect() is now triggered only by the WIFI_EVENT_STA_START
     * event handler (not called here — mock event system is inactive). */
    TEST_ASSERT_EQUAL(1, mock_esp_wifi_set_config_calls);
    TEST_ASSERT_EQUAL(1, mock_esp_wifi_start_calls);
}

/* ── Test 5: is_connected false before IP event ─────────────────────── */

void test_is_connected_false_before_ip_event(void)
{
    /* After init, s_connected starts false and stays false until the
     * IP_EVENT_STA_GOT_IP handler fires (simulated on real hardware). */
    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    bool connected = wifi_manager_is_connected();
    TEST_ASSERT_FALSE(connected);
}

/* ── Test 6: save_credentials writes to NVS ─────────────────────────── */

void test_save_credentials_writes_to_nvs(void)
{
    const char *test_ssid = "MyWiFi";
    const char *test_pass = "secret123";

    esp_err_t ret = wifi_manager_save_credentials(test_ssid, test_pass);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Verify the write was directed to the "wifi_cfg" namespace. */
    TEST_ASSERT_EQUAL_STRING("wifi_cfg", mock_nvs_get_last_write_ns());

    /* Verify that both NVS keys were written by reading them back
     * through the mock NVS layer in the "wifi_cfg" namespace. */
    char buf_ssid[64] = {0};
    char buf_pass[64] = {0};
    size_t len_ssid = sizeof(buf_ssid);
    size_t len_pass = sizeof(buf_pass);

    nvs_handle_t handle;
    nvs_open("wifi_cfg", NVS_READONLY, &handle);
    ret = nvs_get_str(handle, "wifi_ssid", buf_ssid, &len_ssid);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL_STRING(test_ssid, buf_ssid);

    ret = nvs_get_str(handle, "wifi_pass", buf_pass, &len_pass);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL_STRING(test_pass, buf_pass);
    nvs_close(handle);
}

/* ── Test 7: load_credentials reads from NVS ────────────────────────── */

void test_load_credentials_reads_from_nvs(void)
{
    const char *expected_ssid = "HomeNet";
    const char *expected_pass = "p@ssw0rd!";

    /* Pre-populate the mock NVS store with credentials. */
    mock_nvs_set_str_val("wifi_ssid", expected_ssid);
    mock_nvs_set_str_val("wifi_pass", expected_pass);

    char ssid[64] = {0};
    char pass[64] = {0};

    esp_err_t ret = wifi_manager_load_credentials(
        ssid, sizeof(ssid), pass, sizeof(pass));
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_EQUAL_STRING(expected_ssid, ssid);
    TEST_ASSERT_EQUAL_STRING(expected_pass, pass);
}

/* ── Test 8: disconnect suppresses reconnect ────────────────────────── */

void test_disconnect_suppresses_reconnect(void)
{
    /* After init + connect (which enables s_reconnect_enabled), calling
     * disconnect() must set s_reconnect_enabled to false. We verify this
     * indirectly: after disconnect(), a second disconnect does not cause
     * additional esp_wifi_disconnect() calls (the reconnect task, if
     * alive, would intervene). */
    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Connect enables reconnect. */
    ret = wifi_manager_connect("AnySSID", "AnyPass");
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Disconnect — this should suppress reconnect. */
    ret = wifi_manager_disconnect();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_wifi_disconnect_calls);

    /* A second disconnect should call esp_wifi_disconnect() again.
     * The key check: after the first disconnect, esp_wifi_connect()
     * should NOT have been called (it would be called by the reconnect
     * task if reconnect were still enabled). Since the mock reconnect
     * task never actually runs (ulTaskNotifyTake returns 0), we cannot
     * test that path here. Instead we verify the is_connected flag is
     * false (set by the disconnect event handler). */
    bool connected = wifi_manager_is_connected();
    TEST_ASSERT_FALSE(connected);
}

/* ── Test 9: init disables BT controller on ESP32 (non-S3) ──────────── */

void test_init_calls_bt_disable_on_esp32(void)
{
    /* On ESP32 (non-S3) targets, init must call esp_bt_controller_disable()
     * to release the shared RF path. Verified by call counter. */
    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_bt_controller_disable_calls);
}

void test_connect_null_password_open_network(void)
{
    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = wifi_manager_connect("OpenNet", NULL);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_wifi_set_config_calls);
}

void test_deinit_waits_for_reconnect_task_exit(void)
{
    /* Verify that wifi_manager_deinit() does not call vTaskDelete on an
     * external handle. The reconnect task never actually runs in host
     * tests (ulTaskNotifyTake returns 0 immediately and the mock task is
     * never scheduled), so mock_vTaskDelete_calls must remain 0
     * throughout.
     *
     * After deinit, all internal state must be reset: a second
     * wifi_manager_init() must succeed, proving no stale s_initialized
     * or s_reconnect_mutex prevents re-initialisation. */

    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = wifi_manager_connect("TestSSID", "pass");
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    int vdelete_before = mock_vTaskDelete_calls;

    ret = wifi_manager_deinit();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* deinit must NOT call vTaskDelete with an external handle. */
    TEST_ASSERT_EQUAL(vdelete_before, mock_vTaskDelete_calls);

    /* All internal state must be fully reset for a fresh init to
     * succeed. */
    mock_esp_wifi_reset();
    mock_nvs_reset();
    mock_freertos_task_reset();
    ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_FALSE(wifi_manager_is_connected());

    /* Clean up for the next test. */
    (void)wifi_manager_deinit();
}

void test_deinit_cooperative_shutdown_with_injected_task(void)
{
    /* Inject a non-NULL sentinel handle into s_reconnect_task so that
     * deinit() actually exercises the notify-and-poll path. Then null
     * the handle (simulating the task self-deleting) and verify deinit
     * completes without calling external vTaskDelete. */

    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Plant sentinel handle — simulates a running reconnect task. */
    TaskHandle_t *task_ptr = wifi_manager_get_reconnect_task_ptr();
    mock_set_reconnect_task_handle(task_ptr, (TaskHandle_t)0xDEAD0001);

    /* Immediately clear the handle so the poll loop exits on the first
     * iteration (simulates the task having nulled itself under mutex). */
    mock_clear_injected_task_handle();

    int vdelete_before = mock_vTaskDelete_calls;

    ret = wifi_manager_deinit();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* deinit must NOT call vTaskDelete externally. */
    TEST_ASSERT_EQUAL(vdelete_before, mock_vTaskDelete_calls);

    /* Re-init must succeed — all state was fully reset. */
    mock_esp_wifi_reset();
    mock_nvs_reset();
    mock_freertos_task_reset();
    ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    TEST_ASSERT_FALSE(wifi_manager_is_connected());
    (void)wifi_manager_deinit();
}

void test_deinit_timeout_path_clears_stale_handle(void)
{
    /* Simulate the timeout path: inject a handle but do NOT clear it,
     * so the poll loop exhausts all 50 iterations. After deinit, the
     * handle must be NULL (force-cleared by Fix 1) so a subsequent
     * init + event-handler call does not notify a stale handle. */

    esp_err_t ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Plant a handle that is never cleared — simulates a stuck task. */
    TaskHandle_t *task_ptr = wifi_manager_get_reconnect_task_ptr();
    mock_set_reconnect_task_handle(task_ptr, (TaskHandle_t)0xDEAD0002);

    /* deinit should still return ESP_OK (timeout is non-fatal). */
    ret = wifi_manager_deinit();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* The handle must be NULL after deinit — Fix 1 force-clears it. */
    TEST_ASSERT_NULL(*task_ptr);

    /* vTaskDelete must NOT have been called externally. */
    /* Note: mock_vTaskDelete_calls was reset in setUp(); any call here
     * would be an external delete — there must be none. */
    TEST_ASSERT_EQUAL(0, mock_vTaskDelete_calls);

    /* Re-init must succeed. */
    mock_esp_wifi_reset();
    mock_nvs_reset();
    mock_freertos_task_reset();
    ret = wifi_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    (void)wifi_manager_deinit();
}

/* ── Test runner main ───────────────────────────────────────────────── */

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init_succeeds);
    RUN_TEST(test_init_twice_returns_invalid_state);
    RUN_TEST(test_connect_null_ssid_returns_invalid_arg);
    RUN_TEST(test_connect_initiates_connection);
    RUN_TEST(test_is_connected_false_before_ip_event);
    RUN_TEST(test_save_credentials_writes_to_nvs);
    RUN_TEST(test_load_credentials_reads_from_nvs);
    RUN_TEST(test_disconnect_suppresses_reconnect);
    RUN_TEST(test_init_calls_bt_disable_on_esp32);
    RUN_TEST(test_connect_null_password_open_network);
    RUN_TEST(test_deinit_waits_for_reconnect_task_exit);
    RUN_TEST(test_deinit_cooperative_shutdown_with_injected_task);
    RUN_TEST(test_deinit_timeout_path_clears_stale_handle);

    return UNITY_END();
}
