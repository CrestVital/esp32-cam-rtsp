# ADR-004 — Board Abstraction Strategy

**Date:** 2026-06-28
**Status:** Accepted
**Ticket:** ESPCAMFW-54

## Context

The firmware must support multiple ESP32-based boards with different camera pin
assignments and capability flags (WiFi, Ethernet, display, PSRAM). A mechanism
is needed to select board-specific data at compile time without modifying
component source files when adding new boards.

## Variant A — `#if`/`#elif` chain in `include/board.h` (rejected)

All board data is listed directly in `board.h` as a series of
`#if CONFIG_BOARD_X / #elif CONFIG_BOARD_Y` blocks.

**Pros:** Simple, no indirection.
**Cons:** Adding a new board requires editing `board.h` (a shared header used by
all components). This creates unnecessary merge conflicts and violates the
open/closed principle.

**Rejected** because every new board requires modifying a core shared header.

## Variant B — Data files + Kconfig `CONFIG_BOARD_DATA_FILE` (accepted)

Each board's data lives in `boards/<board>.h` as pure `#define` macros.
`include/board.h` includes the selected file via:

    #include CONFIG_BOARD_DATA_FILE

where `CONFIG_BOARD_DATA_FILE` is a string Kconfig option set in
`components/board_config/Kconfig.projbuild`.

**Pros:**
- Adding a new board = new `boards/<board>.h` + one Kconfig entry + one
  PlatformIO env + one sdkconfig fragment. Zero changes to `board.h` or any `.c` file.
- Board data files are isolated: no file touches shared logic.
- Compile-time completeness validation in `board.h` catches missing macros.

**Cons:**
- Slightly more indirection (Kconfig string → file path → include).
- Requires `boards/` directory on the include path (done via
  `components/board_config/CMakeLists.txt`).

**Accepted.** Implemented in ESPCAMFW-54.

## Prior Failed Approach (ESPCAMFW-39)

An earlier attempt used `platformio.ini` `build_flags = -DBOARD_HEADER="..."` to
pass the board file path as a preprocessor define. This failed because
`build_flags -D` defines do not propagate into the ESP-IDF ninja build system —
they are applied to the PlatformIO wrapper layer only. Kconfig is the correct
mechanism for compile-time selection in ESP-IDF projects.

## Consequences

- All board capability flags and pin assignments live in `boards/` only.
- `include/board.h` contains no board-specific data — only dispatch and validation.
- Future boards require no changes to any existing `.c` or `.h` files outside `boards/`.
- The `boards/` directory must be on the include path (via `board_config` component).