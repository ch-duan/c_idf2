#ifndef H_C_IDF_LOG_H
#define H_C_IDF_LOG_H

#include <stdio.h>

#include "FreeRTOS.h"
// #include "SEGGER_RTT.h"
#include "cmsis_os2.h"
typedef void (*log_check_send_finish_t)();
typedef void (*log_out_t)(uint8_t *buffer, uint32_t len);

typedef struct {
  log_check_send_finish_t check_send_finish;
  log_out_t log_out;
  uint8_t *log_buf;
  uint32_t buf_size;
} log_t;
#ifdef __cplusplus
extern "C" {
#endif

void init_log(log_t *log);
void init_log_check(log_check_send_finish_t check_send_finish);
void call_log_check();
void printf_log(const char *fmt, ...);
extern log_t *_log_t;
extern osSemaphoreId_t printf_mutex;

#define LOGGER(fmt, ...)            \
  if (_log_t->log_buf != NULL) {    \
    printf_log(fmt, ##__VA_ARGS__); \
  } else {                          \
    printf(fmt, ##__VA_ARGS__);     \
  }                                 \
  // #define LOGGER(fmt, ...) SEGGER_RTT_printf(0, fmt, ##__VA_ARGS__)

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

#define CLOG(...)                                      \
  call_log_check();                                    \
  if (printf_mutex != NULL) {                          \
    do {                                               \
      osSemaphoreAcquire(printf_mutex, portMAX_DELAY); \
      LOGGER(__VA_ARGS__);                             \
      LOGGER("\r\n");                                  \
      osSemaphoreRelease(printf_mutex);                \
    } while (0);                                       \
  } else {                                             \
    LOGGER(__VA_ARGS__);                               \
  }

#define CLOG_P(...)                                    \
  call_log_check();                                    \
  if (printf_mutex != NULL) {                          \
    do {                                               \
      osSemaphoreAcquire(printf_mutex, portMAX_DELAY); \
      LOGGER(__VA_ARGS__);                             \
      osSemaphoreRelease(printf_mutex);                \
    } while (0);                                       \
  } else {                                             \
    LOGGER(__VA_ARGS__);                               \
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
}
#endif

#endif
