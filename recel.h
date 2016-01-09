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

/* 2. Line tracing */

typedef struct {
  uint32_t w, h;
  uint32_t *distance;

  int32_t level;
  int16_t x, y, n;
  char dx, dy;
} recel_tracer;

typedef struct {
  uint16_t x, y, n;
  int8_t dx, dy;
} recel_line;

recel_tracer recel_tracer_init(uint32_t w, uint32_t h, uint32_t *distance);

bool recel_tracer_begin(recel_tracer *t, int32_t x, int32_t y);
void recel_tracer_next(recel_tracer *t);

recel_line recel_tracer_next_hv_line(recel_tracer *t);
recel_line recel_tracer_next_hvd_line(recel_tracer *t);

#endif /*!_RECEL_H__*/
