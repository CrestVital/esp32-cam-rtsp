# ADR-001: MJPEG vs H.264 for RTP Stream

**Date:** 2026-06-20
**Status:** Accepted
**Ticket:** ESPCAMFW-12

## Context

The `rtp_packetizer` component must encode camera frames into an RTP payload
before transmission. The choice of encoding format affects: implementation
complexity, CPU load on the ESP32-S3, latency, bandwidth, and compatibility
with `crestvital-edge`'s RTSP client (FFmpeg-based).

Two options were evaluated:

**Option A — MJPEG (Motion JPEG)**
Each frame is an independent JPEG image transmitted as an RTP payload per
RFC 2435. The OV2640/OV5640 sensors produce JPEG output natively in hardware,
so the ESP32-S3 CPU does not need to encode anything.

**Option B — H.264**
Inter-frame compressed video transmitted as RTP payload per RFC 6184. Requires
a software encoder running on the ESP32-S3 CPU. The ESP32-S3 has no hardware
H.264 encoder. Available open-source C encoders (e.g. x264, libopenh264) are
too heavy for the available SRAM/PSRAM budget and CPU headroom at 320×240 15 fps.

## Decision

**Use MJPEG (Option A) for Phase 1 (MVP).**

## Rationale

| Criterion | MJPEG | H.264 |
|-----------|-------|-------|
| CPU cost | Near zero — sensor outputs JPEG | High — software encoder ~80% CPU at 15 fps |
| Implementation complexity | Low — RFC 2435 packetiser, ~200 LOC | High — encoder integration + RFC 6184 packetiser |
| Latency | One frame delay | GOP delay (typically 0.5–2 s with I-frames) |
| Bandwidth at 320×240 15 fps | ~2–4 Mbit/s | ~0.3–0.8 Mbit/s |
| crestvital-edge compatibility | FFmpeg supports MJPEG-over-RTP natively | FFmpeg supports H.264-over-RTP natively |
| PSRAM scratch needed | ~40 KB (one frame buffer) | ~500 KB (encoder working memory) |
| Seek/random access | Every frame is a keyframe | Only on I-frames |

The OV2640/OV5640 JPEG hardware output eliminates the encoding bottleneck
entirely. The bandwidth increase (4× over H.264) is acceptable on a 2.4 GHz
WiFi link — a 320×240 MJPEG stream at 15 fps uses ~3 Mbit/s, well within
the capacity of an 802.11n link at close range.

`crestvital-edge` runs FFmpeg locally and can consume MJPEG-over-RTP without
any additional configuration.

## Consequences

- `rtp_packetizer` implements RFC 2435 (RTP Payload Format for JPEG-compressed Video).
- Each RTP packet carries one JPEG frame (or a fragment if the frame exceeds
  the MTU; typical 320×240 JPEG fits in one packet at ~15–40 KB, so
  fragmentation is rare but must be handled).
- `camera_driver` must configure the OV sensor for JPEG output mode
  (`PIXFORMAT_JPEG`), not raw YUV or RGB.
- Bandwidth budget: plan for up to 4 Mbit/s per camera stream on the WiFi AP.
- **Phase 2 revisit:** If `crestvital-edge` requires lower bandwidth (e.g. for
  4+ cameras on one AP), H.264 encoding should be re-evaluated. At that point,
  consider offloading encoding to `crestvital-edge` by streaming raw YUV over
  RTP, or using an external hardware encoder module.
