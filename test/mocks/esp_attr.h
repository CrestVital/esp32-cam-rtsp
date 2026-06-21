#pragma once

/* Host-side stubs for ESP-IDF memory placement attributes.
 *
 * In the real ESP-IDF these macros expand to GCC __attribute__ directives
 * that place functions and data in specific memory regions (IRAM, DRAM,
 * RTC RAM, external RAM). On the host (x86/amd64) no such regions exist
 * — all macros are defined as empty so the compiler accepts annotated
 * declarations without error.
 *
 * Guards prevent redefinition if an including translation unit already
 * pulls in a definition from another path. */

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#ifndef DRAM_ATTR
#define DRAM_ATTR
#endif

#ifndef IRAM_DATA_ATTR
#define IRAM_DATA_ATTR
#endif

#ifndef IRAM_BSS_ATTR
#define IRAM_BSS_ATTR
#endif

#ifndef DRAM_DATA_ATTR
#define DRAM_DATA_ATTR
#endif

#ifndef DRAM_BSS_ATTR
#define DRAM_BSS_ATTR
#endif

#ifndef RTC_IRAM_ATTR
#define RTC_IRAM_ATTR
#endif

#ifndef RTC_DATA_ATTR
#define RTC_DATA_ATTR
#endif

#ifndef RTC_RODATA_ATTR
#define RTC_RODATA_ATTR
#endif

#ifndef EXT_RAM_ATTR
#define EXT_RAM_ATTR
#endif
