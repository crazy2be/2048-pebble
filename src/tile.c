#include "tile.h"
#include "grid.h"

static const int TILE_SIZE = 30;
static const int BORDER_SIZE = 12;

static GBitmap* s_tiles_bitmap = NULL;
static GBitmap* s_tile_bitmap[TILE_MAX_VAL] = {NULL};

void tile_draw_glyph(GContext* ctx, int xs, int ys, int val) {
  if (s_tiles_bitmap == NULL) {
    s_tiles_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TILES_BITMAP);
  }
  if (val > TILE_MAX_VAL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Val %d out of bounds! Ignoring", val);
    return;
  }
  if (s_tile_bitmap[val] == NULL) {
    int s = TILE_SIZE;
    int col = val % 4;
    int row = val / 4;
    GRect bitmap_rect = GRect(col*s + 1, row*s + 1, s - 2, s - 2);
    s_tile_bitmap[val] = gbitmap_create_as_sub_bitmap(s_tiles_bitmap, bitmap_rect);
  }

  GRect screen_rect = GRect(xs + 1, ys + 1, TILE_SIZE - 2, TILE_SIZE - 2);
//   graphics_context_set_compositing_mode(ctx, GCompOpAnd);
  graphics_draw_bitmap_in_rect(ctx, s_tile_bitmap[val], screen_rect);
}

void tile_draw_rects(GContext* ctx, int xs, int ys, int val) {
  val++;
  if (val < 7) {
    for (int i = val; i > 0; i--) {
      if ((val - i) % 2 == 1) continue;
      graphics_draw_rect(ctx, (GRect) {
        .origin = GPoint(xs + TILE_SIZE/2 - i, ys + TILE_SIZE/2 - i),
        .size = GSize(i*2, i*2),
      });
    }
  } else if (val < 9) {
    for (int i = val; i > 0; i--) {
      if (i % 3 == 0) continue;
      graphics_draw_rect(ctx, (GRect) {
        .origin = GPoint(xs + TILE_SIZE/2 - i, ys + TILE_SIZE/2 - i),
        .size = GSize(i*2, i*2),
      });
    }
  } else if (val < 11) {
    for (int i = val; i > 0; i--) {
      if (i % 4 == 0) continue;
      graphics_draw_rect(ctx, (GRect) {
        .origin = GPoint(xs + TILE_SIZE/2 - i, ys + TILE_SIZE/2 - i),
        .size = GSize(i*2, i*2),
      });
    }
  } else {
    for (int i = val; i > 0; i--) {
      if (i % 5 == 0) continue;
      graphics_draw_rect(ctx, (GRect) {
        .origin = GPoint(xs + TILE_SIZE/2 - i, ys + TILE_SIZE/2 - i),
        .size = GSize(i*2, i*2),
      });
    }
  }
}

void tile_draw_text(GContext* ctx, int xs, int ys, int val) {
  static char score_buf[100];
  snprintf(score_buf, 100, "%d", 1 << val);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx,
                     score_buf,
                     fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                     GRect(xs + 1, ys + 5, TILE_SIZE - 2, TILE_SIZE - 2),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentCenter,
                     NULL);
}

static long unsigned int ms_time() {
  return time_ms(NULL, NULL) + time(NULL)*1000;
}

typedef void (*DrawMethod)(GContext*, int, int, int);

void profile_draw_method(GContext* ctx, int xs, int ys, int val, DrawMethod func, const char* name) {
  long unsigned int start_time = ms_time();
  for (int i = 0; i < 100; i++) {
    func(ctx, xs, ys, val);
  }
  long unsigned int end_time = ms_time();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Running method %s took %lu ms", name, end_time - start_time);
}

void tile_draw(GContext *ctx, GPoint origin, int val) {
  if (val == 0) return;

  int xs = origin.x;
  int ys = origin.y;

//   graphics_draw_rect(ctx, GRect(xs + 1, ys + 1, TILE_SIZE - 2, TILE_SIZE - 2));

  tile_draw_glyph(ctx, xs, ys, val);
//   profile_draw_method(ctx, xs, ys, val, tile_draw_glyph, "glpyh");
//   profile_draw_method(ctx, xs, ys, val, tile_draw_rects, "rects");
//   profile_draw_method(ctx, xs, ys, val, tile_draw_text, "text");
}

GPoint tile_position(Cell cell) {
  return GPoint(
    cell.x*TILE_SIZE + BORDER_SIZE,
    (168 - BORDER_SIZE) - (GRID_SIZE - cell.y)*TILE_SIZE);
}
