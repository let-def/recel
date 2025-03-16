#include <stdlib.h>
#include <string.h>
#include "recel.h"
#include "stb_image.h"
#include "stb_image_write.h"

typedef struct {
  int length;
  bool l, r;
} segment_t;

void inflate_segment(const uint32_t *d1, const uint32_t *d2,
                     const uint32_t *i1, const uint32_t *i2,
                     uint32_t *o1, uint32_t *o2,
                     int x0, int w, int w0, int w1)
{
  // Classify corners
  int l = (x0 > 0) ? d1[x0-1] >= d2[x0] : 0;
  int r = (x1 < w-1) ? d1[x1] >= d2[x1-1] : 0;
  
  // Interpolate
  if (l && r)
  {
    int x = x0;
    int d = (x1 - x0 + 1) / 4;

    for (; x < x0 + d; x++)
    {
      o1[x] = o2[x] = i2[x];
    }

    for (; x < x1 - d; x++)
    {
      o1[x] = i1[x];
      o2[x] = i2[x];
    }

    for (; x < x1; x++)
    {
      o1[x] = o2[x] = i2[x];
    }
  }
  else if (l)
  {
    int x = x0;
    int d = (x1 - x0) / 2;

    for (; x < x0 + d; x++)
      o1[x] = o2[x] = i2[x];

    for (; x < x1; x++)
    {
      o1[x] = i1[x];
      o2[x] = i2[x];
    }
  }
  else if (r)
  {
    int x = x0;
    int d = (x1 - x0 + 1) / 2;

    for (; x < x0 + d; x++)
    {
      o1[x] = i1[x];
      o2[x] = i2[x];
    }

    for (; x < x1; x++)
      o1[x] = o2[x] = i2[x];
  }
  else 
  {
    int x = x0;
    int d = (x1 - x0 + 3) / 4;

    for (; x < x0 + d; x++)
    {
      o1[x] = i1[x];
      o2[x] = i2[x];
    }

    for (; x < x1 - d; x++)
    {
      o1[x] = o2[x] = i2[x];
    }

    for (; x < x1; x++)
    {
      o1[x] = i1[x];
      o2[x] = i2[x];
    }
  }
}

static void swap(uint32_t **x, uint32_t **y)
{
  uint32_t *t = *x;
  *x = *y;
  *y = t;
}

static void swapc(const uint32_t **x, const uint32_t **y)
{
  const uint32_t *t = *x;
  *x = *y;
  *y = t;
}

int segment_bound(const uint32_t *d1, const uint32_t *d2, int x, int w)
{
  // Length of the segment: x1-x0
  do x += 1;
  while (x < w && d1[x] < d2[x - 1] && d1[x - 1] < d2[x]);
  return x;
}

static void order(const uint32_t **pd1, const uint32_t **pd2,
                  const uint32_t **pi1, const uint32_t **pi2,
                  uint32_t **po1, uint32_t **po2,
                  int x)
{
  if ((*pd1)[x] > (*pd2)[x])
  {
    swapc(pd1, pd2);
    swapc(pi1, pi2);
    swap(po1, po2);
  }
}

void inflate(const uint32_t *dist,
             const uint32_t *imag,
             int w, int h,
             uint32_t *out)
{
  if (w == 0)
    return;

  for (int y = 0; y < h-1; y++)
  {
    const uint32_t *pd1 = dist + w * (y + 0), *nd1;
    const uint32_t *pd2 = dist + w * (y + 1), *nd2;
    const uint32_t *pi1 = imag + w * (y + 0), *ni1;
    const uint32_t *pi2 = imag + w * (y + 1), *ni2;
    uint32_t *po1 = out + w * (2 * y + 0), *no1;
    uint32_t *po2 = out + w * (2 * y + 1), *no2;

    int px = 0;
    order(&pd1, &pd2, &pi1, &pi2, &po1, &po2, px);
    int x0 = segment_bound(pd1, pd2, px, w);

    do
    {
      nd1 = pd1, nd2 = pd2;
      ni1 = pi1, ni2 = pi2;
      no1 = po1, no2 = po2;


    }
    while (x0 < w);

      if (pd1[x0] == pd2[x0])
      {
        po1[x0] = pi1[x0];
        po2[x0] = pi2[x0];
        x0++;
        continue;
      }

      int x1 = x0 + 1;

      // Length of the segment: x1-x0
      while (x1 < w && pd1[x1] < pd2[x1 - 1] && pd1[x1 - 1] < pd2[x1])
        x1 += 1;

      if (pd1[x0] < pd2[x0])
        x0 = inflate_line(pd1, pd2, pi1, pi2, po1, po2, x0, w);
      else if (pd1[x0] > pd2[x0])
        x0 = inflate_line(pd2, pd1, pi2, pi1, po2, po1, x0, w);
    }
  }
}

void fliph(uint32_t *image, int w, int h)
{
  for (int y = 0; y < h - 1; y++)
  {
    uint32_t *p = image + w * y;
    for (int x = 0; x < w / 2; x++)
    {
      uint32_t v = p[x];
      p[x] = p[w - 1 - x];
      p[w - 1 - x] = v;
    }
  }
}

void interleave(uint32_t *out, const uint32_t *outer, const uint32_t *inner, int w, int h)
{
  memcpy(out, outer, sizeof(uint32_t)*w);
  for (int y = 0; y < h - 1; y++)
  {
    memcpy(out + w * (3 * y + 1), inner + w * (2 * y + 0), sizeof(uint32_t)*w);
    memcpy(out + w * (3 * y + 2), inner + w * (2 * y + 1), sizeof(uint32_t)*w);
    memcpy(out + w * (3 * y + 3), outer + w * (y + 1), sizeof(uint32_t)*w);
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

  bool do_fliph = 0;
  bool do_flipv = 0;
  char *input = 0;

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0)
      do_fliph = 1;
    if (strcmp(argv[i], "-v") == 0)
      do_flipv = 1;
  }


  imag = (uint32_t*)stbi_load(argv[1], &w, &h, &n, 4);
  printf("loaded '%s', %d*%d*%d\n", argv[1], w, h, n);

  dist = recel_distance(w, h, imag);
  recel_save_dist("dist.png", w, h, dist);

  for (int i = 0; i < 2; i++)
  {
    disti = NEW_IMAGE(uint32_t, w, 2*h-1);
    imagi = NEW_IMAGE(uint32_t, w, 2*h-1);
    inflate(dist, dist, w, h, disti);
    inflate(dist, imag, w, h, imagi);

    distii = NEW_IMAGE(uint32_t, w, 3*h-1);
    imagii = NEW_IMAGE(uint32_t, w, 3*h-1);
    interleave(distii, dist, disti, w, h);
    interleave(imagii, imag, imagi, w, h);
    if (i == 0)
      stbi_write_png("outh.png", w, 3*h-1, 4, imagii, 0);
    h = h * 3 - 1;

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
