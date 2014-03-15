#include "board.h"
#include "grid.h"
#include "tile.h"

typedef void (*BoardTraversalCallback)(GPoint cell, void* context);

// (empty), 2, 4, 8, 16,  32, 64, 128, 256,  512, 1024, 2048
static const int MAX_VAL = 11;
static int s_total_score = 0;
static bool s_won_game = false;
static bool s_lost_game = false;
static Grid s_grid;

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
GPoint point_lerp(GPoint from, GPoint to, int alpha) {
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

  board_draw_animations(ctx);

  for (int i = 0; i < GRID_SIZE; i++) {
    for (int j = 0; j < GRID_SIZE; j++) {
      Cell cell = Cell(i, j);
      if (s_animation_state.cell_animating[cell.x][cell.y]) {
        continue;
      }
      int val = grid_cell_value(&s_grid, cell);
      GPoint pos = tile_position(cell);
      tile_draw(ctx, pos, val);
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

void board_add_random_tile() {
  assert(grid_has_empty_cells(&s_grid));
  while (true) {
    int x = rand() % GRID_SIZE;
    int y = rand() % GRID_SIZE;
    GPoint cell = GPoint(x, y);
    if (grid_cell_empty(&s_grid, cell)) {
      int r = rand() % 10;
      int val = r == 0 ? 2 : 1;
      grid_cell_set_value(&s_grid, cell, val);
      return;
    }
  }
}

void board_tile_sampler() {
  for (int i = 0; i < GRID_SIZE*GRID_SIZE; i++) {
    int x = i % GRID_SIZE;
    int y = i / GRID_SIZE;
    grid_cell_set_value(&s_grid, Cell(x, y), i > MAX_VAL ? 0 : i);
  }
}

void board_init() {
  grid_init(&s_grid);
  board_animations_init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Almost done board init!");
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Has empty %d", grid_has_empty_cells(&s_grid));
  board_add_random_tile();
  board_add_random_tile();
}

bool board_tile_matches_possible() {
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      Cell cell = Cell(x, y);
      if (grid_cell_empty(&s_grid, cell)) {
        continue;
      }
      int val = grid_cell_value(&s_grid, cell);
      for (Direction d = 0; d < 4; d++) {
        GPoint dir = direction_to_vector(d);
        Cell other = Cell(x + dir.x, y + dir.y);
        if (!grid_cell_valid(&s_grid, other)) {
          continue;
        }
        int otherVal = grid_cell_value(&s_grid, other);
        if (val == otherVal) {
          return true;
        }
      }
    }
  }
  return false;
}

bool board_moves_available() {
  return grid_has_empty_cells(&s_grid) || board_tile_matches_possible();
}

bool board_move_tile(Cell start, Cell end) {
  if (start.x == end.x && start.y == end.y) {
    return false;
  }
  if (!grid_cell_empty(&s_grid, end)) {
    return false;
  }
  int val = grid_cell_value(&s_grid, start);
  grid_cell_set_value(&s_grid, start, 0);
  grid_cell_set_value(&s_grid, end, val);
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

typedef struct {
  bool moved;
  Direction direction;
  // No recursive merges allowed
  bool merged_tile[GRID_SIZE][GRID_SIZE];
} BoardMoveState;

void board_move_traversal_callback(GPoint cell, void* context) {
  BoardMoveState* move_state = (BoardMoveState*)context;

  if (grid_cell_empty(&s_grid, cell)) return;

  int val = grid_cell_value(&s_grid, cell);
  Cell next;
  Cell furthest = grid_find_furthest_empty_cell(&s_grid, cell, move_state->direction, &next);

  if (!grid_cell_valid(&s_grid, next)) {
    move_state->moved |= board_move_tile(cell, furthest);
    return;
  }

  int nextVal = grid_cell_value(&s_grid, next);
  if (nextVal != val || move_state->merged_tile[next.x][next.y]) {
    move_state->moved |= board_move_tile(cell, furthest);
    return;
  }

  grid_cell_set_value(&s_grid, cell, 0);
  grid_cell_set_value(&s_grid, next, val + 1);

  move_state->merged_tile[next.x][next.y] = true;
  board_add_animation(cell, next, val);

  move_state->moved = true;

  s_total_score += 1 << val;
  if (val == MAX_VAL) {
    s_won_game = true;
  }
}

void board_move(Direction dir) {
  if (s_won_game || s_lost_game) {
    return;
  }

  BoardMoveState move_state;
  memset(&move_state, 0, sizeof(move_state));
  move_state.direction = dir;

  board_traverse(dir, board_move_traversal_callback, &move_state);

  if (move_state.moved) {
    board_add_random_tile();
    if (!board_moves_available()) {
      s_lost_game = true;
    }
  }
}
