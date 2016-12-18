#include "recel.h"
#include <assert.h>
#include <stdlib.h>
#include "stb_image_write.h"
#include "fasttable.h"

/* Distance map */

// Content of distance map:
// - >0 => actual distance
// - =0 => not yet processed
// - <0 => linked-list of pixels to process

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
    assert (PIX(distance, x, y) == 0); \
    PIX(distance, x, y) = list; \
    colorcounter_incr(counter, PIX(input, x, y)); \
    list = encode(x, y); \
  } while (0)

// Compute distance map

static int32_t distance_init(uint32_t w, uint32_t h, colorcounter_t *counter,  const uint32_t *input, int32_t *distance)
{
  // Fill with 0
  for (uint32_t i = 0, last = w * h - 1 ; i <= last; ++i)
    distance[i] = 0;

  int32_t worklist = -1;

  colorcounter_start(counter);

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

#define PROPAGATE(worklist, col, x, y) \
  do { \
    uint32_t tmp_x = (x), tmp_y = (y); \
    if (tmp_x >= 0 && tmp_y >= 0 && tmp_x < w && tmp_y < h && \
        PIX(input, tmp_x, tmp_y) == col && PIX(distance, tmp_x, tmp_y) == 0) \
      PUSH(worklist, tmp_x, tmp_y); \
  } while (0)

// Fill current level

static int32_t distance_propagate(uint32_t w, uint32_t h,
    colorcounter_t *counter,
    const uint32_t *input, int32_t *distance, int32_t worklist)
{
  int32_t sentinel = -1;

  while (worklist != sentinel)
  {
    int32_t sentinel1 = worklist, cursor = worklist;
    do {
      uint32_t x = DECODE_X(cursor), y = DECODE_Y(cursor);
      uint32_t col = PIX(input, x, y);

      PROPAGATE(worklist, col, x - 1, y - 1);
      PROPAGATE(worklist, col, x - 1, y + 0);
      PROPAGATE(worklist, col, x - 1, y + 1);
      PROPAGATE(worklist, col, x + 0, y - 1);
      PROPAGATE(worklist, col, x + 0, y + 1);
      PROPAGATE(worklist, col, x + 1, y - 1);
      PROPAGATE(worklist, col, x + 1, y + 0);
      PROPAGATE(worklist, col, x + 1, y + 1);

      cursor = PIX(distance, x, y);
    } while (cursor != sentinel);
    sentinel = sentinel1;
  }

  return worklist;
}

#define PUSHNEXT(worklist, x, y) \
  do { \
    uint32_t tmp_x = (x), tmp_y = (y); \
    if (tmp_x >= 0 && tmp_y >= 0 && \
        tmp_x < w && tmp_y < h && \
        PIX(distance, tmp_x, tmp_y) == 0) \
      PUSH(worklist, tmp_x, tmp_y);  \
  } while (0)

static int32_t distance_nextlevel(uint32_t w, uint32_t h,
    colorcounter_t *counter,
    const uint32_t *input, int32_t *distance,
    int32_t level, int32_t worklist)
{
  int32_t cursor = worklist;
  worklist = -1;

  while (cursor != -1)
  {
    uint32_t x = DECODE_X(cursor), y = DECODE_Y(cursor);

    PUSHNEXT(worklist, x + 0, y - 1);
    PUSHNEXT(worklist, x - 1, y + 0);
    PUSHNEXT(worklist, x + 1, y + 0);
    PUSHNEXT(worklist, x + 0, y + 1);

    cursor = PIX(distance, x, y);

    PIX(distance, x, y) = level +
      colorcounter_get_rank(counter, PIX(input, x, y));
  }

  return worklist;
}

uint32_t *recel_distance(uint32_t w, uint32_t h, uint32_t *input)
{
  int32_t *distance = NEW_IMAGE(int32_t, w, h);
  colorcounter_t *counter = colorcounter_new();
  int32_t worklist = distance_init(w, h, counter, input, distance);
  int32_t level = 1;

  while (worklist != -1)
  {
    level += colorcounter_distinct_count(counter);

    colorcounter_start(counter);
    worklist = distance_propagate(w, h, counter, input, distance, worklist);

    colorcounter_rank(counter);
    worklist = distance_nextlevel(w, h, counter, input, distance, level, worklist);
  }

  return (uint32_t*)distance;
}

uint8_t *recel_dist_to_u8(uint32_t w, uint32_t h, uint32_t *input)
{
  uint32_t max = 0;
  for (uint32_t y = 0; y < h; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      if (PIX(input, x, y) > max)
        max = PIX(input, x, y);
    }
  }

  uint8_t *out = NEW_IMAGE(uint8_t, w, h);
  for (uint32_t y = 0; y < h; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      PIX(out, x, y) = PIX(input, x, y) * 255 / max;
    }
  }

  return out;
}

void recel_save_dist(const char *name, uint32_t w, uint32_t h, uint32_t *dist)
{
  uint8_t *out = recel_dist_to_u8(w, h, dist);
  stbi_write_png(name, w, h, 1, out, 0);
  free(out);
}
