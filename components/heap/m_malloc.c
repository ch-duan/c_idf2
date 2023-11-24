#include "m_malloc.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clog.h"

#define TAG "m_malloc"

#if USE_EXTERNAL_SRAM == 1
EXT_RAM_ATTR uint8_t ext_ram[MEMORY_POOL_EXT_RAM_SIZE];
#include "lv_mem.h"
#endif

uint8_t ucHeap[configTOTAL_HEAP_SIZE] __attribute__((section(".ccmram")));
HeapRegion_t xHeapRegions[] = {
    {ucHeap, configTOTAL_HEAP_SIZE},
    {NULL,   0                    }
};

void malloc_init() {
#if USE_EXTERNAL_SRAM == 1
  lv_mem_init(ext_ram);
#endif
}

void *m_malloc(size_t size) {
#if USE_EXTERNAL_SRAM == 1
  void *ptr = lv_mem_alloc(size);
  if (ptr == NULL) {
    ptr = lv_mem_alloc(size);
  }
  return ptr;
#else
  return malloc(size);
#endif
}

void m_free(void *f) {
  if (f != NULL) {
#if USE_EXTERNAL_SRAM == 1
    lv_mem_free(f);
#else
    free(f);
#endif
  }
}

// 获取空闲堆大小
int heap_free_size(void) {
  int size = 0;
  struct mallinfo info;
  info = mallinfo();
  CLOG_I(TAG, "Internal flash total size:  %u bytes\n", info.arena);
  CLOG_I(TAG, "Internal flash total allocated space:  %u bytes\n", info.uordblks);
  CLOG_I(TAG, "Internal flash total free space:       %u bytes\n", info.fordblks);
  size = info.fordblks;
#if USE_EXTERNAL_SRAM == 1
  CLOG_I(TAG, "freeRTOS free heap size is %u\r\n", xPortGetFreeHeapSize());
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  uint32_t used_size = mon.total_size - mon.free_size;
  uint32_t used_kb = used_size / 1024;
  uint32_t used_kb_tenth = (used_size - (used_kb * 1024)) / 102;
  uint32_t total_kb = mon.total_size / 1024;
  uint32_t total_kb_tenth = (mon.total_size - (total_kb * 1024)) / 102;
  uint32_t free_kb = mon.free_size / 1024;
  uint32_t free_kb_tenth = (mon.free_size - (free_kb * 1024)) / 102;
  CLOG_I(TAG,
         "External flash %d.%d  kB used (%d %%)\r\n"
         "total size %d %d.%dkB\r\n"
         "free size %d %d.%dkB\r\n"
         "%d%% frag.\r\n",
         used_kb, used_kb_tenth, mon.used_pct, mon.total_size, total_kb, total_kb_tenth, mon.free_size, free_kb, free_kb_tenth, mon.frag_pct);
#endif
  return size;
}
