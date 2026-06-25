idf_component_register(
    SRCS "status_indicator.c"
    INCLUDE_DIRS "include"
    REQUIRES log esp_driver_ledc esp_driver_gpio
)

if(CONFIG_STATUS_INDICATOR_HAS_DISPLAY)
    if(EXISTS "${CMAKE_SOURCE_DIR}/components/display_manager")
        idf_component_get_property(dm_lib display_manager COMPONENT_LIB)
        target_link_libraries(${COMPONENT_LIB} PRIVATE ${dm_lib})
    endif()
endif()
