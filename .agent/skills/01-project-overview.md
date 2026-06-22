# 01 — Project Overview

## What This Repository Does

`esp32-cam-rtsp` is the **camera edge firmware** for the CrestVital livestock
monitoring platform. It runs on LilyGo T-Display S3 (ESP32-S3) devices mounted
above livestock pens and streams live video to the CrestVital edge pipeline.

## Role in CrestVital Platform

```text
Camera device (this firmware)
        |
        | RTSP stream (TCP/UDP over WiFi)
        v
CrestVital edge pipeline     — pulls RTSP stream, segments clips
        |
        |
        v
inference                    — Re-ID, disease detection, activity tracking
API                          — stores events, sends alerts
```

## Target Hardware

- **Board:** LilyGo T-Display S3
- **MCU:** ESP32-S3, dual-core Xtensa LX7, 240 MHz
- **Flash:** 16 MB (QIO, 80 MHz)
- **PSRAM:** OPI PSRAM, 8 MB (Octal, 80 MHz)
- **Camera:** OV-based sensor (OV2640 or OV5640) via DVP interface (LCDCAM peripheral)
- **Display:** ST7789 1.9" LCD — secondary output for status/debug

## Development Status

See `STATUS.md` for current state. See `DEVELOPMENT.md` for phased plan.

## Jira

- Project key: `ESPCAMFW`
- Instance: `crestvital.atlassian.net`
