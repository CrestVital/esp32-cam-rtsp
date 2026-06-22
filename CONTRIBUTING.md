# Contributing to esp32-cam-rtsp

## Table of Contents

1. [Getting Started](#1-getting-started)
2. [Development Environment](#2-development-environment)
3. [Repository Structure](#3-repository-structure)
4. [Coding Conventions](#4-coding-conventions)
5. [Commit Message Format](#5-commit-message-format)
6. [Branch Naming](#6-branch-naming)
7. [Pre-PR Checklist](#7-pre-pr-checklist)
8. [Pull Request Process](#8-pull-request-process)
9. [AI-Assisted Development](#9-ai-assisted-development)

---

## 1. Getting Started

1. Clone the repository:

   ```bash
   git clone https://github.com/CrestVital/esp32-cam-rtsp.git
   cd esp32-cam-rtsp
   ```

2. Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html).

3. Verify the build:

   ```powershell
   pio run -e lilygo-t-display-s3
   ```

4. Connect the device and flash:

   ```powershell
   pio run -e lilygo-t-display-s3 --target upload
   pio device monitor
   ```

---

## 2. Development Environment

| Tool | Version | Purpose |
|------|---------|---------|
| PlatformIO Core | latest | Build, upload, monitor |
| espressif32 platform | 7.0.0 | ESP-IDF toolchain via PlatformIO |
| ESP-IDF | bundled with platform | Framework |
| clang-format | system | Code formatter (optional, run manually) |

**Framework is ESP-IDF, not Arduino.** Do not use Arduino APIs (`Serial`, `delay`, `Wire`, etc.).

---

## 3. Repository Structure

```text
esp32-cam-rtsp/
├── src/
│   ├── main.c               # Entry point — app_main()
│   └── CMakeLists.txt       # ESP-IDF component registration
├── components/              # Private ESP-IDF components (one subdir each)
├── include/                 # Shared headers
├── test/                    # Unity-based unit tests
├── partitions/
│   └── partitions_ota.csv   # Flash layout
├── docs/
│   └── adr/                 # Architecture Decision Records
├── scripts/
│   └── pre-pr.ps1           # Pre-PR check script
├── .agent/                  # AI agent configuration (committed, not gitignored)
├── .github/
│   └── workflows/           # GitHub Actions CI
├── platformio.ini
├── sdkconfig.defaults
└── CMakeLists.txt
```

---

## 4. Coding Conventions

See `.agent/CODING_GUIDELINES.md` for the full specification. Key rules:

- **C only** — no C++, no Arduino API
- `ESP_ERROR_CHECK()` or explicit `esp_err_t` check on **every** ESP-IDF API call
- All buffers > 8 KB allocated from PSRAM: `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
- FreeRTOS task priorities: **1–24** only
- No blocking calls inside ISRs; ISR functions marked `IRAM_ATTR`
- All log output via `ESP_LOG*` macros — no `printf()`
- Log tag: `static const char *TAG = "module_name";` per source file
- All public functions documented with Doxygen `@brief` / `@param` / `@return`

---

## 5. Commit Message Format

This project uses **Conventional Commits**:

```text
<type>: <short description> #ESPCAMFW-<N>

[optional body]
```

Types: `feat`, `fix`, `test`, `docs`, `refactor`, `chore`

Examples:

```text
feat: add camera_init with PSRAM frame buffer allocation #ESPCAMFW-101
fix: correct DVP pixel clock divider for OV5640 at 20 fps #ESPCAMFW-108
docs: add ADR for MJPEG vs H.264 encoding choice #ESPCAMFW-112
```

> **Commits are created exclusively by the repository owner.**
> AI tools must never run `git commit` or `git push`.
> See `.agent/CODING_GUIDELINES.md` for the full rule.

---

## 6. Branch Naming

| Type | Pattern | Example |
|------|---------|---------|
| Feature | `feature/ESPCAMFW-<N>-<desc>` | `feature/ESPCAMFW-101-camera-init` |
| Bug fix | `fix/ESPCAMFW-<N>-<desc>` | `fix/ESPCAMFW-108-dvp-clock` |
| Docs | `docs/ESPCAMFW-<N>-<desc>` | `docs/ESPCAMFW-112-adr-encoding` |
| Chore | `chore/<desc>` | `chore/update-platformio-platform` |

---

## 7. Pre-PR Checklist

Before opening a pull request, run the pre-PR script:

```powershell
.\scripts\pre-pr.ps1
```

The script checks:

- Not on `main` branch
- `pio run -e lilygo-t-display-s3` builds with zero errors and zero warnings
- `CHANGELOG.md` updated

Fix all reported issues before opening the PR.

---

## 8. Pull Request Process

1. Run `.\scripts\pre-pr.ps1` and fix all issues.
2. Push your branch and open a PR on GitHub.
3. Fill in the PR description: what changed and why, link the Jira ticket.
4. At least one review approval is required before merge.
5. Update `CHANGELOG.md` under `[Unreleased]` for every user-visible change.
6. Squash or rebase before merge to keep `main` history linear.

---

## 9. AI-Assisted Development

AI coding agents are used for implementation and code review on this project.

**Critical rule:** AI tools must never commit. All commits are created by the
repository owner after manual review of every changed file.

Agent configuration files are in `.agent/` (committed to git, never gitignored):

| File | Purpose |
|------|---------|
| `CODING_GUIDELINES.md` | Rules for all coding agents |
| `REVIEW_GUIDELINES.md` | Rules for code review agents |
| `agents/` | Per-agent configuration files |
| `skills/` | Detailed project knowledge for agents |
