#include <stdlib.h>
#include <string.h>
#include "recel.h"
#include "stb_image.h"
#include "stb_image_write.h"

int inflate_line(const uint32_t *d1, const uint32_t *d2, const uint32_t *i1, const uint32_t *i2, uint32_t *o, int x0, int w)
{
  int x1 = x0 + 1;

  // Length of the line: x1-x0
  while (x1 < w && d1[x1] < d2[x1-1] && d1[x1-1] < d2[x1])
    x1 += 1;

  // Classify corners
  int l = (x0 > 0) ? d1[x0-1] >= d2[x0] : 0;
  int r = (x1 < w-1) ? d1[x1] >= d2[x1-1] : 0;
  
  // Interpolate
  if (l && r)
  {
    int x = x0;
    int d = (x1 - x0) / 4;

    for (; x < x0 + d; x++)
      o[x] = i2[x];

    for (; x < x1 - d; x++)
      o[x] = i1[x];

    for (; x < x1; x++)
      o[x] = i2[x];
  }
  else if (l)
  {
    int x = x0;
    int d = (x1 - x0) / 2;

    for (; x < x0 + d; x++)
      o[x] = i2[x];

    for (; x < x1; x++)
      o[x] = i1[x];
  }
  else if (r)
  {
    int x = x0;
    int d = (x1 - x0) / 2;

    for (; x < x0 + d; x++)
      o[x] = i1[x];

    for (; x < x1; x++)
      o[x] = i2[x];
  }
  else 
  {
    int x = x0;
    int d = (x1 - x0) / 4;

    for (; x < x0 + d; x++)
      o[x] = i1[x];

    for (; x < x1 - d; x++)
      o[x] = i2[x];

    for (; x < x1; x++)
      o[x] = i1[x];
  }

  return x1;
}

void inflate(const uint32_t *dist, const uint32_t *imag, int w, int h, uint32_t *out)
{
  for (int y = 0; y < h-1; y++)
  {
    const uint32_t *pd1 = dist + w * (y + 0);
    const uint32_t *pd2 = dist + w * (y + 1);
    const uint32_t *pi1 = imag + w * (y + 0);
    const uint32_t *pi2 = imag + w * (y + 1);
    uint32_t *po = out + w * y;

    int x = 0;
    while (x < w)
    {
      if (pd1[x] < pd2[x])
        x = inflate_line(pd1, pd2, pi1, pi2, po, x, w);
      else if (pd1[x] > pd2[x])
        x = inflate_line(pd2, pd1, pi2, pi1, po, x, w);
      else
      {
        po[x] = pi1[x];
        x++;
        continue;
      }
    }
  }
}

void interleave(uint32_t *out, const uint32_t *outer, const uint32_t *inner, int w, int h)
{
  memcpy(out, outer, sizeof(uint32_t)*w);
  for (int y = 0; y < h - 1; y++)
  {
    memcpy(out + w * (2 * y + 1), inner + w * (y + 0), sizeof(uint32_t)*w);
    memcpy(out + w * (2 * y + 2), outer + w * (y + 1), sizeof(uint32_t)*w);
  }
}

uint32_t *transpose(const uint32_t *in, int w, int h)
{
  uint32_t *result = NEW_IMAGE(uint32_t, h, w);
  for (int y = 0; y < h; y++)
    for (int x = 0; x < w; x++)
      result[x * h + y] = in[y * w + x];
  return result;
}

int main(int argc, char **argv)
{
  int w, h, n;
  uint32_t *imag, *dist, *disti, *imagi, *distii, *imagii;

  imag = (uint32_t*)stbi_load(argv[1], &w, &h, &n, 4);
  printf("loaded '%s', %d*%d*%d\n", argv[1], w, h, n);

  dist = recel_distance(w, h, imag);
  recel_save_dist("dist.png", w, h, dist);

  for (int i = 0; i < 2; i++)
  {
    disti = NEW_IMAGE(uint32_t, w, h-1);
    imagi = NEW_IMAGE(uint32_t, w, h-1);
    inflate(dist, dist, w, h, disti);
    inflate(dist, imag, w, h, imagi);

    distii = NEW_IMAGE(uint32_t, w, 2*h-1);
    imagii = NEW_IMAGE(uint32_t, w, 2*h-1);
    interleave(distii, dist, disti, w, h);
    interleave(imagii, imag, imagi, w, h);
    h = h * 2 - 1;

    free(imag);
    free(dist);
    free(disti);
    free(imagi);

    char fdist[] = "dist-_.png";
    char fimag[] = "imag-_.png";
    fdist[5] = fimag[5] = '0' + i;

    stbi_write_png(fimag, w, h, 4, imagii, 0);
    recel_save_dist(fdist, w, h, distii);

    imag = transpose(imagii, w, h);
    dist = transpose(distii, w, h);
    free(imagii);
    free(distii);

    int t = w;
    w = h;
    h = t;
  }

  stbi_write_png("outi.png", w, h, 4, imag, 0);
  recel_save_dist("outd.png", w, h, dist);

  free(dist);
  free(imag);

  return 0;
}
