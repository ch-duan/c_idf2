#include "FreeRTOS.h"
#ifndef uxTopUsedPriority
#ifdef __GNUC__
#define USED __attribute__((used))
#else
#define USED
#endif

const int USED uxTopUsedPriority = configMAX_PRIORITIES - 1;
#endif