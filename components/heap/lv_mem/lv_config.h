#pragma once

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "m_malloc.h"

#ifndef LV_EXPORT_CONST_INT
#ifdef CONFIG_LV_EXPORT_CONST_INT
#define LV_EXPORT_CONST_INT CONFIG_LV_EXPORT_CONST_INT
#else
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning /*The default value just prevents GCC warning*/
#endif
#endif

#define LV_USE_DEBUG    0
#define LV_USE_OS       1
#define LV_MEM_ADD_JUNK 0
#define LV_USE_LOG      0
#define LV_ATTRIBUTE_FAST_MEM
#define LV_LOG_LEVEL         LV_LOG_LEVEL_INFO
#define LV_MEM_BUF_MAX_NUM   16
#define LV_MEMCPY_MEMSET_STD 1
#define LV_ASSERT_HANDLER \
  do {                    \
    ;                     \
  } while (1);
#define LV_LOG_TRACE_MEM 1

#define LV_MEM_SIZE      MEMORY_POOL_EXT_RAM_SIZE

#define LV_MUTEX         osSemaphoreId_t
#define LV_MUTEX_CREATE(mutex)          \
  do {                                  \
    mutex = osSemaphoreNew(1, 1, NULL); \
  } while (0)

#define LV_MUTEX_DELETE(mutex) osSemaphoreDelete(mutex);

#define LV_MUTEX_LOCK(mutex)   osSemaphoreAcquire(mutex, portMAX_DELAY);

#define LV_MUTEX_UNLOCK(mutex) osSemaphoreRelease(mutex);
