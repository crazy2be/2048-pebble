#include "board.h"
#include "grid.h"

#define BOARD_SIZE 4

typedef void (*BoardTraversalCallback)(GPoint cell);

typedef struct {
  GPoint furthest;
  GPoint next;
} BoardFurthestPosition;

// (empty), 2, 4, 8, 16,  32, 64, 128, 256,  512, 1024, 2048
static const int MAX_VAL = 11;
static const int TILE_SIZE = 30;
static const int BORDER_SIZE = 12;
static int s_board_merged[BOARD_SIZE][BOARD_SIZE];
static int s_total_score = 0;
static bool s_won_game = false;
static bool s_lost_game = false;

static void tile_draw(GContext *ctx, int xs, int ys, int w, int h, int val) {
  if (val == 0) return;
  graphics_draw_rect(ctx, GRect(xs + 1, ys + 1, w - 2, h - 2));
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

void board_draw(GContext *ctx) {
  static char score_buf[100];
  snprintf(score_buf, 100, "Score: %d", s_total_score);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx,
                     score_buf,
                     fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                     GRect(0, 0, 144, 20),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentCenter,
                     NULL);

  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      int x = i*TILE_SIZE + BORDER_SIZE;
      int y = j*TILE_SIZE + BORDER_SIZE + 20;
      Cell cell = Cell(i, j);
      int val = grid_cell_value(cell);
      tile_draw(ctx, x, y, TILE_SIZE, TILE_SIZE, val);
    }
  }

  if (s_won_game || s_lost_game) {
    for (int xp = 0; xp < 144; xp++) {
      for (int yp = 0; yp < 168; yp++) {
        if ((xp + yp) % 2 == 0) {
          graphics_draw_pixel(ctx, GPoint(xp, yp));
        }
      }
    }
    graphics_draw_text(ctx,
                       s_won_game ? "Win!" : "Lose :(",
                       fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
                       GRect(0, (168 - 42)/2, 144, 50),
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);
  }
}

void board_merged_reset() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      s_board_merged[i][j] = 0;
    }
  }
}

void board_add_random_tile() {
  assert(grid_has_empty_cells());
  while (true) {
    int x = rand() % BOARD_SIZE;
    int y = rand() % BOARD_SIZE;
    GPoint cell = GPoint(x, y);
    if (grid_cell_empty(cell)) {
      int r = rand() % 10;
      int val = r == 0 ? 2 : 1;
      grid_cell_set_value(cell, val);
      return;
    }
  }
}

void board_tile_sampler() {
  for (int i = 0; i < BOARD_SIZE*BOARD_SIZE; i++) {
    int x = i % BOARD_SIZE;
    int y = i / BOARD_SIZE;
    grid_cell_set_value(Cell(x, y), i > MAX_VAL ? 0 : i);
  }
}

void board_init() {
  grid_init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Almost done board init!");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Has empty %d", grid_has_empty_cells());
  board_add_random_tile();
  board_add_random_tile();
  board_merged_reset();
}

bool board_merged_cell_already(GPoint cell) {
  assert(grid_cell_valid(cell));
  return s_board_merged[cell.x][cell.y];
}

void board_merged_cell_set_merged(GPoint cell) {
  assert(grid_cell_valid(cell));
  s_board_merged[cell.x][cell.y] = 1;
}

BoardFurthestPosition board_find_furthest_position(GPoint cell, GPoint dir) {
  GPoint prev;

  do {
    prev = cell;
    cell.x = prev.x + dir.x;
    cell.y = prev.y + dir.y;
  } while (grid_cell_valid(cell) && grid_cell_empty(cell));

  return (BoardFurthestPosition) {
    .furthest = prev,
    .next = cell,
  };
}

void board_traverse(GPoint dir, BoardTraversalCallback callback) {
  int xs = 0, xd = 1;
  int ys = 0, yd = 1;
  if (dir.x == 1) {
    xs = BOARD_SIZE - 1;
    xd = -1;
  }
  if (dir.y == 1) {
    ys = BOARD_SIZE - 1;
    yd = -1;
  }
  for (int x = xs; x >= 0 && x < BOARD_SIZE; x += xd) {
    for (int y = ys; y >= 0 && y < BOARD_SIZE; y += yd) {
      GPoint cell = GPoint(x, y);
      callback(cell);
    }
  }
}


GPoint vector_from_direction(Direction raw_dir) {
  switch (raw_dir) {
  case DIRECTION_UP:
    return GPoint(0, -1);
  case DIRECTION_RIGHT:
    return GPoint(1, 0);
  case DIRECTION_DOWN:
    return GPoint(0, 1);
  case DIRECTION_LEFT:
    return GPoint(-1, 0);
  }
  assert(false);
  return GPoint(0, 0);
}

bool s_moved_this_turn = false;

void board_move_tile(GPoint start, GPoint end) {
  if (start.x == end.x && start.y == end.y) {
    return;
  }
  assert(grid_cell_empty(end));
  int val = grid_cell_value(start);
  grid_cell_set_value(start, 0);
  grid_cell_set_value(end, val);
  s_moved_this_turn = true;
}

GPoint s_move_vector;

void board_move_traversal_callback(GPoint cell) {
  if (grid_cell_empty(cell)) return;

  int val = grid_cell_value(cell);
  BoardFurthestPosition positions = board_find_furthest_position(cell, s_move_vector);

  if (!grid_cell_valid(positions.next)) {
    board_move_tile(cell, positions.furthest);
    return;
  }

  int nextVal = grid_cell_value(positions.next);
  if (nextVal != val || board_merged_cell_already(positions.next)) {
    board_move_tile(cell, positions.furthest);
    return;
  }

  grid_cell_set_value(cell, 0);
  grid_cell_set_value(positions.next, val + 1);
  s_moved_this_turn = true;
  s_total_score += 1 << val;
  if (val == MAX_VAL) {
    s_won_game = true;
  }
}

bool board_tile_matches_possible() {
  for (int x = 0; x < BOARD_SIZE; x++) {
    for (int y = 0; y < BOARD_SIZE; y++) {
      Cell cell = Cell(x, y);
      if (grid_cell_empty(cell)) {
        continue;
      }
      int val = grid_cell_value(cell);
      for (Direction d = 0; d < 4; d++) {
        GPoint dir = vector_from_direction(d);
        Cell other = Cell(x + dir.x, y + dir.y);
        if (!grid_cell_valid(other)) {
          continue;
        }
        int otherVal = grid_cell_value(other);
        if (val == otherVal) {
          return true;
        }
      }
    }
  }
  return false;
}

bool board_moves_available() {
  return grid_has_empty_cells() || board_tile_matches_possible();
}

void board_move(Direction raw_dir) {
  if (s_won_game || s_lost_game) {
    return;
  }
  s_move_vector = vector_from_direction(raw_dir);
  s_moved_this_turn = false;
  board_merged_reset();
  board_traverse(s_move_vector, board_move_traversal_callback);
  if (s_moved_this_turn) {
    board_add_random_tile();
    if (!board_moves_available()) {
      s_lost_game = true;
    }
  }
}
