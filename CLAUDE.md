# esp32-cam-rtsp — Claude Project Context

## Project Overview

ESP32 firmware for devices equipped with OV-based cameras (OV2640, OV5640 and
compatible sensors). Captures video frames over DVP/MIPI, encodes them, and
streams via RTSP to the CrestVital platform edge pipeline.

**Role in CrestVital platform:** This firmware runs on the camera nodes and feeds
raw video into the CrestVital edge pipeline for further processing.

---

## Hardware

| Property | Value |
|----------|-------|
| Board | LilyGo T-Camera Plus |
| MCU | ESP32-D0WDQ6-V3 (dual-core Xtensa LX6, 240 MHz) |
| Flash | 4 MB (QIO, 40 MHz) |
| PSRAM | 8 MB quad-SPI |
| Camera interface | DVP (Digital Video Port via LCDCAM peripheral) |
| Display | ST7789 (1.3" LCD) — secondary output |

---

## Tech Stack

| Tool | Version / Detail |
|------|-----------------|
| Language | C (ESP-IDF conventions) |
| Build system | PlatformIO (`platformio.ini`) |
| Framework | ESP-IDF (`framework = espidf`) |
| PlatformIO platform | `espressif32@7.0.0` |
| Partition table | `partitions/partitions_4mb_ota.csv` (OTA + LittleFS) |
| CI | GitHub Actions (`.github/workflows/`) |
| Jira | crestvital.atlassian.net — project key `ESPCAMFW` |

---

## Repository Structure

```text
esp32-cam-rtsp/
├── src/
│   ├── main.c               # Entry point — app_main()
│   └── CMakeLists.txt       # ESP-IDF component registration
├── components/              # Private ESP-IDF components (add subdirs here)
├── include/                 # Shared headers
├── lib/                     # PlatformIO libraries
├── test/                    # Unit tests (Unity framework)
├── partitions/
│   └── partitions_4mb_ota.csv   # Flash layout: nvs, ota_0, ota_1, littlefs
├── docs/
│   └── adr/                 # Architecture Decision Records
├── scripts/
│   └── pre-pr.ps1           # Pre-PR check script (Windows)
├── .agent/
│   ├── CODING_GUIDELINES.md # Read before writing any code
│   ├── REVIEW_GUIDELINES.md # Read before doing any code review
│   ├── agents/              # Per-agent configuration files
│   └── skills/              # Detailed project knowledge
├── platformio.ini           # Board config, build flags, monitor settings
├── sdkconfig.defaults       # ESP-IDF Kconfig defaults (PSRAM, WiFi, LCDCAM)
├── CMakeLists.txt           # Top-level CMake (ESP-IDF project)
└── CHANGELOG.md
```

---

## Architecture

### Firmware Component Layout (planned)

```text
app_main()
    |
    +-- wifi_manager          Connects to AP, handles reconnection
    |
    +-- camera_driver         Initialises OV sensor over DVP/LCDCAM
    |       |
    |       +-- frame_buffer  DMA frame buffer management (PSRAM)
    |
    +-- rtsp_server           Minimal RTSP/RTP stack
    |       |
    |       +-- rtp_packetizer  Splits frames into RTP packets
    |
    +-- ota_manager           OTA update via HTTPS
    |
    +-- status_led            Heartbeat, error codes on RGB LED
```

### Data Flow

```text
OV Camera (DVP)
    |  DMA (PSRAM frame buffer)
    v
camera_driver  -->  frame_buffer  -->  rtsp_server  -->  WiFi (TCP/UDP)
                                                              |
                                                    edge pipeline (RTSP client)
```

### Flash Partition Layout

```text
0x009000  nvs        (24 KB)    — WiFi credentials, config
0x00F000  otadata    (8 KB)     — OTA slot selector
0x020000  app0/ota_0 (3968 KB)  — Running firmware
0x400000  app1/ota_1 (3968 KB)  — OTA staging slot
0x7E0000  littlefs   (8 MB)     — Logs, config files
```

---

## sdkconfig.defaults Key Settings

- `CONFIG_SPIRAM=y` — OPI PSRAM enabled; use `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)` for large buffers
- `CONFIG_SPIRAM_USE_MALLOC=y` — PSRAM used for general heap above 16 KB threshold
- `CONFIG_CAM_CTLR_DVP_CAM_ISR_CACHE_SAFE=y` — required for DVP camera ISR
- `CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y` — 240 MHz for RTSP encoding headroom
- `CONFIG_ESP_MAIN_TASK_STACK_SIZE=8192` — main task enlarged for RTSP workload

---

## Coding Conventions

Read `.agent/CODING_GUIDELINES.md` before writing any code. Key rules:

- C only — no C++
- `ESP_ERROR_CHECK()` or explicit `esp_err_t` check on every ESP-IDF API call
- All large/DMA buffers allocated from PSRAM: `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
- FreeRTOS task priorities: 1–24 only (`configMAX_PRIORITIES - 1 = 25`)
- No blocking calls inside ISRs
- All log tags defined as `static const char *TAG = "module_name";`

---

## ⛔ ABSOLUTE RULE — NEVER COMMIT

**Claude must never run `git commit`, `git push`, `git merge`, `git rebase`,
`git tag`, or any command that creates or modifies git history.**

All commits are created exclusively by the repository owner after manual review
of every changed file. Claude may run `git add` (stage files), but never commit.

This rule has no exceptions.

---

## Build & Flash

```powershell
# Build
pio run -e lilygo-t-camera-plus

# Upload (serial)
pio run -e lilygo-t-camera-plus --target upload

# Monitor
pio device monitor

# Build + upload + monitor in one step
pio run -e lilygo-t-camera-plus --target upload && pio device monitor
```

Alternative (ESP-IDF native):
```bash
idf.py build
idf.py -p COM3 flash monitor
```

---

## Jira

- Instance: `crestvital.atlassian.net`
- Project key: `ESPCAMFW`
- All Jira text (summaries, descriptions, comments): **Turkish**
- Reports to Jira tickets: **Russian**
- Code, comments, log messages: **English**

---

## Testing

Host-side unit tests use the Unity framework (vendored in `third-party/unity/`).
Tests compile with standard GCC — no ESP-IDF required.

### Rules
- Every new component must have a corresponding test file in `test/components/<component_name>/`
- Mock headers for ESP-IDF APIs live in `test/mocks/` — never mix with `components/`
- All third-party libraries live in `third-party/` — the only location for vendored code
- Run tests locally: `make -f test/Makefile` from the repository root
- Tests run automatically in CI (job `test`, parallel to `build`)

### Adding tests for a new component
1. Create `test/components/<component_name>/test_<component_name>.c`
2. Add mock headers to `test/mocks/` if new ESP-IDF APIs are needed
3. Add the new source files to `SRCS` in `test/Makefile`

## Key Constraints

- Frame buffers are large (320×240 JPEG ≈ 15–40 KB each). Always allocate from PSRAM.
- The DVP interface is driven by the LCDCAM peripheral; camera init must happen
  after `nvs_flash_init()` and WiFi stack is up.
- OTA uses two app partitions (`ota_0` / `ota_1`). Never write to flash from task
  other than the dedicated OTA task.
- LittleFS partition (`littlefs`) is mounted at `/littlefs`. Use it for config
  files and rolling logs. Do not use NVS for large blobs.
