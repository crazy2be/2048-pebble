#include "accel.h"
#include "2048-board.h"

static const GPathInfo TRIANGLE_PATH_INFO = {
  .num_points = 3,
  .points = (GPoint []) {{-10, 10}, {0, 0}, {10, 10}},
};

static GPath *s_triangle_path = NULL;

static bool s_select_down = false;
static Direction s_current_direction;

void accel_init() {
  s_triangle_path = gpath_create(&TRIANGLE_PATH_INFO);  accel_data_service_subscribe(0, NULL);
}

static void select_down_handler(ClickRecognizerRef recognizer, void *context) {
  s_select_down = true;
}

static void select_up_handler(ClickRecognizerRef recognizer, void *context) {
  s_select_down = false;
  board_move(s_current_direction);
}

void accel_click_config_provider(void *context) {
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_down_handler, select_up_handler, NULL);
}

static int16_t abs_int16(int16_t n) {
  return n > 0 ? n : -n;
}

static GPoint base_path_position(Direction dir) {
  switch (dir) {
  case DIRECTION_UP:
    return GPoint(144/2, 0);
  case DIRECTION_RIGHT:
    return GPoint(144, 144/2);
  case DIRECTION_DOWN:
    return GPoint(144/2, 144);
  case DIRECTION_LEFT:
    return GPoint(0, 144/2);
  }
  assert(false);
  return GPoint(0, 0);
}

static void transform_path(GPath* path, Direction dir, GPoint accel) {
  int32_t two_pi = TRIG_MAX_ANGLE;
  gpath_rotate_to(path, two_pi/4 * dir);
  GPoint base = base_path_position(dir);
  switch (dir) {
  case DIRECTION_UP: case DIRECTION_DOWN:
    gpath_move_to(path, GPoint(base.x + accel.x, base.y));
    break;
  case DIRECTION_LEFT: case DIRECTION_RIGHT:
    gpath_move_to(path, GPoint(base.x, base.y + accel.y));
    break;
  }
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

void accel_draw(GContext *ctx) {
  AccelData accel = {0};
  accel_service_peek(&accel);

  graphics_draw_circle(ctx, GPoint(144/2, 144/2), 10);
  graphics_fill_circle(ctx, GPoint(accel.x/10 + 144/2, -accel.y/10 + 144/2), (accel.z + 4000)/200);

  Direction dir = read_accel_direction(accel);
  s_current_direction = dir;
  GPoint offset = GPoint(
    accel.x*50 / abs_int16(accel.y),
    -accel.y*50 / abs_int16(accel.x));
  transform_path(s_triangle_path, dir, offset);
  // Draw filled doesn't also draw the outline...
  gpath_draw_outline(ctx, s_triangle_path);
  if (s_select_down) {
    gpath_draw_filled(ctx, s_triangle_path);
  }
}
