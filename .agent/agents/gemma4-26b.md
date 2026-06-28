# Gemma4-26b Agent Context

## Project
- Repository: esp32-cam-rtsp
- Target board: LilyGo T-Camera Plus (ESP32-D0WDQ6-V3)
- Build system: PlatformIO + ESP-IDF (framework = espidf)
- Language: C (ESP-IDF conventions) unless explicitly stated otherwise

## Conventions
- All identifiers, comments, and log messages in English
- Use ESP_ERROR_CHECK() or explicit esp_err_t checks for all ESP-IDF API calls
- Allocate large/DMA buffers from PSRAM using heap_caps_malloc(size, MALLOC_CAP_SPIRAM)
- FreeRTOS task priorities: use values between 1 and 24 (configMAX_PRIORITIES - 1)
- No blocking calls inside ISRs
