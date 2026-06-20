#include "esp_event.h"
#include <string.h>

esp_err_t mock_event_loop_create_ret = ESP_OK;
esp_err_t mock_event_loop_delete_ret = ESP_OK;
esp_err_t mock_event_post_to_ret = ESP_OK;
esp_err_t mock_event_handler_register_ret = ESP_OK;
esp_err_t mock_event_handler_unregister_ret = ESP_OK;

int mock_event_loop_create_calls = 0;
int mock_event_loop_delete_calls = 0;
int mock_event_post_to_calls = 0;
int mock_event_handler_register_calls = 0;
int mock_event_handler_unregister_calls = 0;

esp_event_loop_args_t mock_last_loop_args;
esp_event_base_t      mock_last_post_base;
int32_t               mock_last_post_id;

void mock_esp_event_reset(void)
{
    /* Restore all injected return values, call counters, and captured
     * arguments to their zero/default state so each test case starts
     * with a clean, deterministic mock environment. */

    mock_event_loop_create_ret = ESP_OK;
    mock_event_loop_delete_ret = ESP_OK;
    mock_event_post_to_ret = ESP_OK;
    mock_event_handler_register_ret = ESP_OK;
    mock_event_handler_unregister_ret = ESP_OK;

    mock_event_loop_create_calls = 0;
    mock_event_loop_delete_calls = 0;
    mock_event_post_to_calls = 0;
    mock_event_handler_register_calls = 0;
    mock_event_handler_unregister_calls = 0;

    /* Zero the argument capture structs so stale data does not
     * accidentally cause a test to pass when it should fail. */
    memset(&mock_last_loop_args, 0, sizeof(mock_last_loop_args));
    mock_last_post_base = NULL;
    mock_last_post_id   = 0;
}

esp_err_t esp_event_loop_create(const esp_event_loop_args_t *args,
                                esp_event_loop_handle_t *loop)
{
    /* Increment counter and snapshot the caller's configuration so
     * tests can assert exact parameter values. On success, store a
     * non-NULL sentinel pointer to satisfy the real component's NULL
     * check on s_app_loop. */
    mock_event_loop_create_calls++;
    if (args != NULL) {
        memcpy(&mock_last_loop_args, args, sizeof(*args));
    }

    if (mock_event_loop_create_ret == ESP_OK) {
        *loop = (void *)0xDEADBEEF;
    }

    return mock_event_loop_create_ret;
}

esp_err_t esp_event_loop_delete(esp_event_loop_handle_t loop)
{
    mock_event_loop_delete_calls++;
    (void)loop;
    return mock_event_loop_delete_ret;
}

esp_err_t esp_event_post_to(esp_event_loop_handle_t loop,
                            esp_event_base_t base,
                            int32_t id,
                            const void *data,
                            size_t data_size,
                            unsigned int ticks)
{
    mock_event_post_to_calls++;

    /* Capture arguments so tests can verify the posted event ID and
     * event base match expectations. */
    mock_last_post_base = base;
    mock_last_post_id   = id;

    /* Suppress "unused" warnings for parameters we don't need to
     * capture but must accept to match the real prototype. */
    (void)loop;
    (void)data;
    (void)data_size;
    (void)ticks;

    return mock_event_post_to_ret;
}

esp_err_t esp_event_handler_register_with(esp_event_loop_handle_t loop,
                                          esp_event_base_t base,
                                          int32_t id,
                                          esp_event_handler_t handler,
                                          void *arg)
{
    mock_event_handler_register_calls++;
    (void)loop;
    (void)base;
    (void)id;
    (void)handler;
    (void)arg;
    return mock_event_handler_register_ret;
}

esp_err_t esp_event_handler_unregister_with(esp_event_loop_handle_t loop,
                                            esp_event_base_t base,
                                            int32_t id,
                                            esp_event_handler_t handler)
{
    mock_event_handler_unregister_calls++;
    (void)loop;
    (void)base;
    (void)id;
    (void)handler;
    return mock_event_handler_unregister_ret;
}