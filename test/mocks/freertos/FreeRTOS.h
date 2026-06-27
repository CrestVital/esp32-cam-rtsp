#pragma once

#include <stdint.h>

typedef void *TaskHandle_t;

#ifndef portMAX_DELAY
#define portMAX_DELAY  ( ( TickType_t ) 0xFFFFFFFFUL )
#endif