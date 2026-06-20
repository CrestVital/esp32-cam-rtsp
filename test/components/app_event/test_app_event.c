#include "unity.h"
#include "app_event.h"
#include <string.h>

/* Externs from the mock layer — needed to inject return values and
 * inspect captured arguments. */
extern esp_err_t mock_event_loop_create_ret;
extern esp_err_t mock_event_loop_delete_ret;
extern esp_err_t mock_event_post_to_ret;
extern esp_err_t mock_event_handler_register_ret;
extern esp_err_t mock_event_handler_unregister_ret;

extern int mock_event_loop_create_calls;
extern int mock_event_loop_delete_calls;
extern int mock_event_post_to_calls;
extern int mock_event_handler_register_calls;
extern int mock_event_handler_unregister_calls;

extern esp_event_loop_args_t mock_last_loop_args;
extern esp_event_base_t mock_last_post_base;
extern int32_t mock_last_post_id;

void setUp(void)
{
    /* Every test starts with mocks in their default-success state and
     * the application event loop torn down. This guarantees test
     * isolation regardless of what the previous test did. */
    mock_esp_event_reset();
    app_event_loop_deinit();
}

void tearDown(void)
{
    /* No-op: all cleanup is done in setUp() for the next test. */
}

/* -----------------------------------------------------------------------
 * Test 1: init succeeds with correct loop parameters
 * ----------------------------------------------------------------------- */

void test_init_succeeds(void)
{
    esp_err_t ret = app_event_loop_init();

    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_event_loop_create_calls);
    TEST_ASSERT_EQUAL(APP_EVENT_QUEUE_SIZE, mock_last_loop_args.queue_size);
    TEST_ASSERT_EQUAL(APP_EVENT_TASK_PRIORITY, (int)mock_last_loop_args.task_priority);
    TEST_ASSERT_EQUAL(APP_EVENT_TASK_STACK_SIZE, (int)mock_last_loop_args.task_stack_size);
}

/* -----------------------------------------------------------------------
 * Test 2: double init returns ESP_ERR_INVALID_STATE
 * ----------------------------------------------------------------------- */

void test_init_twice_returns_invalid_state(void)
{
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_event_loop_create_calls);

    /* Second call must be rejected without forwarding to
     * esp_event_loop_create. */
    ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
    TEST_ASSERT_EQUAL(1, mock_event_loop_create_calls);
}

/* -----------------------------------------------------------------------
 * Test 3: deinit without init returns ESP_ERR_INVALID_STATE
 * ----------------------------------------------------------------------- */

void test_deinit_without_init_returns_invalid_state(void)
{
    /* Capture the call count after setUp() already ran its own
     * deinit; verify that no additional delete happened when this
     * test calls deinit with s_app_loop already NULL. */
    int calls_before = mock_event_loop_delete_calls;

    esp_err_t ret = app_event_loop_deinit();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
    TEST_ASSERT_EQUAL(calls_before, mock_event_loop_delete_calls);
}

/* -----------------------------------------------------------------------
 * Test 4: init then deinit succeeds
 * ----------------------------------------------------------------------- */

void test_deinit_after_init_succeeds(void)
{
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = app_event_loop_deinit();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_event_loop_delete_calls);
}

/* -----------------------------------------------------------------------
 * Test 5: post without init returns ESP_ERR_INVALID_STATE
 * ----------------------------------------------------------------------- */

void test_post_without_init_returns_invalid_state(void)
{
    esp_err_t ret = app_event_post(APP_EVENT_WIFI_CONNECTED, NULL, 0);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
    TEST_ASSERT_EQUAL(0, mock_event_post_to_calls);
}

/* -----------------------------------------------------------------------
 * Test 6: post propagates esp_event error
 * ----------------------------------------------------------------------- */

void test_post_propagates_esp_error(void)
{
    mock_event_post_to_ret = ESP_FAIL;

    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = app_event_post(APP_EVENT_CAMERA_READY, NULL, 0);
    TEST_ASSERT_EQUAL(ESP_FAIL, ret);
}

/* -----------------------------------------------------------------------
 * Test 7: post sends the correct event ID
 * ----------------------------------------------------------------------- */

void test_post_sends_correct_event_id(void)
{
    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Post a non-zero event to distinguish a real capture from the
     * zeroed initial state of mock_last_post_id. */
    ret = app_event_post(APP_EVENT_CAMERA_READY, NULL, 0);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
    TEST_ASSERT_EQUAL(1, mock_event_post_to_calls);
    TEST_ASSERT_EQUAL((int32_t)APP_EVENT_CAMERA_READY, mock_last_post_id);
    TEST_ASSERT_EQUAL_PTR(APP_EVENT_BASE, mock_last_post_base);
}

/* -----------------------------------------------------------------------
 * Test 8: register without init returns ESP_ERR_INVALID_STATE
 * ----------------------------------------------------------------------- */

static void dummy_handler(void *arg, esp_event_base_t base, int32_t id,
                          void *data)
{
    (void)arg;
    (void)base;
    (void)id;
    (void)data;
}

void test_register_without_init_returns_invalid_state(void)
{
    esp_err_t ret = app_event_handler_register(APP_EVENT_CAMERA_READY,
                                               dummy_handler, NULL);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
    TEST_ASSERT_EQUAL(0, mock_event_handler_register_calls);
}

/* -----------------------------------------------------------------------
 * Test 9: register propagates esp_event error
 * ----------------------------------------------------------------------- */

void test_register_propagates_esp_error(void)
{
    mock_event_handler_register_ret = ESP_ERR_NO_MEM;

    esp_err_t ret = app_event_loop_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = app_event_handler_register(APP_EVENT_OTA_STARTED, dummy_handler,
                                     NULL);
    TEST_ASSERT_EQUAL(ESP_ERR_NO_MEM, ret);
}

/* -----------------------------------------------------------------------
 * Test 10: unregister without init returns ESP_ERR_INVALID_STATE
 * ----------------------------------------------------------------------- */

void test_unregister_without_init_returns_invalid_state(void)
{
    esp_err_t ret = app_event_handler_unregister(APP_EVENT_RTSP_CLIENT_CONNECTED,
                                                 dummy_handler);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
    TEST_ASSERT_EQUAL(0, mock_event_handler_unregister_calls);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init_succeeds);
    RUN_TEST(test_init_twice_returns_invalid_state);
    RUN_TEST(test_deinit_without_init_returns_invalid_state);
    RUN_TEST(test_deinit_after_init_succeeds);
    RUN_TEST(test_post_without_init_returns_invalid_state);
    RUN_TEST(test_post_propagates_esp_error);
    RUN_TEST(test_post_sends_correct_event_id);
    RUN_TEST(test_register_without_init_returns_invalid_state);
    RUN_TEST(test_register_propagates_esp_error);
    RUN_TEST(test_unregister_without_init_returns_invalid_state);

    return UNITY_END();
}