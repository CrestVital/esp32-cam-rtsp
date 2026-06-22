#ifndef INCLUDE_BOARD_H
#define INCLUDE_BOARD_H

/* board.h — common board abstraction layer.
 *
 * Includes the per-board header selected at compile time via the
 * BOARD_HEADER preprocessor macro, then validates at compile time that
 * every mandatory macro is defined.
 *
 * Each PlatformIO environment passes -DBOARD_HEADER="boards/<name>.h"
 * in its build_flags.  This file is included by any component that
 * needs board-specific pin or capability information.
 *
 * Usage in source files:
 *   #include "board.h"
 */

#ifdef BOARD_HEADER
#include BOARD_HEADER
#else
#error "BOARD_HEADER is not defined. Pass -DBOARD_HEADER=\"boards/<name>.h\" in build_flags."
#endif

/* ── Compile-time validation ─────────────────────────────────────────── */

#ifndef BOARD_CAM_PIN_D0
#error "BOARD_HEADER must define BOARD_CAM_PIN_D0"
#endif
#ifndef BOARD_CAM_PIN_D1
#error "BOARD_HEADER must define BOARD_CAM_PIN_D1"
#endif
#ifndef BOARD_CAM_PIN_D2
#error "BOARD_HEADER must define BOARD_CAM_PIN_D2"
#endif
#ifndef BOARD_CAM_PIN_D3
#error "BOARD_HEADER must define BOARD_CAM_PIN_D3"
#endif
#ifndef BOARD_CAM_PIN_D4
#error "BOARD_HEADER must define BOARD_CAM_PIN_D4"
#endif
#ifndef BOARD_CAM_PIN_D5
#error "BOARD_HEADER must define BOARD_CAM_PIN_D5"
#endif
#ifndef BOARD_CAM_PIN_D6
#error "BOARD_HEADER must define BOARD_CAM_PIN_D6"
#endif
#ifndef BOARD_CAM_PIN_D7
#error "BOARD_HEADER must define BOARD_CAM_PIN_D7"
#endif
#ifndef BOARD_CAM_PIN_XCLK
#error "BOARD_HEADER must define BOARD_CAM_PIN_XCLK"
#endif
#ifndef BOARD_CAM_PIN_PCLK
#error "BOARD_HEADER must define BOARD_CAM_PIN_PCLK"
#endif
#ifndef BOARD_CAM_PIN_VSYNC
#error "BOARD_HEADER must define BOARD_CAM_PIN_VSYNC"
#endif
#ifndef BOARD_CAM_PIN_HREF
#error "BOARD_HEADER must define BOARD_CAM_PIN_HREF"
#endif
#ifndef BOARD_CAM_PIN_SIOD
#error "BOARD_HEADER must define BOARD_CAM_PIN_SIOD"
#endif
#ifndef BOARD_CAM_PIN_SIOC
#error "BOARD_HEADER must define BOARD_CAM_PIN_SIOC"
#endif
#ifndef BOARD_CAM_PIN_PWDN
#error "BOARD_HEADER must define BOARD_CAM_PIN_PWDN"
#endif
#ifndef BOARD_CAM_PIN_RESET
#error "BOARD_HEADER must define BOARD_CAM_PIN_RESET"
#endif
#ifndef BOARD_HAS_WIFI
#error "BOARD_HEADER must define BOARD_HAS_WIFI"
#endif
#ifndef BOARD_HAS_ETHERNET
#error "BOARD_HEADER must define BOARD_HAS_ETHERNET"
#endif
#ifndef BOARD_PSRAM_SIZE_MB
#error "BOARD_HEADER must define BOARD_PSRAM_SIZE_MB"
#endif
#if !defined(BOARD_SENSOR_OV2640) && !defined(BOARD_SENSOR_OV5640)
#error "BOARD_HEADER must define BOARD_SENSOR_OV2640 or BOARD_SENSOR_OV5640"
#endif
#if defined(BOARD_SENSOR_OV2640) && defined(BOARD_SENSOR_OV5640)
#error "BOARD_HEADER must define exactly one sensor: BOARD_SENSOR_OV2640 or BOARD_SENSOR_OV5640, not both"
#endif

#endif /* INCLUDE_BOARD_H */