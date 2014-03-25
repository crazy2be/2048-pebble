#pragma once

#include "shared.h"

// Maximum tile number that we know how to draw.
#define TILE_MAX_VAL 16

void tile_draw(GContext *ctx, GPoint origin, int val);
GPoint tile_position(Cell cell);
