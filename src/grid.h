#include "shared.h"

#define GRID_SIZE 4

typedef GPoint Cell;
#define Cell(x, y) ((Cell){(x), (y)})

typedef struct {
  int val;
} Tile;

typedef struct {
  Tile tiles[GRID_SIZE][GRID_SIZE];
} Grid;

bool grid_cell_valid(Grid*, Cell);
Tile* grid_cell_tile(Grid*, Cell);
int grid_cell_value(Grid*, Cell);
void grid_cell_set_value(Grid*, Cell, int val);
bool grid_cell_empty(Grid*, Cell cell);
bool grid_has_empty_cells(Grid*);
void grid_init(Grid*);
