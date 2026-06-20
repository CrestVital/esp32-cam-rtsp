# esp32-cam-rtsp — Codex / AI Agent Project Context

## Project Overview

ESP32 firmware for devices equipped with OV-based cameras (OV2640, OV5640 and
compatible sensors). Captures video frames over DVP/MIPI, encodes them, and
streams via RTSP to the CrestVital platform edge pipeline.

**Role in CrestVital platform:** This firmware runs on the camera nodes and feeds
raw video into `crestvital-edge`, which publishes `segment.ready` and
`roi_clip.ready` events to Kafka.

---

## Tech Stack

- **Language:** C (ESP-IDF conventions)
- **Build:** PlatformIO (`platformio.ini`) + ESP-IDF (`framework = espidf`)
- **Board:** LilyGo T-Display S3 (ESP32-S3, 240 MHz, 16 MB Flash, OPI PSRAM)
- **Camera interface:** DVP via LCDCAM peripheral (OV2640/OV5640)
- **CI:** GitHub Actions (`.github/workflows/`)
- **Jira:** crestvital.atlassian.net — project key `MVP`

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
│   └── partitions_ota.csv   # Flash layout
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

**No AI tool — Claude, Codex, GPT-5, Gemma, DeepSeek, Cursor, Copilot, or any
other — may ever run `git commit`, `git push`, `git merge`, `git rebase`,
`git tag`, or any command that creates or modifies git history.**

All commits are created exclusively by the repository owner, after manual review
of every changed file.

Agents may run `git add` (stage files). Nothing else that touches git history.

---

- Unit tests are mandatory for every new component: `test/components/<name>/test_<name>.c`
  must exist and pass `make -f test/Makefile` before the ticket is considered done.

## Jira

- Instance: `crestvital.atlassian.net`
- All Jira text (summaries, descriptions, comments): **Turkish**
- Reports to Jira tickets: **Russian**
- Code, comments, log messages: **English**
