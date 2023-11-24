#include "delay.h"

#include <stdint.h>

void delay_us(uint32_t u_sec) {
  uint16_t cnt = 0;

  while (u_sec--) {
    for (cnt = 168 / 5; cnt > 0; cnt--)
      ;
  }
}

void delay_ms(uint16_t nms) {
  uint32_t i;
  for (i = 0; i < nms; i++)
    delay_us(1000);
}
