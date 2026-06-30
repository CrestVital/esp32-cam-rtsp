# ADR-002 — WiFi Reconnect Task Teardown Strategy

**Date:** 2026-06-30
**Status:** Accepted
**Ticket:** ESPCAMFW-55

## Context

The `wifi_manager` component creates a FreeRTOS reconnect task on WiFi
disconnection. The task runs an exponential-backoff loop calling
`esp_wifi_connect()` and terminates when `s_reconnect_enabled` is cleared or
the maximum attempt count is reached.

Deinitialisation (`wifi_manager_deinit()`) uses a **cooperative shutdown**
model: it sets `s_reconnect_enabled = false`, wakes the reconnect task via
`xTaskNotifyGive`, and polls for the task to clear `s_reconnect_task` under a
mutex. If the task does not exit within 500 ms, deinit force-clears the handle
and proceeds — the reconnect task becomes **orphaned** and its stack and TCB
are not freed (Risk R2).

A subsequent `wifi_manager_init()` increments a generation counter so the
orphaned task can detect that it has been superseded and skip the handle-clearing
write (the ESPCAMFW-43 fix). However, this guard existed only in the cleanup
section — the main `while(...)` loop body had no generation check, so an orphaned
task that wakes up and passes the `s_reconnect_enabled` check would call
`esp_wifi_connect()` and corrupt the new init cycle's `s_attempt_count`.

An additional gap: the Bluetooth controller disable call in
`wifi_manager_init()` required the `bt` ESP-IDF component to be listed in
`CMakeLists.txt` REQUIRES. Without it, builds with `CONFIG_BT_CONTROLLER_ENABLED=y`
would fail to link.

## Decision

### 1. Conditional `bt` dependency in CMakeLists.txt

Add `target_link_libraries(${COMPONENT_LIB} PRIVATE idf::bt)` guarded by
`CONFIG_BT_CONTROLLER_ENABLED`. This satisfies the linker requirement without
pulling in the Bluetooth stack when BT is disabled.

### 2. Generation guard in the main loop of `reconnect_task()`

After `ulTaskNotifyTake` and the `s_reconnect_enabled` check, a generation
guard is inserted **under mutex**: if the task's captured generation does not
match the current counter, the task logs a warning and `break`s from the loop
without calling `esp_wifi_connect()` or modifying `s_attempt_count`.

### 3. Active reconnect task counter (tripwire)

A new static counter `s_active_reconnect_tasks` is:
- Incremented at the start of `reconnect_task()`, under mutex
- Decremented in the cleanup section, under the existing mutex
- Reset to 0 in `wifi_manager_init()`

The increment site fires `ESP_LOGE` when the counter exceeds 1, signalling
that two reconnect tasks are concurrently active — a protocol violation.

### 4. Deferred: Join-based teardown

Join-based teardown (where deinit blocks until the task's `vTaskDelete` is
known to have completed) is deferred to **ESPCAMFW-47**. Two TODO anchors
mark the locations where the join logic would be inserted.

## Accepted Risk R2

**Stack/TCB leak on timeout path:** When deinit times out after 500 ms, the
reconnect task's stack and TCB are not freed. This is acceptable for a
**one-shot shutdown** — the typical device lifecycle ends with `esp_restart()`
after deinit, which reclaims all memory. If deinit becomes a regular runtime
path (e.g., deep-sleep cycles without reboot), ESPCAMFW-47 must be resolved
first.

## Generation Guard in Main Loop

The generation guard in the cleanup section (ESPCAMFW-43) prevents an orphaned
task from writing `s_reconnect_task = NULL` and overwriting a new task's handle.
However, the loop body had no such guard:

1. An orphaned task wakes from `ulTaskNotifyTake`
2. `s_reconnect_enabled` is `true` (set by the new init cycle's connect call)
3. The task calls `esp_wifi_connect()` — interfering with the new cycle
4. The task increments `s_attempt_count` — corrupting the new cycle's backoff counter

The new generation guard in the loop body breaks out before step 3, ensuring
the orphaned task exits cleanly without any side effects on the new cycle.

## Tripwire

The `s_active_reconnect_tasks` counter is a safety net: under normal operation,
only one reconnect task exists at a time. If the counter ever exceeds 1, an
`ESP_LOGE` message is emitted. This catches potential race conditions in the
task-creation logic that could otherwise go unnoticed.

## Alternatives Considered

- **External `vTaskDelete`:** Deinit calling `vTaskDelete(s_reconnect_task)` on
  the stuck task. Rejected — this re-introduces the kernel panic risk from
  ESPCAMFW-43 if the task has already called `vTaskDelete(NULL)` internally.
- **Full redesign with join-based teardown:** Deferred to ESPCAMFW-47. The
  current cooperative shutdown is sufficient for one-shot deinit use cases.

## Consequences

- No breaking API changes. All public function signatures remain unchanged.
- Orphaned reconnect tasks exit cleanly without interfering with a new init cycle.
- Stack/TCB leak risk on the deinit-timeout path is documented and tracked via
  ESPCAMFW-47.
- Builds with `CONFIG_BT_CONTROLLER_ENABLED=y` will no longer fail to link due
  to the missing `bt` dependency.
- The tripwire provides early detection of concurrent reconnect tasks in
  production logs.
