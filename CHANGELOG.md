# Changelog

All notable changes to this project will be documented in this file.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
Versioning: [Semantic Versioning](https://semver.org/)

## [Unreleased]

### Fixed — ESPCAMFW-55

- **M3 — BT linker dependency:** `components/wifi_manager/CMakeLists.txt` now
  links `idf::bt` privately, conditional on `CONFIG_BT_CONTROLLER_ENABLED`
  (`target_link_libraries(${COMPONENT_LIB} PRIVATE idf::bt)`); previously
  `esp_bt_controller_disable()` was called with no corresponding component
  dependency, so a build with the BT controller enabled would fail to link.
  Builds with BT disabled are unaffected — the Bluetooth stack is not pulled
  in.
- **P4-B — orphaned reconnect task guard in main loop:** `reconnect_task()`
  now re-checks `s_reconnect_generation` (via the existing
  `reconnect_should_clear_handle()` helper) under `s_reconnect_mutex` inside
  the main loop, immediately after `ulTaskNotifyTake()` and the
  `s_reconnect_enabled` check. A task orphaned by a deinit-timeout + new
  init() cycle now exits (`ESP_LOGW`) before calling `esp_wifi_connect()` and
  before incrementing `s_attempt_count`, instead of corrupting the new
  init cycle's reconnect state. The existing cleanup-section guard
  (ESPCAMFW-46) is unchanged.
- **Tripwire — concurrent reconnect task detector:** new static counter
  `s_active_reconnect_tasks`, incremented under mutex at task start and
  decremented in the cleanup section; emits `ESP_LOGE` if the counter
  exceeds 1, signalling two reconnect tasks running simultaneously (a
  protocol violation that should never occur). Reset to 0 in
  `wifi_manager_init()`.
- Two `TODO #ESPCAMFW-47` anchors added (cleanup section of
  `reconnect_task()`, and the start of `wifi_manager_deinit()`) marking the
  deferred join-based teardown that would eliminate the accepted
  stack/TCB-leak risk (R2) on the deinit-timeout path.
- `docs/adr/ADR-002-wifi-teardown.md` (new): documents the cooperative
  shutdown model, the generation-guard fix, the tripwire, Accepted Risk R2,
  and the alternatives considered (external `vTaskDelete`, full join-based
  redesign — both rejected/deferred).
- Test suite extended to 19 cases (from 17): added
  `test_tripwire_counter_reset_by_init` and
  `test_tripwire_fires_when_counter_exceeds_one`, plus test-only accessors
  `wifi_manager_get_active_reconnect_tasks()` /
  `wifi_manager_set_active_reconnect_tasks_for_test()` under `UNIT_TEST`.
  `test/components/wifi_manager/test_wifi_manager.c` and
  `test/native/test_wifi_manager/test_wifi_manager.c` kept in sync
  (byte-for-byte identical after the sync comment).

### Changed — ESPCAMFW-54

- **Board re-identification:** primary board corrected from LilyGo T-Display S3
  (ESP32-S3, OV5640, 16 MB flash, OPI PSRAM) to **LilyGo T-Camera Plus**
  (ESP32-D0WDQ6-V3 rev 3.0, OV2640, 4 MB flash, 8 MB quad-SPI PSRAM, ST7789
  display, CH9102F USB-UART); verified via `esptool flash_id` and schematic
  scan (MAC `b0:b2:1c:50:6d:e4`)
- **Data-driven board abstraction (Variant B):** board data extracted from
  `include/board.h` into standalone data files `boards/<board>.h` (pure
  `#define` macros, no logic); `include/board.h` rewritten as a dispatcher —
  `#include CONFIG_BOARD_DATA_FILE` from Kconfig, followed by compile-time
  completeness validation (`#ifndef`/`#error` for all 6 capability flags, both
  sensor flags with mutual-exclusion check, and all 17 `BOARD_CAM_PIN_*`
  macros); adding a new board now requires only a new `boards/<board>.h` +
  one Kconfig entry + one PlatformIO env — no changes to `board.h` or any
  `.c` file
- `boards/lilygo_t_camera_plus.h` — verified DVP camera pins (XCLK=4,
  SIOD=18/SIOC=23 shared with IP5306 PMIC), ST7789 display SPI pins, SD SPI
  pins; `boards/ai_thinker_esp32_cam.h` — standard AI Thinker pinout;
  `boards/olimex_esp32_poe.h` — placeholder pins marked UNVERIFIED pending
  hardware (ESPCAMFW-48)
- `components/board_config/Kconfig.projbuild`: `choice BOARD_TARGET` rewritten
  with new `config BOARD_DATA_FILE` string option; default changed to
  `BOARD_LILYGO_T_CAMERA_PLUS`; `BOARD_LILYGO_T_DISPLAY_S3` removed
- `components/board_config/CMakeLists.txt`: `INCLUDE_DIRS
  "${CMAKE_SOURCE_DIR}/boards"` added so `#include CONFIG_BOARD_DATA_FILE`
  resolves across all components
- `sdkconfig.defaults` (shared): removed S3-specific settings
  (`CONFIG_SPIRAM_MODE_OCT`, `CONFIG_SPIRAM_SPEED_80M`,
  `CONFIG_ESPTOOLPY_FLASHSIZE_16MB`, `CONFIG_CAM_CTLR_DVP_CAM_ISR_CACHE_SAFE`,
  USB-JTAG console options); cross-platform settings retained
- `sdkconfig.defaults.lilygo-t-camera-plus` (new, replaces
  `sdkconfig.defaults.lilygo-t-display-s3`): `CONFIG_SPIRAM_MODE_QUAD`,
  `CONFIG_ESPTOOLPY_FLASHSIZE_4MB`, `CONFIG_STATUS_INDICATOR_HAS_DISPLAY=y`
- `sdkconfig.defaults.ai-thinker-esp32-cam` / `olimex-esp32-poe`: updated
  with explicit `CONFIG_BOARD_*`, PSRAM mode and flash size
- `partitions/partitions_4mb_ota.csv` (new): dual-slot OTA layout for 4 MB
  flash (nvs 24 KB, otadata 8 KB, app0 1792 KB, app1 1792 KB, littlefs
  384 KB); replaces `partitions/partitions_ota.csv` (16 MB S3 layout, removed);
  `board_build.partitions` now set for all three firmware environments
- `platformio.ini`: `[env:lilygo-t-display-s3]` replaced by
  `[env:lilygo-t-camera-plus]` (`board = esp32dev`, 4 MB flash, 40 MHz);
  `build_flags = -DCONFIG_STATUS_INDICATOR_HAS_DISPLAY=1` workaround removed
  (display selection now via sdkconfig fragment); `default_envs` updated
- Sitewide rename — `lilygo-t-display-s3` / `BOARD_LILYGO_T_DISPLAY_S3` /
  `LilyGo T-Display S3` removed from all active files (ci.yml, release.yml,
  agent context, scripts, docs); CI `permissions:` blocks preserved unchanged
- `docs/adr/ADR-004-board-abstraction.md`: documents Variant A (`#if/#elif`
  chain, rejected), Variant B (data files + Kconfig, accepted), and the prior
  failed `-DBOARD_HEADER` approach (ESPCAMFW-39)
- `DEVELOPMENT.md`: "How to add a board" section; board table updated; Purpose
  paragraph corrected (`ESP32-S3` → `ESP32`)
- **82 host tests, 0 failures** (2 new tests added in ESPCAMFW-46 cycle
  account for count going from 80 to 82; this ticket adds no new tests)
- All three firmware builds pass with zero errors and zero warnings

### Security — ESPCAMFW-53

- CI: restrict `GITHUB_TOKEN` to least privilege in GitHub Actions workflows —
  silences CodeQL alert `actions/missing-workflow-permissions`
- `.github/workflows/ci.yml`: added workflow-level `permissions: contents: read`
  (both `build` and `test` jobs only read the repo; artifact upload needs no
  write scope)
- `.github/workflows/release.yml`: narrowed top-level `permissions` from
  `contents: write` to `contents: read`; `contents: write` granted at job level
  only to the `release` job (the one running `gh release create`); `build` job
  left to inherit read-only
- No build logic, matrix, install steps, artifact rename, CHANGELOG extraction,
  or env names changed; both workflows remain valid YAML

### Fixed — ESPCAMFW-46

- `wifi_manager`: fix orphaned reconnect task after deinit timeout — if
  `reconnect_task()` did not exit within the 500 ms cooperative shutdown
  window, a subsequent `wifi_manager_init()` + `wifi_manager_connect()` cycle
  created a new mutex; the orphaned task would eventually wake, acquire the
  new mutex, and overwrite `s_reconnect_task = NULL` in its cleanup section,
  clobbering the new task's handle and triggering a FreeRTOS kernel panic on
  subsequent `WIFI_EVENT_STA_DISCONNECTED` (ESP32-S3 dual-core Xtensa LX7)
- Introduced `s_reconnect_generation` (`static uint32_t`) incremented on
  every `wifi_manager_init()`; reconnect task receives the generation value
  at creation time via `pvParameters` (captured under `s_reconnect_mutex`
  in `xTaskCreate` call), eliminating a scheduler-dependent capture window
- Cleanup section of `reconnect_task()` guards `s_reconnect_task = NULL`
  with `reconnect_should_clear_handle(my_generation)` — exits silently with
  `ESP_LOGW` (logging both captured and current generation) if the generation
  has advanced, indicating orphaned status
- `static bool reconnect_should_clear_handle(uint32_t)` extracted as a
  standalone helper to make the guard logic directly testable from host tests
  (FreeRTOS tasks cannot run under the native scheduler)
- New `#if defined(UNIT_TEST)` accessor
  `wifi_manager_reconnect_should_clear_handle_test()` exposes the helper;
  new accessor `wifi_manager_get_reconnect_generation()` exposes the counter
- 2 new Unity host tests covering the guard logic directly
  (`test_reconnect_guard_logic_matching_generation`,
  `test_reconnect_guard_logic_stale_generation`); total host suite: **80 tests**
  across 5 suites, 0 failures

### Fixed — ESPCAMFW-44

- `wifi_manager`: eliminate race condition on `s_reconnect_task` — the static
  `TaskHandle_t` was read and written from three concurrent contexts
  (`wifi_event_handler`, `wifi_manager_deinit`, `reconnect_task`) with no
  synchronisation, causing FreeRTOS kernel panics on ESP32-S3 (dual-core
  Xtensa LX7)
- Introduced `s_reconnect_mutex` (`SemaphoreHandle_t`) guarding every access
  to `s_reconnect_task`; mutex created before `esp_event_handler_register()`
  so no event arrives with a NULL mutex; deleted after the shutdown poll loop
  in `wifi_manager_deinit()`
- Replaced external `vTaskDelete(s_reconnect_task)` with cooperative shutdown:
  `deinit()` sets `s_reconnect_enabled = false`, wakes the task via
  `xTaskNotifyGive`, and polls `s_reconnect_task` under the mutex
  (50 × 10 ms) until the task self-deletes; force-clears `s_reconnect_task =
  NULL` before `vSemaphoreDelete` to guard the timeout path
- `wifi_event_handler()` WIFI_EVENT_STA_DISCONNECTED: handle copied to local
  variable before `xSemaphoreGive`; `xTaskNotifyGive` called outside the
  critical section — eliminates priority inversion risk
- `wifi_manager_init()`: unconditional reset of `s_reconnect_task`,
  `s_attempt_count`, `s_reconnect_enabled`, `s_connected` after `s_initialized`
  guard — guarantees clean state after a timed-out deinit cycle
- 2 new Unity host tests (`test_deinit_cooperative_shutdown_with_injected_task`,
  `test_deinit_timeout_path_clears_stale_handle`); total host suite: **78 tests**
  across 5 suites, 0 failures
- New test-only accessor `wifi_manager_get_reconnect_task_ptr()` under
  `#if defined(UNIT_TEST)`; `mock_set/clear_reconnect_task_handle()` injection
  helpers in `test/mocks/mock_freertos_task.c`; `-DUNIT_TEST=1` added to
  `CFLAGS_WIFI_MANAGER` in `test/Makefile`
- Follow-up ESPCAMFW-46 created for orphaned-task edge case on extreme
  timeout path (Medium priority, pre-`camera_driver`)

### Added — ESPCAMFW-43

- `test/mocks/mock_nvs_state.c`: namespace isolation for mock NVS --
  handle table `s_handles[MOCK_MAX_HANDLES]` maps each `nvs_open()` call
  to a namespace string; `nvs_get_*` / `nvs_set_*` stubs resolve namespace
  from handle before every operation
- `find_ns()` lookup: exact `(namespace, key)` match first, wildcard
  `("*", key)` fallback second -- preserves all 39 nvs_config tests without
  source modification
- Legacy helpers (`mock_nvs_set_u8/i8/u16/str_val/key_error`) store under
  wildcard namespace `"*"`; SUT writes go under the exact resolved namespace
- `s_last_write_ns`: tracks namespace of the most recent `nvs_set_*` call
  from the SUT; updated only after successful entry allocation --
  `nvs_set_*` now returns `ESP_ERR_NO_MEM` on table overflow instead of
  false `ESP_OK`
- New public helpers in `test/mocks/nvs.h`:
  `mock_nvs_set_str_val_ns(ns, key, value)` and
  `mock_nvs_get_last_write_ns()`
- `test_save_credentials_writes_to_nvs` updated in both
  `test/components/wifi_manager/` and `test/native/test_wifi_manager/`:
  asserts `mock_nvs_get_last_write_ns() == "wifi_cfg"` and verifies
  read-back via a `"wifi_cfg"`-scoped handle
- Removed unused `#include <stdio.h>` / `<stdlib.h>` from
  `mock_nvs_state.c`; decorative box-drawing separators replaced with
  plain comment headers
- Host test suite: 75 tests, 0 failures (count unchanged -- this ticket
  modifies infrastructure, not test count)

### Added — ESPCAMFW-45

- `components/board_config/` — new ESP-IDF component (no SRCS) providing
  `Kconfig.projbuild` with `choice BOARD_TARGET`: `BOARD_LILYGO_T_DISPLAY_S3` /
  `BOARD_AI_THINKER_ESP32_CAM` / `BOARD_OLIMEX_ESP32_POE`; derived invisible
  bools `BOARD_HAS_WIFI` and `BOARD_HAS_ETHERNET` outside the choice block
- `include/board.h` restored as a pure compile-time macro generator keyed on
  `CONFIG_BOARD_*` (no computed `#include`); defines per board:
  `BOARD_HAS_WIFI`, `BOARD_HAS_ETHERNET`, `BOARD_HAS_DISPLAY`,
  `BOARD_PSRAM_SIZE_MB`, `BOARD_SENSOR_OV5640/OV2640`, and all 16
  `BOARD_CAM_PIN_*` constants; `#error` fallback if no board is selected

### Changed — ESPCAMFW-45

- `sdkconfig.defaults.lilygo-t-display-s3`: `CONFIG_BOARD_LILYGO_T_DISPLAY_S3=y`
  appended; existing `STATUS_INDICATOR_*` entries preserved
- `sdkconfig.defaults.ai-thinker-esp32-cam`: `CONFIG_BOARD_AI_THINKER_ESP32_CAM=y`
  appended; existing entries preserved
- `sdkconfig.defaults.olimex-esp32-poe`: `CONFIG_BOARD_OLIMEX_ESP32_POE=y`
  appended; existing entries preserved
- `components/wifi_manager/Kconfig.projbuild`: `WIFI_MANAGER_ENABLED` default
  changed from `SOC_WIFI_SUPPORTED` to `BOARD_HAS_WIFI` for consistency with
  the new board abstraction layer
- `src/CMakeLists.txt`: `board_config` added to `REQUIRES`
- `test/mocks/sdkconfig.h`: `CONFIG_BOARD_AI_THINKER_ESP32_CAM=1` added
  (simulates AI Thinker for host builds, consistent with existing
  `CONFIG_IDF_TARGET_ESP32=1`)
- `test/Makefile`: `-DCONFIG_BOARD_AI_THINKER_ESP32_CAM=1` added to
  `CFLAGS_WIFI_MANAGER`

### Added — ESPCAMFW-42

- `status_indicator` ESP-IDF component — single entry-point for device status
  indication; backend selected at compile time via Kconfig:
  - **Display path** (`CONFIG_STATUS_INDICATOR_HAS_DISPLAY=y`): calls
    `display_manager_update_status()` with `state_name` + `state_id`; stub
    with `TODO #ESPCAMFW-26` active until `display_manager` is implemented
  - **LED path** (`LED_PIN != -1`, `ESP_PLATFORM`): LEDC PWM (13-bit, 5 kHz),
    per-state blink patterns, active-low inversion, FreeRTOS `indicator_task`
    (priority 2, stack 2048 B); deterministic shutdown via task notification
    (3000 ms timeout, force-kill fallback)
  - **Log-only path** (`LED_PIN == -1`): `ESP_LOGI` on state change, no hardware
- Public API: `status_indicator_init()`, `status_indicator_deinit()`,
  `status_indicator_set_state(indicator_state_t)`
- `indicator_state_t` enum: `BOOT`, `WIFI_CONNECTING`, `WIFI_CONNECTED`,
  `WIFI_ERROR`, `STREAMING`, `OTA`, `ERROR` (steady-on via LEDC duty)
- LED blink patterns per ticket spec: BOOT 1 Hz, WIFI_CONNECTING 5 Hz,
  WIFI_CONNECTED 2×fast+2 s pause, WIFI_ERROR 3×fast+1 s pause,
  STREAMING 0.5 Hz, OTA fast/slow, ERROR steady-on
- 8 Unity host tests (8/8 pass, `-Wall -Wextra -Werror`, 0 warnings);
  total host suite: 75 tests across 5 components, 0 failures
- `test/mocks/freertos/semphr.h` — new mock for `SemaphoreHandle_t` host build
- `test/mocks/freertos/task.h` — extended with `xTaskGetCurrentTaskHandle`,
  `ulTaskNotifyTake`, `xTaskNotifyGive` stubs

### Changed — ESPCAMFW-42

- `platformio.ini`: `board_build.sdkconfig_defaults` added to all three
  firmware envs pointing to per-board sdkconfig fragments
- `sdkconfig.defaults.lilygo-t-display-s3`: `CONFIG_STATUS_INDICATOR_HAS_DISPLAY=y`
- `sdkconfig.defaults.ai-thinker-esp32-cam`: `LED_PIN=33`, `ACTIVE_LOW=y`
- `sdkconfig.defaults.olimex-esp32-poe`: `LED_PIN=-1` (log-only)
- `.gitignore`: added `!sdkconfig.defaults.*` exception to allow tracking of
  board-specific sdkconfig fragments without `git add -f`
- `src/main.c`: `status_indicator_init()` + `set_state(INDICATOR_STATE_BOOT)`
  after `power_manager_init()`; `set_state(INDICATOR_STATE_WIFI_CONNECTING)`
  before `wifi_manager_connect()`
- Test counts: 75 host tests (39 nvs_config + 10 app_event + 8 power_manager +
  10 wifi_manager + 8 status_indicator), previously 67

### Added — ESPCAMFW-41

- `wifi_manager` ESP-IDF component — full WiFi connection lifecycle for
  boards where `CONFIG_WIFI_MANAGER_ENABLED=y`; stub implementation
  returning `ESP_ERR_NOT_SUPPORTED` on Ethernet-only boards (Olimex)
- Public API: `wifi_manager_init()`, `wifi_manager_deinit()`,
  `wifi_manager_connect(ssid, password)`, `wifi_manager_disconnect()`,
  `wifi_manager_is_connected()`, `wifi_manager_save_credentials()`,
  `wifi_manager_load_credentials()`
- Auto-reconnect FreeRTOS task `"wifi_reconnect"` (priority 6, stack 4096 B):
  exponential backoff 1 s → 2 s → 4 s → 8 s → 16 s → 30 s (cap), 10 attempts
  max per disconnect episode; counter resets on each new episode
- `APP_EVENT_WIFI_CONNECTED` posted on `IP_EVENT_STA_GOT_IP`;
  `APP_EVENT_WIFI_DISCONNECTED` posted on `WIFI_EVENT_STA_DISCONNECTED`
- NVS credential storage in dedicated namespace `"wifi_cfg"`, keys
  `"wifi_ssid"` / `"wifi_pass"`; NULL password accepted (open networks)
- Bluetooth RF guard: `esp_bt_controller_disable()` on ESP32 non-S3 +
  `CONFIG_BT_CONTROLLER_ENABLED` to release shared RF path for WiFi
- `components/wifi_manager/Kconfig.projbuild`: `CONFIG_WIFI_MANAGER_ENABLED`
  bool — `default y if SOC_WIFI_SUPPORTED`; replaces the removed
  `BOARD_HAS_WIFI` flag from board headers
- 10 Unity host tests (wifi_manager suite): init, double-init guard, NULL ssid,
  NULL password (open network), connect sequence, is_connected state, NVS
  save/load, disconnect-suppresses-reconnect, BT disable on ESP32;
  total host suite: 67 tests across 4 components, 0 failures
- `test/mocks/`: new stubs for `esp_wifi`, `esp_bt`, `esp_netif`,
  `sdkconfig`; extended `freertos/task.h` with FreeRTOS task API stubs;
  `mock_esp_wifi.c`, `mock_freertos_task.c` with injectable return values
  and call counters
- `src/main.c`: conditional `wifi_manager_init()` / `wifi_manager_connect()`
  call gated on `CONFIG_WIFI_MANAGER_ENABLED` and `cfg.network_mode`

### Changed — ESPCAMFW-41

- `include/board.h` and `boards/` directory removed; the `BOARD_HEADER`
  macro approach is not viable with PlatformIO + ESP-IDF (`build_flags -D`
  flags are not propagated into the ESP-IDF ninja build). Will be restored with
  a Kconfig-based mechanism before `camera_driver` (see ESPCAMFW-45)
- `src/CMakeLists.txt`: added `power_manager` and `wifi_manager` to
  `REQUIRES` (`power_manager` was previously missing)
- `components/wifi_manager/CMakeLists.txt`: added `esp_netif` to `REQUIRES`
- `platformio.ini`: removed dead `board_build.cmake_extra_args` and
  `build_flags -DBOARD_HEADER` entries from all three ESP32 environments
- Test counts: 67 host tests (39 nvs_config + 10 app_event + 8 power_manager +
  10 wifi_manager), previously 57

### Added — ESPCAMFW-40

- `nvs_config`: `network_mode_t` enum (`NETWORK_MODE_WIFI = 0`,
  `NETWORK_MODE_ETHERNET = 1`, `NETWORK_MODE_BOTH = 2`) — selects the
  active network transport; stored in NVS as `uint8_t` under key `"net_mode"`
- `app_config_t` extended with `network_mode` field (last member);
  default `NETWORK_MODE_WIFI` ensures backward compatibility on all
  existing WiFi boards when key is absent from NVS
- `config_load()` / `config_save()` updated: load validates values
  `> NET_MODE_MAX (2)` — replace with default; save validates using
  `(unsigned)` cast to prevent silent truncation of out-of-range enum values
- 5 new Unity host tests (Group F) in both `test/components/nvs_config/` and
  `test/native/test_nvs_config/`: save-invalid, load-wifi, load-ethernet,
  load-invalid-uses-default, save-large-value-rejected (value 258 pins
  the `(unsigned)` cast behaviour)
- Test counts: 39 tests in `nvs_config` suite, 57 total across all suites

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
  - test/test_custom_runner.py — custom runner inheriting UnityTestRunner;
    sets EXTRA_LIB_DEPS = None to prevent throwtheswitch/Unity injection
    (would conflict with third-party/unity/); configure_build_env() compiles
    third-party/unity/unity.c plus correct component sources and mock stubs
    per suite via BuildSources
  - test/native/test_nvs_config/, test/native/test_app_event/,
    test/native/test_power_manager/ — PlatformIO suite directories (copies
    of canonical sources in test/components/; kept in sync manually)
  - test/README updated: documents both pio test -e native and
    make -f test/Makefile invocation methods
- Fix: test/Makefile SRCS_NVS was missing mock_esp_log_counters.c
  (pre-existing bug exposed by GCC 16 / MinGW-w64 16.1.0)
- Fix: test/mocks/esp_err.h missing trailing newline (GCC 16 warning with -Werror)


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
