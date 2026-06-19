# Changelog

All notable changes to this project will be documented in this file.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
Versioning: [Semantic Versioning](https://semver.org/)

## [Unreleased]

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
- `.agent/agents/gemma4-26b.md` — initial Gemma agent context
