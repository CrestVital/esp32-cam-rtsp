# 03 — Architecture Decisions

## Flash Partition Layout

```text
Address     Name        Size     Purpose
0x009000    nvs          24 KB   WiFi credentials, device config (NVS key-value store)
0x00F000    otadata       8 KB   OTA slot selector (managed by esp_ota API)
0x020000    app0/ota_0 3968 KB   Active firmware slot
0x400000    app1/ota_1 3968 KB   OTA staging slot
0x7E0000    littlefs     8 MB    Rolling logs, config files (mounted at /littlefs)
```

**Why two OTA slots:** Enables safe over-the-air updates triggered remotely.
OTA via HTTPS. On failure, the device boots from the previous slot automatically.

**Why LittleFS (not FAT or SPIFFS):** Power-loss safe. SPIFFS is deprecated in
ESP-IDF 5.x. LittleFS handles fragmentation better for rolling log files.

---

## Memory Architecture

The ESP32-S3 has two memory regions:

| Region | Size | Access | Use |
|--------|------|--------|-----|
| Internal SRAM | ~512 KB usable | Fast, DMA-capable | Stack, FreeRTOS heap, DMA descriptors |
| OPI PSRAM | 8 MB | Slower (via SPI) | Frame buffers, network buffers, large data |

**Rule:** Allocate all buffers > 8 KB from PSRAM. Internal SRAM is precious.
Frame buffers are typically 15–40 KB each (320×240 JPEG). Always use:
```c
heap_caps_malloc(size, MALLOC_CAP_SPIRAM)
```

DMA-mapped buffers (camera DMA descriptors, DMA receive buffers) need:
```c
heap_caps_aligned_alloc(64, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA)
```

---

## Camera Interface

The ESP32-S3's **LCDCAM peripheral** drives the DVP (Digital Video Port) interface
to OV-series sensors. Key points:

- Configured via `esp_cam_ctlr_dvp_config_t` (ESP-IDF 5.x Camera Controller API)
- The ISR that signals frame-ready must be `IRAM_ATTR` and cache-safe
  (`CONFIG_CAM_CTLR_DVP_CAM_ISR_CACHE_SAFE=y` in sdkconfig.defaults)
- Frame buffers must be in PSRAM and 64-byte aligned for DMA

---

## FreeRTOS Task Priority Map

```text
Priority 24  (reserved — idle hook level)
Priority 16  camera_capture_task   — highest user priority; latency-sensitive
Priority 12  rtp_send_task          — RTSP packetisation and UDP send
Priority  8  wifi_manager_task      — reconnect loop, DHCP
Priority  6  ota_manager_task       — HTTPS OTA download
Priority  4  status_led_task        — heartbeat blink, error codes
Priority  1  (minimum user task)
```

---

## RTSP Architecture

The firmware implements a **minimal RTSP/RTP stack** (no third-party library):

```text
rtsp_server
    |-- RTSP control connection (TCP :8554)   — OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN
    |-- RTP data stream (UDP or TCP interleaved)  — video frames as RTP packets
    |
    camera_driver --> frame_queue --> rtp_packetizer --> UDP/TCP send
```

SDP describes a single video track (H.264 or MJPEG, TBD per ADR).

---

## OTA Update Flow

```text
OTA trigger  --HTTPS POST-->  ota_manager_task
                                       |
                               esp_ota_begin()
                               [download & write chunks to ota_1]
                               esp_ota_end()
                               esp_ota_set_boot_partition(ota_1)
                               esp_restart()
                                       |
                               [boot from ota_1]
                               esp_ota_mark_app_valid_cancel_rollback()
```

If the new firmware crashes before marking itself valid, the bootloader rolls
back to `ota_0` automatically.

---

## Architecture Decision Records

See `docs/adr/` for individual ADR documents.
