#pragma once

#include <stdint.h>
#include "esp_err.h"

/* Minimal host-side stubs for esp_event types and macros.
 * Only the symbols used by app_event.c are stubbed here. */

typedef const char *esp_event_base_t;
typedef void       *esp_event_loop_handle_t;
typedef void (*esp_event_handler_t)(void *arg,
                                    esp_event_base_t base,
                                    int32_t id,
                                    void *data);

typedef struct {
    int32_t      queue_size;
    const char  *task_name;
    unsigned int task_priority;
    unsigned int task_stack_size;
    int          task_core_id;
} esp_event_loop_args_t;

#define ESP_EVENT_ANY_BASE  NULL
#define ESP_EVENT_ANY_ID    (-1)
#define tskNO_AFFINITY      (-1)
#ifndef portMAX_DELAY
#define portMAX_DELAY       0xFFFFFFFFUL
#endif

/* Macro expansions matching ESP-IDF behaviour */
#define ESP_EVENT_DECLARE_BASE(id)  extern esp_event_base_t const id
#define ESP_EVENT_DEFINE_BASE(id)   esp_event_base_t const id = #id

/* Injected return values for each stubbed function (set per-test) */
extern esp_err_t mock_event_loop_create_ret;
extern esp_err_t mock_event_loop_delete_ret;
extern esp_err_t mock_event_post_to_ret;
extern esp_err_t mock_event_handler_register_ret;
extern esp_err_t mock_event_handler_unregister_ret;

/* Call counters */
extern int mock_event_loop_create_calls;
extern int mock_event_loop_delete_calls;
extern int mock_event_post_to_calls;
extern int mock_event_handler_register_calls;
extern int mock_event_handler_unregister_calls;

/* Captured arguments */
extern esp_event_loop_args_t mock_last_loop_args;
extern esp_event_base_t      mock_last_post_base;
extern int32_t               mock_last_post_id;

/* Reset all injected values and counters to defaults */
void mock_esp_event_reset(void);

/* Stubbed ESP-IDF event API */
esp_err_t esp_event_loop_create(const esp_event_loop_args_t *args,
                                esp_event_loop_handle_t *loop);
esp_err_t esp_event_loop_delete(esp_event_loop_handle_t loop);
esp_err_t esp_event_post_to(esp_event_loop_handle_t loop,
                            esp_event_base_t base,
                            int32_t id,
                            const void *data,
                            size_t data_size,
                            unsigned int ticks);
esp_err_t esp_event_handler_register_with(esp_event_loop_handle_t loop,
                                          esp_event_base_t base,
                                          int32_t id,
                                          esp_event_handler_t handler,
                                          void *arg);
esp_err_t esp_event_handler_unregister_with(esp_event_loop_handle_t loop,
                                            esp_event_base_t base,
                                            int32_t id,
                                            esp_event_handler_t handler);

/* Default-event-loop variants — used by wifi_manager */
esp_err_t esp_event_handler_register(esp_event_base_t base, int32_t id,
                                     esp_event_handler_t handler, void *arg);
esp_err_t esp_event_handler_unregister(esp_event_base_t base, int32_t id,
                                       esp_event_handler_t handler);