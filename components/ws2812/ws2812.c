#include "ws2812.h"

#include "main.h"
#include "tim.h"

uint16_t RGB_buffur[WS2812_SIZE] = {0};

void ws2812b_output(uint32_t *pData, uint16_t Length) {
  HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_1, pData, Length);
}
void ws2812b_output_all(void) {
  HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_1, (uint32_t *) RGB_buffur, WS2812_SIZE);
}

uint32_t ws2812_rgb(uint16_t num) {
  uint16_t *p = (RGB_buffur + RESET_PULSE) + (num * LED_DATA_LEN);
  uint32_t color = 0;
  uint32_t r = 0;
  uint32_t g = 0;
  uint32_t b = 0;
  for (uint16_t i = 0; i < 8; i++) {
    g |= *(p + i) == ONE_PULSE ? 1 << (7 - i) : 0 << (7 - i);
    r |= *(p + i + 8) == ONE_PULSE ? 1 << (7 - i) : 0 << (7 - i);
    b |= *(p + i + 16) == ONE_PULSE ? 1 << (7 - i) : 0 << (7 - i);
  }
  color = r << 16 | g << 8 | b;
  return color;
}
void ws2812_set_RGB(uint8_t R, uint8_t G, uint8_t B, uint16_t num) {
  uint16_t *p = (RGB_buffur + RESET_PULSE) + (num * LED_DATA_LEN);
  for (uint16_t i = 0; i < 8; i++) {
    p[i] = G & (1 << (7 - i)) ? ONE_PULSE : ZERO_PULSE;
    p[i + 8] = R & (1 << (7 - i)) ? ONE_PULSE : ZERO_PULSE;
    p[i + 16] = B & (1 << (7 - i)) ? ONE_PULSE : ZERO_PULSE;
  }
}
void ws2812_set_color(uint32_t rgb, uint16_t num) {
  uint16_t *p = (RGB_buffur + RESET_PULSE) + (num * LED_DATA_LEN);
  uint8_t R = (rgb & 0x00FF0000) >> 16;
  uint8_t G = (rgb & 0x0000FF00) >> 8;
  uint8_t B = rgb & 0x000000FF;
  for (uint16_t i = 0; i < 8; i++) {
    p[i] = G & (1 << (7 - i)) ? ONE_PULSE : ZERO_PULSE;
    p[i + 8] = R & (1 << (7 - i)) ? ONE_PULSE : ZERO_PULSE;
    p[i + 16] = B & (1 << (7 - i)) ? ONE_PULSE : ZERO_PULSE;
  }
  //  printf("[set color] num:%d,color:%06X\r\n",num,rgb);
}
/*ws2812 初始化*/
void ws2812_init(uint8_t led_nums) {
  for (uint8_t i = 0; i < led_nums; i++) {
    ws2812_set_RGB(0x0, 0x00, 0x00, i);
  }
  ws2812b_output_all();
}

/*全亮蓝灯*/
void ws2812_blue(uint8_t led_nums) {
  uint16_t num_data;
  num_data = RESET_PULSE + led_nums * LED_DATA_LEN;
  for (uint8_t i = 0; i < led_nums; i++) {
    ws2812_set_RGB(0x00, 0x00, 0x22, i);
  }
  ws2812b_output((uint32_t *) RGB_buffur, (num_data));
}
/*全亮红灯*/
void ws2812_red(uint8_t led_nums) {
  uint16_t num_data;
  num_data = RESET_PULSE + led_nums * LED_DATA_LEN;
  for (uint8_t i = 0; i < led_nums; i++) {
    ws2812_set_RGB(0x22, 0x00, 0x00, i);
  }
  ws2812b_output((uint32_t *) RGB_buffur, (num_data));
}
/*全亮绿灯*/
void ws2812_green(uint8_t led_nums) {
  uint16_t num_data;
  num_data = RESET_PULSE + led_nums * LED_DATA_LEN;
  for (uint8_t i = 0; i < led_nums; i++) {
    ws2812_set_RGB(0x00, 0x22, 0x00, i);
  }
  ws2812b_output((uint32_t *) RGB_buffur, (num_data));
}
