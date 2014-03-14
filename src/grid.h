#include "shared.h"

typedef GPoint Cell;
#define Cell(x, y) ((Cell){(x), (y)})

bool grid_cell_valid(Cell cell);
int grid_cell_value(Cell cell);
void grid_cell_set_value(Cell cell, int val);
bool grid_cell_empty(Cell cell);
bool grid_has_empty_cells();
void grid_init();
