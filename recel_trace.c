#include "recel.h"
#include <assert.h>

#define DISTANCE(x,y) (t->distance[(y) * t->w + (x)])

static void tracer_invariant(recel_tracer *t)
{
  int32_t x0 = t->x, y0 = t->y;

  char dx = t->dx, dy = t->dy;
  assert (dx == 0 || dy == 0);
  assert (!(dx == 0 && dy == 0));

  int32_t x1 = x0 + dx * t->n, y1 = y0 + dy * t->n;

  // Line is in shape
  assert (DISTANCE(x0, y0) >= t->level);
  assert (DISTANCE(x1, y1) >= t->level);

  // Extremities are corner
  assert (DISTANCE(x0 - dx, y0 - dy) < t->level ||
          DISTANCE(x0 - dy, y0 + dx) < t->level);

  assert (DISTANCE(x1 + dx, y1 + dy) < t->level ||
          DISTANCE(x1 - dy, y1 + dx) < t->level);
}

recel_tracer recel_tracer_init(uint32_t w, uint32_t h, uint32_t *distance)
{
  return (recel_tracer){
    .w = w,
    .h = h,
    .distance = distance
  };
}

bool recel_tracer_begin(recel_tracer *t, int32_t x, int32_t y)
{
  uint32_t level = DISTANCE(x, y);

  assert (level > 0);
  if (level == 1)
    return 0;

  char dx = 0, dy = 0;

  if (DISTANCE(x - 1, y) < level)
    dy = 1;
  else if (DISTANCE(x, y + 1) < level)
    dx = 1;
  else if (DISTANCE(x + 1, y) < level)
    dy = -1;
  else if (DISTANCE(x, y - 1) < level)
    dx = -1;
  else
    return 0;

  // Maximize line
  int32_t x0 = x - dx, y0 = y - dy, n = 0;
  while (DISTANCE(x0, y0) >= level &&
         DISTANCE(x0 - dy, y0 + dx) < level)
  {
    x0 -= dx;
    y0 -= dy;
    n += 1;
  }

  int32_t x1 = x + dx, y1 = y + dy;
  while (DISTANCE(x1, y1) >= level &&
         DISTANCE(x1 - dy, y1 + dx) < level)
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

  tracer_invariant(t);

  return 1;
}

void recel_tracer_next(recel_tracer *t)
{
  char dx = t->dx, dy = t->dy;

  int32_t x0 = t->x + dx * t->n, y0 = t->y + dy * t->n;

  tracer_invariant(t);

  if (DISTANCE(x0 + dx - dy, y0 + dy + dx) >= t->level)
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
  while (DISTANCE(x1, y1) >= t->level &&
         DISTANCE(x1 - dy, y1 + dx) < t->level)
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

  tracer_invariant(t);
}

bool recel_line_same(recel_line l0, recel_line l1)
{
  return (
      l0.x == l1.x && l0.y == l1.y &&
      l0.dx == l1.dx && l0.dy == l1.dy &&
      l0.n == l1.n
      );
}

static recel_line extract_line(recel_tracer *t)
{
  return (recel_line){
    .x = (t->x * 2) + ((1 - t->dy) / 2 + (1 - t->dx) / 2),
    .y = (t->y * 2) + ((1 - t->dy) / 2 + (1 + t->dx) / 2),
    .n = (t->n + 1) * 2,
    .dx = t->dx,
    .dy = t->dy
  };
}

recel_line recel_tracer_next_hv_line(recel_tracer *t)
{
  recel_line l = extract_line(t);
  recel_tracer_next(t);

  return l;
}

recel_line recel_tracer_next_hvd_line(recel_tracer *t)
{
  recel_line l = extract_line(t);
  recel_tracer_next(t);

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
      recel_tracer_next(t);
    }
  }

  return l;
}
