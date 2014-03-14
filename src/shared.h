#include <pebble.h>

#define assert(exp)\
  if (!exp) {\
    APP_LOG(APP_LOG_LEVEL_ERROR, "Assertion failed! %s:%d", __FILE__, __LINE__);\
  }

typedef enum {
  DIRECTION_UP,
  DIRECTION_RIGHT,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
} Direction;
