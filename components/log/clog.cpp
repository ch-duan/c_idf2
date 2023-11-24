#include "clog.h"

#include <stdio.h>

#include "cmsis_os2.h"

osSemaphoreId_t printf_mutex = NULL;

void init_log(void) {
  printf_mutex = osSemaphoreNew(1, 1, NULL);
  if (printf_mutex == NULL) {
    printf("log init error");
  }
}
