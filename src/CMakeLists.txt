idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi freertos sys_log nvs_config app_event power_manager wifi_manager
)
