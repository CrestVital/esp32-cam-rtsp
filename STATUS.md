# Status — esp32-cam-rtsp

**Last updated:** 2026-06-20
**Version:** 0.0.1-dev
**Active branch:** `main`

---

## Current State

Two firmware components implemented, reviewed (2 cycles each), and merged to `main`.
Logging infrastructure and NVS configuration storage are in place for all subsequent
components. Next: `wifi_manager` component.

---

## What's Done

- **[ESPCAMFW-13]** ✅ `nvs_config` component — `app_config_t` with 10 fields,
  `config_init/load/save/reset()` API, per-field range validation with named
  constants, graceful fallback to defaults on any NVS error, factory reset;
  integrated into `app_main()` after NVS flash init
- **[ESPCAMFW-12]** ✅ `sys_log` component — per-module tags & macro wrappers,
  `sys_log_set_level()` runtime API, `sys_log_print_system_info()` boot diagnostics;
  `REQUIRES` minimised to `esp_system log`; full `esp_reset_reason_t` coverage (16 values)
- **[Infra]** ✅ Repository scaffold: `platformio.ini`, `CMakeLists.txt`,
  `src/main.c` (NVS init, config load, sys_log boot info, main loop)
- **[Infra]** ✅ `sdkconfig.defaults` — PSRAM (OPI), LCDCAM DVP ISR, WiFi buffers,
  CPU 240 MHz, main task stack 8192
- **[Infra]** ✅ `partitions/partitions_ota.csv` — NVS + OTA dual-slot + LittleFS 8 MB
- **[Phase 1]** ✅ Agent context: `CLAUDE.md`, `AGENTS.md`
- **[Phase 1]** ✅ `.agent/CODING_GUIDELINES.md` — C/ESP-IDF conventions, NEVER COMMIT rule
- **[Phase 1]** ✅ `.agent/REVIEW_GUIDELINES.md` — firmware review checklist
- **[Phase 1]** ✅ `.agent/skills/` — project overview, repo structure, architecture decisions
- **[Phase 2]** ✅ `README.md`, `CONTRIBUTING.md`, `CHANGELOG.md`, `STATUS.md`
- **[Phase 2]** ✅ `scripts/pre-pr.ps1` — pre-PR build + branch + CHANGELOG check
- **[Phase 3]** ✅ `.github/workflows/ci.yml` — PlatformIO build CI
- **[Phase 3]** ✅ `.github/workflows/release.yml` — release pipeline on v* tags
- **[Phase 3]** ✅ `.github/PULL_REQUEST_TEMPLATE.md`, `ISSUE_TEMPLATE/`
- **[Phase 3]** ✅ `.editorconfig`
- **[Docs]** ✅ `DEVELOPMENT.md` — phased plan with component dependency order
- **[Docs]** ✅ `docs/architecture.md` — component layout, task map, memory budget,
  data flow, RTSP lifecycle, OTA flow
- **[Docs]** ✅ `docs/adr/ADR-001-mjpeg-vs-h264.md` — **MJPEG accepted for MVP**

---

## In Progress

Nothing currently in progress.

---

## Blocked / Open Questions

- OTA trigger protocol — HTTPS push from `crestvital-edge` vs pull on schedule — TBD
- Target frame rate and resolution — confirmation needed from `crestvital-edge` team
- Number of concurrent RTSP clients required for MVP

---

## Next Up

- **[ESPCAMFW-??]** `wifi_manager` component — connect, reconnect, NVS credential storage
- **[ESPCAMFW-??]** `camera_driver` component — DVP init, PSRAM frame buffer pool, OV2640/OV5640
- **[ESPCAMFW-??]** `rtsp_server` component — RTSP/RTP stack, RFC 2435 MJPEG packetiser
- **[ESPCAMFW-??]** `ota_manager` component — HTTPS OTA, dual-slot, rollback
