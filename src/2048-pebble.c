#include "2048-board.h"

static Window *window;
static Layer *game_layer;

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

static void click_config_provider(void *context) {
//   accel_click_config_provider(context);
  window_single_click_subscribe(BUTTON_ID_BACK, button_pressed_handler);
  window_single_click_subscribe(BUTTON_ID_UP, button_pressed_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, button_pressed_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, button_pressed_handler);
}

static void game_layer_update_callback(Layer *me, GContext *ctx) {
//   accel_draw(ctx);
  board_draw(ctx);
}

static void timer_callback(void *data) {
  layer_mark_dirty(game_layer);
  app_timer_register(100, timer_callback, NULL);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  game_layer = layer_create(bounds);
  layer_add_child(window_layer, game_layer);
  layer_set_update_proc(game_layer, game_layer_update_callback);
}

static void window_unload(Window *window) {
  layer_destroy(game_layer);
}

static void init(void) {
  board_init();

  // accel_init()
  app_timer_register(100, timer_callback, NULL);

  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  accel_data_service_unsubscribe();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
