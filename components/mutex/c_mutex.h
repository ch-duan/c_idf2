#pragma once
#include "cmsis_os2.h"
typedef osSemaphoreId_t C_MUTEX;
#ifdef __cplusplus
extern "C" {
#endif

void qb_sys_mutex_create(C_MUTEX m);

void qb_sys_mutex_delete(C_MUTEX m);

void qb_sys_mutex_lock(C_MUTEX m);

void qb_sys_mutex_unlock(C_MUTEX m);
#ifdef __cplusplus
}
#endif
