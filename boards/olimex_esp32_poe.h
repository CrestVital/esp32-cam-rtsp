#ifndef BOARDS_OLIMEX_ESP32_POE_H
#define BOARDS_OLIMEX_ESP32_POE_H

/* Olimex ESP32-POE — OV2640 DVP camera pins.
 *
 * The ESP32-POE board has no on-board camera connector.  Camera modules
 * are attached via the GPIO extension header.  The assignments below are
 * PLACEHOLDERS pending hardware verification.
 *
 * Ethernet is handled by LAN8710/LAN8720 via RMII.  The following GPIO
 * pins are reserved for Ethernet and MUST NOT be used for the camera bus:
 *   GPIO0  (REF_CLK / EMAC_TX_CLK)
 *   GPIO18 (MDIO)
 *   GPIO19 (TXD0)
 *   GPIO21 (TX_EN)
 *   GPIO22 (TXD1)
 *   GPIO23 (MDC)
 *   GPIO25 (RXD0)
 *   GPIO26 (RXD1)
 *   GPIO27 (CRS_DV)
 *
 * TODO(hardware): Confirm OV2640 wiring with the hardware team before
 * implementing camera_driver for this board (ESPCAMFW-40+).
 */

/* DVP data bus — TODO: verify; must not overlap with RMII pins above */
#define BOARD_CAM_PIN_D0     32   /* TODO: verify */
#define BOARD_CAM_PIN_D1     33   /* TODO: verify */
#define BOARD_CAM_PIN_D2     34   /* TODO: verify — input-only on ESP32 */
#define BOARD_CAM_PIN_D3     35   /* TODO: verify — input-only on ESP32 */
#define BOARD_CAM_PIN_D4     36   /* TODO: verify — input-only on ESP32 */
#define BOARD_CAM_PIN_D5     39   /* TODO: verify — input-only on ESP32 */
#define BOARD_CAM_PIN_D6     4    /* TODO: verify */
#define BOARD_CAM_PIN_D7     2    /* TODO: verify */

/* DVP control — TODO: verify */
#define BOARD_CAM_PIN_XCLK   16   /* TODO: verify */
#define BOARD_CAM_PIN_PCLK   17   /* TODO: verify */
#define BOARD_CAM_PIN_VSYNC  13   /* TODO: verify */
#define BOARD_CAM_PIN_HREF   14   /* TODO: verify */

/* SCCB */
#define BOARD_CAM_PIN_SIOD   15   /* TODO: verify */
#define BOARD_CAM_PIN_SIOC   5    /* TODO: verify */

/* Power control */
#define BOARD_CAM_PIN_PWDN   (-1)
#define BOARD_CAM_PIN_RESET  (-1)

/* Board capability flags */
#define BOARD_HAS_WIFI       0
#define BOARD_HAS_ETHERNET   1
#define BOARD_SENSOR_OV2640  1
#define BOARD_SENSOR_OV5640  0
#define BOARD_PSRAM_SIZE_MB  0

#endif /* BOARDS_OLIMEX_ESP32_POE_H */