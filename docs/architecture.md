# Architecture — esp32-cam-rtsp

## Role in the CrestVital Platform

`esp32-cam-rtsp` is the **camera edge firmware**. It runs on LilyGo T-Display S3
devices mounted above livestock pens and feeds raw video into the CrestVital edge pipeline.

```text
esp32-cam-rtsp  (this repo — runs on device)
        |
        | RTSP stream (TCP/UDP over WiFi)
        v
CrestVital edge pipeline — pulls RTSP, segments clips, publishes events
        |
        +---> inference  (Re-ID, disease detection, activity)
        +---> API        (stores events, sends alerts)
```

This firmware has **no dependencies** on other CrestVital repos. All inference
and analytics depend on the quality and continuity of the video stream it produces.

---

## Hardware

| Property | Value |
|----------|-------|
| MCU | ESP32-S3, dual-core Xtensa LX7, 240 MHz |
| Flash | 16 MB, QIO, 80 MHz |
| PSRAM | OPI PSRAM, 8 MB, Octal 80 MHz |
| Camera | OV2640 / OV5640 via DVP (LCDCAM peripheral) |
| Display | ST7789, 1.9" 170×320 (secondary) |
| Connectivity | 802.11 b/g/n WiFi (2.4 GHz) |

---

## Firmware Component Architecture

```text
app_main()  [core 0, priority 1, stack 8192]
    |
    +-- wifi_manager        [priority 8]   Connect, reconnect, NVS credentials
    |
    +-- camera_driver       [priority 16]  DVP init, PSRAM frame buffers, ISR
    |       |
    |       +-- frame_queue  [FreeRTOS queue, depth 3]
    |
    +-- rtsp_server         [priority 12]  RTSP/RTP stack, SDP, TCP :8554
    |       |
    |       +-- rtp_packetizer             Frames → RTP packets → UDP/TCP
    |
    +-- ota_manager         [priority 6]   HTTPS OTA, dual-slot, rollback
    |
    +-- status_led          [priority 4]   Heartbeat, error codes on RGB LED
```

### Component Responsibilities

**wifi_manager** — Connects to AP using credentials stored in NVS. Runs a
reconnection loop with exponential backoff on drop. Signals readiness via an
event group so dependent components wait before starting.

**camera_driver** — Initialises the OV sensor via the ESP32-S3 LCDCAM
peripheral (DVP interface). Allocates a pool of 3 DMA frame buffers in PSRAM.
The frame-ready ISR posts buffer pointers to `frame_queue`. Consumers call
`camera_frame_get()` / `camera_frame_return()` to borrow and release buffers.

**rtsp_server** — Listens on TCP :8554. Handles one RTSP session (Phase 1).
On PLAY, starts `rtp_send_task` which drains `frame_queue` and packetises
frames into RTP packets. Encoding format: see ADR-001.

**ota_manager** — Waits for an HTTPS POST carrying a firmware binary.
firmware binary. Downloads to the inactive OTA slot, verifies, sets boot
partition, restarts. Calls `esp_ota_mark_app_valid_cancel_rollback()` after
successful boot; bootloader rolls back to the previous slot on crash.

**status_led** — Drives the RGB LED to signal device state: green heartbeat
(running), blue fast blink (connecting), red N-blink (error code).

---

## Data Flow

```text
OV Camera (DVP, up to 20 fps)
    │
    │  DMA transfer — LCDCAM peripheral
    ▼
frame_buffer[0..2]  (PSRAM, 64-byte aligned, ~20–40 KB each)
    │
    │  frame_queue (FreeRTOS queue, depth 3)
    ▼
rtp_send_task
    │  RTP packetisation (RFC 2435 MJPEG or RFC 6184 H.264 — see ADR-001)
    ▼
UDP socket  ──►  WiFi  ──►  CrestVital edge pipeline (RTSP client)
```

---

## Memory Layout

### Internal SRAM (~512 KB usable)

| Region | Content |
|--------|---------|
| FreeRTOS heap | Task stacks, queues, semaphores, small allocations |
| DMA descriptors | LCDCAM DMA linked-list entries |
| WiFi/LwIP buffers | Allocated by ESP-IDF WiFi stack internally |

### OPI PSRAM (8 MB)

| Region | Content |
|--------|---------|
| Frame buffer pool | 3 × ~40 KB = ~120 KB (64-byte aligned, DMA-capable) |
| RTP send buffer | ~64 KB (one RTP packet assembly buffer per session) |
| LwIP pbuf pool | Extended by `CONFIG_SPIRAM_USE_MALLOC` for large payloads |
| Future: H.264 encoder scratch | ~500 KB (Phase 2, if H.264 chosen in ADR-001) |

### Flash (16 MB)

```text
0x009000  nvs        24 KB    WiFi credentials, device config
0x00F000  otadata     8 KB    OTA slot selector
0x020000  ota_0    3968 KB    Active firmware
0x400000  ota_1    3968 KB    OTA staging slot
0x7E0000  littlefs    8 MB    Rolling logs, config files
```

---

## FreeRTOS Task Map

| Task | Core | Priority | Stack | Notes |
|------|------|----------|-------|-------|
| `app_main` | 0 | 1 | 8192 B | Boot sequence, then suspends |
| `wifi_manager_task` | 0 | 8 | 4096 B | Reconnect loop |
| `camera_capture_task` | 1 | 16 | 4096 B | Drains ISR queue, posts to frame_queue |
| `rtp_send_task` | 1 | 12 | 4096 B | Packetises and sends RTP; spawned on PLAY |
| `ota_manager_task` | 0 | 6 | 8192 B | HTTPS download, large stack for TLS |
| `status_led_task` | 0 | 4 | 2048 B | LED state machine |

Camera capture and RTP send run on **core 1** to avoid contention with WiFi
stack (core 0). All other tasks run on core 0.

---

## RTSP Session Lifecycle

```text
Client (RTSP client)                      Device (rtsp_server)
        │                                 │
        │── OPTIONS ──────────────────►  │
        │◄─ 200 OK ──────────────────── │
        │                                 │
        │── DESCRIBE ────────────────►  │
        │◄─ 200 OK + SDP ────────────── │  (SDP describes one video track)
        │                                 │
        │── SETUP ───────────────────►  │
        │◄─ 200 OK + session/transport ─ │
        │                                 │
        │── PLAY ────────────────────►  │
        │◄─ 200 OK ──────────────────── │
        │                                 │
        │◄═ RTP packets (video) ════════ │  (continuous stream)
        │                                 │
        │── TEARDOWN ────────────────►  │
        │◄─ 200 OK ──────────────────── │
```

---

## OTA Update Flow

```text
OTA trigger
    │  HTTPS POST /ota  (firmware binary)
    ▼
ota_manager_task
    │  esp_ota_begin(ota_1)
    │  [write chunks in loop]
    │  esp_ota_end()
    │  esp_ota_set_boot_partition(ota_1)
    │  esp_restart()
    ▼
[boot from ota_1]
    │  [run self-tests / wait for WiFi]
    │  esp_ota_mark_app_valid_cancel_rollback()
    ▼
[ota_1 is now the active slot]
```

If the device crashes before calling `mark_app_valid`, the bootloader
automatically reverts to `ota_0` on the next boot.

---

## Architecture Decision Records

See `docs/adr/` for individual ADR documents.

| ADR | Title | Status |
|-----|-------|--------|
| [ADR-001](adr/ADR-001-mjpeg-vs-h264.md) | MJPEG vs H.264 for RTP stream | Accepted |
