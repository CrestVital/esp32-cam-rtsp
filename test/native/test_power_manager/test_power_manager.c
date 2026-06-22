/* PlatformIO native test suite copy — canonical source is
 * test/components/power_manager/test_power_manager.c
 * Keep in sync with the canonical source manually. */

#include "unity.h"
#include "power_manager.h"
#include "app_event.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_log.h"
#include <string.h>

/* Mock injectables from app_event layer */
extern esp_err_t mock_event_loop_create_ret;
extern esp_err_t mock_event_loop_delete_ret;
extern esp_err_t mock_event_post_to_ret;
extern esp_err_t mock_event_handler_register_ret;

extern int mock_event_loop_create_calls;
extern int mock_event_loop_delete_calls;
extern int mock_event_post_to_calls;
extern int mock_event_handler_register_calls;

extern int32_t mock_last_post_id;

/* Mock injectables from power helper layer */
extern esp_err_t mock_esp_task_wdt_init_ret;
extern esp_err_t mock_esp_task_wdt_add_ret;
extern esp_err_t mock_esp_task_wdt_delete_ret;

extern int mock_esp_task_wdt_init_calls;
extern int mock_esp_task_wdt_add_calls;
extern int mock_esp_task_wdt_delete_calls;
extern int mock_esp_task_wdt_reset_calls;
extern int mock_esp_task_wdt_deinit_calls;

extern esp_task_wdt_config_t mock_last_twdt_config;

extern esp_reset_reason_t mock_esp_reset_reason_ret;

/* Log counters from mock_esp_log layer */
extern int mock_esp_log_warning_count;
extern int mock_esp_log_info_count;

void setUp(void)
{
    /* Reset every mock layer to its default-OK state, then tear down
     * the event loop and power manager so each test starts from a
     * pristine, uninitialised environment. */
    mock_esp_event_reset();
    mock_esp_task_wdt_reset();
    mock_esp_system_reset();
    mock_esp_log_reset();
    app_event_loop_deinit();
    power_manager_deinit();
}

void tearDown(void)
{
    /* Intentionally empty — all cleanup is done in setUp() for the
     * next test to guarantee isolation regardless of test outcome. */
}

/* -----------------------------------------------------------------------
 * Test 1: power_manager_init() succeeds and returns ESP_OK
 * ----------------------------------------------------------------------- */

void test_init_succeeds(void)
{
    /* The event loop must exist before power_manager_init() can register
     * the shutdown handler. */
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Verify TWDT was configured with the correct parameters. */
    TEST_ASSERT_EQUAL(1, mock_esp_task_wdt_init_calls);
    TEST_ASSERT_EQUAL(30000, (int)mock_last_twdt_config.timeout_ms);
    TEST_ASSERT_EQUAL(3, (int)mock_last_twdt_config.idle_core_mask);
    TEST_ASSERT_EQUAL(1, mock_last_twdt_config.trigger_panic);

    /* Verify the shutdown handler was registered. */
    TEST_ASSERT_EQUAL(1, mock_event_handler_register_calls);
}

/* -----------------------------------------------------------------------
 * Test 2: double power_manager_init() returns ESP_ERR_INVALID_STATE
 * ----------------------------------------------------------------------- */

void test_init_twice_returns_invalid_state(void)
{
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* First call must succeed. */
    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_task_wdt_init_calls);

    /* Second call must be rejected without re-initialising the TWDT. */
    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_task_wdt_init_calls);
}

/* -----------------------------------------------------------------------
 * Test 3: power_manager_wdt_subscribe(NULL, "test") returns ESP_OK
 * ----------------------------------------------------------------------- */

void test_subscribe_null_returns_ok(void)
{
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Subscribe the calling task (NULL handle). */
    ret = power_manager_wdt_subscribe(NULL, "test");
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_task_wdt_add_calls);
}

/* -----------------------------------------------------------------------
 * Test 4: power_manager_wdt_unsubscribe(NULL) returns ESP_OK
 * ----------------------------------------------------------------------- */

void test_unsubscribe_null_returns_ok(void)
{
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = power_manager_wdt_unsubscribe(NULL);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_esp_task_wdt_delete_calls);
}

/* -----------------------------------------------------------------------
 * Test 5: power_manager_wdt_reset() does not crash
 * ----------------------------------------------------------------------- */

void test_reset_does_not_crash(void)
{
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* The reset function must be safe to call and must delegate to the
     * underlying esp_task_wdt_reset(). */
    power_manager_wdt_reset();
    TEST_ASSERT_EQUAL(1, mock_esp_task_wdt_reset_calls);
}

/* -----------------------------------------------------------------------
 * Test 6: power_manager_shutdown() posts the event successfully
 * ----------------------------------------------------------------------- */

void test_shutdown_posts_event(void)
{
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Trigger shutdown. The event is posted asynchronously; the
     * handler is not actually invoked in the mock environment. */
    ret = power_manager_shutdown();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_event_post_to_calls);
    TEST_ASSERT_EQUAL((int32_t)APP_EVENT_SHUTDOWN, mock_last_post_id);
}

/* -----------------------------------------------------------------------
 * Test 7: ESP_RST_TASK_WDT reset reason triggers ESP_LOGW path
 * ----------------------------------------------------------------------- */

void test_reset_reason_task_wdt_logs_warning(void)
{
    /* Inject a TWDT reset reason. The init function must log this at
     * WARNING level because it indicates a preceding watchdog timeout. */
    mock_esp_reset_reason_ret = ESP_RST_TASK_WDT;

    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Capture log counters before init to isolate the init's log calls. */
    int warn_before = mock_esp_log_warning_count;

    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* The TWDT reset reason must increment the warning counter. */
    TEST_ASSERT(mock_esp_log_warning_count > warn_before);
}

/* -----------------------------------------------------------------------
 * Test 8: ESP_RST_POWERON reset reason triggers ESP_LOGI path
 * ----------------------------------------------------------------------- */

void test_reset_reason_poweron_logs_info(void)
{
    /* Populate a known state with a non-POWERON reason first, then
     * verify that POWERON is logged at INFO level. */

    /* Inject a power-on reset reason. */
    mock_esp_reset_reason_ret = ESP_RST_POWERON;

    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    int info_before = mock_esp_log_info_count;

    ret = power_manager_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* The power-on reason must increment the info counter. */
    TEST_ASSERT(mock_esp_log_info_count > info_before);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init_succeeds);
    RUN_TEST(test_init_twice_returns_invalid_state);
    RUN_TEST(test_subscribe_null_returns_ok);
    RUN_TEST(test_unsubscribe_null_returns_ok);
    RUN_TEST(test_reset_does_not_crash);
    RUN_TEST(test_shutdown_posts_event);
    RUN_TEST(test_reset_reason_task_wdt_logs_warning);
    RUN_TEST(test_reset_reason_poweron_logs_info);

    return UNITY_END();
}
