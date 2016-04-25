#ifndef _RECEL_H__
#define _RECEL_H__

#include <stdint.h>
#include <stdbool.h>

/* 1. Distance map */

/* Returns a (w * h) array of uint32_t representing the distance map computed
 * from input.
 * Array has to be freed with free(3).
 */
uint32_t *recel_distance(uint32_t w, uint32_t h, uint32_t *input);

/* 2. Tracing */

typedef struct {
  uint16_t x, y;
  int8_t dx, dy;
} recel_edge;

typedef struct {
  recel_edge e;
  uint16_t len;
} recel_straight;

typedef struct {
  recel_edge e;
  uint16_t len;
  uint16_t count;
  uint8_t delta;
} recel_perfect;

typedef struct {
  uint32_t w, h;
  uint32_t *distance;

  int32_t level;
  recel_edge e;
  uint16_t len;
} recel_tracer;

recel_tracer recel_tracer_init(uint32_t w, uint32_t h, uint32_t *distance);

bool recel_tracer_begin(recel_tracer *t, int32_t x2, int32_t y2);
void recel_tracer_next(recel_tracer *t);
recel_straight recel_tracer_next_line(recel_tracer *t);

bool recel_edge_same(recel_edge e0, recel_edge e1);
bool recel_straight_same(recel_straight l0, recel_straight l1);
bool recel_perfect_same(recel_perfect l0, recel_perfect l1);

recel_straight recel_tracer_next_line(recel_tracer *t);

/*
typedef struct {
  uint32_t w, h;
  int16_t (*hlines)[2];
  int16_t (*vlines)[2];
} offset_map_t;

offset_map_t recel_line_map(uint32_t w, uint32_t h, uint32_t *distance);

typedef struct {
  int16_t x, y;
} offset_t;

offset_t *recel_solve(uint32_t w, uint32_t h, uint32_t *distance);

uint32_t *recel_render(uint32_t w, uint32_t h, uint32_t *input, offset_t *offsets);
*/

#endif /*!_RECEL_H__*/
