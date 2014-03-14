#include "2048-board.h"

static Window *window;
static Layer *game_layer;

static const GPathInfo TRIANGLE_PATH_INFO = {
  .num_points = 3,
  .points = (GPoint []) {{-10, 10}, {0, 0}, {10, 10}},
};

static GPath *s_triangle_path = NULL;
static bool s_select_down = false;
static Direction s_current_direction;

static void select_down_handler(ClickRecognizerRef recognizer, void *context) {
  s_select_down = true;
}

static void select_up_handler(ClickRecognizerRef recognizer, void *context) {
  s_select_down = false;
  board_move(s_current_direction);
}

static void click_config_provider(void *context) {
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_down_handler, select_up_handler, NULL);
}

static int16_t abs_int16(int16_t n) {
  return n > 0 ? n : -n;
}

static Direction read_accel_direction(AccelData accel) {
  if (abs_int16(accel.x) > abs_int16(accel.y)) {
    if (accel.x > 0) {
      return DIRECTION_RIGHT;
    }
    return DIRECTION_LEFT;
  } else {
    if (accel.y > 0) {
      return DIRECTION_UP;
    }
    return DIRECTION_DOWN;
  }
}

static void transform_path(GPath* path, Direction dir) {
  int32_t two_pi = TRIG_MAX_ANGLE;
  gpath_rotate_to(path, two_pi/4 * dir);
  switch (dir) {
  case DIRECTION_UP:
    gpath_move_to(path, GPoint(144/2, 0));
    break;
  case DIRECTION_RIGHT:
    gpath_move_to(path, GPoint(144, 144/2));
    break;
  case DIRECTION_DOWN:
    gpath_move_to(path, GPoint(144/2, 144));
    break;
  case DIRECTION_LEFT:
    gpath_move_to(path, GPoint(0, 144/2));
    break;
  }
}

static void game_layer_update_callback(Layer *me, GContext *ctx) {
  AccelData accel = {0};
  accel_service_peek(&accel);

  graphics_draw_circle(ctx, GPoint(144/2, 144/2), 10);
  graphics_fill_circle(ctx, GPoint(accel.x/10 + 144/2, -accel.y/10 + 144/2), (accel.z + 4000)/200);

  Direction dir = read_accel_direction(accel);
  s_current_direction = dir;
  transform_path(s_triangle_path, dir);
  // Draw filled doesn't also draw the outline...
  gpath_draw_outline(ctx, s_triangle_path);
  if (s_select_down) {
    gpath_draw_filled(ctx, s_triangle_path);
  }

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

  s_triangle_path = gpath_create(&TRIANGLE_PATH_INFO);
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
