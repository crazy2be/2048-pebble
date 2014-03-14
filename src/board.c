#include "board.h"
#include "grid.h"

#define BOARD_SIZE 4

typedef void (*BoardTraversalCallback)(GPoint cell);

typedef struct {
  GPoint furthest;
  GPoint next;
} BoardFurthestPosition;

// (empty), 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
static const int MAX_VAL = 11;
static const int TILE_SIZE = 30;
static const int BORDER_SIZE = 12;
static int s_board_merged[BOARD_SIZE][BOARD_SIZE];

static void tile_draw(GContext *ctx, int xs, int ys, int w, int h, int val) {
  if (val == 0) return;
  graphics_draw_rect(ctx, (GRect) {
    .origin = GPoint(xs + TILE_SIZE/2 - val, ys + TILE_SIZE/2 - val),
    .size = GSize(val*2, val*2),
  });
}

void board_draw(GContext *ctx) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      int x = i*TILE_SIZE + BORDER_SIZE;
      int y = j*TILE_SIZE + BORDER_SIZE;
      Cell cell = Cell(i, j);
      int val = grid_cell_value(cell);
      tile_draw(ctx, x, y, TILE_SIZE, TILE_SIZE, val);
    }
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
    xd = -1;
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
  if (nextVal != val || board_merged_cell_already(cell)) {
    board_move_tile(cell, positions.furthest);
    return;
  }

  grid_cell_set_value(cell, 0);
  grid_cell_set_value(positions.next, val + 1);
  s_moved_this_turn = true;
}

void board_move(Direction raw_dir) {
  s_move_vector = vector_from_direction(raw_dir);
  s_moved_this_turn = false;
  board_merged_reset();
  board_traverse(s_move_vector, board_move_traversal_callback);
  if (s_moved_this_turn) {
    board_add_random_tile();
  }
}
