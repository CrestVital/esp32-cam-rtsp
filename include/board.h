#pragma once

/*
 * board.h -- board capability flags and camera pin assignments.
 *
 * This file is the single source of truth for all board-specific constants.
 * It derives every macro from the Kconfig choice CONFIG_BOARD_* set in
 * components/board_config/Kconfig.projbuild.
 *
 * For host-side unit tests, test/mocks/sdkconfig.h provides the
 * CONFIG_BOARD_* values via -D flags passed by test/Makefile.
 */

#include "sdkconfig.h"

#if CONFIG_BOARD_LILYGO_T_DISPLAY_S3

#define BOARD_HAS_WIFI          1
#define BOARD_HAS_ETHERNET      0
#define BOARD_HAS_DISPLAY       1
#define BOARD_PSRAM_SIZE_MB     8

#define BOARD_SENSOR_OV5640     1
#define BOARD_SENSOR_OV2640     0

#define BOARD_CAM_PIN_PWDN      (-1)
#define BOARD_CAM_PIN_RESET     (-1)
#define BOARD_CAM_PIN_XCLK      2
#define BOARD_CAM_PIN_SIOD      3
#define BOARD_CAM_PIN_SIOC      1
#define BOARD_CAM_PIN_D7        46
#define BOARD_CAM_PIN_D6        45
#define BOARD_CAM_PIN_D5        10
#define BOARD_CAM_PIN_D4        11
#define BOARD_CAM_PIN_D3        12
#define BOARD_CAM_PIN_D2        13
#define BOARD_CAM_PIN_D1        42
#define BOARD_CAM_PIN_D0        41
#define BOARD_CAM_PIN_VSYNC     21
#define BOARD_CAM_PIN_HREF      38
#define BOARD_CAM_PIN_PCLK      40

#elif CONFIG_BOARD_AI_THINKER_ESP32_CAM

#define BOARD_HAS_WIFI          1
#define BOARD_HAS_ETHERNET      0
#define BOARD_HAS_DISPLAY       0
#define BOARD_PSRAM_SIZE_MB     4

#define BOARD_SENSOR_OV5640     0
#define BOARD_SENSOR_OV2640     1

#define BOARD_CAM_PIN_PWDN      32
#define BOARD_CAM_PIN_RESET     (-1)
#define BOARD_CAM_PIN_XCLK      0
#define BOARD_CAM_PIN_SIOD      26
#define BOARD_CAM_PIN_SIOC      27
#define BOARD_CAM_PIN_D7        35
#define BOARD_CAM_PIN_D6        34
#define BOARD_CAM_PIN_D5        39
#define BOARD_CAM_PIN_D4        36
#define BOARD_CAM_PIN_D3        21
#define BOARD_CAM_PIN_D2        19
#define BOARD_CAM_PIN_D1        18
#define BOARD_CAM_PIN_D0        5
#define BOARD_CAM_PIN_VSYNC     25
#define BOARD_CAM_PIN_HREF      23
#define BOARD_CAM_PIN_PCLK      22

#elif CONFIG_BOARD_OLIMEX_ESP32_POE

#define BOARD_HAS_WIFI          0
#define BOARD_HAS_ETHERNET      1
#define BOARD_HAS_DISPLAY       0
#define BOARD_PSRAM_SIZE_MB     0

#define BOARD_SENSOR_OV5640     0
#define BOARD_SENSOR_OV2640     1

#define BOARD_CAM_PIN_PWDN      (-1)
#define BOARD_CAM_PIN_RESET     (-1)
#define BOARD_CAM_PIN_XCLK      4
#define BOARD_CAM_PIN_SIOD      18
#define BOARD_CAM_PIN_SIOC      23
#define BOARD_CAM_PIN_D7        36
#define BOARD_CAM_PIN_D6        37
#define BOARD_CAM_PIN_D5        38
#define BOARD_CAM_PIN_D4        39
#define BOARD_CAM_PIN_D3        35
#define BOARD_CAM_PIN_D2        26
#define BOARD_CAM_PIN_D1        13
#define BOARD_CAM_PIN_D0        34
#define BOARD_CAM_PIN_VSYNC     5
#define BOARD_CAM_PIN_HREF      27
#define BOARD_CAM_PIN_PCLK      25

#else
#error "No board selected. Set CONFIG_BOARD_* in sdkconfig.defaults.<board> or via Kconfig."
#endif