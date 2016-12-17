#include <stdlib.h>
#include "recel.h"
#include "stb_image.h"
#include "stb_image_write.h"

uint32_t *interleavev(uint32_t w, uint32_t h, const uint32_t *i1, const uint32_t *i2)
{
  uint32_t *o = NEW_IMAGE(uint32_t, w, h*2-1);

  for (uint32_t y = 0; y < h; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      PIX(o, x, y * 2) = PIX(i1, x, y);
    }
  }
  for (uint32_t y = 0; y < h - 1; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      PIX(o, x, y * 2 + 1) = PIX(i2, x, y);
    }
  }

  return o;
}

uint32_t *interleaveh(uint32_t w, uint32_t h, const uint32_t *i1, const uint32_t *i2)
{
  uint32_t *o = NEW_IMAGE(uint32_t, w*2 - 1, h);
  uint32_t w2 = w * 2 - 1;

  for (uint32_t y = 0; y < h; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      o[x * 2 + y * w2] = PIX(i1, x, y);
    }
  }
  for (uint32_t y = 0; y < h - 1; ++y)
  {
    for (uint32_t x = 0; x < w; ++x)
    {
      o[x * 2 + 1 + y * w2] = PIX(i2, x, y);
    }
  }

  return o;
}

uint32_t *rotate(uint32_t w, uint32_t h, uint32_t *i)
{
  uint32_t *o = NEW_IMAGE(uint32_t, h, w);
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      o[y + x * h] = i[x + y * w];
    }
  }

  return o;
}

void scanv(uint32_t w, uint32_t h, uint32_t *i, uint32_t *id, uint32_t **o, uint32_t **od)
{
  *o = NEW_IMAGE(uint32_t, w, h);
  *od = NEW_IMAGE(uint32_t, w, h);
  recel_scan(w, h, id, i, *od, *o);
}

void scanh(uint32_t w, uint32_t h, uint32_t *i, uint32_t *id, uint32_t **o, uint32_t **od)
{
  i = rotate(w, h, i);
  id = rotate(w, h, id);
  uint32_t *t, *td;
  scanv(h, w, i, id, &t, &td);
  *o = rotate(h, w, t);
  *od = rotate(h, w, td);
  free(t);
  free(td);
  free(i);
  free(id);
}

int main(int argc, char **argv)
{
  int w, h, n;
  void *input = stbi_load(argv[1], &w, &h, &n, 4);
  printf("loaded '%s', %d*%d*%d\n", argv[1], w, h, n);

  uint32_t *dist = recel_distance(w, h, input);
  recel_save_dist("dist.png", w, h, dist);

  uint32_t *horz, *horzd, *vert, *vertd;
  scanv(w, h, input, dist, &vert, &vertd);

  stbi_write_png("vert.png", w, h*2-1, 4, interleavev(w, h, input, vert), 0);
  recel_save_dist("vert_dist.png", w, h*2-1, interleavev(w, h, dist, vertd));

  uint32_t *ih, *ihd;
  scanh(w, h, input, dist, &horz, &horzd);
  ih = interleaveh(w, h, input, horz);
  ihd = interleaveh(w, h, dist, horzd);
  free(horz);
  free(horzd);

  stbi_write_png("horz.png", w*2-1, h, 4, ih, 0);
  recel_save_dist("horz_dist.png", w*2-1, h, ihd);

  scanv(w*2-1, h, ih, ihd, &horz, &horzd);
  vert = interleavev(w*2-1, h, ih, horz);
  vertd = interleavev(w*2-1, h, ihd, horzd);

  stbi_write_png("out.png", w*2-1, h*2-1, 4, vert, 0);
  recel_save_dist("out_dist.png", w*2-1, h*2-1, vertd);

  free(dist);
  free(input);

  return 0;
}
