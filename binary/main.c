#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "../stb_image.h"
#include "../stb_image_write.h"

#define NEW_IMAGE(t,w,h) ((t*)malloc((w) * (h) * sizeof(t)))

uint32_t *transpose(const uint32_t *in, int w, int h)
{
  uint32_t *result = NEW_IMAGE(uint32_t, h, w);
  for (int y = 0; y < h; y++)
    for (int x = 0; x < w; x++)
      result[x * h + y] = in[y * w + x];
  return result;
}

bool test(uint32_t x)
{
  return (x & 0xff) > 0x7f;
}

void stretch_line(uint32_t *L0, uint32_t *l0, uint32_t *l1, uint32_t *L1,
                  int p, int w, int n)
{
  if (!test(L0[-1]))
    p = 0;
  if (!test(L0[w]))
    n = 0;

  if (w == 1)
  {
    // Short-edge
    if (p && n)
      l0[0] = l1[0] = L1[0];
    else
    {
      l0[0] = L0[0];
      l1[0] = L1[0];
    }
  }
  else
  {
    // Long-edge
    if (p && n)

    else if (
  }
}

uint32_t *stretch3(const uint32_t *in, int w, int h)
{
  if (h == 0)
    return NULL;

  uint32_t *out = NEW_IMAGE(uint32_t, w, h * 3 - 2);

  memcpy(out, in, sizeof(uint32_t) * w);

  for (int y = 1; y < h; y++)
  {
    uint32_t
      *L0 = out + w * (y * 3 - 3),
      *l0 = out + w * (y * 3 - 2),
      *l1 = out + w * (y * 3 - 1),
      *L1 = out + w * (y * 3 - 0);

    memcpy(L1, in + y * w, sizeof(uint32_t) * w);

    // NN

    l0[0] = L0[0];
    l1[0] = L1[0];
    l0[w-1] = L0[w-1];
    l1[w-1] = L1[w-1];

    // Length of previous edge
    int p = 1;
    int x0, x1 = 1, x2 = 2;

    while (x2 < w)
    {
      x0 = x1;
      x1 = x2;

      // Find next edge
      {
        bool t0 = test(L0[x1]), t1 = test(L1[x1]);
        do x2++;
        while (x2 < w && test(L0[x2]) == t0 && test(L1[x2]) == t1);
      }

      // Current edge goes for x0 to x1, next edge for x1 to x2
      bool t0 = test(L0[x0]), t1 = test(L1[x0]);
      if (t0 == t1)
      {
        for (int i = x0; i < x1; i++)
        {
          l0[i] = L0[i];
          l1[i] = L1[i];
        }
      }
      else if (t0)
        stretch_line(L0 + x0, l0 + x0, l1 + x0, L1 + x0,
                     p, x1 - x0, x2 - x1);
      else if (t1)
        stretch_line(L1 + x0, l1 + x0, l0 + x0, L0 + x0,
                     p, x1 - x0, x2 - x1);
      p = (x1 - x0);
    }
  }

  return out;
}

int main(int argc, char **argv)
{
  if (argc <= 1)
    return 1;

  int w, h, n;
  uint32_t *i1, *i2;

  i1 = (uint32_t *)stbi_load(argv[1], &w, &h, &n, 4);
  if (!i1)
    return 2;

  i2 = stretch3(i1, w, h);
  free(i1);
  h = h * 3 - 2;

  i1 = transpose(i2, w, h);
  free(i2);

  i2 = stretch3(i1, h, w);
  free(i1);
  w = w * 3 - 2;

  i1 = transpose(i2, h, w);
  free(i2);

  stbi_write_png("out.png", w, h, 4, i1, 0);
  free(i1);

  return 0;
}
