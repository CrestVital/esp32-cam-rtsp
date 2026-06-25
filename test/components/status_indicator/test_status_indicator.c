#include "unity.h"
#include "status_indicator.h"

extern void mock_esp_log_reset(void);

void setUp(void)
{
    /* Every test starts with the indicator deinitialised and mock
     * counters reset. This guarantees test isolation regardless of
     * what the previous test did. */
    mock_esp_log_reset();
    status_indicator_deinit();
}

void tearDown(void)
{
    /* No-op: all cleanup is done in setUp() for the next test. */
}

/* -----------------------------------------------------------------------
 * Test 1: init succeeds
 * ----------------------------------------------------------------------- */

void test_init_succeeds(void)
{
    esp_err_t ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

/* -----------------------------------------------------------------------
 * Test 2: double init returns ESP_ERR_INVALID_STATE
 * ----------------------------------------------------------------------- */

void test_init_twice_returns_invalid_state(void)
{
    esp_err_t ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Second init must be rejected without re-creating the mutex. */
    ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
}

/* -----------------------------------------------------------------------
 * Test 3: set_state before init returns ESP_ERR_INVALID_STATE
 * ----------------------------------------------------------------------- */

void test_set_state_before_init_returns_invalid_state(void)
{
    esp_err_t ret = status_indicator_set_state(INDICATOR_STATE_BOOT);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_STATE, ret);
}

/* -----------------------------------------------------------------------
 * Test 4: set_state with invalid arg returns ESP_ERR_INVALID_ARG
 * ----------------------------------------------------------------------- */

void test_set_state_invalid_arg(void)
{
    esp_err_t ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* INDICATOR_STATE_COUNT is the sentinel — must be rejected. */
    ret = status_indicator_set_state(INDICATOR_STATE_COUNT);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, ret);
}

/* -----------------------------------------------------------------------
 * Test 5: set_state INDICATOR_STATE_BOOT succeeds after init
 * ----------------------------------------------------------------------- */

void test_set_state_boot(void)
{
    esp_err_t ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_BOOT);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

/* -----------------------------------------------------------------------
 * Test 6: set_state with all 7 valid states returns ESP_OK
 * ----------------------------------------------------------------------- */

void test_set_state_all_valid_states(void)
{
    esp_err_t ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_BOOT);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_WIFI_CONNECTING);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_WIFI_CONNECTED);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_WIFI_ERROR);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_STREAMING);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_OTA);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_ERROR);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

/* -----------------------------------------------------------------------
 * Test 7: set same state twice is a no-op and returns ESP_OK
 * ----------------------------------------------------------------------- */

void test_set_same_state_no_op(void)
{
    esp_err_t ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_set_state(INDICATOR_STATE_STREAMING);
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Setting the same state again should succeed without side effects. */
    ret = status_indicator_set_state(INDICATOR_STATE_STREAMING);
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

/* -----------------------------------------------------------------------
 * Test 8: deinit after init succeeds, then re-init succeeds
 * ----------------------------------------------------------------------- */

void test_deinit_after_init_succeeds(void)
{
    esp_err_t ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    ret = status_indicator_deinit();
    TEST_ASSERT_EQUAL(ESP_OK, ret);

    /* Re-init after a clean deinit must succeed. */
    ret = status_indicator_init();
    TEST_ASSERT_EQUAL(ESP_OK, ret);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init_succeeds);
    RUN_TEST(test_init_twice_returns_invalid_state);
    RUN_TEST(test_set_state_before_init_returns_invalid_state);
    RUN_TEST(test_set_state_invalid_arg);
    RUN_TEST(test_set_state_boot);
    RUN_TEST(test_set_state_all_valid_states);
    RUN_TEST(test_set_same_state_no_op);
    RUN_TEST(test_deinit_after_init_succeeds);

    return UNITY_END();
}
