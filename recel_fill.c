#include "recel.h"
#include <stdlib.h>
#include <assert.h>
#include <strings.h>

static offset_map_t offset_alloc(uint32_t w, uint32_t h)
{
  size_t bytes = sizeof (int16_t[2]) * w * h;

  offset_map_t map = {
    .w = w, .h = h,
    .hlines = malloc(bytes),
    .vlines = malloc(bytes),
  };

  if (map.hlines == NULL || map.vlines == NULL) abort();

  for (uint32_t i = 0; i < w * h; ++i) {
    map.hlines[i][0] = -32768;
    map.hlines[i][1] = -32768;
    map.vlines[i][0] = -32768;
    map.vlines[i][1] = -32768;
  }

  return map;
};

#define HLINE(x,y) (map.hlines[(y) * w + (x)])
#define VLINE(x,y) (map.vlines[(x) * h + (y)])

static void fill_lerp(int16_t *p, uint32_t count, int16_t lo, int16_t hi)
{
  if (count == 0)
    *p = 0;
  else
    for (uint32_t i = 0; i < count; ++i)
      p[i] = lo + ((hi - lo) * i) / (count);
}

offset_map_t recel_line_map(uint32_t w, uint32_t h, uint32_t *distance)
{
  offset_map_t map = offset_alloc(w, h);
  recel_tracer t = recel_tracer_init(w, h, distance);

  for (uint32_t y = 1; y < h * 2 - 1; ++y) {
    for (uint32_t x = 1; x < w * 2 - 1; ++x) {
      if (recel_tracer_begin(&t, x, y) &&
          (t.e.dx == 0 && VLINE(x / 2, y / 2)[0] == -32768 ||
           t.e.dy == 0 && HLINE(x / 2, y / 2)[0] == -32768))
      {
        recel_straight prev = recel_tracer_next_line(&t);
        recel_straight curr = recel_tracer_next_line(&t);
        recel_straight next = recel_tracer_next_line(&t);

        recel_straight first = curr;
        do {
          bool hard_from = (prev.e.dx == curr.e.dy && prev.e.dy == - curr.e.dx);
          bool hard_to = (next.e.dx == - curr.e.dy && next.e.dy == curr.e.dx);

          int16_t *p =
            (curr.e.dx == 0)
            ? VLINE(curr.e.x, curr.e.y)
            : HLINE(curr.e.x, curr.e.y);

          // Curved
          if (hard_from == hard_to)
          {
            int16_t inner = hard_from ? - 32767 : 32767;
            int16_t outer = -inner;
            fill_lerp(p, curr.len, outer, inner);
            fill_lerp(p + curr.len, curr.len, inner, outer);
          }
          else // Linear
          {
            int16_t from = hard_from ? 32767 : -32767;
            int16_t to = hard_to ? 32767 : -32767;
            bool dir = (curr.e.dx | curr.e.dy) > 0;
            fill_lerp(p, curr.len * 2, dir ? from : to, dir ? to : from);
          }

          prev = curr;
          curr = next;
          next = recel_tracer_next_line(&t);
        }
        while (!recel_straight_same(curr, first));
      }
    }
  }

  return map;
}

#define IN(x,y) (input[(y) * w + (x)])
#define OUT(x,y) (output[(y) * (w * 2) + (x)])

uint32_t *recel_render_map(uint32_t w, uint32_t h, offset_map_t map, uint32_t *input)
{
  uint32_t *output = malloc (sizeof (uint32_t) * w * 2 * h * 2);

  for (uint32_t y = 0; y < h; ++y) {
    for (uint32_t x = 0; x < w; ++x) {
      int16_t *hline = HLINE(x, y);
      int16_t *vline = VLINE(x, y);

      if (*hline == -32768 && *vline == -32768)
      { // FLAT
        OUT(2 * x + 0, 2 * y + 0) = IN(x, y);
        OUT(2 * x + 1, 2 * y + 0) = IN(x, y);
        OUT(2 * x + 0, 2 * y + 1) = IN(x, y);
        OUT(2 * x + 1, 2 * y + 1) = IN(x, y);
      }
      else if (*hline == -32768)
      { // VERT
        uint32_t c0 = IN(x, y);
        uint32_t c1 = IN(x + 1, y);
        OUT(2 * x + 0, 2 * y + 0) = vline[0] > -16384 ? c0 : c1;
        OUT(2 * x + 1, 2 * y + 0) = vline[0] < 16384 ? c1 : c0;
        OUT(2 * x + 0, 2 * y + 1) = vline[1] > -16384 ? c0 : c1;
        OUT(2 * x + 1, 2 * y + 1) = vline[1] < 16384 ? c1 : c0;
      }
      else
      { // HORZ
        uint32_t c0 = IN(x, y);
        uint32_t c1 = IN(x, y + 1);
        OUT(2 * x + 0, 2 * y + 0) = IN(x, y);
        OUT(2 * x + 1, 2 * y + 0) = IN(x, y);
        OUT(2 * x + 0, 2 * y + 1) = IN(x, y);
        OUT(2 * x + 1, 2 * y + 1) = IN(x, y);
      }
    }
  }

  return output;
}
