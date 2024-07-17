#ifndef H_C_IDF_MALLOC_H
#define H_C_IDF_MALLOC_H
#include <stddef.h>

#if USE_EXTERNAL_SRAM == 1
#define EXT_RAM_ATTR             __attribute__((section(".sram")))
#define MEMORY_POOL_EXT_RAM_SIZE 1024 * 1024 * 6
#else
#define EXT_RAM_ATTR
#define MEMORY_POOL_EXT_RAM_SIZE 0
#endif

#ifdef __cplusplus
extern "C" {
#endif
void malloc_init();
void *m_malloc(size_t size);
void m_free(void *f);
int heap_free_size(void);

#ifdef __cplusplus
}
#endif
#endif