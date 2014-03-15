#include "shared.h"

GPoint direction_to_vector(Direction dir) {
  switch (dir) {
  case DIRECTION_UP:
    return GPoint(0, -1);
  case DIRECTION_RIGHT:
    return GPoint(1, 0);
  case DIRECTION_DOWN:
    return GPoint(0, 1);
  case DIRECTION_LEFT:
    return GPoint(-1, 0);
  default:
    assert(false);
  }
  return GPoint(0, 0);
}
