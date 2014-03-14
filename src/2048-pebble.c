#include <pebble.h>

static Window *window;
static Layer *game_layer;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
//   text_layer_set_text(text_layer, "Select");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void game_layer_update_callback(Layer *me, GContext *ctx) {
  AccelData accel = {0};
  accel_service_peek(&accel);
//   graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_draw_circle(ctx, GPoint(144/2, 144/2), 10);
  graphics_fill_circle(ctx, GPoint(accel.x/10 + 144/2, -accel.y/10 + 144/2), (accel.z + 4000)/200);

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
  accel_data_service_subscribe(0, NULL);
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
