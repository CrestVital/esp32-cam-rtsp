#include "unity.h"
#include "nvs_config.h"
#include "nvs.h"
#include <string.h>

static app_config_t g_cfg;

void setUp(void)
{
    /* Reset the entire mock NVS store and zero the config struct before
     * every test case. Called automatically by the Unity framework —
     * this guarantees no state leaks from a previous test, even if that
     * test crashed or was aborted mid-way.
     *
     * Directly calling mock_nvs_reset() also restores the flash-init
     * injection variables (mock_nvs_flash_init_ret, etc.) to ESP_OK. */
    mock_nvs_reset();
    memset(&g_cfg, 0, sizeof(g_cfg));
}

void tearDown(void)
{
    /* Intentionally empty — cleanup is done in setUp() to ensure a
     * clean state regardless of the test outcome (pass, fail, skip). */
}

/* ── Group A: config_save() validation — invalid values rejected ──────── */

void test_save_null_returns_invalid_arg(void)
{
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(NULL));
}

void test_save_rtsp_port_zero_rejected(void)
{
    g_cfg.rtsp_port = 0;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_cam_width_below_min_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 159;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_cam_width_above_max_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1921;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_cam_height_below_min_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 119;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_cam_height_above_max_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 1081;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_cam_fps_zero_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 0;
    g_cfg.cam_brightness = 0;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_cam_fps_above_max_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 61;
    g_cfg.cam_brightness = 0;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_cam_brightness_below_min_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = -3;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_cam_brightness_above_max_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 3;
    g_cfg.mdns_name[0] = 'x';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

void test_save_empty_mdns_name_rejected(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    g_cfg.mdns_name[0] = '\0';
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_save(&g_cfg));
}

/* ── Group B: config_save() boundary values accepted ──────────────────── */

void test_save_rtsp_port_min_accepted(void)
{
    g_cfg.rtsp_port = 1;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_rtsp_port_max_accepted(void)
{
    g_cfg.rtsp_port = 65535;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_cam_width_min_accepted(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 160;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_cam_width_max_accepted(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1920;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_cam_height_min_accepted(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 120;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_cam_height_max_accepted(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 1080;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 0;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_cam_fps_min_accepted(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 1;
    g_cfg.cam_brightness = 0;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_cam_fps_max_accepted(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 60;
    g_cfg.cam_brightness = 0;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_cam_brightness_min_accepted(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = -2;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

void test_save_cam_brightness_max_accepted(void)
{
    g_cfg.rtsp_port = 554;
    g_cfg.cam_width = 1280;
    g_cfg.cam_height = 720;
    g_cfg.cam_fps = 15;
    g_cfg.cam_brightness = 2;
    strncpy(g_cfg.mdns_name, "espcam", sizeof(g_cfg.mdns_name));
    TEST_ASSERT_EQUAL(ESP_OK, config_save(&g_cfg));
}

/* ── Group C: NULL and fallback paths ─────────────────────────────────── */

void test_init_null_returns_invalid_arg(void)
{
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, config_init(NULL));
}

void test_load_nvs_not_found_applies_defaults(void)
{
    mock_nvs_set_open_ret(ESP_ERR_NVS_NOT_FOUND);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(554, g_cfg.rtsp_port);
    TEST_ASSERT_EQUAL(1280, g_cfg.cam_width);
    TEST_ASSERT_EQUAL(720, g_cfg.cam_height);
    TEST_ASSERT_EQUAL(15, g_cfg.cam_fps);
    TEST_ASSERT_EQUAL(0, g_cfg.cam_brightness);
    TEST_ASSERT_EQUAL_STRING("espcam", g_cfg.mdns_name);
}

void test_load_nvs_open_error_applies_defaults(void)
{
    mock_nvs_set_open_ret(ESP_FAIL);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(554, g_cfg.rtsp_port);
    TEST_ASSERT_EQUAL(1280, g_cfg.cam_width);
    TEST_ASSERT_EQUAL(720, g_cfg.cam_height);
    TEST_ASSERT_EQUAL(15, g_cfg.cam_fps);
    TEST_ASSERT_EQUAL(0, g_cfg.cam_brightness);
    TEST_ASSERT_EQUAL_STRING("espcam", g_cfg.mdns_name);
}

/* ── Group D: valid values read from NVS ──────────────────────────────── */

void test_load_reads_valid_rtsp_port(void)
{
    mock_nvs_set_u16("rtsp_port", 8554);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(8554, g_cfg.rtsp_port);
}

void test_load_reads_valid_cam_width(void)
{
    mock_nvs_set_u16("cam_width", 640);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(640, g_cfg.cam_width);
}

void test_load_reads_valid_cam_height(void)
{
    mock_nvs_set_u16("cam_height", 480);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(480, g_cfg.cam_height);
}

void test_load_reads_valid_cam_fps(void)
{
    mock_nvs_set_u8("cam_fps", 30);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(30, g_cfg.cam_fps);
}

void test_load_reads_valid_cam_brightness(void)
{
    mock_nvs_set_i8("cam_bright", -1);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(-1, g_cfg.cam_brightness);
}

void test_load_reads_valid_mdns_name(void)
{
    mock_nvs_set_str_val("mdns_name", "mycam");
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL_STRING("mycam", g_cfg.mdns_name);
}

/* ── Group E: invalid NVS values replaced with defaults ───────────────── */

void test_load_invalid_rtsp_port_uses_default(void)
{
    mock_nvs_set_u16("rtsp_port", 0);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(554, g_cfg.rtsp_port);
}

void test_load_invalid_cam_fps_above_max_uses_default(void)
{
    mock_nvs_set_u8("cam_fps", 100);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(15, g_cfg.cam_fps);
}

void test_load_invalid_cam_brightness_out_of_range_uses_default(void)
{
    mock_nvs_set_i8("cam_bright", 5);
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL(0, g_cfg.cam_brightness);
}

void test_load_empty_mdns_name_uses_default(void)
{
    mock_nvs_set_str_val("mdns_name", "");
    TEST_ASSERT_EQUAL(ESP_OK, config_load(&g_cfg));
    TEST_ASSERT_EQUAL_STRING("espcam", g_cfg.mdns_name);
}

int main(void)
{
    /* Unity test runner entry point.
     *
     * UNITY_BEGIN() resets internal counters and state.
     * Each RUN_TEST() macro executes one test case with setUp/tearDown.
     * UNITY_END() prints the summary and returns the failure count
     * (0 = all passed). */
    UNITY_BEGIN();
    RUN_TEST(test_save_null_returns_invalid_arg);
    RUN_TEST(test_save_rtsp_port_zero_rejected);
    RUN_TEST(test_save_cam_width_below_min_rejected);
    RUN_TEST(test_save_cam_width_above_max_rejected);
    RUN_TEST(test_save_cam_height_below_min_rejected);
    RUN_TEST(test_save_cam_height_above_max_rejected);
    RUN_TEST(test_save_cam_fps_zero_rejected);
    RUN_TEST(test_save_cam_fps_above_max_rejected);
    RUN_TEST(test_save_cam_brightness_below_min_rejected);
    RUN_TEST(test_save_cam_brightness_above_max_rejected);
    RUN_TEST(test_save_empty_mdns_name_rejected);
    RUN_TEST(test_save_rtsp_port_min_accepted);
    RUN_TEST(test_save_rtsp_port_max_accepted);
    RUN_TEST(test_save_cam_width_min_accepted);
    RUN_TEST(test_save_cam_width_max_accepted);
    RUN_TEST(test_save_cam_height_min_accepted);
    RUN_TEST(test_save_cam_height_max_accepted);
    RUN_TEST(test_save_cam_fps_min_accepted);
    RUN_TEST(test_save_cam_fps_max_accepted);
    RUN_TEST(test_save_cam_brightness_min_accepted);
    RUN_TEST(test_save_cam_brightness_max_accepted);
    RUN_TEST(test_init_null_returns_invalid_arg);
    RUN_TEST(test_load_nvs_not_found_applies_defaults);
    RUN_TEST(test_load_nvs_open_error_applies_defaults);
    RUN_TEST(test_load_reads_valid_rtsp_port);
    RUN_TEST(test_load_reads_valid_cam_width);
    RUN_TEST(test_load_reads_valid_cam_height);
    RUN_TEST(test_load_reads_valid_cam_fps);
    RUN_TEST(test_load_reads_valid_cam_brightness);
    RUN_TEST(test_load_reads_valid_mdns_name);
    RUN_TEST(test_load_invalid_rtsp_port_uses_default);
    RUN_TEST(test_load_invalid_cam_fps_above_max_uses_default);
    RUN_TEST(test_load_invalid_cam_brightness_out_of_range_uses_default);
    RUN_TEST(test_load_empty_mdns_name_uses_default);
    return UNITY_END();
}