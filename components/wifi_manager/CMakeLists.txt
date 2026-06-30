idf_component_register(
    SRCS "wifi_manager.c"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash esp_wifi esp_netif esp_event log app_event nvs_config
)

if(CONFIG_BT_CONTROLLER_ENABLED)
    target_link_libraries(${COMPONENT_LIB} PRIVATE idf::bt)
endif()
