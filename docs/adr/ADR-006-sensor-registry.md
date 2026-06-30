# ADR-006 — Sensor Registry Strategy

**Date:** 2026-06-30
**Status:** Accepted
**Ticket:** ESPCAMFW-56

## Context

The firmware must support multiple OV-based camera sensors (OV2640, OV5640, and
future MIPI CSI-2 sensors). Each sensor has a unique set of capabilities: name,
vendor, interface type (DVP vs MIPI CSI-2), ISP requirement, maximum resolution,
maximum frame rate, and driver availability flag. A mechanism is needed to select
sensor-specific data at compile time without modifying component source files
when adding new sensors.

The board abstraction (ADR-004, ESPCAMFW-54) already established a data-driven
pattern via `CONFIG_BOARD_DATA_FILE` → `boards/<board>.h`. This ADR extends that
pattern to sensors.

## Variant (i) — `#if`/`#elif` chain in `sensor_caps.h` (rejected)

A single dispatcher header enumerates all sensors:

```c
#if CONFIG_SENSOR_OV2640
    #define SENSOR_NAME "OV2640"
    ...
#elif CONFIG_SENSOR_OV5640
    #define SENSOR_NAME "OV5640"
    ...
#endif
```

**Pros:** Simple, no indirection.
**Cons:** Adding a sensor requires modifying `sensor_caps.h` (a shared header
used by all components). Each new sensor or update to existing sensor data
produces merge conflicts in the core dispatcher. Violates the open/closed
principle established by ADR-004.

**Rejected** because it duplicates the mistakes avoided by ADR-004's Variant B.

## Variant (ii) — Separate sensor `choice` in Kconfig + computed include (accepted)

Each sensor's data lives in `sensors/<sensor>.h` as pure `#define` macros.
`include/sensor_caps.h` includes the selected file via:

    #include CONFIG_SENSOR_DATA_FILE

where `CONFIG_SENSOR_DATA_FILE` is a string Kconfig option set in
`components/sensor_registry/Kconfig.projbuild`. A dedicated `choice SENSOR_TARGET`
selects the sensor, and each board's `sdkconfig.defaults.<board>` fragment pins the
selection (mirroring the board selection mechanism).

Sensor Kconfig location: `components/sensor_registry/Kconfig.projbuild`. A new
component `sensor_registry` was created rather than appending to `board_config`
to maintain clean separation of concerns — board selection and sensor selection
are orthogonal decisions, even though each board fragment picks one.

**Pros:**
- Adding a board that uses an **existing** sensor requires no edits to the sensor
  registry. The board fragment just selects the existing `SENSOR_<X>`.
- Adding a **new** sensor requires only a new `sensors/<sensor>.h` data file +
  new Kconfig entries in `components/sensor_registry/Kconfig.projbuild`. No
  changes to `sensor_caps.h` or any `.c` file.
- Compile-time completeness validation in `sensor_caps.h` catches missing macros.
- Interface safety checks prevent building a MIPI CSI-2 or ISP-requiring sensor
  on a non-MIPI/non-ISP board (see epic ESPCAMFW-49, ADR-007).

**Cons:**
- Slightly more indirection (Kconfig string → file path → include).
- Requires a dedicated `sensor_registry` component on the include path.

**Accepted.** Implemented in ESPCAMFW-56.

## Data-vs-Driver Boundary

The sensor data files include `SENSOR_HAS_DRIVER` (0 or 1). This marks the
boundary between the compile-time data registry and the runtime camera driver
(epic ESPCAMFW-2). Sensor data files are pure `#define` — no logic, no
initialisation code, no register tables. The real capture driver lives in
the `camera_driver` component (not yet implemented).

## DVP-vs-MIPI/ISP Hardware Boundary

The sensor registry classifies each sensor's interface as `SENSOR_IFACE_DVP` or
`SENSOR_IFACE_MIPI_CSI2`. Board data files declare `BOARD_HAS_MIPI_CSI` and
`BOARD_HAS_ISP` capability flags (currently `0` for all three supported boards,
which are ESP32/ESP32-S3 based).

`include/sensor_caps.h` performs a compile-time interface safety check: if the
active sensor requires MIPI CSI-2 or an ISP, and the selected board lacks that
capability, the build fails with a `#error` referencing epic ESPCAMFW-49 and
ADR-007.

This check ensures that the hardware boundary is enforced at compile time, not
discovered at runtime. Current sensors (OV2640, OV5640) are DVP-based and do
not require ISP; the MIPI safety gate exists for future ESP32-P4 sensors.

See also: ADR-004 (Variant-B board abstraction, which this mirrors exactly).

## Consequences

- All sensor capability data lives in `sensors/` only.
- `include/sensor_caps.h` contains no sensor-specific data — only dispatch and
  validation.
- Future sensors require no changes to any existing `.c` or `.h` files outside
  `sensors/` and `components/sensor_registry/Kconfig.projbuild`.
- The `sensors/` and `include/` directories must be on the include path (via
  `components/sensor_registry/CMakeLists.txt`).
- Sensor selection is via Kconfig `choice SENSOR_TARGET`, with the active
  selection set in each board's `sdkconfig.defaults.<board>` fragment.
- `sdkconfig.defaults` (shared) does **not** set a sensor — sensor selection is
  per-board.
- Interface safety checks are compile-time (not runtime).
- `platformio.ini` `build_flags -D` are **not** used for sensor selection,
  consistent with the lesson from ESPCAMFW-39 (Kconfig is the correct mechanism).
