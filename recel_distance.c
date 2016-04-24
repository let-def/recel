#include "recel.h"
#include <assert.h>
#include <stdlib.h>

/* Distance map */

// Content of distance map:
// - >0 => actual distance
// - =0 => not yet processed
// - <0 => linked-list of pixels to process

#define INPUT(x,y) (input[(y) * w + (x)])
#define DISTANCE(x,y) (distance[(y) * w + (x)])

static int32_t encode(uint32_t x, uint32_t y)
{
  assert(x < 32768);
  assert(y < 32768);
  int32_t r = ~((((1 << 15) | x) << 15) | y);
  return r;
}

#define DECODE_X(d) ((~(d) >> 15) & 0x7FFF)
#define DECODE_Y(d) ((~(d)) & 0x7FFF)

#define PUSH(list, x, y) \
  do { \
    assert (DISTANCE(x, y) == 0); \
    DISTANCE(x, y) = list; \
    list = encode(x, y); \
  } while (0)

// Compute distance map

static int32_t distance_init(uint32_t w, uint32_t h, int32_t *distance)
{
  // Fill with 0
  for (uint32_t i = 0, last = w * h - 1 ; i <= last; ++i)
    distance[i] = 0;

  int32_t worklist = -1;

  // Fill worklist with borders (horizontal)
  for (uint32_t i = 0, j = h - 1, last = w - 1; i <= last; ++i)
  {
    PUSH(worklist, i, 0);
    PUSH(worklist, i, j);
  }

  // Initialize border with 1 (vertical)
  for (uint32_t j = 1, i = w - 1, last = h - 2; j <= last; ++j)
  {
    PUSH(worklist, 0, j);
    PUSH(worklist, i, j);
  }

  return worklist;
}

#define CHECK8(worklist, col, x, y) \
  do { \
    uint32_t tmp_x = (x), tmp_y = (y); \
    if (tmp_x >= 0 && tmp_y >= 0 && tmp_x < w && tmp_y < h && \
        INPUT(tmp_x, tmp_y) == col && DISTANCE(tmp_x, tmp_y) == 0) \
      PUSH(worklist, tmp_x, tmp_y); \
  } while (0)

// Fill current level

static int32_t distance_propagate(uint32_t w, uint32_t h, uint32_t *input,
    int32_t *distance, int32_t worklist)
{
  int32_t sentinel = -1;

  while (worklist != sentinel)
  {
    int32_t sentinel1 = worklist, cursor = worklist;
    do {
      uint32_t x = DECODE_X(cursor), y = DECODE_Y(cursor);
      uint32_t col = INPUT(x, y);

      CHECK8(worklist, col, x - 1, y - 1);
      CHECK8(worklist, col, x - 1, y + 0);
      CHECK8(worklist, col, x - 1, y + 1);
      CHECK8(worklist, col, x + 0, y - 1);
      CHECK8(worklist, col, x + 0, y + 1);
      CHECK8(worklist, col, x + 1, y - 1);
      CHECK8(worklist, col, x + 1, y + 0);
      CHECK8(worklist, col, x + 1, y + 1);

      cursor = DISTANCE(x, y);
    } while (cursor != sentinel);
    sentinel = sentinel1;
  }

  return worklist;
}

#define CHECK4(worklist, x, y) \
  do { \
    uint32_t tmp_x = (x), tmp_y = (y); \
    if (tmp_x >= 0 && tmp_y >= 0 && \
        tmp_x < w && tmp_y < h && \
        DISTANCE(tmp_x, tmp_y) == 0) \
      PUSH(worklist, tmp_x, tmp_y);  \
  } while (0)

static int32_t distance_nextlevel(uint32_t w, uint32_t h, uint32_t *input,
    int32_t *distance, int32_t level, int32_t worklist)
{
  int32_t cursor = worklist;
  worklist = -1;

  while (cursor != -1)
  {
    uint32_t x = DECODE_X(cursor), y = DECODE_Y(cursor);

    CHECK4(worklist, x + 0, y - 1);
    CHECK4(worklist, x - 1, y + 0);
    CHECK4(worklist, x + 1, y + 0);
    CHECK4(worklist, x + 0, y + 1);

    cursor = DISTANCE(x, y);

    DISTANCE(x, y) = level;
  }

  return worklist;
}

uint32_t *recel_distance(uint32_t w, uint32_t h, uint32_t *input)
{
  int32_t *distance = malloc(w * h * sizeof(int32_t));
  int32_t worklist = distance_init(w, h, distance);
  int32_t level = 0;

  while (worklist != -1)
  {
    level += 1;

    worklist = distance_propagate(w, h, input, distance, worklist);
    worklist = distance_nextlevel(w, h, input, distance, level, worklist);
  }

  return distance;
}
