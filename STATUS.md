# Status — esp32-cam-rtsp

**Last updated:** 2026-06-20
**Version:** 0.0.1-dev
**Active branch:** `main`

---

## Current State

Repository fully documented and ready for first firmware component implementation.
All infrastructure phases complete. ADR-001 accepted — MJPEG chosen for MVP.
Next: `wifi_manager` component (ESPCAMFW-12).

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
- **[Фаза 3]** ✅ `.github/workflows/ci.yml` — PlatformIO build CI
- **[Фаза 3]** ✅ `.github/workflows/release.yml` — release pipeline on v* tags
- **[Фаза 3]** ✅ `.github/PULL_REQUEST_TEMPLATE.md`, `ISSUE_TEMPLATE/`
- **[Фаза 3]** ✅ `.editorconfig`
- **[Docs]** ✅ `DEVELOPMENT.md` — phased plan with component dependency order
- **[Docs]** ✅ `docs/architecture.md` — component layout, task map, memory budget,
  data flow, RTSP lifecycle, OTA flow
- **[Docs]** ✅ `docs/adr/ADR-001-mjpeg-vs-h264.md` — **MJPEG accepted for MVP**

---

## In Progress

Nothing currently in progress.

---

## Blocked / Open Questions

- OTA trigger protocol — HTTPS push from `crestvital-edge` vs pull on schedule — TBD
- Target frame rate and resolution — confirmation needed from `crestvital-edge` team
- Number of concurrent RTSP clients required for MVP

---

## Next Up

- **[ESPCAMFW-12]** `wifi_manager` component — connect, reconnect, NVS credential storage
- **[ESPCAMFW-??]** `camera_driver` component — DVP init, PSRAM frame buffer pool, OV2640/OV5640
- **[ESPCAMFW-??]** `rtsp_server` component — RTSP/RTP stack, RFC 2435 MJPEG packetiser
- **[ESPCAMFW-??]** `ota_manager` component — HTTPS OTA, dual-slot, rollback
