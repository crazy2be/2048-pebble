#include "tile.h"
#include "grid.h"

static const int TILE_SIZE = 30;
static const int BORDER_SIZE = 12;

static GBitmap* s_tiles_bitmap = NULL;
static GBitmap* s_tile_bitmap[TILE_MAX_VAL] = {NULL};

void tile_draw(GContext *ctx, GPoint origin, int val) {
  if (val > TILE_MAX_VAL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Val %d out of bounds! Ignoring", val);
    return;
  }

  // We don't draw "empty" tiles, so that you can see the "ground" under
  // them when tiles are animating.
  if (val == 0) return;

  int xs = origin.x;
  int ys = origin.y;

  if (s_tiles_bitmap == NULL) {
    s_tiles_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TILES_BITMAP);
  }

  if (s_tile_bitmap[val] == NULL) {
    int s = TILE_SIZE;
    int col = val % 4;
    int row = val / 4;
    // Leaving a little bit of a gap between tiles ensures you can always
    // see tile animations. Ideally this would be represented by transparency
    // in the PNG, but that needs more memory, and doesn't seem to be
    // natively supported by the Pebble SDK. So we just ignore a few pixels
    // instead.
    GRect bitmap_rect = GRect(col*s + 1, row*s + 1, s - 2, s - 2);
    s_tile_bitmap[val] = gbitmap_create_as_sub_bitmap(s_tiles_bitmap, bitmap_rect);
  }

  GRect screen_rect = GRect(xs + 1, ys + 1, TILE_SIZE - 2, TILE_SIZE - 2);
  graphics_draw_bitmap_in_rect(ctx, s_tile_bitmap[val], screen_rect);
}

GPoint tile_position(Cell cell) {
  return GPoint(
    cell.x*TILE_SIZE + BORDER_SIZE,
    (168 - BORDER_SIZE) - (GRID_SIZE - cell.y)*TILE_SIZE);
}
