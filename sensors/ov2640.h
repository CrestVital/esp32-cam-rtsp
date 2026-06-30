#pragma once

/*
 * sensors/ov2640.h — Sensor data for OV2640
 *
 * OmniVision OV2640: 2-megapixel CMOS sensor with embedded JPEG encoder.
 * Uses parallel DVP (8-bit) camera interface. Supported on all current
 * ESP32 boards.
 *
 * Sensor registry (include/sensor_caps.h) validates this data at compile
 * time. Real capture driver implementation: epic ESPCAMFW-2.
 */

#define SENSOR_NAME            "OV2640"
#define SENSOR_VENDOR          "OmniVision"
#define SENSOR_IFACE           SENSOR_IFACE_DVP
#define SENSOR_REQUIRES_ISP    0
#define SENSOR_MAX_WIDTH       1600
#define SENSOR_MAX_HEIGHT      1200
#define SENSOR_MAX_FPS         60
#define SENSOR_HAS_DRIVER      1
