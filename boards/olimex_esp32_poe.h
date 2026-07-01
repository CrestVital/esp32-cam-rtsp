#pragma once

/*
 * boards/olimex_esp32_poe.h — Board data for Olimex ESP32-POE
 *
 * MCU:      ESP32 (dual-core Xtensa LX6, 240 MHz)
 * Flash:    4 MB
 * PSRAM:    none
 * Camera:   OV2640 via DVP (pins UNVERIFIED — see ESPCAMFW-48)
 * Network:  Ethernet only (LAN8710A), no WiFi
 */

#define BOARD_HAS_WIFI          0
#define BOARD_HAS_ETHERNET      1
#define BOARD_HAS_DISPLAY       0
#define BOARD_HAS_MIPI_CSI      0
#define BOARD_HAS_ISP           0
#define BOARD_PSRAM_SIZE_MB     0

/* Supported sensor set (not exclusive selection) -- see ADR-008. */
#define BOARD_SENSOR_OV2640     1
#define BOARD_SENSOR_OV5640     1

#define BOARD_CAM_PIN_PWDN      (-1)    /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_RESET     (-1)    /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_XCLK      4       /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_SIOD      18      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_SIOC      23      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_D7        36      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_D6        37      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_D5        38      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_D4        39      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_D3        35      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_D2        26      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_D1        13      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_D0        34      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_VSYNC     5       /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_HREF      27      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */
#define BOARD_CAM_PIN_PCLK      25      /* UNVERIFIED — pending hardware verification, see ESPCAMFW-48 */