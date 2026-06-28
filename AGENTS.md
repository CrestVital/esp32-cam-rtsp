# esp32-cam-rtsp — Codex / AI Agent Project Context

## Project Overview

ESP32 firmware for devices equipped with OV-based cameras (OV2640, OV5640 and
compatible sensors). Captures video frames over DVP/MIPI, encodes them, and
streams via RTSP to the CrestVital platform edge pipeline.

**Role in CrestVital platform:** This firmware runs on the camera nodes and feeds
raw video into the CrestVital edge pipeline for further processing.

---

## Tech Stack

- **Language:** C (ESP-IDF conventions)
- **Build:** PlatformIO (`platformio.ini`) + ESP-IDF (`framework = espidf`)
- **Board:** LilyGo T-Camera Plus (ESP32-D0WDQ6-V3, 240 MHz, 4 MB Flash, 8 MB quad PSRAM)
- **Camera interface:** DVP via LCDCAM peripheral (OV2640/OV5640)
- **CI:** GitHub Actions (`.github/workflows/`)
- **Jira:** crestvital.atlassian.net — project key `ESPCAMFW`

---

## Repository Structure

```text
esp32-cam-rtsp/
├── src/
│   ├── main.c               # Entry point — app_main()
│   └── CMakeLists.txt       # ESP-IDF component registration
├── components/              # Private ESP-IDF components
├── include/                 # Shared headers
├── test/                    # Unit tests (Unity framework)
├── partitions/
│   └── partitions_4mb_ota.csv # Flash layout
├── scripts/
│   └── pre-pr.ps1           # Pre-PR check script
├── .agent/
│   ├── CODING_GUIDELINES.md
│   ├── REVIEW_GUIDELINES.md
│   └── agents/
├── platformio.ini
├── sdkconfig.defaults
└── CMakeLists.txt
```

---

## Coding Conventions

Read `.agent/CODING_GUIDELINES.md` before writing any code. Key rules:

- C only — no C++, no Arduino API, no PlatformIO Arduino framework
- `ESP_ERROR_CHECK()` on every ESP-IDF API call without exception
- Large/DMA buffers: `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)` (PSRAM mandatory)
- FreeRTOS task priorities: 1–24 only
- No blocking calls inside ISRs
- All log tags: `static const char *TAG = "module_name";`
- Naming: `snake_case` for everything, `UPPER_SNAKE` for constants

---

## ⛔ ABSOLUTE RULE — NEVER COMMIT

**No AI tool may ever run `git commit`, `git push`, `git merge`, `git rebase`,
`git tag`, or any command that creates or modifies git history.**

All commits are created exclusively by the repository owner, after manual review
of every changed file.

Agents may run `git add` (stage files). Nothing else that touches git history.

---

- Unit tests are mandatory for every new component: `test/components/<name>/test_<name>.c`
  must exist and pass `make -f test/Makefile` before the ticket is considered done.

## Jira

- Instance: `crestvital.atlassian.net`
- Project key: `ESPCAMFW`
- All Jira text (summaries, descriptions, comments): **Turkish**
- Reports to Jira tickets: **Russian**
- Code, comments, log messages: **English**
