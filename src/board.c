#include "board.h"
#include "grid.h"
#include "tile.h"

static Grid* s_grid = NULL;

typedef struct {
  GPoint from;
  GPoint to;
  int val;
  uint64_t start_time;
  int duration;
} TileAnimation;

#define MAX_ANIMATIONS (GRID_SIZE*GRID_SIZE)
struct {
  TileAnimation list[MAX_ANIMATIONS];
  int start_index;
  int end_index;
  int max_animations;
  bool cell_animating[GRID_SIZE][GRID_SIZE];
} s_animation_state;

uint64_t ms_time() {
  return ((uint64_t)time_ms(NULL, NULL)) + ((uint64_t)time(NULL))*1000;
}

void board_animations_init() {
  memset(&s_animation_state, 0, sizeof(s_animation_state));
}

void board_add_animation(Cell from, Cell to, int val) {
  int wrapped = s_animation_state.end_index % MAX_ANIMATIONS;
  s_animation_state.list[wrapped] = (TileAnimation) {
    .from = from,
    .to = to,
    .val = val,
    .start_time = ms_time(),
    .duration = 500,
  };
  s_animation_state.end_index++;
}

void board_add_move_animation(Cell from, Cell to, int val) {
  board_add_animation(from, to, val);
  s_animation_state.cell_animating[to.x][to.y] = true;
}

#define ALPHA_MAX 100000
static GPoint point_lerp(GPoint from, GPoint to, int alpha) {
  int x = (from.x*(ALPHA_MAX - alpha) + to.x*(alpha))/ALPHA_MAX;
  int y = (from.y*(ALPHA_MAX - alpha) + to.y*(alpha))/ALPHA_MAX;
  return GPoint(x, y);
}

void board_draw_animations(GContext* ctx) {
  uint64_t cur_time = ms_time();
  for (int i = s_animation_state.start_index; i < s_animation_state.end_index; i++) {
    int wrapped = i % MAX_ANIMATIONS;
    TileAnimation a = s_animation_state.list[wrapped];

    if (cur_time >= a.start_time + a.duration) {
      s_animation_state.start_index++;
      s_animation_state.cell_animating[a.to.x][a.to.y] = false;
      continue;
    }

    int alpha = (cur_time - a.start_time)*ALPHA_MAX/a.duration;

    GPoint point = point_lerp(tile_position(a.from), tile_position(a.to), alpha);
    tile_draw(ctx, point, a.val);
  }
}

void board_draw(GContext *ctx) {
  board_draw_animations(ctx);

  for (int i = 0; i < GRID_SIZE; i++) {
    for (int j = 0; j < GRID_SIZE; j++) {
      Cell cell = Cell(i, j);
      if (s_animation_state.cell_animating[cell.x][cell.y]) {
        continue;
      }
      int val = grid_cell_value(s_grid, cell);
      GPoint pos = tile_position(cell);
      tile_draw(ctx, pos, val);
    }
  }
}

void board_set_grid(Grid* g) {
  s_grid = g;
}

void board_add_random_tile() {
  assert(grid_has_empty_cells(s_grid));
  while (true) {
    int x = rand() % GRID_SIZE;
    int y = rand() % GRID_SIZE;
    GPoint cell = GPoint(x, y);
    if (grid_cell_empty(s_grid, cell)) {
      int r = rand() % 10;
      int val = r == 0 ? 2 : 1;
      grid_cell_set_value(s_grid, cell, val);
      return;
    }
  }
}

void board_init() {
  board_animations_init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Almost done board init!");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Has empty %d", grid_has_empty_cells(s_grid));
  board_add_random_tile();
  board_add_random_tile();
}

bool board_tile_matches_possible() {
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      Cell cell = Cell(x, y);
      if (grid_cell_empty(s_grid, cell)) {
        continue;
      }
      int val = grid_cell_value(s_grid, cell);
      for (Direction d = 0; d < 4; d++) {
        GPoint dir = direction_to_vector(d);
        Cell other = Cell(x + dir.x, y + dir.y);
        if (!grid_cell_valid(s_grid, other)) {
          continue;
        }
        int otherVal = grid_cell_value(s_grid, other);
        if (val == otherVal) {
          return true;
        }
      }
    }
  }
  return false;
}

bool board_moves_available() {
  return grid_has_empty_cells(s_grid) || board_tile_matches_possible();
}

bool board_move_tile(Grid* g, Cell start, Cell end) {
  if (start.x == end.x && start.y == end.y) {
    return false;
  }
  if (!grid_cell_empty(g, end)) {
    return false;
  }
  int val = grid_cell_value(g, start);
  grid_cell_set_value(g, start, 0);
  grid_cell_set_value(g, end, val);
  board_add_move_animation(start, end, val);
  return true;
}

void board_traverse(Direction dir, BoardTraversalCallback callback, void* context) {
  Cell start;
  GPoint delta;

  // Always traverse from the farthest cell in the chosen direction
  switch (dir) {
  case DIRECTION_UP:
    start = Cell(0, 0);
    delta = GPoint(1, 1);
    break;
  case DIRECTION_RIGHT:
    start = Cell(GRID_SIZE - 1, 0);
    delta = GPoint(-1, 1);
    break;
  case DIRECTION_DOWN:
    start = Cell(0, GRID_SIZE - 1);
    delta = GPoint(1, -1);
    break;
  case DIRECTION_LEFT:
    start = Cell(0, 0);
    delta = GPoint(1, 1);
    break;
  default:
    assert(false);
    return;
  }

  for (int x = start.x; x >= 0 && x < GRID_SIZE; x += delta.x) {
    for (int y = start.y; y >=  0 && y < GRID_SIZE; y += delta.y) {
      callback(Cell(x, y), context);
    }
  }
}
