#include "tile.h"
#include "grid.h"

static const int TILE_SIZE = 30;
static const int BORDER_SIZE = 12;

void tile_draw(GContext *ctx, GPoint origin, int val) {
  if (val == 0) return;
  int xs = origin.x;
  int ys = origin.y;
  graphics_draw_rect(ctx, GRect(xs + 1, ys + 1, TILE_SIZE - 2, TILE_SIZE - 2));
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

GPoint tile_position(Cell cell) {
  return GPoint(
    cell.x*TILE_SIZE + BORDER_SIZE,
    (168 - BORDER_SIZE) - (GRID_SIZE - cell.y)*TILE_SIZE);
}
