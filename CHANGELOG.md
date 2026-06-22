# Changelog

All notable changes to this project will be documented in this file.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
Versioning: [Semantic Versioning](https://semver.org/)

## [Unreleased]

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
