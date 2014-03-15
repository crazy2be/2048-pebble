#include "shared.h"
#include "grid.h"

typedef void (*BoardTraversalCallback)(GPoint cell, void* context);

void board_add_animation(Cell from, Cell to, int val);
void board_add_move_animation(Cell from, Cell to, int val);
void board_draw(GContext *ctx);
void board_add_random_tile();
void board_tile_sampler();
void board_init();
bool board_tile_matches_possible();
bool board_moves_available();
bool board_move_tile(Grid* g, Cell start, Cell end);
void board_traverse(Direction dir, BoardTraversalCallback callback, void* context);
void board_set_grid(Grid* g);
