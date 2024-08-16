#include "clog.h"

#include <stdarg.h>
#include <stdio.h>

#include "cmsis_os2.h"

osSemaphoreId_t printf_mutex = NULL;
log_t* _log_t = NULL;

void init_log(log_t* log) {
  printf_mutex = osSemaphoreNew(1, 1, NULL);
  if (printf_mutex == NULL) {
    printf("log init error");
  }
  if (log != NULL) {
    _log_t = log;
  }
}

void call_log_check() {
  if (_log_t != NULL && _log_t->check_send_finish != NULL) {
    _log_t->check_send_finish();
  }
}

void printf_log(const char* fmt, ...) {
  if (_log_t->log_buf == NULL || _log_t->log_out == NULL || _log_t->buf_size == 0) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf((char*) _log_t->log_buf, _log_t->buf_size, fmt, args);
  va_end(args);
  if (_log_t->log_out != NULL) {
    _log_t->log_out(_log_t->log_buf, ret);
  }
}
