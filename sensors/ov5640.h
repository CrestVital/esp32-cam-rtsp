#pragma once

/*
 * sensors/ov5640.h — Sensor data for OV5640
 *
 * OmniVision OV5640: 5-megapixel CMOS sensor. Uses parallel DVP (8-bit)
 * camera interface. Higher resolution than OV2640; requires more PSRAM
 * bandwidth for full-resolution capture.
 *
 * Sensor registry (include/sensor_caps.h) validates this data at compile
 * time. Real capture driver implementation: epic ESPCAMFW-2.
 */

#define SENSOR_NAME            "OV5640"
#define SENSOR_VENDOR          "OmniVision"
#define SENSOR_IFACE           SENSOR_IFACE_DVP
#define SENSOR_REQUIRES_ISP    0
#define SENSOR_MAX_WIDTH       2592
#define SENSOR_MAX_HEIGHT      1944
#define SENSOR_MAX_FPS         60
#define SENSOR_HAS_DRIVER      1
