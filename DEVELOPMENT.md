# Development Plan — esp32-cam-rtsp

## Purpose

ESP32 firmware for OV-based camera devices. Streams live video via RTSP
to the CrestVital edge pipeline.

This firmware is a **leaf node** in the CrestVital platform — it has no
dependencies on other CrestVital repos, but all inference and analytics
depend on the quality of the video stream it produces.

---

## Development Order

Components must be implemented in dependency order:

```text
1. wifi_manager        — network is a prerequisite for everything
2. camera_driver       — frame capture; prerequisite for RTSP
3. rtsp_server         — streams frames; core deliverable
4. ota_manager         — enables remote firmware updates
5. status_led          — heartbeat and error signalling
```

---

## Phase 1 — MVP (current focus)

### 1.1 wifi_manager component

WiFi connection management: connect on boot, reconnect on drop, store
credentials in NVS.

| Task | Ticket | Status |
|------|--------|--------|
| Connect to AP from NVS credentials | ESPCAMFW-12 | To Do |
| Reconnection loop with exponential backoff | ESPCAMFW-12 | To Do |
| Provisioning mode (SoftAP + HTTP config page) | — | Post-MVP |

**Acceptance criteria:**
- Device connects to AP within 10 s of boot
- Reconnects automatically after WiFi drop within 30 s
- Credentials persisted across reboots via NVS

### 1.2 camera_driver component

OV sensor initialisation over DVP, PSRAM frame buffer management,
frame-ready ISR.

| Task | Ticket | Status |
|------|--------|--------|
| DVP init for OV2640 at 320×240 JPEG | — | To Do |
| OV5640 support | — | To Do |
| PSRAM frame buffer pool (3 buffers) | — | To Do |
| Frame-ready ISR → FreeRTOS queue | — | To Do |

**Acceptance criteria:**
- `camera_init()` returns `ESP_OK` with OV2640 attached
- 3 frame buffers allocated from PSRAM (MALLOC_CAP_SPIRAM)
- Frame rate ≥ 15 fps at 320×240 JPEG

### 1.3 rtsp_server component

Minimal RTSP/RTP stack. No third-party library.

Encoding format decision: see [ADR-001](docs/adr/ADR-001-mjpeg-vs-h264.md).

| Task | Ticket | Status |
|------|--------|--------|
| RTSP TCP control connection (:8554) | — | To Do |
| OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN | — | To Do |
| SDP generation | — | To Do |
| RTP packetiser (MJPEG over RTP, RFC 2435) | — | To Do |
| UDP transport | — | To Do |
| TCP interleaved transport | — | To Do |

**Acceptance criteria:**
- An RTSP client can open a session and receive frames
- Stream stable for ≥ 1 hour without restart
- At least 1 concurrent client supported

### 1.4 ota_manager component

OTA firmware update triggered via HTTPS POST.

| Task | Ticket | Status |
|------|--------|--------|
| HTTPS OTA download to `ota_1` slot | — | To Do |
| Verify image and set boot partition | — | To Do |
| Auto-rollback on crash before validation | — | To Do |

**Acceptance criteria:**
- Device updates firmware without physical access
- Failed update rolls back to previous slot automatically

### 1.5 status_led component

RGB LED heartbeat and error code signalling.

| Task | Ticket | Status |
|------|--------|--------|
| Heartbeat blink (1 Hz, green) | — | To Do |
| WiFi connecting (blue, fast blink) | — | To Do |
| Error codes (red, N blinks) | — | To Do |

---

## Phase 2 — Post-MVP

- WiFi provisioning via SoftAP + captive portal
- Multi-client RTSP (up to 4 concurrent viewers)
- H.264 encoding (hardware accelerated via ESP32-S3 if feasible — see ADR-001)
- MQTT status reporting (`camera.status` event to edge pipeline)
- LittleFS rolling log with timestamps
- Display output (ST7789): IP address, stream URL, frame rate, heap free

---

## Dependencies

No CrestVital repo dependencies.

External ESP-IDF components (managed via `idf_component.yml` when added):
- `espressif/esp_codec_dev` — potential H.264 helper (Phase 2, TBD)

---

## Board Abstraction

The firmware supports multiple ESP32-based boards. Board-specific pin assignments
and capability flags are isolated in `boards/<name>.h` data files (pure `#define`
macros, no logic).

`include/board.h` includes the selected board file via `#include CONFIG_BOARD_DATA_FILE`,
where `CONFIG_BOARD_DATA_FILE` is a Kconfig string option set in
`components/board_config/Kconfig.projbuild`. This is Variant B of the board abstraction
strategy — see `docs/adr/ADR-004-board-abstraction.md`.

### How to add a board

1. Create `boards/<new_board_name>.h` with all required `#define` macros
   (capability flags + all `BOARD_CAM_PIN_*`). Copy an existing board file as a template.
2. Add an entry in `components/board_config/Kconfig.projbuild`:
   - One `config BOARD_<NEW_BOARD>` bool in the `choice` block.
   - One `default "boards/<new_board_name>.h" if BOARD_<NEW_BOARD>` line in
     `config BOARD_DATA_FILE`.
3. Add a PlatformIO environment in `platformio.ini` and a
   `sdkconfig.defaults.<new-board>` fragment.
4. That is all. No changes to `include/board.h` or any `.c` file are required.

If the board data file is missing any required macro, the build fails with a
`#error` message naming the missing macro.

### Supported boards

| PlatformIO env | Board | MCU | Camera | Network | PSRAM |
|---|---|---|---|---|---|
| `lilygo-t-camera-plus` | LilyGo T-Camera Plus | ESP32-D0WDQ6-V3 | OV2640 | WiFi | 8 MB quad |
| `ai-thinker-esp32-cam` | AI Thinker ESP32-CAM | ESP32 | OV2640 | WiFi | 4 MB |
| `olimex-esp32-poe` | Olimex ESP32-POE | ESP32 | OV2640 | Ethernet | none |

All boards use the DVP (parallel 8-bit) camera interface. MIPI CSI-2 is not
supported on these ESP32 variants without an external ISP.

## Sensor Registry

The firmware supports multiple OV-based camera sensors. Sensor-specific capability
data (interface type, resolution, ISP requirement, driver availability) is isolated
in `sensors/<name>.h` data files (pure `#define` macros, no logic).

`include/sensor_caps.h` includes the selected sensor file via `#include CONFIG_SENSOR_DATA_FILE`,
where `CONFIG_SENSOR_DATA_FILE` is a Kconfig string option set in
`components/sensor_registry/Kconfig.projbuild`. This mirrors the data-driven board
abstraction (Variant B — see `docs/adr/ADR-004-board-abstraction.md` and
`docs/adr/ADR-006-sensor-registry.md`).

### How to add a sensor

1. Create `sensors/<new_sensor>.h` with all required `#define` macros
   (SENSOR_NAME, SENSOR_VENDOR, SENSOR_IFACE, SENSOR_REQUIRES_ISP,
   SENSOR_MAX_WIDTH, SENSOR_MAX_HEIGHT, SENSOR_MAX_FPS, SENSOR_HAS_DRIVER).
   Copy an existing sensor file as a template.
2. Add entries in `components/sensor_registry/Kconfig.projbuild`:
   - One `config SENSOR_<NEW_SENSOR>` bool in the `choice SENSOR_TARGET` block.
   - One `default "sensors/<new_sensor>.h" if SENSOR_<NEW_SENSOR>` line in
     `config SENSOR_DATA_FILE`.
3. Set the sensor selection in each board's `sdkconfig.defaults.<board>` fragment
   (e.g. `CONFIG_SENSOR_<NEW_SENSOR>=y`).
4. That is all. No changes to `include/sensor_caps.h` or any `.c` file are required.

If the sensor data file is missing any required macro, the build fails with a
`#error` message naming the missing macro.

The registry classifies the camera interface as DVP (parallel 8-bit) or
MIPI CSI-2. Building a MIPI CSI-2 or ISP-requiring sensor on a non-MIPI/non-ISP
board fails at compile time with a `#error` referencing epic ESPCAMFW-49 and
ADR-007. This ensures the hardware boundary is enforced before any code reaches
the device.

### Supported sensors

| Kconfig choice | Sensor | Interface | Max Resolution | Max FPS | ISP Required |
|---|---|---|---|---|---|
| `SENSOR_OV2640` | OV2640 (OmniVision) | DVP | 1600×1200 | 60 | No |
| `SENSOR_OV5640` | OV5640 (OmniVision) | DVP | 2592×1944 | 60 | No |

---

## Open Questions

- [ ] **ADR-001** — MJPEG vs H.264: decision needed before implementing `rtp_packetizer`
- [ ] Target frame rate and resolution — confirm with edge pipeline team
- [ ] OTA trigger protocol — HTTPS push from edge vs pull on schedule
- [ ] Number of concurrent RTSP clients required for MVP

---

## Definition of Done (Phase 1)

- [ ] Device boots, connects to WiFi, and streams RTSP within 30 s
- [ ] Edge pipeline successfully receives and segments the stream
- [ ] OTA update works end-to-end
- [ ] All components build with zero errors and zero warnings
- [ ] All public APIs documented with Doxygen comments
- [ ] `v0.1.0` tag created and GitHub Release published with `.bin` artifact
