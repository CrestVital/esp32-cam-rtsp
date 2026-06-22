# esp32-cam-rtsp

ESP32-S3 firmware for OV-based camera devices. Captures frames via DVP interface,
encodes them, and streams live video via RTSP to the CrestVital edge pipeline.

## Hardware

| Property | Value |
|----------|-------|
| Board | LilyGo T-Display S3 |
| MCU | ESP32-S3, 240 MHz, dual-core |
| Flash | 16 MB (QIO) |
| PSRAM | 8 MB OPI |
| Camera | OV2640 / OV5640 via DVP (LCDCAM peripheral) |

## Quick Start

### Prerequisites

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) (CLI or VS Code extension)
- USB-C cable for flashing
- Windows / Linux / macOS

### Build

```powershell
pio run -e lilygo-t-display-s3
```

### Flash

```powershell
pio run -e lilygo-t-display-s3 --target upload
```

### Monitor

```powershell
pio device monitor        # 115200 baud, with esp32_exception_decoder
```

### Build + flash + monitor in one step

```powershell
pio run -e lilygo-t-display-s3 --target upload && pio device monitor
```

## Repository Structure

```text
esp32-cam-rtsp/
├── src/
│   ├── main.c               # Entry point — app_main()
│   └── CMakeLists.txt       # ESP-IDF component registration
├── components/              # Private ESP-IDF components
├── partitions/
│   └── partitions_ota.csv   # Flash layout (NVS, OTA x2, LittleFS)
├── docs/
│   └── adr/                 # Architecture Decision Records
├── scripts/
│   └── pre-pr.ps1           # Pre-PR check script (Windows)
├── .agent/                  # AI agent configuration (committed)
├── platformio.ini           # Board, flash, upload, monitor settings
└── sdkconfig.defaults       # ESP-IDF Kconfig defaults
```

## Flash Partition Layout

| Partition | Size | Purpose |
|-----------|------|---------|
| `nvs` | 24 KB | WiFi credentials, device config |
| `otadata` | 8 KB | OTA slot selector |
| `app0 / ota_0` | 3968 KB | Active firmware slot |
| `app1 / ota_1` | 3968 KB | OTA staging slot |
| `littlefs` | 8 MB | Rolling logs, config files |

## Role in CrestVital Platform

```text
Camera device (this firmware)
        │  RTSP stream over WiFi
        ▼
CrestVital edge pipeline  ──►  inference  ──►  API
```

## Development

See [CONTRIBUTING.md](CONTRIBUTING.md) for branch naming, commit format, and
the pre-PR checklist.

See [DEVELOPMENT.md](DEVELOPMENT.md) for the phased implementation plan.

See [STATUS.md](STATUS.md) for current development state.
