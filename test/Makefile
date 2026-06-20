CC = gcc

CFLAGS_COMMON = -std=c99 -Wall -Wextra -Werror \
                -Ithird-party/unity \
                -Itest/mocks

CFLAGS_NVS       = $(CFLAGS_COMMON) -Icomponents/nvs_config/include
CFLAGS_APP_EVENT = $(CFLAGS_COMMON) -Icomponents/app_event/include

SRCS_NVS = third-party/unity/unity.c \
           test/mocks/mock_nvs_state.c \
           components/nvs_config/nvs_config.c \
           test/components/nvs_config/test_nvs_config.c

SRCS_APP_EVENT = third-party/unity/unity.c \
                 test/mocks/mock_esp_event.c \
                 components/app_event/app_event.c \
                 test/components/app_event/test_app_event.c

.PHONY: all clean test_nvs test_app_event

all: test_runner_nvs test_runner_app_event
	./test_runner_nvs
	./test_runner_app_event

test_runner_nvs: $(SRCS_NVS)
	$(CC) $(CFLAGS_NVS) $(SRCS_NVS) -o test_runner_nvs

test_runner_app_event: $(SRCS_APP_EVENT)
	$(CC) $(CFLAGS_APP_EVENT) $(SRCS_APP_EVENT) -o test_runner_app_event

test_nvs: test_runner_nvs
	./test_runner_nvs

test_app_event: test_runner_app_event
	./test_runner_app_event

clean:
	rm -f test_runner_nvs test_runner_app_event