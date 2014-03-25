#include "board.h"
#include "grid.h"
#include "tile.h"

// (empty), 2, 4, 8, 16,  32, 64, 128, 256,  512, 1024, 2048
static const int WINNING_VAL = 11;
#define MAX_UNDO 10

typedef struct {
  int score;
  bool won;
  bool lost;
  Grid grid;
} GameState;

static GameState s_state;

typedef struct {
  GameState states[MAX_UNDO];
  int cur;
  int top;
} UndoStack;
static UndoStack s_undo_stack;

static void tile_sampler() {
  for (int i = 0; i < GRID_SIZE*GRID_SIZE; i++) {
    int x = i % GRID_SIZE;
    int y = i / GRID_SIZE;
    grid_cell_set_value(&s_state.grid, Cell(x, y), i < WINNING_VAL ? i : 0);
  }
}

void game_init() {
  memset(&s_state, 0, sizeof(s_state));

  grid_init(&s_state.grid);
  board_set_grid(&s_state.grid);
  board_init();

  s_undo_stack.states[0] = s_state;
  s_undo_stack.cur = 0;
  s_undo_stack.top = 0;
}

void game_draw(GContext* ctx) {
  board_draw(ctx);

  static char score_buf[100];
  snprintf(score_buf, 100, "Score: %d", s_state.score);

  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx,
                     score_buf,
                     fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
                     GRect(0, 0, 144, 20),
                     GTextOverflowModeWordWrap,
                     GTextAlignmentCenter,
                     NULL);

  if (s_state.won || s_state.lost) {
    for (int xp = 0; xp < 144; xp++) {
      for (int yp = 0; yp < 168; yp++) {
        if ((xp + yp) % 2 == 0) {
          graphics_draw_pixel(ctx, GPoint(xp, yp));
        }
      }
    }
    graphics_draw_text(ctx,
                       s_state.won ? "Win!" : "Lose :(",
                       fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD),
                       GRect(0, (168 - 42)/2, 144, 50),
                       GTextOverflowModeWordWrap,
                       GTextAlignmentCenter,
                       NULL);
  }
}

typedef struct {
  bool moved;
  Direction direction;
  // No recursive merges allowed
  bool merged_tile[GRID_SIZE][GRID_SIZE];
} GameMoveState;

void game_move_traversal_callback(GPoint cell, void* context) {
  GameMoveState* move_state = (GameMoveState*)context;

  if (grid_cell_empty(&s_state.grid, cell)) return;

  int val = grid_cell_value(&s_state.grid, cell);
  Cell next;
  Cell furthest = grid_find_furthest_empty_cell(&s_state.grid, cell, move_state->direction, &next);

  if (!grid_cell_valid(&s_state.grid, next)) {
    move_state->moved |= board_move_tile(&s_state.grid, cell, furthest);
    return;
  }

  int nextVal = grid_cell_value(&s_state.grid, next);
  if (nextVal != val || move_state->merged_tile[next.x][next.y]) {
    move_state->moved |= board_move_tile(&s_state.grid, cell, furthest);
    return;
  }

  // Merge tiles
  val++;
  grid_cell_set_value(&s_state.grid, cell, 0);
  grid_cell_set_value(&s_state.grid, next, val);

  move_state->merged_tile[next.x][next.y] = true;
  board_add_animation(cell, next, val);

  move_state->moved = true;

  s_state.score += 1 << val;
  if (val == WINNING_VAL) {
    s_state.won = true;
  }
}

void game_move(Direction dir) {
  if (s_state.won || s_state.lost) {
    return;
  }

//   APP_LOG(APP_LOG_LEVEL_DEBUG, "do: %d %d %d", s_undo_stack.cur, s_undo_stack.top, MAX_UNDO);
  s_undo_stack.cur++;
  s_undo_stack.top = s_undo_stack.cur;
  s_undo_stack.states[s_undo_stack.cur % MAX_UNDO] = s_state;

  GameMoveState move_state;
  memset(&move_state, 0, sizeof(move_state));
  move_state.direction = dir;

  board_traverse(dir, game_move_traversal_callback, &move_state);

  board_set_grid(&s_state.grid);

  if (move_state.moved) {
    board_add_random_tile();
    if (!board_moves_available()) {
      s_state.lost = true;
    }
  }
}

void game_undo() {
//   APP_LOG(APP_LOG_LEVEL_DEBUG, "undo: %d %d %d", s_undo_stack.cur, s_undo_stack.top, MAX_UNDO);
  if (s_undo_stack.cur <= s_undo_stack.top - MAX_UNDO
    || s_undo_stack.cur <= 0
  ) {
    return;
  }
  s_state = s_undo_stack.states[s_undo_stack.cur % MAX_UNDO];
  s_undo_stack.cur--;
}

