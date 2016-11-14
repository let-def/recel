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

/*uint8_t *recel_dist_to_u8(uint32_t w, uint32_t h, uint32_t *distance);*/
void recel_save_dist(const char *name, uint32_t w, uint32_t h, uint32_t *dist);

#endif /*!_RECEL_H__*/
