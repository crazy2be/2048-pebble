#include "grid.h"

#define GRID_SIZE 4

static int s_grid[GRID_SIZE][GRID_SIZE];

bool grid_cell_valid(Cell cell) {
  return cell.x >= 0 && cell.x < GRID_SIZE &&
    cell.y >= 0 && cell.y < GRID_SIZE;
}

int grid_cell_value(Cell cell) {
  assert(grid_cell_valid(cell));
  return s_grid[cell.x][cell.y];
}

void grid_cell_set_value(Cell cell, int val) {
  assert(grid_cell_valid(cell));
  s_grid[cell.x][cell.y] = val;
}

bool grid_cell_empty(Cell cell) {
  return grid_cell_value(cell) == 0;
}

bool grid_has_empty_cells() {
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      Cell cell = Cell(x, y);
      if (grid_cell_empty(cell)) {
        return true;
      }
    }
  }
  return false;
}

void grid_init() {
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      Cell cell = Cell(x, y);
      grid_cell_set_value(cell, 0);
    }
  }
}
