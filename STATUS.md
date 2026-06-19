# Status — esp32-cam-rtsp

**Last updated:** 2026-06-19
**Version:** 0.0.1-dev
**Active branch:** `main`

---

## Current State

Repository scaffold complete. Agent context files in place (Фаза 1).
Documentation and workflow infrastructure added (Фаза 2).
No functional firmware components implemented yet.

---

## What's Done

- **[Infra]** ✅ Repository scaffold: `platformio.ini`, `CMakeLists.txt`,
  `src/main.c` (NVS init, main loop placeholder)
- **[Infra]** ✅ `sdkconfig.defaults` — PSRAM (OPI), LCDCAM DVP ISR, WiFi buffers,
  CPU 240 MHz, main task stack 8192
- **[Infra]** ✅ `partitions/partitions_ota.csv` — NVS + OTA dual-slot + LittleFS 8 MB
- **[Фаза 1]** ✅ Agent context: `CLAUDE.md`, `AGENTS.md`
- **[Фаза 1]** ✅ `.agent/CODING_GUIDELINES.md` — C/ESP-IDF conventions, NEVER COMMIT rule
- **[Фаза 1]** ✅ `.agent/REVIEW_GUIDELINES.md` — firmware review checklist
- **[Фаза 1]** ✅ `.agent/skills/` — project overview, repo structure, architecture decisions
- **[Фаза 2]** ✅ `README.md`, `CONTRIBUTING.md`, `CHANGELOG.md`, `STATUS.md`
- **[Фаза 2]** ✅ `scripts/pre-pr.ps1` — pre-PR build + branch + CHANGELOG check

---

## In Progress

Nothing currently in progress.

---

## Blocked / Open Questions

- Video encoding format: MJPEG vs H.264 — decision needed before implementing
  `rtp_packetizer` component (pending ADR)
- OTA trigger mechanism: push from `crestvital-edge` vs pull on schedule — TBD
- Target resolution and frame rate: confirmation needed from inference team

---

## Next Up

- **Фаза 3** — GitHub infrastructure: CI workflow, issue templates, PR template, `.editorconfig`
- **[MVP-???]** `wifi_manager` component — connect, reconnect, NVS credential storage
- **[MVP-???]** `camera_driver` component — DVP init, PSRAM frame buffer allocation, OV2640/OV5640
- **[MVP-???]** `rtsp_server` component — minimal RTSP/RTP stack
