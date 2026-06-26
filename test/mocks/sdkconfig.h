#pragma once

/* Minimal sdkconfig.h stub for host-side unit tests.
 *
 * Simulates ESP32 (non-S3) target so that the BT-disable guard in
 * wifi_manager.c is compiled and exercised. Real build-time values
 * (CONFIG_WIFI_MANAGER_ENABLED etc.) are provided via -D flags in
 * test/Makefile and take precedence over any defines here. */

/* Target selection — simulate ESP32 classic (AI Thinker board class). */
#define CONFIG_IDF_TARGET_ESP32        1
#define CONFIG_IDF_TARGET_ESP32S3      0

/* Bluetooth controller available on ESP32 classic. */
#define CONFIG_BT_CONTROLLER_ENABLED   1

/* Board selection for host-side tests: simulate AI Thinker ESP32-CAM. */
#define CONFIG_BOARD_AI_THINKER_ESP32_CAM  1
