#pragma once

/*
 * sensor_caps.h — Data-driven sensor dispatcher.
 *
 * Includes the sensor data file selected by Kconfig (CONFIG_SENSOR_DATA_FILE),
 * then performs compile-time completeness and interface-safety validation.
 *
 * To add a new sensor:
 *   1. Create sensors/<new_sensor>.h with all required #define macros.
 *   2. Add entries in components/sensor_registry/Kconfig.projbuild.
 *   3. Add sensor selection to each board's sdkconfig.defaults.<board> fragment.
 *   No changes to this file or any .c file are required.
 *
 * For host-side unit tests, test/mocks/sdkconfig.h provides
 * CONFIG_SENSOR_DATA_FILE.
 */

/*
 * Interface enumeration. Uses preprocessor #define (not C enum) so that
 * values are usable in #if comparisons within this header and in callers.
 */
#define SENSOR_IFACE_DVP        1
#define SENSOR_IFACE_MIPI_CSI2  2

#include "sdkconfig.h"
#include "board.h"

#ifndef CONFIG_SENSOR_DATA_FILE
#error "CONFIG_SENSOR_DATA_FILE not set. Select a sensor in Kconfig (Sensor Selection menu)."
#endif

#include CONFIG_SENSOR_DATA_FILE

/* ---- Compile-time completeness validation -------------------------------- */

#ifndef SENSOR_NAME
#error "Sensor data file missing: SENSOR_NAME"
#endif

#ifndef SENSOR_VENDOR
#error "Sensor data file missing: SENSOR_VENDOR"
#endif

#ifndef SENSOR_IFACE
#error "Sensor data file missing: SENSOR_IFACE"
#endif

#ifndef SENSOR_REQUIRES_ISP
#error "Sensor data file missing: SENSOR_REQUIRES_ISP"
#endif

#ifndef SENSOR_MAX_WIDTH
#error "Sensor data file missing: SENSOR_MAX_WIDTH"
#endif

#ifndef SENSOR_MAX_HEIGHT
#error "Sensor data file missing: SENSOR_MAX_HEIGHT"
#endif

#ifndef SENSOR_MAX_FPS
#error "Sensor data file missing: SENSOR_MAX_FPS"
#endif

#ifndef SENSOR_HAS_DRIVER
#error "Sensor data file missing: SENSOR_HAS_DRIVER"
#endif

/* ---- Interface validity check -------------------------------------------- */

#if (SENSOR_IFACE != SENSOR_IFACE_DVP) && (SENSOR_IFACE != SENSOR_IFACE_MIPI_CSI2)
#error "SENSOR_IFACE must be SENSOR_IFACE_DVP or SENSOR_IFACE_MIPI_CSI2"
#endif

/* ---- Interface safety check: guard against MIPI/ISP sensors on non-MIPI boards */

#if (SENSOR_IFACE == SENSOR_IFACE_MIPI_CSI2) && (BOARD_HAS_MIPI_CSI == 0)
#error "Active sensor uses MIPI CSI-2 but the selected board has no MIPI CSI-2 interface. MIPI/ISP support is ESP32-P4 only — see epic ESPCAMFW-49 and ADR-007."
#endif

#if (SENSOR_REQUIRES_ISP == 1) && (BOARD_HAS_ISP == 0)
#error "Active sensor requires an ISP but the selected board has no ISP. MIPI/ISP support is ESP32-P4 only — see epic ESPCAMFW-49 and ADR-007."
#endif
