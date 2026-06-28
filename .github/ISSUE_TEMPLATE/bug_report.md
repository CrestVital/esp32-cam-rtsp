---
name: Bug Report
about: Report a firmware crash, hang, or incorrect behaviour
title: '[BUG] '
labels: bug
assignees: ''
---

## Description

A clear and concise description of what the bug is.

## Component

Which firmware component is affected?

- [ ] `camera_driver` (DVP init, frame capture, OV sensor)
- [ ] `rtsp_server` (RTSP/RTP streaming, SDP)
- [ ] `wifi_manager` (connection, reconnection, credentials)
- [ ] `ota_manager` (OTA update, rollback)
- [ ] `status_led` (heartbeat, error codes)
- [ ] `main` / boot sequence
- [ ] Build system / CI
- [ ] Other (describe below)

## Steps to Reproduce

1.
2.
3.

## Expected Behaviour

A clear description of what you expected to happen.

## Actual Behaviour

What actually happened? Include UART log output, panic traces, or error messages.

To collect logs from the device:
```bash
pio device monitor   # 115200 baud, esp32_exception_decoder filter active
```

Paste the relevant log section here:
```
[paste UART output here]
```

## Environment

- **Firmware version / git commit:**
- **Board:** LilyGo T-Camera Plus (ESP32-D0WDQ6-V3)
- **Camera sensor:** (OV2640 / OV5640 / other)
- **Host OS:** (e.g. Ubuntu 22.04, Windows 11)
- **PlatformIO version:** (output of `pio --version`)
- **Edge pipeline version:** (if relevant)

## Jira Ticket

<!-- https://crestvital.atlassian.net/browse/ESPCAMFW-XX -->
