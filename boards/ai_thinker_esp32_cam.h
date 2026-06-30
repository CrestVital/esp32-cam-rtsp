#pragma once

/*
 * boards/ai_thinker_esp32_cam.h — Board data for AI Thinker ESP32-CAM
 *
 * MCU:    ESP32 (dual-core Xtensa LX6, 240 MHz)
 * Flash:  4 MB
 * PSRAM:  4 MB
 * Camera: OV2640 via DVP
 */

#define BOARD_HAS_WIFI          1
#define BOARD_HAS_ETHERNET      0
#define BOARD_HAS_DISPLAY       0
#define BOARD_HAS_MIPI_CSI      0
#define BOARD_HAS_ISP           0
#define BOARD_PSRAM_SIZE_MB     4

#define BOARD_SENSOR_OV2640     1
#define BOARD_SENSOR_OV5640     0

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