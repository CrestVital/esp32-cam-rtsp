# board_config: Kconfig-only component that drives board selection.
#
# The boards/ directory is added to the global include path so that
# include/board.h can resolve:
#   #include CONFIG_BOARD_DATA_FILE   (e.g. "boards/lilygo_t_camera_plus.h")
# across all components that include board.h.
idf_component_register(
    INCLUDE_DIRS
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/boards"
)