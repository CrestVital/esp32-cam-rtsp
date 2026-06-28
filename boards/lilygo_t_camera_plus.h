#pragma once

/*
 * boards/lilygo_t_camera_plus.h — Board data for LilyGo T-Camera Plus
 *
 * MCU:         ESP32-D0WDQ6-V3 rev 3.0 (dual-core Xtensa LX6, 240 MHz)
 * Flash:       4 MB (Winbond W25Q32, QIO)
 * PSRAM:       8 MB quad-SPI
 * Camera:      OV2640 via DVP
 * Display:     ST7789 1.3" via SPI
 * USB-UART:    CH9102F
 * Verified:    esptool flash_id + schematic (MAC b0:b2:1c:50:6d:e4)
 *
 * Note: SCCB camera (SIOD=18/SIOC=23) shares I2C bus with PMIC IP5306.
 *       Display and SD share SPI bus; differ only by CS pin.
 */

#define BOARD_HAS_WIFI          1
#define BOARD_HAS_ETHERNET      0
#define BOARD_HAS_DISPLAY       1
#define BOARD_PSRAM_SIZE_MB     8

#define BOARD_SENSOR_OV2640     1
#define BOARD_SENSOR_OV5640     0

#define BOARD_CAM_PIN_PWDN      (-1)
#define BOARD_CAM_PIN_RESET     (-1)
#define BOARD_CAM_PIN_XCLK      4
#define BOARD_CAM_PIN_SIOD      18      /* shared I2C with IP5306 PMIC */
#define BOARD_CAM_PIN_SIOC      23      /* shared I2C with IP5306 PMIC */
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

#define BOARD_DISPLAY_PIN_MISO  22
#define BOARD_DISPLAY_PIN_MOSI  19
#define BOARD_DISPLAY_PIN_SCLK  21
#define BOARD_DISPLAY_PIN_CS    12
#define BOARD_DISPLAY_PIN_DC    15
#define BOARD_DISPLAY_PIN_BK    2

#define BOARD_SD_PIN_MISO       22
#define BOARD_SD_PIN_MOSI       19
#define BOARD_SD_PIN_SCLK       21
#define BOARD_SD_PIN_CS         0