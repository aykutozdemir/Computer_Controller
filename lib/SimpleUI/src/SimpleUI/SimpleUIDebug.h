#pragma once

// Centralised debug macros for SimpleUI
// Include esp_log when debugging is enabled; otherwise compile to no-ops.
#ifdef SIMPLEUI_DEBUG
  #include "esp_log.h"
  // If a compilation unit wishes to override the default tag,
  // define SIMPLEUI_TAG *before* including this header. Otherwise
  // the file name will be used as the logging tag.
  #ifndef SIMPLEUI_TAG
    #define SIMPLEUI_TAG __FILE__
  #endif
  #define SUI_LOGI(fmt, ...) ESP_LOGI(SIMPLEUI_TAG, fmt, ##__VA_ARGS__)
  #define SUI_LOGD(fmt, ...) ESP_LOGD(SIMPLEUI_TAG, fmt, ##__VA_ARGS__)
  #define SUI_LOGW(fmt, ...) ESP_LOGW(SIMPLEUI_TAG, fmt, ##__VA_ARGS__)
  #define SUI_LOGE(fmt, ...) ESP_LOGE(SIMPLEUI_TAG, fmt, ##__VA_ARGS__)
#else
  // Strip out log calls when SIMPLEUI_DEBUG is not defined
  #define SUI_LOGI(fmt, ...)
  #define SUI_LOGD(fmt, ...)
  #define SUI_LOGW(fmt, ...)
  #define SUI_LOGE(fmt, ...)
#endif 