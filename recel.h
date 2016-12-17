#ifndef _RECEL_H__
#define _RECEL_H__

#include <stdint.h>
#include <stdbool.h>

#define PIX(img,x,y) (img[(y) * w + (x)])

#define NEW_IMAGE(t,w,h) ((t*)malloc((w) * (h) * sizeof(t)))

/* 1. Distance map */

/* Returns a (w * h) array of uint32_t representing the distance map computed
 * from input.
 * Array has to be freed with free(3).
 */
uint32_t *recel_distance(uint32_t w, uint32_t h, uint32_t *input);

/*uint8_t *recel_dist_to_u8(uint32_t w, uint32_t h, uint32_t *distance);*/
void recel_save_dist(const char *name, uint32_t w, uint32_t h, uint32_t *dist);

void recel_scanline(
    uint32_t w,
    const uint32_t *dista, const uint32_t *distb,
    const uint32_t *linea, const uint32_t *lineb,
    uint32_t *disto, uint32_t *lineo
    );

void recel_scan(
    uint32_t w, uint32_t h,
    const uint32_t *dist, const uint32_t *line,
    uint32_t *disto, uint32_t *lineo
    );

#endif /*!_RECEL_H__*/
