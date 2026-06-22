# Status — esp32-cam-rtsp

**Last updated:** 2026-06-22
**Version:** 0.0.1-dev
**Active branch:** main

---

## Current State

Infrastructure components merged to main: sys_log, nvs_config, app_event,
power_manager. Test infrastructure (Unity host tests) in place.
[env:native] PlatformIO environment ready — host tests runnable via both
pio test -e native and make -f test/Makefile.
Multi-board platform abstraction in place — three target boards supported.

---

## What's Done

- **[ESPCAMFW-12]** ✅ sys_log component — per-module tags & macro wrappers,
  sys_log_set_level() runtime API, sys_log_print_system_info() boot diagnostics
- **[ESPCAMFW-13]** ✅ nvs_config component — NVS-backed key/value config store,
  typed getters/setters, default values, Unity host tests
- **[ESPCAMFW-14]** ✅ app_event component — centralized FreeRTOS event loop,
  event bitmask API, shutdown/reboot events, Unity host tests
- **[ESPCAMFW-15]** ✅ power_manager component — TWDT 30 s, ISR with
  IRAM_ATTR/DRAM_ATTR, graceful shutdown via APP_EVENT_SHUTDOWN,
  8 Unity host tests
- **[ESPCAMFW-35]** ✅ Test infrastructure — Unity 2.6.0 vendored, mock headers,
  test/Makefile, CI test job parallel to build
- **[ESPCAMFW-38]** ✅ [env:native] PlatformIO environment — pio test -e native
  runs 52 host tests across 3 suites; custom runner resolves Unity duplication
  conflict with PlatformIO 6; both pio test and make -f test/Makefile work
- **[ESPCAMFW-39]** ✅ boards/ structure and multi-platform platformio.ini —
  board headers for 3 targets (AI Thinker ESP32-CAM, LilyGo T-Display S3,
  Olimex ESP32-POE); include/board.h with compile-time validation;
  .gitattributes; sdkconfig.* added to .gitignore
- **[Infra]** ✅ Repository scaffold: platformio.ini, CMakeLists.txt, src/main.c
- **[Infra]** ✅ sdkconfig.defaults, partitions/partitions_ota.csv
- **[Infra]** ✅ Agent context: CLAUDE.md, AGENTS.md, .agent/ guidelines
- **[Infra]** ✅ README.md, CONTRIBUTING.md, CHANGELOG.md, STATUS.md
- **[Infra]** ✅ scripts/pre-pr.ps1, scripts/hooks/pre-commit
- **[Infra]** ✅ .github/workflows/ci.yml, release.yml, PR/issue templates
- **[Docs]** ✅ DEVELOPMENT.md, docs/architecture.md, docs/adr/ADR-001
- **[chore]** ✅ Public repository cleanup — LICENSE (Apache 2.0), SECURITY.md,
  internal references removed from all docs and templates

---

## In Progress

*(none)*

---

## Blocked / Open Questions

- OTA trigger protocol — HTTPS push from edge vs pull on schedule — TBD
- Target frame rate and resolution — confirmation needed from edge pipeline team
- Number of concurrent RTSP clients required for MVP
- LilyGo T-Display S3 and Olimex ESP32-POE camera pin assignments are
  placeholders — require hardware verification before camera_driver (ESPCAMFW-40+)

---

## Next Up

- **[ESPCAMFW-40]** Extend nvs_config with network_mode field
  (wifi / ethernet / both) — prerequisite for wifi_manager
- **[ESPCAMFW-41]** wifi_manager component — connect, reconnect, NVS credential
  storage; conditional compile on BOARD_HAS_WIFI
- **[ESPCAMFW-??]** camera_driver component — DVP HAL, PSRAM frame buffer pool,
  OV2640 and OV5640 sensor support; requires board pin verification for
  T-Display S3 and Olimex ESP32-POE
- **[ESPCAMFW-??]** rtsp_server component — RTSP/RTP stack, RFC 2435 MJPEG packetiser
- **[ESPCAMFW-??]** ota_manager component — HTTPS OTA, dual-slot, rollback