# 02 — Repository and Build Overview

## Build System

This project uses **PlatformIO** as the primary build and upload tool, but the
underlying framework is **ESP-IDF** (not Arduino). The two are not interchangeable.

```ini
; platformio.ini (summary)
[env:lilygo-t-camera-plus]
platform  = espressif32@7.0.0
board     = esp32dev
framework = espidf
```

### Key build commands

```powershell
# Build only
pio run -e lilygo-t-camera-plus

# Build + flash (serial)
pio run -e lilygo-t-camera-plus --target upload

# Open serial monitor
pio device monitor          # 115200 baud, with esp32_exception_decoder

# Clean build artifacts
pio run --target clean
```

### Alternative — ESP-IDF native (idf.py)

```bash
# Requires IDF_PATH environment variable to be set
idf.py build
idf.py -p COM3 flash monitor
```

## Directory Layout

```text
esp32-cam-rtsp/
├── src/
│   ├── main.c               Entry point. app_main() runs on core 0.
│   └── CMakeLists.txt       Registers the main component with ESP-IDF.
│                            Currently REQUIRES: nvs_flash, esp_wifi, freertos
├── components/              Private ESP-IDF components (each in its own subdir).
│   └── .gitkeep             Empty until components are added.
├── include/                 Shared headers not belonging to a specific component.
├── lib/                     PlatformIO-managed libraries (if any).
├── test/                    Unity-based unit tests.
├── partitions/
│   └── partitions_4mb_ota.csv   Flash layout. See 03-architecture.md for details.
├── docs/
│   └── adr/                 Architecture Decision Records.
├── scripts/
│   └── pre-pr.ps1           Pre-PR check script (runs build, checks CHANGELOG).
├── .agent/                  AI agent configuration (committed, never gitignored).
├── .github/
│   └── workflows/ci.yml     GitHub Actions: PlatformIO build on every push/PR.
├── platformio.ini           Board, flash, upload, monitor configuration.
├── sdkconfig.defaults       ESP-IDF Kconfig defaults (PSRAM, WiFi, LCDCAM, CPU freq).
└── CMakeLists.txt           Top-level CMake (required by ESP-IDF project model).
```

## Adding a New Component

1. Create `components/my_component/` directory.
2. Add `components/my_component/CMakeLists.txt`:
   ```cmake
   idf_component_register(
       SRCS "my_component.c"
       INCLUDE_DIRS "include"
       REQUIRES nvs_flash       # list only what this component directly uses
   )
   ```
3. Add `components/my_component/include/my_component.h` with `#pragma once`.
4. Reference it from `src/CMakeLists.txt` by adding to `REQUIRES`.

## sdkconfig.defaults

Do not modify `sdkconfig.defaults` unless a Jira ticket explicitly requires it.
The file sets:
- PSRAM (OPI, 80 MHz, `SPIRAM_USE_MALLOC` with 16 KB internal threshold)
- LCDCAM DVP ISR cache-safe flag (required for OV camera)
- WiFi buffer counts
- CPU frequency: 240 MHz
- Main task stack: 8192 bytes
- Flash size: 16 MB

## .gitignore Notes

The following are intentionally **not** gitignored:
- `.agent/` — agent configuration is part of the repo, not IDE state
- `sdkconfig.defaults` — committed default Kconfig values

The following are gitignored:
- `.pio/` — PlatformIO build output
- `build/` — ESP-IDF build output
- `sdkconfig`, `sdkconfig.old` — generated Kconfig files (local only)
- `managed_components/` — ESP-IDF managed components (downloaded on build)
