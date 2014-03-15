#include "grid.h"

bool grid_cell_valid(Grid* g, Cell cell) {
  return cell.x >= 0 && cell.x < GRID_SIZE &&
    cell.y >= 0 && cell.y < GRID_SIZE;
}

Tile* grid_cell_tile(Grid* g, Cell cell) {
  assert(grid_cell_valid(g, cell));
  return &g->tiles[cell.x][cell.y];
}

int grid_cell_value(Grid* g, Cell cell) {
  return grid_cell_tile(g, cell)->val;
}

void grid_cell_set_value(Grid* g, Cell cell, int val) {
  grid_cell_tile(g, cell)->val = val;
}

bool grid_cell_empty(Grid* g, Cell cell) {
  return grid_cell_value(g, cell) == 0;
}

bool grid_has_empty_cells(Grid* g) {
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      Cell cell = Cell(x, y);
      if (grid_cell_empty(g, cell)) {
        return true;
      }
    }
  }
  return false;
}

void grid_init(Grid* g) {
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      Cell cell = Cell(x, y);
      grid_cell_set_value(g, cell, 0);
    }
  }
}
