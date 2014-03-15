#include "accel.h"
#include "game.h"

static const GPathInfo TRIANGLE_PATH_INFO = {
  .num_points = 3,
  .points = (GPoint []) {{-10, 10}, {0, 0}, {10, 10}},
};
static GPath *s_triangle_path = NULL;
static GPath *s_triangle_strength[10] = {NULL};

static bool s_select_down = false;
static Direction s_current_direction;

void accel_init() {
  s_triangle_path = gpath_create(&TRIANGLE_PATH_INFO);
  static GPoint point_arrays[10][2];
  GPathInfo triangle_strength_info;
  for (int i = 0; i < 10; i++) {
    int n = 10 - (i + 1);
    triangle_strength_info.num_points = 2;
    point_arrays[i][0] = GPoint(-n, n);
    point_arrays[i][1] = GPoint(n, n);
    triangle_strength_info.points = point_arrays[i];
    s_triangle_strength[i] = gpath_create(&triangle_strength_info);
  }
  accel_data_service_subscribe(0, NULL);
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
//   board_undo(s_current_direction);
}

void accel_click_config_provider(void *context) {
//   window_click_subscribe(BUTTON_ID_BACK, back_click_handler, NULL);
}

static int16_t abs_int16(int16_t n) {
  return n > 0 ? n : -n;
}

static GPoint base_path_position(Direction dir) {
  switch (dir) {
  case DIRECTION_UP:
    return GPoint(144/2, 25);
  case DIRECTION_RIGHT:
    return GPoint(144 - 2, 144/2 + (168-144));
  case DIRECTION_DOWN:
    return GPoint(144/2, 168 - 2);
  case DIRECTION_LEFT:
    return GPoint(1, 144/2 + (168-144));
  default:
    assert(false);
  }
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
  default:
    assert(false);
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

static int read_accel_strength(AccelData accel) {
  if (abs_int16(accel.x) > abs_int16(accel.y)) {
    return abs_int16(accel.x);
  } else {
    return abs_int16(accel.y);
  }
}

void draw_all_directions(GContext *ctx) {
  for (Direction d = 0; d < 4; d++) {
    transform_path(s_triangle_path, d, GPoint(0, 0));
    gpath_draw_outline(ctx, s_triangle_path);
  }
}

static Direction s_last_direction = DIRECTION_NONE;
void accel_draw(GContext *ctx) {
  AccelData accel = {0};
  accel_service_peek(&accel);

  graphics_draw_circle(ctx, GPoint(144/2, 144/2 + (168-144)), 10);
  graphics_fill_circle(ctx, GPoint(accel.x/10 + 144/2, -accel.y/10 + 144/2 + (168-144)), (accel.z + 4000)/200);

  if ((abs_int16(accel.x) < 50 && abs_int16(accel.y) < 50)) {
    draw_all_directions(ctx);
    s_last_direction = DIRECTION_NONE;
    return;
  }

  Direction dir = read_accel_direction(accel);
  int strength = read_accel_strength(accel);

  transform_path(s_triangle_path, dir, GPoint(0, 0));
  gpath_draw_outline(ctx, s_triangle_path);
  for (int i = 0; i < 10; i++) {
    if (strength < (i+1)*50) continue;
    transform_path(s_triangle_strength[i], dir, GPoint(0, 0));
    gpath_draw_outline(ctx, s_triangle_strength[i]);
  }

  if (strength / 50 >= 10 && dir != s_last_direction) {
    game_move(dir);
    s_last_direction = dir;
  } else if (strength / 50 <= 7) {
    s_last_direction = DIRECTION_NONE;
  }
}

