# Coding Guidelines — esp32-cam-rtsp

These guidelines apply to ALL coding agents working on this repository.
Read this file before writing a single line of code.

---

## Language and Standard

- **C only.** No C++, no Arduino API, no PlatformIO Arduino framework.
- Target: **C99** minimum. C11 features are allowed if supported by ESP-IDF's toolchain (xtensa-esp32s3-elf-gcc).
- Do not use GCC extensions unless clearly marked with a comment explaining why.

---

## Code Style

- Formatter: **clang-format** (if available). Otherwise follow the style of the existing file.
- Indentation: **4 spaces**. No tabs.
- Line length: **100 characters** max.
- No trailing whitespace.
- One blank line between function definitions.
- Opening brace on the **same line** as the statement (K&R style):
  ```c
  if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Init failed: %s", esp_err_to_name(ret));
      return ret;
  }
  ```

---

## Naming Conventions

| Entity | Convention | Example |
|--------|-----------|---------|
| Functions | `snake_case` | `camera_init()`, `rtsp_server_start()` |
| Variables | `snake_case` | `frame_count`, `buf_ptr` |
| Constants / macros | `UPPER_SNAKE` | `FRAME_BUF_COUNT`, `RTSP_PORT` |
| Types (typedef) | `snake_case_t` | `camera_config_t`, `rtsp_handle_t` |
| Struct members | `snake_case` | `.frame_size`, `.quality` |
| Log tag | `static const char *TAG` | `static const char *TAG = "camera";` |
| Component directory | `snake_case` | `components/rtsp_server/` |

---

## ESP-IDF Error Handling

Every ESP-IDF API call that returns `esp_err_t` **must** be checked. No exceptions.

**Preferred forms:**

```c
/* Fatal errors — use ESP_ERROR_CHECK(); aborts on failure */
ESP_ERROR_CHECK(nvs_flash_init());
ESP_ERROR_CHECK(esp_wifi_start());

/* Non-fatal / recoverable errors — check explicitly */
esp_err_t ret = camera_init(&config);
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Camera init failed: %s", esp_err_to_name(ret));
    return ret;
}
```

**Never** ignore a return value silently:
```c
/* WRONG */
nvs_flash_init();

/* WRONG */
(void)esp_wifi_start();
```

---

## Memory Allocation

### PSRAM (mandatory for large/DMA buffers)

Frame buffers, network buffers, RTSP packet buffers — anything above ~8 KB:

```c
uint8_t *frame_buf = heap_caps_malloc(FRAME_BUF_SIZE, MALLOC_CAP_SPIRAM);
if (frame_buf == NULL) {
    ESP_LOGE(TAG, "Failed to allocate frame buffer (%u bytes)", FRAME_BUF_SIZE);
    return ESP_ERR_NO_MEM;
}
```

DMA-capable buffers (requires both PSRAM and internal DMA alignment):

```c
uint8_t *dma_buf = heap_caps_aligned_alloc(
    64,                                      /* alignment in bytes */
    DMA_BUF_SIZE,
    MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA
);
```

### Internal heap

Small control structures, configuration objects — `malloc()` / `calloc()` is fine.

### Rules

- Always check the return value of every allocation.
- Always free allocations on the error path.
- Never allocate frame buffers or camera DMA descriptors from the internal heap — they will fragment it.

---

## FreeRTOS

### Task priorities

Use values between **1 and 24** (`configMAX_PRIORITIES - 1 = 25` is reserved for the idle task hook).

| Priority range | Use |
|----------------|-----|
| 1–5 | Background tasks (logging, status LED) |
| 6–10 | Network tasks (WiFi, RTSP server, OTA) |
| 11–16 | Camera capture and frame processing |
| 17–20 | (Reserved for future real-time tasks) |
| 21–24 | (Reserved — do not use without explicit justification) |

### Stack sizes

Always specify stack sizes explicitly in bytes. Use `configMINIMAL_STACK_SIZE`
only as a floor for very simple tasks. RTSP-related tasks need at least 4096 bytes.

```c
xTaskCreate(rtsp_task, "rtsp", 4096, NULL, 8, NULL);
```

### ISR rules

- **No blocking calls inside ISRs** — no `vTaskDelay`, no `xQueueSend`, no `malloc`.
- Use `xQueueSendFromISR()` / `xSemaphoreGiveFromISR()` and yield if needed:
  ```c
  BaseType_t higher_priority_woken = pdFALSE;
  xQueueSendFromISR(frame_queue, &frame, &higher_priority_woken);
  portYIELD_FROM_ISR(higher_priority_woken);
  ```
- Mark ISR functions with `IRAM_ATTR`:
  ```c
  static void IRAM_ATTR camera_frame_isr(void *arg) { ... }
  ```

---

## Logging

- Always define a module-level log tag:
  ```c
  static const char *TAG = "rtsp_server";
  ```
- Use the correct log level:
  - `ESP_LOGE` — errors that prevent operation
  - `ESP_LOGW` — warnings (recoverable issues, retries)
  - `ESP_LOGI` — key lifecycle events (init, connect, disconnect)
  - `ESP_LOGD` — debug info (verbose, disabled in production build)
  - `ESP_LOGV` — verbose trace (disabled in production build)
- No `printf()`. Use `ESP_LOG*` macros exclusively.

---

## Component Structure

Each ESP-IDF component in `components/` must have:

```text
components/my_component/
├── CMakeLists.txt       idf_component_register(SRCS ... INCLUDE_DIRS ... REQUIRES ...)
├── include/
│   └── my_component.h  Public API header — all public types and functions declared here
└── my_component.c      Implementation
```

Public header rules:
- Include guard: `#pragma once` (preferred over `#ifndef` guards)
- Document every public function with a Doxygen comment block

---

## Doxygen Comments

Required on every public function and type:

```c
/**
 * @brief Initialise the OV camera driver.
 *
 * Must be called after nvs_flash_init() and before starting the RTSP server.
 * Allocates frame buffers from PSRAM.
 *
 * @param config  Pointer to camera configuration (resolution, pixel format, quality).
 * @return        ESP_OK on success, ESP_ERR_NO_MEM if PSRAM allocation fails,
 *                ESP_ERR_NOT_FOUND if the sensor is not detected.
 */
esp_err_t camera_init(const camera_config_t *config);
```

---

## Branching and Commits

- Branch: `feature/MVP-{ticket_number}-{short-description}`
  - Example: `feature/MVP-101-camera-init`
- Commit format (Conventional Commits):
  ```
  {type}: {short description} #{ticket}
  ```
  Types: `feat`, `fix`, `test`, `docs`, `refactor`, `chore`
  - Example: `feat: add camera_init with PSRAM frame buffer allocation #MVP-101`
- One logical change per commit. Do not batch unrelated changes.

---

## ⛔ ABSOLUTE RULE — AI AGENTS NEVER COMMIT

**No AI tool — Claude, Codex, GPT-5, Gemma, DeepSeek, Cursor, Copilot, or any
other — may ever run `git commit`, `git push`, `git merge`, `git rebase`,
`git tag`, or any command that creates or modifies git history.**

All commits are created exclusively by the repository owner, after manual review
of every changed file.

Agents may stage files with `git add`. Nothing else.

---

## What NOT to Do

- **NEVER run `git commit`, `git push`, or any git command that writes to history.**
- Do not use C++ (`std::`, `new`, `delete`, `class`, `template`, etc.).
- Do not call Arduino APIs (`Serial.begin()`, `delay()`, `Wire.begin()`, etc.).
- Do not allocate frame buffers or DMA descriptors from the internal heap.
- Do not call blocking functions from ISR context.
- Do not use `printf()` — use `ESP_LOG*` macros.
- Do not leave `TODO` comments without a linked Jira ticket number.
- Do not modify files outside the scope of the current ticket.
- Do not change `sdkconfig.defaults` unless the ticket explicitly requires it.

---

## Definition of Done (applies to all tickets)

- [ ] All new functions documented with Doxygen comments
- [ ] All `esp_err_t` return values checked
- [ ] No allocations from internal heap for buffers > 8 KB
- [ ] ISR functions marked `IRAM_ATTR`, no blocking calls inside ISRs
- [ ] `pio run -e lilygo-t-display-s3` passes with zero errors and zero warnings
- [ ] `test/components/<component_name>/test_<component_name>.c` exists with >= 8 test cases and `make -f test/Makefile` passes
- [ ] Implementation report written (see agent-specific file for format)
