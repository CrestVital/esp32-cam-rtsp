# Changelog

All notable changes to this project will be documented in this file.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
Versioning: [Semantic Versioning](https://semver.org/)

## [Unreleased]

### Added — ESPCAMFW-39

- `boards/` directory — per-board DVP pin definitions and capability flags:
  - `boards/ai_thinker_esp32_cam.h` — OV2640, ESP32, WiFi, 4 MB PSRAM;
    complete pin set from AI Thinker schematic Rev 1.6
  - `boards/lilygo_t_display_s3.h` — OV5640, ESP32-S3, WiFi, 8 MB OPI PSRAM;
    placeholder pins with TODO(hardware) pending wiring verification
  - `boards/olimex_esp32_poe.h` — OV2640, ESP32, Ethernet (no WiFi), no PSRAM;
    placeholder pins verified to not overlap LAN8710/LAN8720 RMII GPIOs
- `include/board.h` — common board abstraction header; `#include BOARD_HEADER`
  macro expansion; compile-time `#error` checks for 19 mandatory macros;
  mutual-exclusion check for OV2640/OV5640 sensor flags
- `platformio.ini` — `[platformio]` section with `default_envs`; new
  `[env:ai-thinker-esp32-cam]` and `[env:olimex-esp32-poe]` environments;
  all three ESP32 envs pass `-DBOARD_HEADER=`; `[env:native]` unchanged
- `.gitattributes` — `text=auto eol=lf` for all text files; binary files
  excluded from normalization
- `.gitignore` — `sdkconfig.*` pattern added to exclude auto-generated
  per-env sdkconfig build artifacts

### Added — ESPCAMFW-38

- [env:native] PlatformIO environment for running host tests via pio test -e native:
  - 	est/test_custom_runner.py — custom runner inheriting UnityTestRunner;
    sets EXTRA_LIB_DEPS = None to prevent 	hrowtheswitch/Unity injection
    (would conflict with 	hird-party/unity/); configure_build_env() compiles
    	hird-party/unity/unity.c plus correct component sources and mock stubs
    per suite via BuildSources
  - 	est/native/test_nvs_config/, 	est/native/test_app_event/,
    	est/native/test_power_manager/ — PlatformIO suite directories (copies
    of canonical sources in 	est/components/; kept in sync manually)
  - 	est/README updated: documents both pio test -e native and
    make -f test/Makefile invocation methods
- Fix: 	est/Makefile SRCS_NVS was missing mock_esp_log_counters.c
  (pre-existing bug exposed by GCC 16 / MinGW-w64 16.1.0)
- Fix: 	est/mocks/esp_err.h missing trailing newline (GCC 16 warning with -Werror)


### Changed

- Public repository cleanup — removed internal platform service names and
  internal Jira project key from all documentation and templates:
  - CLAUDE.md, AGENTS.md — Jira project key corrected to ESPCAMFW;
    internal platform service references replaced with generic labels
  - CONTRIBUTING.md — branch and commit examples updated to ESPCAMFW-N;
    AI agent model names removed from section 9
  - DEVELOPMENT.md — internal service names replaced with generic labels;
    wifi_manager ticket column corrected
  - README.md, docs/architecture.md, docs/adr/ADR-001-mjpeg-vs-h264.md,
    .agent/skills/ — platform diagram and service references generalised
  - .github/PULL_REQUEST_TEMPLATE.md, .github/ISSUE_TEMPLATE/ — Jira
    ticket links updated to ESPCAMFW project key
  - STATUS.md — updated to reflect current state (ESPCAMFW-12..15 merged)
  - CHANGELOG.md — specific AI agent filenames removed from history entries
- .gitignore — added rules for secrets and local environment overrides
  (.env, secrets.*, config.local.*, wifi_credentials.*)

### Added

- LICENSE — Apache License 2.0
- SECURITY.md — private vulnerability reporting policy

---


### Added

- Test infrastructure: Unity host-side unit tests (ESPCAMFW-35)
  - Unity 2.6.0 vendored in `third-party/unity/`
  - ESP-IDF mock headers in `test/mocks/` (esp_err, esp_log, nvs, nvs_flash)
  - 34 unit tests for `nvs_config` component in `test/components/nvs_config/`
  - `test/Makefile` for GCC-only build (no ESP-IDF required)
  - CI job `test` runs parallel to `build` on every push
  - `scripts/hooks/pre-commit` with `printf()` detector excluding `third-party/`
  - `scripts/install-hooks.ps1` for hook installation on developer machines
- `components/sys_log/` — logging infrastructure component (ESPCAMFW-12):
  per-module log tags (`LOG_TAG_CAM/RTSP/WIFI/WEBUI/OTA/SYS`) with
  `LOG_*_E/W/I/D/V` macro wrappers; `sys_log_set_level()` runtime API;
  `sys_log_print_system_info()` — prints firmware version, IDF version,
  chip model/revision/cores, chip ID (EFuse MAC), WiFi STA MAC, free heap,
  min free heap, free PSRAM, and reset reason on boot
- `.agent/agents/` — AI agent context files

### Changed

- `src/main.c` — replaced inline boot log with `sys_log_print_system_info()` call;
  removed `#include <stdio.h>` (no longer needed directly in main)
- `src/CMakeLists.txt` — added `sys_log` to `REQUIRES`
- `sdkconfig.defaults` — added `CONFIG_LOG_MAXIMUM_LEVEL_VERBOSE=y` to allow
  verbose log level at runtime; documented that per-tag compile-time level
  selection is not supported by ESP-IDF (runtime control via `sys_log_set_level()`)

---

### Added

- `DEVELOPMENT.md` — phased implementation plan: component dependency order,
  per-component task tables with acceptance criteria, open questions, DoD
- `docs/architecture.md` — full firmware architecture: component layout,
  FreeRTOS task map with core pinning, memory budget (SRAM vs PSRAM),
  data flow, RTSP session lifecycle, OTA update flow, ADR index
- `docs/adr/README.md` — ADR index and reusable template
- `docs/adr/ADR-001-mjpeg-vs-h264.md` — MJPEG chosen over H.264 for MVP:
  OV sensor outputs JPEG natively (zero CPU cost), RFC 2435 packetiser,
  FFmpeg-compatible; H.264 deferred to Phase 2 revisit
- `.github/workflows/ci.yml` — PlatformIO build on every push/PR;
  compiler warning detection; firmware artifacts uploaded (7-day retention)
- `.github/workflows/release.yml` — triggered on v* tags; builds firmware,
  extracts CHANGELOG entry, publishes GitHub Release with .bin and .elf
- `.github/PULL_REQUEST_TEMPLATE.md` — firmware-specific PR checklist
- `.github/ISSUE_TEMPLATE/` — bug report, feature request, config.yml
- `.editorconfig` — 4 spaces for C/H/CMake, LF line endings, UTF-8
- `CLAUDE.md`, `AGENTS.md` — AI agent project context files
- `.agent/CODING_GUIDELINES.md` — C/ESP-IDF coding conventions for AI agents
- `.agent/REVIEW_GUIDELINES.md` — firmware code review checklist for AI agents
- `.agent/skills/` — detailed project knowledge (overview, repo structure, architecture)
- `README.md` — project overview, quick start, hardware table, platform diagram
- `CONTRIBUTING.md` — branch naming, commit format, pre-PR checklist
- `STATUS.md` — current development state
- `scripts/pre-pr.ps1` — pre-PR check script (build, branch guard, CHANGELOG check)

---

## [0.0.1] — 2026-06-19

### Added

- Initial repository structure: `platformio.ini`, `CMakeLists.txt`, `src/main.c`
- `sdkconfig.defaults` — PSRAM, LCDCAM DVP, WiFi, CPU 240 MHz, main task stack 8192
- `partitions/partitions_ota.csv` — NVS + OTA dual-slot + LittleFS 8 MB layout
- `.agent/agents/` — initial AI agent context files
