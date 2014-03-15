#include "accel.h"
#include "game.h"

static const GPathInfo TRIANGLE_PATH_INFO = {
  .num_points = 3,
  .points = (GPoint []) {{-10, 10}, {0, 0}, {10, 10}},
};
static GPath *s_triangle_path = NULL;
static GPath *s_triangle_strength[10] = {NULL};

static bool s_input_paused = true;

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
  game_undo();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_input_paused = !s_input_paused;
}

void accel_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
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
    return abs_int16(accel.x)/10;
  } else {
    return abs_int16(accel.y)/15;
  }
}

void draw_all_directions(GContext *ctx) {
  for (Direction d = 0; d < 4; d++) {
    transform_path(s_triangle_path, d, GPoint(0, 0));
    gpath_draw_outline(ctx, s_triangle_path);
  }
}

static void draw_accel_debug(GContext* ctx, AccelData accel) {
  graphics_draw_circle(ctx, GPoint(144/2, 144/2 + (168-144)), 10);
  graphics_fill_circle(ctx, GPoint(accel.x/10 + 144/2, -accel.y/10 + 144/2 + (168-144)), (accel.z + 4000)/200);
}

static void draw_accel_arrows(GContext* ctx, Direction dir, int strength) {
  if (strength == 0) {
    draw_all_directions(ctx);
    return;
  }
  transform_path(s_triangle_path, dir, GPoint(0, 0));
  gpath_draw_outline(ctx, s_triangle_path);
  for (int i = 0; i < 10; i++) {
    if (strength < i + 1) {
      continue;
    }
    transform_path(s_triangle_strength[i], dir, GPoint(0, 0));
    gpath_draw_outline(ctx, s_triangle_strength[i]);
  }
}

static void draw_paused_glyph(GContext* ctx) {
  int h = 50;
  int w = 20;
  int gap = 12;
  graphics_fill_rect(ctx, GRect(144/2-w-gap/2, 168/2-h/2, w, h), 0, GCornersAll);
  graphics_fill_rect(ctx, GRect(144/2+gap/2, 168/2-h/2, w, h), 0, GCornersAll);
}

static Direction s_last_move_direction = DIRECTION_NONE;
static Direction s_last_sample_direction = DIRECTION_NONE;
static int s_last_sample_strength = 0;

void accel_draw(GContext *ctx) {
  if (s_input_paused) {
    draw_accel_arrows(ctx, s_last_sample_direction, s_last_sample_strength);
    draw_paused_glyph(ctx);
    return;
  }

  AccelData accel = {0};
  accel_service_peek(&accel);

  if (accel.did_vibrate) {
    return;
  }

  Direction dir = read_accel_direction(accel);
  int strength = read_accel_strength(accel);

//   draw_accel_debug(ctx, accel);
  draw_accel_arrows(ctx, dir, strength);

  if (strength >= 10 && dir != s_last_move_direction) {
    game_move(dir);
    s_last_move_direction = dir;
  } else if (strength <= 7) {
    s_last_move_direction = DIRECTION_NONE;
  }

  s_last_sample_direction = dir;
  s_last_sample_strength = strength;
}

