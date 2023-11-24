#ifndef _H_WS2812_H_
#define _H_WS2812_H_

#include <stdint.h>

#include "main.h"

#define ONE_PULSE       (70)                       // 1 码
#define ZERO_PULSE      (35)                       // 0 码
#define RESET_PULSE     (105)                      // 80 ，复位信号
#define LED_NUMS        (12)                       // led 数量
#define LED_DATA_LEN    (24)                       // led 长度，单个需要24bits
#define WS2812_DATA_LEN (LED_NUMS * LED_DATA_LEN)  // ws2812灯条需要的数组长度
#define WS2812_SIZE     (RESET_PULSE + WS2812_DATA_LEN + RESET_PULSE)
extern uint16_t RGB_buffur[WS2812_SIZE];
#ifdef __cplusplus
extern "C" {
#endif

void ws2812_set_RGB(uint8_t R, uint8_t G, uint8_t B, uint16_t num);
void ws2812_blue(uint8_t led_nums);
void ws2812_red(uint8_t led_nums);
void ws2812_green(uint8_t led_nums);
void ws2812_init(uint8_t led_nums);
void ws2812_set_color(uint32_t rgb, uint16_t num);
uint32_t ws2812_rgb(uint16_t num);
void ws2812b_output(uint32_t *pData, uint16_t Length);
void ws2812b_output_all(void);

#ifdef __cplusplus
}
#endif
#endif
