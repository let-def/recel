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

    memcpy(l0, L0,  sizeof(uint32_t) * w);
    memcpy(l1, L1,  sizeof(uint32_t) * w);

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
