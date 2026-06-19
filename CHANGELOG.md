# Changelog

All notable changes to this project will be documented in this file.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
Versioning: [Semantic Versioning](https://semver.org/)

## [Unreleased]

### Added

- `CLAUDE.md`, `AGENTS.md` — AI agent project context files
- `.agent/CODING_GUIDELINES.md` — C/ESP-IDF coding conventions for AI agents
- `.agent/REVIEW_GUIDELINES.md` — firmware code review checklist for AI agents
- `.agent/skills/` — detailed project knowledge (overview, repo structure, architecture)
- `README.md` — project overview, quick start, hardware table, platform diagram
- `CONTRIBUTING.md` — branch naming, commit format, pre-PR checklist
- `DEVELOPMENT.md` — phased implementation plan
- `STATUS.md` — current development state
- `scripts/pre-pr.ps1` — pre-PR check script (build, branch guard, CHANGELOG check)

---

## [0.0.1] — 2026-06-19

### Added

- Initial repository structure: `platformio.ini`, `CMakeLists.txt`, `src/main.c`
- `sdkconfig.defaults` — PSRAM, LCDCAM DVP, WiFi, CPU 240 MHz, main task stack 8192
- `partitions/partitions_ota.csv` — NVS + OTA dual-slot + LittleFS 8 MB layout
- `.agent/agents/gemma4-26b.md` — initial Gemma agent context
