#pragma once
#include "main.h"
#ifdef __cplusplus
extern "C" {
#endif

uint32_t DWT_Delay_Init(void);

/**
 * @brief  This function provides a delay (in microseconds)
 * @param  microseconds: delay in microseconds
 */
__STATIC_INLINE void DWT_Delay_us(volatile uint32_t microseconds) {
  uint32_t clk_cycle_start = DWT->CYCCNT;

  /* Go to number of cycles for system */
  microseconds *= (HAL_RCC_GetHCLKFreq() / 1000000);

  /* Delay till end */
  while ((DWT->CYCCNT - clk_cycle_start) < microseconds)
    ;
}

__STATIC_INLINE uint32_t OS_TS_GET(void)

{
  uint32_t _get_ts;

  uint32_t _ts;

  static uint32_t _ts_bak; /* 时间戳备份 */

  _get_ts = DWT->CYCCNT;

  if (_get_ts < _ts_bak) {
    /* 做溢出修正 */
    _ts = 0XFFFFFFFF - _ts_bak + _get_ts;

    /* 加上上次数据 即可求出本次时间差*/
    _ts = _ts + _ts_bak;
  } else {
    /* 正常情况 */
    _ts = _get_ts;
  }

  _ts_bak = _ts;

  return _ts;
}

#ifdef __cplusplus
}
#endif
