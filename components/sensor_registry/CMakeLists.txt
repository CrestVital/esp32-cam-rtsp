# sensor_registry: Kconfig-only component that drives sensor selection.
#
# The sensors/ directory is added to the global include path so that
# include/sensor_caps.h can resolve:
#   #include CONFIG_SENSOR_DATA_FILE   (e.g. "sensors/ov2640.h")
# across all components that include sensor_caps.h.
#
# The project-root include/ directory is also exposed so that sensor_caps.h
# itself can be found with #include "sensor_caps.h" from any component.
idf_component_register(
    INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/sensors"
        "${CMAKE_SOURCE_DIR}/include"
)
