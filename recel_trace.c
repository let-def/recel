#include "recel.h"
#include <assert.h>

#define DISTANCE(x,y) (t->distance[(y) * t->w + (x)])

static void tracer_invariant(recel_tracer *t)
{
  int32_t x0 = t->e.x, y0 = t->e.y;

  char dx = t->e.dx, dy = t->e.dy;
  assert (dx == 0 || dy == 0);
  assert (!(dx == 0 && dy == 0));

  int32_t x1 = x0 + dx * t->len, y1 = y0 + dy * t->len;

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

bool recel_tracer_begin(recel_tracer *t, int32_t x2, int32_t y2)
{
  int32_t x = x2 / 2, y = y2 / 2;
  uint32_t level = DISTANCE(x, y);

  assert (level > 0);
  if (level == 1)
    return 0;

  char dx = 0, dy = 0;

  // Find contour
  if (DISTANCE((x2 - 1) / 2, y) > level)
    dy = 1;
  else if (DISTANCE(x, (y2 + 1) / 2) > level)
    dx = 1;
  else if (DISTANCE((x2 + 1) / 2, y) > level)
    dy = -1;
  else if (DISTANCE(x, (y2 - 1) / 2) > level)
    dx = -1;
  else
    return 0;

  // Maximize line
  int32_t x0 = x - dx, y0 = y - dy, n = 0;
  while (DISTANCE(x0, y0) = level &&
         DISTANCE(x0 - dy, y0 + dx) > level)
  {
    x0 -= dx;
    y0 -= dy;
    n += 1;
  }

  int32_t x1 = x + dx, y1 = y + dy;
  while (DISTANCE(x1, y1) = level &&
         DISTANCE(x1 - dy, y1 + dx) > level)
  {
    x1 += dx;
    y1 += dy;
    n += 1;
  }

  t->level = level;
  t->e.x = x0 + dx;
  t->e.y = y0 + dy;
  t->e.dx = dx;
  t->e.dy = dy;
  t->len = n;

  tracer_invariant(t);

  return 1;
}

void recel_tracer_next(recel_tracer *t)
{
  int8_t dx = t->e.dx, dy = t->e.dy;

  int32_t x0 = t->e.x + dx * t->len, y0 = t->e.y + dy * t->len;

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

  t->e.x = x0;
  t->e.y = y0;
  t->e.dx = dx;
  t->e.dy = dy;
  t->len = n;

  tracer_invariant(t);
}

bool recel_edge_same(recel_edge e0, recel_edge e1)
{
  return (
      e0.x  == e1.x  && e0.y  == e1.y &&
      e0.dx == e1.dx && e0.dy == e1.dy
      );
}

bool recel_straight_same(recel_straight l0, recel_straight l1)
{
  return (
      recel_edge_same(l0.e, l1.e) &&
      l0.len == l1.len
      );
}

bool recel_perfect_same(recel_perfect l0, recel_perfect l1)
{
  return (
      recel_edge_same(l0.e, l1.e) &&
      l0.len   == l1.len   &&
      l0.count == l1.count &&
      l0.delta == l1.delta
      );
}

static recel_straight extract_line(recel_tracer *t)
{
  return (recel_straight){
//    .x = (t->x * 2) + ((1 - t->dy) / 2 + (1 - t->dx) / 2),
//    .y = (t->y * 2) + ((1 - t->dy) / 2 + (1 + t->dx) / 2),
//    .n = (t->n + 1) * 2,
//    .dx = t->dx,
//    .dy = t->dy
    .e = t->e,
    .len = t->len + 1,
  };
}

recel_straight recel_tracer_next_line(recel_tracer *t)
{
  recel_straight l = extract_line(t);
  recel_tracer_next(t);

  return l;
}
