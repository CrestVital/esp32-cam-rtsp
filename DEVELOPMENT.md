# Development Plan — esp32-cam-rtsp

## Purpose

ESP32-S3 firmware for OV-based camera devices. Streams live video via RTSP
to `crestvital-edge`, which segments clips and publishes events to Kafka.

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
- `crestvital-edge` can open an RTSP session and receive frames
- Stream stable for ≥ 1 hour without restart
- At least 1 concurrent client supported

### 1.4 ota_manager component

OTA firmware update triggered by `crestvital-edge` via HTTPS POST.

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
- MQTT status reporting (`camera.status` event to `crestvital-edge`)
- LittleFS rolling log with timestamps
- Display output (ST7789): IP address, stream URL, frame rate, heap free

---

## Dependencies

No CrestVital repo dependencies.

External ESP-IDF components (managed via `idf_component.yml` when added):
- `espressif/esp_codec_dev` — potential H.264 helper (Phase 2, TBD)

---

## Open Questions

- [ ] **ADR-001** — MJPEG vs H.264: decision needed before implementing `rtp_packetizer`
- [ ] Target frame rate and resolution — confirm with `crestvital-edge` team
- [ ] OTA trigger protocol — HTTPS push from edge vs pull on schedule
- [ ] Number of concurrent RTSP clients required for MVP

---

## Definition of Done (Phase 1)

- [ ] Device boots, connects to WiFi, and streams RTSP within 30 s
- [ ] `crestvital-edge` successfully receives and segments the stream
- [ ] OTA update works end-to-end from `crestvital-edge`
- [ ] All components build with zero errors and zero warnings
- [ ] All public APIs documented with Doxygen comments
- [ ] `v0.1.0` tag created and GitHub Release published with `.bin` artifact
