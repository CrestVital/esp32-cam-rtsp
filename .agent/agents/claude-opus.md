# Claude Opus Agent Context

## Project
- Repository: esp32-cam-rtsp
- Target board: LilyGo T-Display S3 (ESP32-S3, dual-core Xtensa LX7, 240 MHz)
- Flash: 16 MB (QIO, 80 MHz)
- PSRAM: OPI PSRAM (Octal, 80 MHz) — mandatory for large/DMA buffers
- Camera interface: DVP via LCDCAM peripheral (OV2640 / OV5640)
- Build system: PlatformIO (`platformio.ini`) + ESP-IDF (`framework = espidf`, platform `espressif32@7.0.0`)
- Language: C (ESP-IDF conventions)
- Jira: crestvital.atlassian.net — project key `ESPCAMFW`

## Mandatory Reading
Before performing any code review, read `.agent/REVIEW_GUIDELINES.md` in the repository root.
It is the authoritative source for review mindset, the full review checklist (10 categories),
severity levels (🔴 CRITICAL / 🟠 MAJOR / 🟡 MINOR / 💬 SUGGESTION), and the required
Markdown report format.

Also read `.agent/CODING_GUIDELINES.md` — the reference against which all reviewed code
is evaluated.

## Conventions (what the code under review must conform to)
- All identifiers, comments, and log messages in English
- `ESP_ERROR_CHECK()` for fatal calls; explicit `esp_err_t` check with `ESP_LOGE` for recoverable errors
- Large/DMA buffers (> 8 KB) from PSRAM: `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
- DMA-mapped buffers: `heap_caps_aligned_alloc(64, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA)`
- FreeRTOS task priorities: 1–24 only
- No blocking calls inside ISRs; ISR functions marked `IRAM_ATTR`
- Log tag per module: `static const char *TAG = "module_name";`
- No `printf()` — `ESP_LOG*` macros only
- `snprintf()` instead of `sprintf()`
- C only — no C++, no Arduino API

## Repository Structure
```
esp32-cam-rtsp/
├── src/
│   ├── main.c               # Entry point — app_main()
│   └── CMakeLists.txt
├── components/              # Private ESP-IDF components
├── include/                 # Shared headers
├── test/                    # Unit tests (Unity framework)
├── partitions/
│   └── partitions_ota.csv
├── .agent/
│   ├── CODING_GUIDELINES.md # Coding standard — read before reviewing
│   ├── REVIEW_GUIDELINES.md # Review protocol — read before reviewing
│   └── agents/
├── platformio.ini
├── sdkconfig.defaults
└── CMakeLists.txt
```

## ⛔ ABSOLUTE RULE — NEVER COMMIT
Never run `git commit`, `git push`, `git merge`, `git rebase`, `git tag`,
or any command that modifies git history. Only `git add` (staging) is permitted.
All commits are made exclusively by the repository owner after manual review.

## Report Output
Save the code review report to `OPUS-{TICKET}-{CYCLE}-CODEREVIEW.md`
in the repository root. Format: Markdown. Language: Russian.
Report structure: follow the template in `.agent/REVIEW_GUIDELINES.md` exactly.
