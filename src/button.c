#include "button.h"
#include "board.h"

static void button_pressed_handler(ClickRecognizerRef recognizer, void *context) {
  ButtonId button = click_recognizer_get_button_id(recognizer);
  switch (button) {
  case BUTTON_ID_BACK:
    board_move(DIRECTION_LEFT);
    break;
  case BUTTON_ID_UP:
    board_move(DIRECTION_UP);
    break;
  case BUTTON_ID_DOWN:
    board_move(DIRECTION_DOWN);
    break;
  case BUTTON_ID_SELECT:
    board_move(DIRECTION_RIGHT);
    break;
  default:
    assert(false);
  }
}

void button_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, button_pressed_handler);
  window_single_click_subscribe(BUTTON_ID_UP, button_pressed_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, button_pressed_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, button_pressed_handler);
}

void button_init() {}

void button_draw(GContext *ctx) {}
