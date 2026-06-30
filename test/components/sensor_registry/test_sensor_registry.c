/*
 * Test suite for the sensor registry (include/sensor_caps.h).
 *
 * This is a compile-only verification: the sensor data file is included
 * indirectly through the dispatcher, and we assert that the key macros
 * have their expected values. If the dispatch itself fails (missing file,
 * missing macro, interface mismatch), the compilation stops with #error
 * before any test runs.
 */

#include "sensor_caps.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_sensor_iface_is_dvp(void)
{
    TEST_ASSERT_EQUAL(SENSOR_IFACE_DVP, SENSOR_IFACE);
}

void test_sensor_has_driver(void)
{
    TEST_ASSERT_EQUAL(1, SENSOR_HAS_DRIVER);
}

void test_sensor_name_matches_spec(void)
{
    /* Active host-build sensor is OV2640 (see test/mocks/sdkconfig.h). Pin the
     * exact name so an accidental edit to the data file fails the suite. */
    TEST_ASSERT_EQUAL_STRING("OV2640", SENSOR_NAME);
}

void test_sensor_vendor_matches_spec(void)
{
    TEST_ASSERT_EQUAL_STRING("OmniVision", SENSOR_VENDOR);
}

void test_sensor_width_matches_spec(void)
{
    TEST_ASSERT_EQUAL(1600, SENSOR_MAX_WIDTH);
}

void test_sensor_height_matches_spec(void)
{
    TEST_ASSERT_EQUAL(1200, SENSOR_MAX_HEIGHT);
}

void test_sensor_fps_matches_spec(void)
{
    TEST_ASSERT_EQUAL(60, SENSOR_MAX_FPS);
}

void test_sensor_not_requires_isp(void)
{
    TEST_ASSERT_EQUAL(0, SENSOR_REQUIRES_ISP);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_sensor_iface_is_dvp);
    RUN_TEST(test_sensor_has_driver);
    RUN_TEST(test_sensor_name_matches_spec);
    RUN_TEST(test_sensor_vendor_matches_spec);
    RUN_TEST(test_sensor_width_matches_spec);
    RUN_TEST(test_sensor_height_matches_spec);
    RUN_TEST(test_sensor_fps_matches_spec);
    RUN_TEST(test_sensor_not_requires_isp);

    return UNITY_END();
}
