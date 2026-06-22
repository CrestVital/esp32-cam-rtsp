#ifndef BOARDS_LILYGO_T_DISPLAY_S3_H
#define BOARDS_LILYGO_T_DISPLAY_S3_H

/* LilyGo T-Display S3 — OV5640 DVP camera pins.
 *
 * The T-Display S3 is a display-only development board with no on-board
 * camera connector.  Camera modules are attached via the GPIO header.
 * The assignments below are PLACEHOLDERS pending hardware verification.
 *
 * TODO(hardware): Confirm OV5640 wiring with the hardware team before
 * implementing camera_driver for this board (ESPCAMFW-40+).
 */

/* DVP data bus — TODO: verify against actual wiring harness */
#define BOARD_CAM_PIN_D0     11   /* TODO: verify */
#define BOARD_CAM_PIN_D1     9    /* TODO: verify */
#define BOARD_CAM_PIN_D2     8    /* TODO: verify */
#define BOARD_CAM_PIN_D3     10   /* TODO: verify */
#define BOARD_CAM_PIN_D4     12   /* TODO: verify */
#define BOARD_CAM_PIN_D5     18   /* TODO: verify */
#define BOARD_CAM_PIN_D6     17   /* TODO: verify */
#define BOARD_CAM_PIN_D7     16   /* TODO: verify */

/* DVP control — TODO: verify against actual wiring harness */
#define BOARD_CAM_PIN_XCLK   15   /* TODO: verify */
#define BOARD_CAM_PIN_PCLK   13   /* TODO: verify */
#define BOARD_CAM_PIN_VSYNC  6    /* TODO: verify */
#define BOARD_CAM_PIN_HREF   7    /* TODO: verify */

/* SCCB */
#define BOARD_CAM_PIN_SIOD   4    /* TODO: verify */
#define BOARD_CAM_PIN_SIOC   5    /* TODO: verify */

/* Power control */
#define BOARD_CAM_PIN_PWDN   (-1)
#define BOARD_CAM_PIN_RESET  (-1)

/* Board capability flags */
#define BOARD_HAS_WIFI       1
#define BOARD_HAS_ETHERNET   0
#define BOARD_SENSOR_OV2640  0
#define BOARD_SENSOR_OV5640  1
#define BOARD_PSRAM_SIZE_MB  8

#endif /* BOARDS_LILYGO_T_DISPLAY_S3_H */