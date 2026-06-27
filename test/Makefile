CC = gcc

CFLAGS_COMMON = -std=c99 -Wall -Wextra -Werror \
                -D_CRT_SECURE_NO_WARNINGS \
                -Ithird-party/unity \
                -Itest/mocks

CFLAGS_NVS         = $(CFLAGS_COMMON) -Icomponents/nvs_config/include
CFLAGS_APP_EVENT   = $(CFLAGS_COMMON) -Icomponents/app_event/include
CFLAGS_POWER_MGR   = $(CFLAGS_COMMON) \
                     -Icomponents/app_event/include \
                     -Icomponents/power_manager/include \
                     -Icomponents/sys_log/include

CFLAGS_WIFI_MANAGER = $(CFLAGS_COMMON) \
                      -Wno-string-compare \
                      -Wno-address \
                      -Icomponents/wifi_manager/include \
                      -Icomponents/app_event/include \
                      -DCONFIG_WIFI_MANAGER_ENABLED=1 \
                      -DCONFIG_BOARD_AI_THINKER_ESP32_CAM=1 \
                      -DUNIT_TEST=1

CFLAGS_STATUS_INDICATOR = $(CFLAGS_COMMON) \
                          -Icomponents/status_indicator/include \
                          -DCONFIG_STATUS_INDICATOR_HAS_DISPLAY=0 \
                          -DCONFIG_STATUS_INDICATOR_LED_PIN=33 \
                          -DCONFIG_STATUS_INDICATOR_LED_ACTIVE_LOW=1

SRCS_NVS = third-party/unity/unity.c \
           test/mocks/mock_esp_log_counters.c \
           test/mocks/mock_nvs_state.c \
           components/nvs_config/nvs_config.c \
           test/components/nvs_config/test_nvs_config.c

SRCS_APP_EVENT = third-party/unity/unity.c \
                 test/mocks/mock_esp_event.c \
                 test/mocks/mock_esp_log_counters.c \
                 components/app_event/app_event.c \
                 test/components/app_event/test_app_event.c

SRCS_POWER_MGR = third-party/unity/unity.c \
                 test/mocks/mock_esp_event.c \
                 test/mocks/mock_esp_log_counters.c \
                 test/mocks/mock_power_helpers.c \
                 components/app_event/app_event.c \
                 components/power_manager/power_manager.c \
                 test/components/power_manager/test_power_manager.c

SRCS_WIFI_MANAGER = third-party/unity/unity.c \
                    test/mocks/mock_esp_event.c \
                    test/mocks/mock_esp_log_counters.c \
                    test/mocks/mock_esp_wifi.c \
                    test/mocks/mock_nvs_state.c \
                    test/mocks/mock_freertos_task.c \
                    components/app_event/app_event.c \
                    components/wifi_manager/wifi_manager.c \
                    test/components/wifi_manager/test_wifi_manager.c

SRCS_STATUS_INDICATOR = third-party/unity/unity.c \
                        test/mocks/mock_esp_log_counters.c \
                        test/mocks/mock_freertos_task.c \
                        components/status_indicator/status_indicator.c \
                        test/components/status_indicator/test_status_indicator.c

.PHONY: all clean test_nvs test_app_event test_power_mgr test_wifi_mgr \
        test_status_indicator

all: test_runner_nvs test_runner_app_event test_runner_power_mgr \
     test_runner_wifi_manager test_runner_status_indicator
	./test_runner_nvs
	./test_runner_app_event
	./test_runner_power_mgr
	./test_runner_wifi_manager
	./test_runner_status_indicator

test_runner_nvs: $(SRCS_NVS)
	$(CC) $(CFLAGS_NVS) $(SRCS_NVS) -o test_runner_nvs

test_runner_app_event: $(SRCS_APP_EVENT)
	$(CC) $(CFLAGS_APP_EVENT) $(SRCS_APP_EVENT) -o test_runner_app_event

test_runner_power_mgr: $(SRCS_POWER_MGR)
	$(CC) $(CFLAGS_POWER_MGR) $(SRCS_POWER_MGR) -o test_runner_power_mgr

test_nvs: test_runner_nvs
	./test_runner_nvs

test_app_event: test_runner_app_event
	./test_runner_app_event

test_power_mgr: test_runner_power_mgr
	./test_runner_power_mgr

test_runner_wifi_manager: $(SRCS_WIFI_MANAGER)
	$(CC) $(CFLAGS_WIFI_MANAGER) $(SRCS_WIFI_MANAGER) -o test_runner_wifi_manager

test_wifi_mgr: test_runner_wifi_manager
	./test_runner_wifi_manager

test_runner_status_indicator: $(SRCS_STATUS_INDICATOR)
	$(CC) $(CFLAGS_STATUS_INDICATOR) $(SRCS_STATUS_INDICATOR) -o test_runner_status_indicator

test_status_indicator: test_runner_status_indicator
	./test_runner_status_indicator

clean:
	rm -f test_runner_nvs test_runner_app_event test_runner_power_mgr \
	      test_runner_wifi_manager test_runner_status_indicator
