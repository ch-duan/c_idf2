#include "c_mutex.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "portmacro.h"

void qb_sys_mutex_create(C_MUTEX m) {
  m = osSemaphoreNew(1, 1, NULL);
}

void qb_sys_mutex_delete(C_MUTEX m) {
  osSemaphoreDelete(m);
}

void qb_sys_mutex_lock(C_MUTEX m) {
  if (m != NULL) {
    osSemaphoreAcquire(m, portMAX_DELAY);
  }
}

void qb_sys_mutex_unlock(C_MUTEX m) {
  if (m != NULL) {
    osSemaphoreRelease(m);
  }
}
