CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror \
         -Ithird-party/unity \
         -Itest/mocks \
         -Icomponents/nvs_config/include

SRCS   = third-party/unity/unity.c \
         test/mocks/mock_nvs_state.c \
         components/nvs_config/nvs_config.c \
         test/components/nvs_config/test_nvs_config.c

.PHONY: all clean

all: test_runner
	./test_runner

test_runner: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o test_runner

clean:
	rm -f test_runner