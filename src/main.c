#include "board.h"
// #include "accel.h"
#include "button.h"

static Window *window;
static Layer *game_layer;


static void click_config_provider(void *context) {
//   accel_click_config_provider(context);
  button_click_config_provider(context);
}

static void game_layer_update_callback(Layer *me, GContext *ctx) {
//   accel_draw(ctx);
  button_draw(ctx);

  board_draw(ctx);
}

static void timer_callback(void *data) {
  app_timer_register(16, timer_callback, NULL);
  layer_mark_dirty(game_layer);
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
  button_init();

  app_timer_register(100, timer_callback, NULL);

  window = window_create();
  window_set_fullscreen(window, true);
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
