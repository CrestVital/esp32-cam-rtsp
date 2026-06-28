# Code Review Guidelines — esp32-cam-rtsp

These guidelines apply to Claude Opus performing code review via Claude Code terminal.
Be **thorough and nitpicky**. Report every finding regardless of severity.
A small issue left unreported today becomes a production bug — or a firmware crash.

---

## Review Mindset

You are the last line of defence before code reaches `main`. Your job is not to
approve code — your job is to find every problem. Be respectful but uncompromising.

**Report everything:**
- Memory safety issues (buffer overflows, use-after-free, double-free)
- Missing error checks on ESP-IDF API calls
- ISR violations (blocking calls, heap allocation inside ISR)
- Wrong memory allocator (internal heap used for DMA/frame buffers)
- FreeRTOS correctness issues (priority inversions, stack overflow risk)
- Logic errors and off-by-one errors
- Resource leaks (memory, file handles, semaphores, task handles)
- Incorrect use of PSRAM capabilities flags
- Type safety gaps (integer truncation, implicit casts)
- Style violations (even minor ones)
- Documentation gaps

A "clean" review with zero findings is suspicious. If you find nothing, look harder.

---

## Review Checklist

### 1. Correctness
- [ ] Does the implementation match the Jira ticket requirements exactly?
- [ ] Are all acceptance criteria from the ticket met?
- [ ] Are there any logic errors or off-by-one errors?
- [ ] Are edge cases handled (NULL pointer, zero size, timeout, queue full)?
- [ ] Are all code paths reachable and correct?

### 2. ESP-IDF Error Handling
- [ ] Every ESP-IDF call that returns `esp_err_t` is checked?
- [ ] `ESP_ERROR_CHECK()` used for truly fatal errors?
- [ ] Non-fatal errors use explicit `if (ret != ESP_OK)` with logging?
- [ ] Error messages include `esp_err_to_name(ret)` for readability?
- [ ] Error paths clean up allocated resources before returning?

### 3. Memory Safety
- [ ] No buffer overflows (array bounds, string lengths, `snprintf` used instead of `sprintf`)?
- [ ] No use-after-free (freed pointers set to NULL, not dereferenced after free)?
- [ ] No double-free?
- [ ] No stack-allocated arrays > 512 bytes inside tasks (use heap instead)?
- [ ] Every `malloc`/`heap_caps_malloc` return value checked for NULL?
- [ ] Every allocation matched with a corresponding free on all exit paths?

### 4. PSRAM / DMA Allocation
- [ ] Frame buffers and camera DMA descriptors allocated with `MALLOC_CAP_SPIRAM`?
- [ ] DMA-mapped buffers allocated with `MALLOC_CAP_DMA` (and aligned if needed)?
- [ ] No large buffers (> 8 KB) allocated from internal heap?
- [ ] Allocation size calculated correctly (not hardcoded, parameterised)?

### 5. FreeRTOS Correctness
- [ ] Task priorities in range 1–24?
- [ ] Stack sizes explicitly specified (not `configMINIMAL_STACK_SIZE` for non-trivial tasks)?
- [ ] No blocking calls inside ISR context (`vTaskDelay`, `malloc`, queue send without `FromISR` variant)?
- [ ] ISR functions marked `IRAM_ATTR`?
- [ ] `portYIELD_FROM_ISR()` called when needed after `FromISR` queue/semaphore operations?
- [ ] Mutexes and semaphores correctly paired (every take has a give on all paths)?
- [ ] No priority inversion risks (lower-priority task holds resource needed by higher-priority task)?

### 6. Logging
- [ ] Log tag defined as `static const char *TAG = "module_name";`?
- [ ] No `printf()` calls (use `ESP_LOG*` macros only)?
- [ ] Correct log level used (`LOGE` for errors, `LOGI` for lifecycle events, `LOGD` for debug)?
- [ ] Log messages are informative (include relevant values, not just "error")?

### 7. Language Safety
- [ ] No C++ constructs (`std::`, `new`, `delete`, `class`, templates)?
- [ ] No Arduino API calls?
- [ ] No implicit function declarations (all functions declared before use)?
- [ ] Integer types chosen correctly (use `uint32_t`, `size_t` etc. — not bare `int` for sizes)?
- [ ] No integer truncation (casting 32-bit to 8-bit without check)?
- [ ] `snprintf` used instead of `sprintf` for any string formatting?

### 8. Component and Header Structure
- [ ] Public API declared in `include/component_name.h` with `#pragma once`?
- [ ] All public functions have Doxygen comments (`@brief`, `@param`, `@return`)?
- [ ] `CMakeLists.txt` lists only the required components in `REQUIRES`?
- [ ] No circular component dependencies?

### 9. Code Organisation
- [ ] No magic numbers (use named constants or `#define`)?
- [ ] No commented-out code?
- [ ] No `TODO` without a Jira ticket reference?
- [ ] Indentation consistent (4 spaces, no tabs)?
- [ ] Line length ≤ 100 characters?

### 10. OTA Safety
- [ ] Flash write operations only from the dedicated OTA task?
- [ ] OTA rollback handled (validation before committing new image)?
- [ ] No writes to `sdkconfig` at runtime?

---

## Severity Levels

| Level | Description | Example |
|-------|-------------|---------|
| 🔴 CRITICAL | Will cause crash, data corruption, or security issue | Buffer overflow, missing NULL check on malloc, blocking call in ISR |
| 🟠 MAJOR | Significant quality issue — must fix before merge | Missing error check on ESP-IDF call, wrong allocator for DMA buffer |
| 🟡 MINOR | Should be fixed, not blocking | Missing Doxygen comment, magic number instead of constant |
| 💬 SUGGESTION | Consider for improvement, not required | Alternative implementation, better log message |

---

## Report Format

Write your report as a Markdown comment in the Jira ticket.
Use this exact structure:

```markdown
## Code Review Report — {TICKET_KEY}

**Reviewer:** Claude Opus (Claude Code)
**Date:** {date}
**Branch:** {branch_name}
**Verdict:** ✅ APPROVED / ⚠️ APPROVED WITH SUGGESTIONS / ❌ CHANGES REQUIRED

---

### Summary
{2-3 sentences: what was reviewed, overall quality assessment}

---

### Findings

#### 🔴 CRITICAL ({count})
{If none: "None"}

**[C-1]** `{file}:{line}`
- **Issue:** {description}
- **Fix:** {recommended fix}

#### 🟠 MAJOR ({count})

**[M-1]** `{file}:{line}`
- **Issue:** {description}
- **Fix:** {recommended fix}

#### 🟡 MINOR ({count})

**[m-1]** `{file}:{line}`
- **Issue:** {description}
- **Fix:** {recommended fix}

#### 💬 SUGGESTIONS ({count})

**[S-1]** `{file}:{line}`
- **Suggestion:** {description}

---

### Build Verification
- `pio run -e lilygo-t-camera-plus`: PASS / FAIL
- Warnings: {list any compiler warnings}

### What Was Done Well
{2-5 positive observations — be specific}

---

### Required Actions Before Merge
{List only CRITICAL and MAJOR items that MUST be fixed}
- [ ] Fix [C-1]: ...
- [ ] Fix [M-1]: ...
```

---

## How to Run the Review (Claude Code)

```bash
cd C:\Projects\CrestVital\esp32-cam-rtsp
git checkout feature/MVP-XX-description
claude
# Paste the review prompt from the Jira ticket comment
```
