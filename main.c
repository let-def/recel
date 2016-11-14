#include <stdlib.h>
#include "recel.h"
#include "stb_image.h"
#include "stb_image_write.h"

int main(int argc, char **argv)
{
  int w, h, n;
  void *input = stbi_load(argv[1], &w, &h, &n, 4);
  printf("loaded '%s', %d*%d*%d\n", argv[1], w, h, n);

  uint32_t *distance = recel_distance(w, h, input);
  recel_save_dist("dist.png", w, h, distance);

  free(distance);
  free(input);

  return 0;
}
