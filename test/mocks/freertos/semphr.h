#pragma once

#include <stdint.h>

typedef void *SemaphoreHandle_t;

static inline SemaphoreHandle_t xSemaphoreCreateMutex(void)
{
    return (SemaphoreHandle_t)0xBEEF0001;
}

static inline int xSemaphoreTake(SemaphoreHandle_t s, unsigned int t)
{
    (void)s;
    (void)t;
    return 1; /* pdTRUE */
}

static inline int xSemaphoreGive(SemaphoreHandle_t s)
{
    (void)s;
    return 1; /* pdTRUE */
}

static inline void vSemaphoreDelete(SemaphoreHandle_t s)
{
    (void)s;
}
