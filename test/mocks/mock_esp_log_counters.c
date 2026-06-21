#include "esp_log.h"

int mock_esp_log_error_count   = 0;
int mock_esp_log_warning_count = 0;
int mock_esp_log_info_count    = 0;

void mock_esp_log_reset(void)
{
    mock_esp_log_error_count   = 0;
    mock_esp_log_warning_count = 0;
    mock_esp_log_info_count    = 0;
}