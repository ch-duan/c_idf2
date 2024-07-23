#ifndef H_C_IDF_BUTTON_CUSTOM_H
#define H_C_IDF_BUTTON_CUSTOM_H
#include "stdint.h"
#include "string.h"

// According to your need to modify the constants.
#define TICKS_INTERVAL       5  // ms
#define DEBOUNCE_TICKS       3  // MAX 7 (0 ~ 7)
#define SHORT_TICKS          (300 / TICKS_INTERVAL)
#define LONG_TICKS           (1000 / TICKS_INTERVAL)
#define PRESS_REPEAT_MAX_NUM 15 /*!< The maximum value of the repeat counter */

typedef void (*BtnCallback)(void*);

typedef enum {
  PRESS_DOWN = 0,
  PRESS_UP,
  PRESS_REPEAT,
  SINGLE_CLICK,
  MULTIPLE_CLICK,
  LONG_PRESS_START,
  LONG_PRESS_HOLD,
  number_of_event,
  NONE_PRESS
} PressEvent;

typedef struct Button {
  uint16_t ticks;
  uint8_t repeat;
  uint8_t event;
  uint8_t state : 3;
  uint8_t debounce_cnt : 3;
  uint8_t active_level : 1;
  uint8_t button_level : 1;
  uint16_t button_id;
  uint8_t multiple_click;
  uint16_t long_ticks;
  uint8_t (*hal_button_Level)(uint16_t button_id_);
  BtnCallback cb[number_of_event];
  struct Button* next;
} Button;

#ifdef __cplusplus
extern "C" {
#endif

void button_init(struct Button* handle, uint8_t (*pin_level)(uint16_t), uint8_t active_level, uint16_t button_id);
void button_attach(struct Button* handle, PressEvent event, BtnCallback cb);
PressEvent get_button_event(struct Button* handle);
int button_start(struct Button* handle);
void button_stop(struct Button* handle);
void button_ticks(void);
void button_set_long_ticks(struct Button* handle, uint16_t long_ticks);
void button_set_multiple_click(struct Button* handle, uint16_t multiple_click);

#ifdef __cplusplus
}
#endif

#endif