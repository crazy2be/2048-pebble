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

Cell grid_find_furthest_empty_cell(Grid* g, Cell start, Direction dir, Cell* next_out) {
  GPoint delta = direction_to_vector(dir);

  Cell prev = start;
  Cell next = Cell(start.x + delta.x, start.y + delta.y);

  // Progress towards the vector direction until an obstacle is found
  while (grid_cell_valid(g, next) && grid_cell_empty(g, next)) {
    prev = next;
    next = Cell(prev.x + delta.x, prev.y + delta.y);
  }

  if (next_out) {
    *next_out = next;
  }
  return prev;
}

void grid_init(Grid* g) {
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      Cell cell = Cell(x, y);
      grid_cell_set_value(g, cell, 0);
    }
  }
}
