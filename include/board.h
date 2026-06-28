#pragma once

/*
 * board.h — Data-driven board dispatcher.
 *
 * Includes the board data file selected by Kconfig (CONFIG_BOARD_DATA_FILE),
 * then performs compile-time completeness validation.
 *
 * To add a new board:
 *   1. Create boards/<new_board>.h with all required #define macros.
 *   2. Add a config entry in components/board_config/Kconfig.projbuild.
 *   3. Add an env and sdkconfig fragment in platformio.ini.
 *   No changes to this file or any .c file are required.
 *
 * For host-side unit tests, test/mocks/sdkconfig.h provides
 * CONFIG_BOARD_DATA_FILE via the -DCONFIG_BOARD_DATA_FILE=... flag
 * passed by test/Makefile.
 */

#include "sdkconfig.h"

#ifndef CONFIG_BOARD_DATA_FILE
#error "CONFIG_BOARD_DATA_FILE not set. Select a board in Kconfig (Board Selection menu)."
#endif

#include CONFIG_BOARD_DATA_FILE

/* ---- Compile-time completeness validation -------------------------------- */

#ifndef BOARD_HAS_WIFI
#error "Board data file missing: BOARD_HAS_WIFI"
#endif

#ifndef BOARD_HAS_ETHERNET
#error "Board data file missing: BOARD_HAS_ETHERNET"
#endif

#ifndef BOARD_HAS_DISPLAY
#error "Board data file missing: BOARD_HAS_DISPLAY"
#endif

#ifndef BOARD_PSRAM_SIZE_MB
#error "Board data file missing: BOARD_PSRAM_SIZE_MB"
#endif

#ifndef BOARD_SENSOR_OV2640
#error "Board data file missing: BOARD_SENSOR_OV2640"
#endif

#ifndef BOARD_SENSOR_OV5640
#error "Board data file missing: BOARD_SENSOR_OV5640"
#endif

#if !BOARD_SENSOR_OV2640 && !BOARD_SENSOR_OV5640
#error "Board data file must define exactly one sensor: BOARD_SENSOR_OV2640=1 or BOARD_SENSOR_OV5640=1"
#endif

#if BOARD_SENSOR_OV2640 && BOARD_SENSOR_OV5640
#error "Board data file must not set both BOARD_SENSOR_OV2640=1 and BOARD_SENSOR_OV5640=1"
#endif

#ifndef BOARD_CAM_PIN_PWDN
#error "Board data file missing: BOARD_CAM_PIN_PWDN"
#endif

#ifndef BOARD_CAM_PIN_RESET
#error "Board data file missing: BOARD_CAM_PIN_RESET"
#endif

#ifndef BOARD_CAM_PIN_XCLK
#error "Board data file missing: BOARD_CAM_PIN_XCLK"
#endif

#ifndef BOARD_CAM_PIN_SIOD
#error "Board data file missing: BOARD_CAM_PIN_SIOD"
#endif

#ifndef BOARD_CAM_PIN_SIOC
#error "Board data file missing: BOARD_CAM_PIN_SIOC"
#endif

#ifndef BOARD_CAM_PIN_D7
#error "Board data file missing: BOARD_CAM_PIN_D7"
#endif

#ifndef BOARD_CAM_PIN_D6
#error "Board data file missing: BOARD_CAM_PIN_D6"
#endif

#ifndef BOARD_CAM_PIN_D5
#error "Board data file missing: BOARD_CAM_PIN_D5"
#endif

#ifndef BOARD_CAM_PIN_D4
#error "Board data file missing: BOARD_CAM_PIN_D4"
#endif

#ifndef BOARD_CAM_PIN_D3
#error "Board data file missing: BOARD_CAM_PIN_D3"
#endif

#ifndef BOARD_CAM_PIN_D2
#error "Board data file missing: BOARD_CAM_PIN_D2"
#endif

#ifndef BOARD_CAM_PIN_D1
#error "Board data file missing: BOARD_CAM_PIN_D1"
#endif

#ifndef BOARD_CAM_PIN_D0
#error "Board data file missing: BOARD_CAM_PIN_D0"
#endif

#ifndef BOARD_CAM_PIN_VSYNC
#error "Board data file missing: BOARD_CAM_PIN_VSYNC"
#endif

#ifndef BOARD_CAM_PIN_HREF
#error "Board data file missing: BOARD_CAM_PIN_HREF"
#endif

#ifndef BOARD_CAM_PIN_PCLK
#error "Board data file missing: BOARD_CAM_PIN_PCLK"
#endif