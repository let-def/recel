#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/* Trace lines */

typedef struct {
  int32_t level;
  int32_t x, y, n;
  char dx, dy;
} tracer;

static bool tracer_same(tracer *t0, tracer *t1)
{
  return (
      t0->level == t1->level &&
      t0->x == t1->x && t0->y == t1->y &&
      t0->dx == t1->dx && t0->dy == t1->dy &&
      t0->n == t1->n
      );
}

static void tracer_invariant(recel_context *ctx, tracer *t)
{
  int32_t x0 = t->x, y0 = t->y;

  char dx = t->dx, dy = t->dy;
  assert (dx == 0 || dy == 0);
  assert (!(dx == 0 && dy == 0));

  int32_t x1 = x0 + dx * t->n, y1 = y0 + dy * t->n;

  // Line is in shape
  assert (distance(ctx, x0, y0) >= t->level);
  assert (distance(ctx, x1, y1) >= t->level);

  // Extremities are corner
  assert (distance(ctx, x0 - dx, y0 - dy) < t->level ||
          distance(ctx, x0 - dy, y0 + dx) < t->level);

  assert (distance(ctx, x1 + dx, y1 + dy) < t->level ||
          distance(ctx, x1 - dy, y1 + dx) < t->level);
}

static bool tracer_init(recel_context *ctx, tracer *t, int32_t x, int32_t y)
{
  int32_t level = distance(ctx, x, y);

  assert (level > 0);
  if (level == 1)
    return 0;

  char dx = 0, dy = 0;

  if (distance(ctx, x - 1, y) < level)
    dy = 1;
  else if (distance(ctx, x, y + 1) < level)
    dx = 1;
  else if (distance(ctx, x + 1, y) < level)
    dy = -1;
  else if (distance(ctx, x, y - 1) < level)
    dx = -1;
  else
    return 0;

  // Maximize line
  int32_t x0 = x - dx, y0 = y - dy, n = 0;
  while (distance(ctx, x0, y0) >= level &&
         distance(ctx, x0 - dy, y0 + dx) < level)
  {
    x0 -= dx;
    y0 -= dy;
    n += 1;
  }

  int32_t x1 = x + dx, y1 = y + dy;
  while (distance(ctx, x1, y1) >= level &&
         distance(ctx, x1 - dy, y1 + dx) < level)
  {
    x1 += dx;
    y1 += dy;
    n += 1;
  }

  t->level = level;
  t->x = x0 + dx;
  t->y = y0 + dy;
  t->dx = dx;
  t->dy = dy;
  t->n = n;

  //printf("trace_init (%d,%d) -> (%d,%d)*%d\n", t->x, t->y, t->dx, t->dy, t->n);
  tracer_invariant(ctx, t);
  //printf("valid\n");

  return 1;
}

static void tracer_next(recel_context *ctx, tracer *t)
{
  char dx = t->dx, dy = t->dy;

  int32_t x0 = t->x + dx * t->n, y0 = t->y + dy * t->n;

  tracer_invariant(ctx, t);

  if (distance(ctx, x0 + dx - dy, y0 + dy + dx) >= t->level)
  {
    x0 = x0 + dx - dy;
    y0 = y0 + dy + dx;

    char dx1, dy1;
    dx1 = - dy;
    dy1 = + dx;

    dx = dx1;
    dy = dy1;
  }
  else
  {
    char dx1, dy1;
    dx1 = + dy;
    dy1 = - dx;

    dx = dx1;
    dy = dy1;
  }

  int32_t x1 = x0 + dx, y1 = y0 + dy, n = 0;
  while (distance(ctx, x1, y1) >= t->level &&
         distance(ctx, x1 - dy, y1 + dx) < t->level)
  {
    x1 += dx;
    y1 += dy;
    n += 1;
  }

  t->x = x0;
  t->y = y0;
  t->dx = dx;
  t->dy = dy;
  t->n = n;

  //printf("trace_next (%d,%d) -> (%d,%d)*%d\n", t->x, t->y, t->dx, t->dy, t->n);
  tracer_invariant(ctx, t);
  //printf("valid\n");
}

typedef struct {
  uint16_t x, y, n;
  int8_t dx, dy;
} line;

static bool line_same(line *l0, line *l1)
{
  return (
      l0->x == l1->x && l0->y == l1->y &&
      l0->dx == l1->dx && l0->dy == l1->dy &&
      l0->n == l1->n
      );
}

line tracer_line(tracer *t)
{
  line l = (line){
    .x = t->x * 2,
    .y = t->y * 2,
    .n = (t->n + 1) * 2,
    .dx = t->dx,
    .dy = t->dy
  };

  l.x += (1 - l.dy) / 2 + (1 - l.dx) / 2;
  l.y += (1 - l.dy) / 2 + (1 + l.dx) / 2;

  return l;
}

line tracer_dline(recel_context *ctx, tracer *t)
{
  line l = tracer_line(t);
  tracer_next(ctx, t);

  if (l.n == 2 && t->n == 0)
  {
    if (l.dx == 0)
    {
      assert (l.dy != 0);
      l.dx = t->dx;
      assert (l.dx != 0);
    }
    else
    {
      assert (l.dy == 0);
      l.dy = t->dy;
      assert (l.dy != 0);
    }

    while (t->n == 0 && (t->dx == l.dx || t->dy == l.dy))
    {
      l.n += 1;
      tracer_next(ctx, t);
    }
  }

  return l;
}

#define MASK(x,y) mask[(y)*2*width(ctx) + (x)]
#define LINES(x,y) lines[(y)*2*width(ctx) + (x)]

int16_t *recel_trace(recel_context *ctx)
{
  int line_count = 0;
  uint32_t *lines = malloc(width(ctx) * 2 * height(ctx) * 2 * sizeof(int32_t));
  int16_t *mask = malloc(width(ctx) * 2 * height(ctx) * 2 * sizeof(int16_t));

  for (int32_t i = 0; i < width(ctx) * height(ctx) * 4; ++i)
  {
    lines[i] = 0;
    mask[i] = -32768;
  }

  uint32_t colors[] = {
    0xFF00FFFF,
    0xFFFFFF00,
    0xFFFF00FF,
    0xFF0000FF,
    0xFF00FF00,
    0xFFFF0000,
  };

  for (uint32_t y = 0; y < height(ctx); ++y)
  {
    for (uint32_t x = 0; x < width(ctx); ++x)
    {
      tracer t;
      if (MASK(2*x+0,2*y+0) == -32768 && MASK(2*x+1,2*y+0) == -32768 &&
          MASK(2*x+0,2*y+1) == -32768 && MASK(2*x+1,2*y+1) == -32768 &&
          tracer_init(ctx, &t, x, y))
      {
        line prev = tracer_dline(ctx, &t);
        line curr = tracer_dline(ctx, &t);
        line next = tracer_dline(ctx, &t);

        line start = curr;

        do {
          bool hard_from = (prev.n != 2) ||
            (prev.dx == curr.dy && prev.dx != 0) ||
            (prev.dy == - curr.dx && prev.dy != 0);
          bool hard_to = (next.n != 2) ||
            (next.dx == - curr.dy && next.dx != 0) ||
            (next.dy == curr.dx && next.dy != 0);


          if (hard_from && hard_to)
            for (int i = 0; i < curr.n; ++i)
            {
              MASK(curr.x + curr.dx * i, curr.y + curr.dy * i) = 0;
              LINES(curr.x + curr.dx * i, curr.y + curr.dy * i) = colors[line_count % 6];
            }
          else if (hard_from)
            for (int i = 0; i < curr.n; ++i)
            {
              MASK(curr.x + curr.dx * i, curr.y + curr.dy * i) = - i * 32767 / curr.n;
              LINES(curr.x + curr.dx * i, curr.y + curr.dy * i) = colors[line_count % 6];
            }
          else if (hard_to)
            for (int i = 0; i < curr.n; ++i)
            {
              MASK(curr.x + curr.dx * i, curr.y + curr.dy * i) = - 32767 + i * 32767 / curr.n;
              LINES(curr.x + curr.dx * i, curr.y + curr.dy * i) = colors[line_count % 6];
            }
          else
          {
            for (int i = 0; i < curr.n/ 2; ++i)
            {
              MASK(curr.x + curr.dx * i, curr.y + curr.dy * i) = - 32767 + i * 32767 / (curr.n / 2);
              LINES(curr.x + curr.dx * i, curr.y + curr.dy * i) = colors[line_count % 6];
            }
            for (int i = curr.n / 2; i <= curr.n; ++i)
            {
              MASK(curr.x + curr.dx * i, curr.y + curr.dy * i) = 32767 - i * 32767 / (curr.n / 2);
              LINES(curr.x + curr.dx * i, curr.y + curr.dy * i) = colors[line_count % 6];
            }
          }

          prev = curr;
          curr = next;
          next = tracer_dline(ctx, &t);

          line_count += 1;
        }
        while (!(line_same(&curr, &start)));
      }
    }
  }

  unsigned char *msk = malloc(width(ctx) * height(ctx) * 4 * sizeof(char));
  for (uint32_t y = 0; y < height(ctx) * 2; ++y)
    for (uint32_t x = 0; x < width(ctx) * 2; ++x)
    {
      if (MASK(x,y) == -32768)
        msk[y * width(ctx) * 2 + x] = 255;
      else
        msk[y * width(ctx) * 2 + x] = (32767 + MASK(x,y)) >> 7;
    }

  stbi_write_png("mask.png", width(ctx) * 2, height(ctx) * 2, 1, msk, 0);
  stbi_write_png("lines.png", width(ctx) * 2, height(ctx) * 2, 4, lines, 0);
  printf("found %ld lines\n", line_count);

  return mask;
}

#define OUT(x,y) output[(y)*width(ctx) + (x)]

void erode(recel_context *ctx)
{
  uint32_t *output = malloc(width(ctx) * height(ctx) * sizeof(uint32_t));

  for (uint32_t y = 0; y < height(ctx); ++y)
  {
    for (uint32_t x = 0; x < width(ctx); ++x)
    {
      int d = distance(ctx, x, y);
      if (d == 1)
        OUT(x,y) = input(ctx, x, y);
      else
      {
        uint32_t col1 = input(ctx, x, y);
        uint32_t col2 = col1;
        for (int j = y - 1; j <= y + 1 && col2 == col1; ++j)
          for (int i = x - 1; i <= x + 1 && col2 == col1; ++i)
            if (distance(ctx, i, j) < d)
              col2 = input(ctx, i, j);
        OUT(x,y) = col2;
      }
    }
  }
  stbi_write_png("eroded.png", width(ctx), height(ctx), 4, output, 0);
}

#undef OUT
#define OUT(x,y) output[(y)*2*width(ctx) + (x)]

void upscale(recel_context *ctx, int16_t *mask)
{
  uint32_t *output = malloc(width(ctx) * 2 * height(ctx) * 2 * sizeof(uint32_t));

  for (uint32_t y = 0; y < height(ctx) * 2; ++y)
  {
    for (uint32_t x = 0; x < width(ctx) * 2; ++x)
    {
      int d = distance(ctx, x / 2, y / 2);
      if (d == 1 || MASK(x, y) == 0 || MASK(x, y) == -32768)
      {
        OUT(x,y) = input(ctx, x / 2, y / 2);
      }
      else
      {
        uint32_t col1 = input(ctx, x / 2, y / 2);
        uint32_t col2 = col1;
        for (int j = y - 1; j <= y + 1 && col2 == col1; ++j)
          for (int i = x - 1; i <= x + 1 && col2 == col1; ++i)
            if (distance(ctx, i / 2, j / 2) < d)
              col2 = input(ctx, i / 2, j / 2);
        if (MASK(x, y) <= -16384)
          OUT(x,y) = col2;
        else
          OUT(x,y) = col1;
      }
    }
  }
  stbi_write_png("out.png", width(ctx) * 2, height(ctx) * 2, 4, output, 0);
}

int main(int argc, char **argv)
{
  int w, h, n;
  void *data = stbi_load(argv[1], &w, &h, &n, 4);

  recel_context ctx;
  recel_init(&ctx, w, h, 0, data);
  compute_distance(&ctx);

  unsigned char *dist = malloc(width(&ctx) * height(&ctx) * sizeof(char));
  for (uint32_t y = 0; y < height(&ctx); ++y)
    for (uint32_t x = 0; x < width(&ctx); ++x)
      dist[y * width(&ctx) +x] = distance(&ctx, x, y);

  stbi_write_png("dist.png", width(&ctx), height(&ctx), 1, dist, 0);
  int16_t *mask = recel_trace(&ctx);
  erode(&ctx);
  upscale(&ctx, mask);
}
