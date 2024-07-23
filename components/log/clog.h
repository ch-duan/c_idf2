#ifndef H_C_IDF_LOG_H
#define H_C_IDF_LOG_H

#include <stdio.h>

#include "FreeRTOS.h"
//#include "SEGGER_RTT.h"
#include "cmsis_os2.h"

// #define CLOG(fmt, ...)  printf(__VA_ARGS__)
// #define CLOG(fmt, ...) SEGGER_RTT_printf(0, __VA_ARGS__)

typedef enum {
  LOG_NONE,
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG,
  LOG_VERBOSE
} log_level_t;

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL LOG_INFO
#endif

extern osSemaphoreId_t printf_mutex;

#define CLOG(...)                                      \
  if (printf_mutex != NULL) {                          \
    do {                                               \
      osSemaphoreAcquire(printf_mutex, portMAX_DELAY); \
      printf(__VA_ARGS__);                             \
      printf("\r\n");                                  \
      osSemaphoreRelease(printf_mutex);                \
    } while (0);                                       \
  } else {                                             \
    printf(__VA_ARGS__);                               \
  }


#define CLOG_P(...)                                      \
  if (printf_mutex != NULL) {                          \
    do {                                               \
      osSemaphoreAcquire(printf_mutex, portMAX_DELAY); \
      printf(__VA_ARGS__);                             \
      printf("\r\n");                                  \
      osSemaphoreRelease(printf_mutex);                \
    } while (0);                                       \
  } else {                                             \
    printf(__VA_ARGS__);                               \
  }


#define LOG_LEVEL_LOCAL(level, tag, format, ...)                   \
  do {                                                             \
    if (LOG_LOCAL_LEVEL != LOG_NONE && LOG_LOCAL_LEVEL >= level) { \
      CLOG("[%s] " format, tag, ##__VA_ARGS__)                     \
    }                                                              \
  } while (0)

#define CLOG_V(TAG, ...) LOG_LEVEL_LOCAL(LOG_VERBOSE, TAG, __VA_ARGS__)
#define CLOG_I(TAG, ...) LOG_LEVEL_LOCAL(LOG_INFO, TAG, __VA_ARGS__)
#define CLOG_D(TAG, ...) LOG_LEVEL_LOCAL(LOG_DEBUG, TAG, __VA_ARGS__)
#define CLOG_W(TAG, ...) LOG_LEVEL_LOCAL(LOG_WARN, TAG, __VA_ARGS__)
#define CLOG_E(TAG, ...) LOG_LEVEL_LOCAL(LOG_ERROR, TAG, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif
void init_log(void);
#ifdef __cplusplus
}
#endif

#endif
