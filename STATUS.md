# Status — esp32-cam-rtsp

**Last updated:** 2026-06-27
**Version:** 0.0.1-dev
**Active branch:** main

---

## Current State

Infrastructure components merged to main: sys_log, nvs_config (extended
with network_mode field), app_event, power_manager, wifi_manager (with
mutex-guarded reconnect task, cooperative shutdown), status_indicator.
Test infrastructure (Unity host tests) in place — 78 tests across 5 suites.
[env:native] PlatformIO environment ready — host tests runnable via both
pio test -e native and make -f test/Makefile. Firmware builds verified on
all three target boards (LilyGo T-Display S3, AI Thinker ESP32-CAM,
Olimex ESP32-POE). Board selection implemented via Kconfig choice
BOARD_TARGET in new board_config component; include/board.h restored with
full pin assignments for all three target boards (ESPCAMFW-45).

---

## What's Done

- **[ESPCAMFW-44]** ✅ wifi_manager race condition fix — `s_reconnect_task`
  now guarded by `s_reconnect_mutex` across all three concurrent contexts;
  cooperative shutdown replaces external `vTaskDelete`; force-clear on
  timeout path; mutex created before event handler registration;
  release-before-notify in event handler; unconditional state reset in
  `init()`; 2 new host tests (injected-handle cooperative path + timeout
  path); 78 host tests total, 0 failures; follow-up ESPCAMFW-46 created
- **[ESPCAMFW-43]** ✅ NVS mock namespace isolation -- handle table maps each
  nvs_open() call to a namespace; nvs_get_*/nvs_set_* scoped per handle;
  wildcard "*" fallback preserves all 39 nvs_config tests unchanged;
  s_last_write_ns tracks SUT write namespace (updated only on successful
  allocation; returns ESP_ERR_NO_MEM on overflow); new
  mock_nvs_set_str_val_ns() and mock_nvs_get_last_write_ns() helpers;
  test_save_credentials_writes_to_nvs now verifies "wifi_cfg" namespace
  explicitly; 75 host tests, 0 failures
- **[ESPCAMFW-45]** ✅ Kconfig-based board selection — `board_config` component
  with `choice BOARD_TARGET` (LilyGo / AI Thinker / Olimex); `include/board.h`
  restored as compile-time macro generator for `BOARD_HAS_*`, `BOARD_PSRAM_SIZE_MB`,
  `BOARD_SENSOR_OV*`, `BOARD_CAM_PIN_*`; `WIFI_MANAGER_ENABLED` default migrated
  from `SOC_WIFI_SUPPORTED` to `BOARD_HAS_WIFI`; all 3 firmware builds verified;
  75 host tests, 0 failures
- **[ESPCAMFW-42]** ✅ status_indicator component — compile-time backend
  selection (display / LED / log-only) via Kconfig; LEDC PWM blink patterns
  for 7 states; deterministic FreeRTOS task shutdown via task notification;
  active-low inversion; per-board sdkconfig.defaults fragments tracked in git;
  8 Unity host tests; 75 total host tests across 5 suites
- **[ESPCAMFW-41]** ✅ wifi_manager component — WiFi connection lifecycle
  (init, connect, disconnect, auto-reconnect with exponential backoff),
  NVS credential storage ("wifi_cfg" namespace), APP_EVENT_WIFI_CONNECTED /
  APP_EVENT_WIFI_DISCONNECTED events, Bluetooth RF guard on ESP32 non-S3;
  stub implementation on Olimex (CONFIG_WIFI_MANAGER_ENABLED=n); 10 Unity
  host tests; firmware verified on all 3 boards
- **[ESPCAMFW-12]** ✅ sys_log component — per-module tags & macro wrappers,
  sys_log_set_level() runtime API, sys_log_print_system_info() boot diagnostics
- **[ESPCAMFW-13]** ✅ nvs_config component — NVS-backed key/value config store,
  typed getters/setters, default values, Unity host tests
- **[ESPCAMFW-40]** ✅ nvs_config extended — network_mode_t enum (WIFI/ETHERNET/BOTH),
  network_mode field in app_config_t; NVS key "net_mode" (uint8_t); validation
  with (unsigned) cast; default NETWORK_MODE_WIFI; 5 new host tests (39 total
  in nvs_config suite, 57 across all suites)
- **[ESPCAMFW-14]** ✅ app_event component — centralized FreeRTOS event loop,
  event bitmask API, shutdown/reboot events, Unity host tests
- **[ESPCAMFW-15]** ✅ power_manager component — TWDT 30 s, ISR with
  IRAM_ATTR/DRAM_ATTR, graceful shutdown via APP_EVENT_SHUTDOWN,
  8 Unity host tests
- **[ESPCAMFW-35]** ✅ Test infrastructure — Unity 2.6.0 vendored, mock headers,
  test/Makefile, CI test job parallel to build
- **[ESPCAMFW-38]** ✅ [env:native] PlatformIO environment — pio test -e native
  runs host tests across 4 suites; custom runner resolves Unity duplication
  conflict with PlatformIO 6; both pio test and make -f test/Makefile work
- **[ESPCAMFW-39]** ✅ Multi-platform platformio.ini — three target board
  environments; board headers (boards/) temporarily removed pending
  ESPCAMFW-45; Kconfig-based WiFi gating (CONFIG_WIFI_MANAGER_ENABLED)
  proven and in place
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

## Open Tickets (spin-offs)

- **[ESPCAMFW-46]** 🟡 Medium — wifi_manager orphaned reconnect task on deinit
  timeout: if reconnect task does not exit within 500 ms, a subsequent
  init+connect cycle creates a new mutex; orphaned task wakes, acquires new
  mutex, and overwrites new task handle. Generation counter or per-cycle
  mutex allocation proposed. Address before camera_driver.

---

## Blocked / Open Questions

- OTA trigger protocol — HTTPS push from edge vs pull on schedule — TBD
- Target frame rate and resolution — confirmation needed from edge pipeline team
- Number of concurrent RTSP clients required for MVP

---

## Next Up

- **[ESPCAMFW-26/27]** camera_driver component — DVP HAL, PSRAM frame buffer pool,
  OV2640 and OV5640 sensor support
- **[ESPCAMFW-??]** rtsp_server component — RTSP/RTP stack, RFC 2435 MJPEG packetiser
- **[ESPCAMFW-??]** ota_manager component — HTTPS OTA, dual-slot, rollback
