#include "recel.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <strings.h>

#define C(m, x, y) (m[(y) * w + (x)])

static inline int32_t avg(int32_t x, int32_t y)
{
  return (x&y)+((x^y)/2);
}

static inline int16_t clamp(int32_t x)
{
  if (x > 32767)
    return 32767;
  else if (x < -32767)
    return -32767;
  else
    return x;
}

static inline uint32_t diff(offset_t o0, offset_t o1)
{
  int32_t dx = o0.x - o1.x;
  int32_t dy = o0.y - o1.y;

  if (dx < 0) dx = -dx;
  if (dy < 0) dy = -dy;

  return dx + dy;
}

uint32_t recel_iter_solver(uint32_t w, uint32_t h, uint32_t *distance,
    offset_t *io,
    offset_t *oo
    )
{
  uint32_t acc = 0;

  for (uint32_t y = 0; y < h - 1; ++y)
  {
    for (uint32_t x = 0; x < w - 1; ++x)
    {
      uint32_t l0 = C(distance, x + 0, y + 0),
               l1 = C(distance, x + 1, y + 0),
               l2 = C(distance, x + 0, y + 1),
               l3 = C(distance, x + 1, y + 1);

      int32_t d0x, d0y, d1x, d1y;

      if (l0 != l1 && l0 != l2 && l2 != l3 && l1 != l3)
        continue;
      else if (l0 != l1)
      {
        d0x = 0, d0y = -1;
        if (l0 != l2)
          d1x = -1, d1y = 0;
        else if (l1 != l3)
          d1x = 1, d1y = 0;
        else
          d1x = 0, d1y = 1;
      }
      else if (l0 != l2)
      {
        d0x = -1, d0y = 0;
        if (l1 != l3)
          d1x = 1, d1y = 0;
        else
          d1x = 0, d1y = 1;
      }
      else if (l1 != l3)
      {
        d0x = 1, d0y = 0;
        d1x = 0, d1y = 1;
      }
      else
        continue;

      offset_t o0 = C(io, x + d0x, y + d0y);
      offset_t o1 = C(io, x + d1x, y + d1y);

      C(oo, x, y).x = clamp(avg(o0.x + d0x * 32768, o1.x + d1x * 32768));
      C(oo, x, y).y = clamp(avg(o0.y + d0y * 32768, o1.y + d1y * 32768));

      acc += diff(C(oo, x, y), C(io, x, y));
    }
  }

  return acc;
}

offset_t *recel_solve(uint32_t w, uint32_t h, uint32_t *distance)
{
  offset_t *o1 = malloc(sizeof(offset_t) * w * h);
  offset_t *o2 = malloc(sizeof(offset_t) * w * h);

  bzero(o1, sizeof(offset_t) * w * h);
  bzero(o2, sizeof(offset_t) * w * h);

  uint32_t oldenergy = 0, energy = 1;
  for (int i = 0; oldenergy != energy; ++i)
  {
    oldenergy = energy;

    offset_t *tmp = o1;
    o1 = o2;
    o2 = tmp;

    energy = recel_iter_solver(w, h, distance, o1, o2);
    printf("pass %d, energy %u\n", i, energy);
  }

  free(o1);
  return o2;
}

#define OUT(x, y) (out[(y) * w * 2 + (x)])

#define ABS(x) ((x) < 0 ? (-x) : (x))

uint32_t clerp(uint32_t c0, uint32_t c1, int16_t coeff)
{
  union {
    uint32_t v;
    struct {
      uint8_t r, g, b, a;
    };
  } v0, v1, v2;
  v0.v = c0;
  v1.v = c1;

  v2.r = (v0.r * (32767 - coeff) + v1.r * coeff) / 32767;
  v2.g = (v0.g * (32767 - coeff) + v1.g * coeff) / 32767;
  v2.b = (v0.b * (32767 - coeff) + v1.b * coeff) / 32767;
  v2.a = (v0.a * (32767 - coeff) + v1.a * coeff) / 32767;

  return v2.v;
}

uint32_t *recel_render(uint32_t w, uint32_t h, uint32_t *input, offset_t *offsets)
{
  uint32_t *out = malloc(sizeof(uint32_t) * w * 2 * h * 2);

  // Nearest neighbor
  for (uint32_t y = 0; y < h; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      OUT(2 * x + 0, 2 * y + 0) = C(input, x, y);
      OUT(2 * x + 1, 2 * y + 0) = C(input, x, y);
      OUT(2 * x + 0, 2 * y + 1) = C(input, x, y);
      OUT(2 * x + 1, 2 * y + 1) = C(input, x, y);
    }
  }

  // Refine
  for (uint32_t y = 0; y < h - 1; ++y)
  {
    for (uint32_t x = 0; x < w - 1; ++x)
    {
      offset_t o = C(offsets, x, y);

      uint32_t c00 = OUT(2 * x + 1, 2 * y + 1);
      uint32_t c10 = OUT(2 * x + 1, 2 * y + 2);
      uint32_t c01 = OUT(2 * x + 2, 2 * y + 1);
      uint32_t c11 = OUT(2 * x + 2, 2 * y + 2);

      if (o.x > 0)
      {
        c01 = clerp(c01, c00, o.x);
        c11 = clerp(c11, c10, o.x);
      }
      else if (o.x < 0)
      {
        c00 = clerp(c00, c01, -o.x);
        c10 = clerp(c10, c11, -o.x);
      }

      if (o.y > 0)
      {
        c10 = clerp(c10, c00, o.y);
        c11 = clerp(c11, c01, o.y);
      }
      else if (o.y < 0)
      {
        c00 = clerp(c00, c10, -o.y);
        c01 = clerp(c01, c11, -o.y);
      }

      OUT(2 * x + 1, 2 * y + 1) = c00;
      OUT(2 * x + 1, 2 * y + 2) = c10;
      OUT(2 * x + 2, 2 * y + 1) = c01;
      OUT(2 * x + 2, 2 * y + 2) = c11;
    }
  }

  return out;
}
