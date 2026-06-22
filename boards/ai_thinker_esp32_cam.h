#ifndef BOARDS_AI_THINKER_ESP32_CAM_H
#define BOARDS_AI_THINKER_ESP32_CAM_H

/* AI Thinker ESP32-CAM — OV2640 DVP camera pins.
 * Source: AI Thinker official schematic Rev 1.6. */

/* DVP data bus (Y2 = LSB D0, Y9 = MSB D7) */
#define BOARD_CAM_PIN_D0     5    /* Y2 */
#define BOARD_CAM_PIN_D1     18   /* Y3 */
#define BOARD_CAM_PIN_D2     19   /* Y4 */
#define BOARD_CAM_PIN_D3     21   /* Y5 */
#define BOARD_CAM_PIN_D4     36   /* Y6 */
#define BOARD_CAM_PIN_D5     39   /* Y7 */
#define BOARD_CAM_PIN_D6     34   /* Y8 */
#define BOARD_CAM_PIN_D7     35   /* Y9 */

/* DVP control */
#define BOARD_CAM_PIN_XCLK   0
#define BOARD_CAM_PIN_PCLK   22
#define BOARD_CAM_PIN_VSYNC  25
#define BOARD_CAM_PIN_HREF   23

/* SCCB (I2C-compatible) */
#define BOARD_CAM_PIN_SIOD   26
#define BOARD_CAM_PIN_SIOC   27

/* Power control */
#define BOARD_CAM_PIN_PWDN   32
#define BOARD_CAM_PIN_RESET  (-1)   /* software reset only */

/* Board capability flags */
#define BOARD_HAS_WIFI       1
#define BOARD_HAS_ETHERNET   0
#define BOARD_SENSOR_OV2640  1
#define BOARD_SENSOR_OV5640  0
#define BOARD_PSRAM_SIZE_MB  4

#endif /* BOARDS_AI_THINKER_ESP32_CAM_H */