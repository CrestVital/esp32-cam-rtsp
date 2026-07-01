# ADR-008 — Board × Sensor Build Matrix

**Date:** 2026-07-01
**Status:** Accepted
**Ticket:** ESPCAMFW-82

## Context

ESPCAMFW-54 (ADR-004) introduced data-driven board selection via
`CONFIG_BOARD_DATA_FILE` → `boards/<board>.h`. ESPCAMFW-56 (ADR-006) added an
orthogonal, data-driven sensor selection via `CONFIG_SENSOR_DATA_FILE` →
`sensors/<sensor>.h`. Sensor selection became fully independent of the board.

However, ESPCAMFW-56 left the legacy pair of flags in each `boards/<board>.h`:

```c
#define BOARD_SENSOR_OV2640     1
#define BOARD_SENSOR_OV5640     0
```

and `include/board.h` interpreted them as a **mutually exclusive** selection
("exactly one must be 1"). This created two conflicting sources of truth for the
active sensor: the board flags (exclusive selection) and the sensor registry
(`CONFIG_SENSOR_DATA_FILE`). The two could disagree, and nothing caught it.

The camera is physically detachable (FFC connector). It is confirmed that **all
three boards** (LilyGo T-Camera Plus, AI Thinker ESP32-CAM, Olimex ESP32-POE)
physically accept both OV2640 and OV5640 modules through the same DVP FFC socket.
The Cartesian product is therefore complete: 3 boards × 2 sensors = 6 valid
board×sensor pairs.

The build system must emit one firmware image per valid pair, and the two
sources of truth must be reconciled into a single coherent model.

## Variant A — `BOARD_SENSOR_OV*` as a supported-sensor set + cross-check (accepted)

Redefine `BOARD_SENSOR_OV2640` / `BOARD_SENSOR_OV5640` as the **set of sensors
the board physically supports** (both `1` on all three boards), not an exclusive
selection. The active sensor is chosen solely by the sensor registry
(`CONFIG_SENSOR_DATA_FILE`). A compile-time cross-check in
`include/sensor_caps.h` verifies the active sensor is a member of the active
board's supported set.

The mutually-exclusive `#error` guards in `include/board.h` are removed; the
`#ifndef BOARD_SENSOR_OV*` completeness guards remain.

**Pros:**
- One source of truth for the *active* sensor (the sensor registry). The board
  flags describe only physical capability.
- The cross-check still catches a genuinely invalid pairing (a sensor the board
  cannot host) at compile time, not at runtime.
- Extends the existing data-driven pattern (ADR-004/006) instead of contradicting
  it. Adding a board that supports only one sensor is expressible by setting the
  other flag to `0`.

**Cons:**
- The `BOARD_SENSOR_OV*` macros change meaning; readers must not interpret them
  as an exclusive choice. Mitigated by an inline comment in every board file and
  by this ADR.

**Accepted.**

## Variant B — Remove `BOARD_SENSOR_OV*` entirely (rejected)

Delete the board sensor flags outright and rely solely on
`CONFIG_SENSOR_DATA_FILE`.

**Pros:** Eliminates the second source of truth completely; no meaning change.
**Cons:** Loses the ability to express and enforce a board that *cannot* host a
given sensor. A future board with a fixed, non-DVP-compatible socket (or a
MIPI-only board) could then be paired with an incompatible sensor with no
compile-time guard beyond the existing MIPI/ISP interface check. The
supported-set model is strictly more expressive and keeps the safety net.

**Rejected** because it discards a useful compile-time invariant with no
offsetting benefit.

## `SENSOR_BOARD_SUPPORT_FLAG` mechanism

Each `sensors/<sensor>.h` defines an indirection macro:

```c
/* sensors/ov2640.h */
#define SENSOR_BOARD_SUPPORT_FLAG   BOARD_SENSOR_OV2640
/* sensors/ov5640.h */
#define SENSOR_BOARD_SUPPORT_FLAG   BOARD_SENSOR_OV5640
```

`include/sensor_caps.h` (which already includes `board.h` before
`CONFIG_SENSOR_DATA_FILE`, so the `BOARD_SENSOR_*` macros are in scope) evaluates:

```c
#if !SENSOR_BOARD_SUPPORT_FLAG
#error "Active sensor is not in the board's supported sensor set ..."
#endif
```

The indirection lets `sensor_caps.h` perform the cross-check **without hardcoding
sensor names** — it never mentions `OV2640`/`OV5640`, so adding a sensor needs no
edit to the dispatcher (open/closed principle, consistent with ADR-006).

### Why not tie the cross-check to the `CONFIG_SENSOR_OV*` Kconfig bool

The flag is deliberately **not** derived from the Kconfig `CONFIG_SENSOR_OV*`
bool produced by the `choice SENSOR_TARGET`. Host-side unit tests bypass Kconfig
entirely: `test/mocks/sdkconfig.h` sets `CONFIG_SENSOR_DATA_FILE` (and
`CONFIG_BOARD_DATA_FILE`) directly, and the Kconfig `choice` bools are never
generated in that build. Binding the cross-check to `CONFIG_SENSOR_OV*` would
make it unevaluable (or spuriously false) under the host test harness. Deriving
`SENSOR_BOARD_SUPPORT_FLAG` from the board flag inside the sensor data file keeps
the check working identically in both the ESP-IDF build and the host tests.

### Include-ordering requirement

The cross-check only works because `sensor_caps.h` includes `board.h` **before**
`CONFIG_SENSOR_DATA_FILE`. If that order were reversed, `BOARD_SENSOR_OV2640` /
`BOARD_SENSOR_OV5640` would be undefined when `SENSOR_BOARD_SUPPORT_FLAG` expands.
The ordering is load-bearing and documented in both `sensor_caps.h` and the
sensor data files.

## Environment naming scheme

Every PlatformIO environment carries an explicit sensor suffix:
`<board>-<sensor>`, e.g. `lilygo-t-camera-plus-ov2640`. The default environment
is `default_envs = lilygo-t-camera-plus-ov2640`.

**Rejected alternative:** keep a suffix-less default env (e.g.
`lilygo-t-camera-plus`) for the "primary" sensor and only suffix the others. This
was rejected because it makes the naming asymmetric and hides which sensor the
default env builds — a reader cannot tell the active sensor from the env name.
Uniform suffixing keeps every image self-describing.

## sdkconfig fragment split (board / sensor)

Sensor selection is removed from every `sdkconfig.defaults.<board>` fragment and
moved into dedicated per-sensor fragments:

- `sdkconfig.sensor.ov2640` → `CONFIG_SENSOR_OV2640=y`
- `sdkconfig.sensor.ov5640` → `CONFIG_SENSOR_OV5640=y`

Each `[env:...]` composes them: `sdkconfig.defaults` (shared) +
`sdkconfig.defaults.<board>` (board) + `sdkconfig.sensor.<sensor>` (sensor). This
keeps board and sensor concerns orthogonal in the config layer, mirroring the
orthogonality already established in the data files (ADR-004 vs ADR-006).

## Build matrix

Six valid board×sensor pairs, all physically confirmed (the FFC socket accepts
both DVP sensors on all three boards):

| PlatformIO env                  | Board                | Sensor |
|---------------------------------|----------------------|--------|
| `lilygo-t-camera-plus-ov2640`   | LilyGo T-Camera Plus | OV2640 |
| `lilygo-t-camera-plus-ov5640`   | LilyGo T-Camera Plus | OV5640 |
| `ai-thinker-esp32-cam-ov2640`   | AI Thinker ESP32-CAM | OV2640 |
| `ai-thinker-esp32-cam-ov5640`   | AI Thinker ESP32-CAM | OV5640 |
| `olimex-esp32-poe-ov2640`       | Olimex ESP32-POE     | OV2640 |
| `olimex-esp32-poe-ov5640`       | Olimex ESP32-POE     | OV5640 |

The CI `build` job matrix enumerates all six. The `[env:native]` host-test
environment is not part of the build matrix.

## Consequences

- `BOARD_SENSOR_OV*` now expresses physical capability (a set), not selection.
  Every board file carries an inline comment stating this.
- The active sensor is selected exclusively via the sensor registry
  (`CONFIG_SENSOR_DATA_FILE`), pinned per env by the composed sensor fragment.
- A build pairing a sensor with a board that does not list it fails at compile
  time with a `#error` referencing this ADR.
- Sensor selection lives only in `sdkconfig.sensor.<sensor>` fragments; board
  fragments no longer set a sensor.
- CI emits one artifact per valid pair (6 images).
- A **universal firmware with runtime sensor auto-detect** is explicitly out of
  scope here and tracked separately in ESPCAMFW-83.

See also: ADR-004 (board abstraction, Variant B), ADR-006 (sensor registry),
ADR-007 (MIPI/ISP hardware boundary).
